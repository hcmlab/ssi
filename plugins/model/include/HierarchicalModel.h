// HierarchicalModel.h
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

#pragma once

#ifndef SSI_MODEL_HIERARCHICALMODEL_H
#define SSI_MODEL_HIERARCHICALMODEL_H

#include "base/IModel.h"
#include "struct/BinTree.h"
#include "base/Factory.h"

namespace ssi {

class HierarchicalModel : public IModel {

public:

	struct content_t {
		BinTree::Node *node;
		IModel *model;
		ssi_size_t n_classes;
		ssi_size_t *classes;
		ssi_size_t n_dims;
		ssi_size_t *dims;
	};

public:

	static const ssi_char_t *GetCreateName() { return "HierarchicalModel"; }
	static IObject *Create(const ssi_char_t *file) { return new HierarchicalModel(file); }
	virtual ~HierarchicalModel();

	IOptions *getOptions() { return 0; }
	const ssi_char_t *getName() { return GetCreateName(); }
	const ssi_char_t *getInfo() { return "A hierarchical classifier."; }

	bool train(ISamples &samples,
		ssi_size_t stream_index);
	bool isTrained() { return _is_trained; }
	bool forward(ssi_stream_t &stream,
		ssi_size_t n_probs,
		ssi_real_t *probs);
	void release();

	virtual bool initTree(ssi_size_t n_level); 
	virtual bool addNode(ssi_size_t level, ssi_size_t index, IModel *model, ssi_size_t n_classes, ssi_size_t *classes, ssi_size_t n_dims = 0, ssi_size_t *dims = 0);
	
	/*
		level		level of hierarchical model
		index		index of node on level "level"
		model		model used for node
		classes		class numbers entering node (eg. "1, 2, 4")
		dims		dimensions of stream used for klassification (eg. "5-10"). leave empty for all dimensions.
	*/
	virtual bool addNode(ssi_size_t level, ssi_size_t index, IModel *model, const ssi_char_t *classes, const ssi_char_t *dims = 0); // indices separated by ,
	virtual BinTree *getTree() { return _tree; };

	bool save (const ssi_char_t *filepath);
	bool load (const ssi_char_t *filepath);

	ssi_size_t getClassSize() { return _n_classes; }
	ssi_size_t getStreamDim() { return _n_features; }
	ssi_size_t getStreamByte() { return sizeof(ssi_real_t); }
	ssi_type_t getStreamType() { return SSI_REAL; }

	static void ToString(ssi_size_t n, ssi_char_t *str, void *node_ptr) {
		HierarchicalModel::content_t *content = ssi_pcast(HierarchicalModel::content_t, node_ptr);
		ssi_array2string(content->n_classes, content->classes, n, str);
	}

	static void ToFile(const ssi_char_t *filepath, FILE *fp, void *node_ptr) {
		HierarchicalModel::content_t *content = ssi_pcast(HierarchicalModel::content_t, node_ptr);
		fwrite(&content->n_classes, sizeof(ssi_size_t), 1, fp);
		fwrite(content->classes, sizeof(ssi_size_t), content->n_classes, fp);
		fwrite(&content->n_dims, sizeof(ssi_size_t), 1, fp);
		fwrite(content->dims, sizeof(ssi_size_t), content->n_dims, fp);
		fwrite(&content->node->index, sizeof(ssi_size_t), 1, fp);
		fwrite(&content->node->level, sizeof(ssi_size_t), 1, fp);
		ssi_size_t has_model = content->model ? 1 : 0;
		fwrite(&has_model, sizeof(ssi_size_t), 1, fp);
		if (has_model == 1) {			
			const ssi_char_t *name = content->model->getName();
			ssi_size_t n_name = ssi_strlen(name);
			fwrite(&n_name, sizeof(ssi_size_t), 1, fp);
			fwrite(name, sizeof(ssi_char_t), n_name, fp);
			ssi_char_t nodepath[SSI_MAX_CHAR];
			ssi_sprint(nodepath, "%s@[%u,%u]", filepath, content->node->index, content->node->level);
			ssi_size_t trained = content->model->isTrained() ? 1 : 0;
			fwrite(&trained, sizeof(ssi_size_t), 1, fp);
			if (trained == 1) {
				content->model->save(nodepath);
			}
		}
	}

	static void FromFile(const ssi_char_t *filepath, FILE *fp, void **node_ptr) {
		HierarchicalModel::content_t *content = new HierarchicalModel::content_t;
		content->node = 0;
		fread(&content->n_classes, sizeof(ssi_size_t), 1, fp);
		content->classes = new ssi_size_t[content->n_classes];
		fread(content->classes, sizeof(ssi_size_t), content->n_classes, fp);
		fread(&content->n_dims, sizeof(ssi_size_t), 1, fp);
		content->dims = new ssi_size_t[content->n_dims];
		fread(content->dims, sizeof(ssi_size_t), content->n_dims, fp);
		ssi_size_t index, level;
		fread(&index, sizeof(ssi_size_t), 1, fp);
		fread(&level, sizeof(ssi_size_t), 1, fp);
		content->model = 0;
		ssi_size_t has_model;
		fread(&has_model, sizeof(ssi_size_t), 1, fp);
		if (has_model == 1) {			
			ssi_char_t nodepath[SSI_MAX_CHAR];
			ssi_sprint(nodepath, "%s@[%u,%u]", filepath, index, level);
			ssi_size_t n_name = 0;
			fread(&n_name, sizeof(ssi_size_t), 1, fp);
			ssi_char_t *name = new ssi_char_t[n_name + 1];
			fread(name, sizeof(ssi_char_t), n_name, fp);
			name[n_name] = '\0';
			IModel *model = ssi_pcast(IModel, Factory::Create(name));
			ssi_size_t trained;
			fread(&trained, sizeof(ssi_size_t), 1, fp);
			if (trained == 1) {
				model->load(nodepath);
			}
			content->model = model;
			delete[] name;
		}
		*node_ptr = content;
	}

protected:

	HierarchicalModel(const ssi_char_t *file = 0);
	ssi_char_t *_file;
	static ssi_char_t *ssi_log_name;

	void init_class_names(ISamples &samples);
	void free_class_names();

	bool trainNode(BinTree::Node *node,
		ISamples &samples,
		ssi_size_t stream_index);
	void forwardNode(BinTree::Node *node,
		ssi_stream_t &stream,
		ssi_size_t n_probs,
		ssi_real_t *probs);

	bool _is_trained;
	ssi_size_t _n_classes;
	ssi_size_t _n_features;
	ssi_size_t _n_samples;
	ssi_char_t **_class_names;

	BinTree *_tree;
};

}

#endif
