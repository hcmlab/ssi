// HierarchicalModel.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2015/02/26
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute itand/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#include "HierarchicalModel.h"
#include "ISHotClass.h"
#include "ISSelectClass.h"
#include "ISSelectDim.h"
#include "Trainer.h"

namespace ssi {

ssi_char_t *HierarchicalModel::ssi_log_name = "hierarch__";

HierarchicalModel::HierarchicalModel(const ssi_char_t *file)
	: _n_features(0),
	_n_classes(0),
	_file(0),
	_class_names(0),
	_tree(0),
	_is_trained (false) {

	/*if (file) {
		if (!OptionList::LoadXML(file, _options)) {
			OptionList::SaveXML(file, _options);
		}
		_file = ssi_strcpy(file);
	}*/
}

HierarchicalModel::~HierarchicalModel() {

	/*if (_file) {
		OptionList::SaveXML(_file, _options);
		delete[] _file;
		}*/

	release();

	if (_tree) {
		BinTree::Node *node = _tree->root();
		while (node) {
			content_t *content = ssi_pcast(content_t, node->ptr);
			if (content) {
				delete[] content->classes;
				delete[] content->dims;
				node = _tree->next(node);
				delete content;
				node->ptr = 0;
			}			
		}
	}
	delete _tree; _tree = 0;
}

void HierarchicalModel::release() {

	free_class_names();
	_n_classes = 0;
	_n_features = 0;
	_is_trained = false;

	if (_tree) {
		BinTree::Node *node = _tree->root();
		while (node) {
			content_t *content = ssi_pcast(content_t, node->ptr);
			if (content) {
				if (content->model) {
					content->model->release();
				}
			}
			node = _tree->next(node);			
		}
	}
}

bool HierarchicalModel::initTree(ssi_size_t n_level) {
	if (_tree) {
		ssi_wrn("already has tree");
		return false;
	}
	_tree = new BinTree(n_level);
	return true;
}

bool HierarchicalModel::addNode(ssi_size_t level, ssi_size_t index, IModel *model, const ssi_char_t *s_classes, const ssi_char_t *s_dims) {
	
	ssi_size_t n_classes = ssi_string2array_count(s_classes, ',');
	if (n_classes == 0) {
		ssi_wrn("could not parse classes '%s'", s_classes);
		return false;
	}
	ssi_size_t *classes = new ssi_size_t[n_classes];
	ssi_string2array(n_classes, classes, s_classes, ',');

	ssi_size_t n_dims = 0;
	ssi_size_t *dims = 0;
	if (s_dims) {
		int from, to;
		if (ssi_parse_indices_range(s_dims, from, to)) {
			n_dims = to - from + 1;
			dims = new ssi_size_t[n_dims];
			for (ssi_size_t i = 0; i < n_dims; i++) {
				dims[i] = i + ssi_cast(ssi_size_t, from);
			}
		} else {
			n_dims = ssi_string2array_count(s_dims, ',');
			if (n_dims == 0) {
				ssi_wrn("could not parse dims '%s'", s_dims);
				return false;
			}
			dims = new ssi_size_t[n_dims];
			ssi_string2array(n_dims, dims, s_dims, ',');
		}
	}

	bool result = addNode(level, index, model, n_classes, classes, n_dims, dims);
	
	delete[] classes;
	delete[] dims;

	return result;
}

bool HierarchicalModel::addNode(ssi_size_t level, ssi_size_t index, IModel *model, ssi_size_t n_classes, ssi_size_t *classes, ssi_size_t n_dims, ssi_size_t *dims) {

	if (!_tree) {
		ssi_wrn("no tree");
		return false;
	}

	BinTree::Node *node = _tree->get(level, index);
	if (!node) {
		ssi_wrn("no node (%u,%u)", level, index);
		return false;
	}

	HierarchicalModel::content_t *content = new HierarchicalModel::content_t;
	content->model = model;
	content->n_classes = n_classes;
	content->classes = new ssi_size_t[n_classes];
	memcpy(content->classes, classes, sizeof(ssi_size_t) * n_classes);
	content->n_dims = 0;	
	content->dims = 0;
	if (n_dims > 0) {
		content->n_dims = n_dims;
		content->dims = new ssi_size_t[n_dims];
		memcpy(content->dims, dims, sizeof(ssi_size_t) * n_dims);
	}
	content->node = node;
	node->ptr = content;

	return true;
}

bool HierarchicalModel::train(ISamples &samples,
	ssi_size_t stream_index) {

	if (!_tree) {
		ssi_wrn("tree not set");
		return false;
	}

	ssi_size_t n_samples = samples.getSize();

	if (n_samples == 0) {
		ssi_wrn("empty sample list");
		return false;
	}

	if (isTrained()) {
		ssi_wrn("already trained");
		return false;
	}

	_n_classes = samples.getClassSize();
	_n_features = samples.getStream(stream_index).dim;
	init_class_names(samples);

	BinTree::Node *node = _tree->root();
	if (node && node->ptr) {
		while (node) {
			if (!trainNode(node, samples, stream_index)) {
				return false;
			}
			node = _tree->next(node);
		}
	}

	_is_trained = true;

	return true;
}

bool HierarchicalModel::trainNode(BinTree::Node *node,
	ISamples &samples,
	ssi_size_t stream_index) {

	content_t *content = ssi_pcast(content_t, node->ptr);
	IModel *model = content->model;
	ssi_size_t n_classes = content->n_classes;
	ssi_size_t *classes = content->classes;
	ssi_size_t n_dims = content->n_dims;
	ssi_size_t *dims = content->dims;

	// only single class is left?
	if (n_classes == 1) {
		return true;
	}
	// leaf and no futher splitting is needed?
	if ((!node->left || !node->left->ptr) && (!node->right || !node->right->ptr)) {

		// classes relevant classes
		ISSelectClass s_classes(&samples);
		s_classes.setSelection(n_classes, classes);

		ssi_char_t name[SSI_MAX_CHAR];
		ssi_strcpy(name, s_classes.getClassName(0));
		for (ssi_size_t i = 1; i < n_classes; i++) {
			ssi_sprint(name, "%s vs %s", name, s_classes.getClassName(i));
		}
		ssi_msg(SSI_LOG_LEVEL_BASIC, "%s", name);

		// select classes & dimensions
		if (n_dims > 0 && dims) {

			ISSelectDim s_classes_dims(&s_classes);
			s_classes_dims.setSelection(0, n_dims, dims);

			ssi_msg(SSI_LOG_LEVEL_BASIC, "select dimensions [%u..%u]", dims[0], dims[n_dims-1]);

			model->train(s_classes_dims, stream_index);
		} else {
			model->train(s_classes, stream_index);
		}

		return true;
	}

	// check if two valid children
	if (!(node->left && node->left->ptr)) {
		ssi_wrn("invalid tree, no left child '%u,%u'", node->level, node->index);
		return false;
	}
	if (!(node->right && node->right->ptr)) {
		ssi_wrn("invalid tree, no right child '%u,%u'", node->level, node->index);
		return false;
	}

	// get left selection
	content = ssi_pcast(content_t, node->left->ptr);	
	ssi_size_t n_hot = content->n_classes;
	ssi_size_t *hot = content->classes;

	// convert to relative indices
	ssi_size_t *hot_rel = new ssi_size_t[n_hot];
	for (ssi_size_t i = 0; i < n_hot; i++) {
		for (ssi_size_t j = 0; j < n_classes; j++) {
			if (hot[i] == classes[j]) {
				hot_rel[i] = j;
			}
		}
	}

	// get right selection
	content = ssi_pcast(content_t, node->right->ptr);
	ssi_size_t n_rest = content->n_classes;
	ssi_size_t *rest = content->classes;

	// convert to relative indices
	ssi_size_t *rest_rel = new ssi_size_t[n_rest];
	for (ssi_size_t i = 0; i < n_rest; i++) {
		for (ssi_size_t j = 0; j < n_classes; j++) {
			if (rest[i] == classes[j]) {
				rest_rel[i] = j;
			}
		}
	}

	// classes relevant classes
	ISSelectClass s_select(&samples);
	s_select.setSelection(n_classes, classes);

	// select hot class
	ISHotClass s_hot(&s_select);
	ssi_char_t hot_name[SSI_MAX_CHAR];
	ssi_char_t rest_name[SSI_MAX_CHAR];
	ssi_strcpy(hot_name, s_select.getClassName(hot_rel[0]));
	ssi_strcpy(rest_name, s_select.getClassName(rest_rel[0]));
	for (ssi_size_t i = 1; i < n_hot; i++) {
		ssi_sprint(hot_name, "%s+%s", hot_name, s_select.getClassName(hot_rel[i]));
	}
	for (ssi_size_t i = 1; i < n_rest; i++) {
		ssi_sprint(rest_name, "%s+%s", rest_name, s_select.getClassName(rest_rel[i]));
	}
	s_hot.setHotClass(n_hot, hot_rel, hot_name, rest_name);

	ssi_msg(SSI_LOG_LEVEL_BASIC, "%s vs %s", hot_name, rest_name);

	// select dimensions
	bool result = false;
	if (n_dims > 0 && dims) {

		ISSelectDim s_hot_dims(&s_hot);
		s_hot_dims.setSelection(0, n_dims, dims);

		ssi_msg(SSI_LOG_LEVEL_BASIC, "select dimensions [%u..%u]", dims[0], dims[n_dims - 1]);

		result = model->train(s_hot_dims, stream_index);
	}
	else {
		result = model->train(s_hot, stream_index);
	}	

	delete[] hot_rel;
	delete[] rest_rel;

	return result;
}

bool HierarchicalModel::forward(ssi_stream_t &stream,
	ssi_size_t n_probs,
	ssi_real_t *probs) {

	if (!isTrained()) {
		ssi_wrn("not trained");
		return false;
	}

	if (n_probs != _n_classes) {
		ssi_wrn("#classes differs");
		return false;
	}

	if (stream.type != SSI_REAL) {
		ssi_wrn("type differs");
		return false;
	}

	if (stream.dim != _n_features) {
		ssi_wrn("feature dimension differs");
		return false;
	}

	for (ssi_size_t i = 0; i < n_probs; i++) {
		probs[i] = 0;
	}

	BinTree::Node *node = _tree->root();
	if (node && node->ptr) {
		forwardNode(node, stream, n_probs, probs);
	}

	ssi_real_t sum = 0;
	for (ssi_size_t i = 0; i < n_probs; i++) {
		sum += probs[i];
	}

	if (sum > 0) {
		for (ssi_size_t i = 0; i < n_probs; i++) {
			probs[i] /= sum;
		}
	}

	return true;
}

void HierarchicalModel::forwardNode(BinTree::Node *node,
	ssi_stream_t &stream,
	ssi_size_t n_probs,
	ssi_real_t *probs) {

	content_t *content = ssi_pcast(content_t, node->ptr);
	IModel *model = content->model;	

	// if no leaf
	if (node->left && node->right && node->left->ptr && node->right->ptr) {

		// node decision
		ssi_real_t probs_tmp[2];
		if (content->n_dims > 0 && content->dims) {
			ssi_stream_t stream_sel;
			ssi_stream_select(stream, stream_sel, content->n_dims, content->dims);
			model->forward(stream_sel, 2, probs_tmp);
			ssi_stream_destroy(stream_sel);
		} else {
			model->forward(stream, 2, probs_tmp);
		}

		// left node probabilites 
		content = ssi_pcast(content_t, node->left->ptr);
		for (ssi_size_t i = 0; i < content->n_classes; i++) {
			probs[content->classes[i]] += probs_tmp[0];
		}

		// right node probabilites 
		content = ssi_pcast(content_t, node->right->ptr);
		for (ssi_size_t i = 0; i < content->n_classes; i++) {
			probs[content->classes[i]] += probs_tmp[1];
		}

		// follow node with higher probability
		node = probs_tmp[0] >= probs_tmp[1] ? node->left : node->right;
		forwardNode(node, stream, n_probs, probs);

	} else {

		ssi_size_t n_classes = content->n_classes;
		ssi_size_t *classes = content->classes;
		ssi_size_t n_dims = content->n_dims;
		ssi_size_t *dims = content->dims;

		if (n_classes > 1) {

			ssi_real_t *probs_tmp = new ssi_real_t[n_classes];
			if (n_dims > 0 && dims) {
				ssi_stream_t stream_sel;
				ssi_stream_select(stream, stream_sel, n_dims, dims);
				model->forward(stream_sel, n_classes, probs_tmp);
				ssi_stream_destroy(stream_sel);
			}
			else {
				model->forward(stream, n_classes, probs_tmp);
			}

			for (ssi_size_t i = 0; i < n_classes; i++) {
				probs[classes[i]] += probs_tmp[i];
			}

			delete[] probs_tmp;
		}
	}
}

bool HierarchicalModel::save(const ssi_char_t *filepath) {

	FILE *fp = fopen(filepath, "wb");
	if (!fp) {
		ssi_wrn("could not open '%s'", filepath);
		return false;
	}

	ssi_size_t trained = _is_trained ? 1 : 0;
	fwrite(&trained, sizeof(ssi_size_t), 1, fp);
	fwrite(&_n_classes, sizeof(ssi_size_t), 1, fp);
	fwrite(&_n_features, sizeof(ssi_size_t), 1, fp);

	ssi_size_t has_tree = _tree ? 1 : 0;
	fwrite(&has_tree, sizeof(ssi_size_t), 1, fp);
	if (_tree) {
		_tree->save(filepath, fp, ToFile);
	}

	fclose(fp);

	return true;
}

bool HierarchicalModel::load(const ssi_char_t *filepath) {

	FILE *fp = fopen(filepath, "rb");
	if (!fp) {
		ssi_wrn("could not open '%s'", filepath);
		return false;
	}

	ssi_size_t trained;
	fread(&trained, sizeof(ssi_size_t), 1, fp);
	_is_trained = trained == 1;
	fread(&_n_classes, sizeof(ssi_size_t), 1, fp);
	fread(&_n_features, sizeof(ssi_size_t), 1, fp);

	ssi_size_t has_tree;
	fread(&has_tree, sizeof(ssi_size_t), 1, fp);
	if (has_tree == 1) {
		_tree = BinTree::Load(filepath, fp, FromFile);
		BinTree::Node *node = _tree->root();
		while (node) {
			content_t *content = ssi_pcast(content_t, node->ptr);
			content->node = node;
			node = _tree->next(node);
		}
	}

	fclose(fp);

	return true;
}

void HierarchicalModel::init_class_names(ISamples &samples) {

	free_class_names();

	_n_classes = samples.getClassSize();
	_class_names = new ssi_char_t *[_n_classes];
	for (ssi_size_t i = 0; i < _n_classes; i++) {
		_class_names[i] = ssi_strcpy(samples.getClassName(i));
	}
}

void HierarchicalModel::free_class_names() {

	if (_class_names) {
		for (ssi_size_t i = 0; i < _n_classes; i++) {
			delete[] _class_names[i];
		}
		delete[] _class_names;
		_class_names = 0;
	}
}

}
