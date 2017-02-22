// BinTree.cpp
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

#include "struct/BinTree.h"

namespace ssi {

BinTree::BinTree(ssi_size_t n_level)
	: _n_level (0), 
	_n_nodes(0),
	_nodes(0) {

	init(n_level);
}

BinTree::~BinTree() {

	clear();
}

ssi_size_t BinTree::mypow(ssi_size_t x, ssi_size_t y) {
	ssi_size_t z = 1;
	for (ssi_size_t i = 0; i < y; i++) {
		z *= x;
	}
	return z;
}

ssi_size_t BinTree::nTree(ssi_size_t level){
	return mypow (2, level + 1) - 1;
}

ssi_size_t BinTree::nLevel(ssi_size_t level) {
	return mypow (2, level);
}

bool BinTree::isValid(ssi_size_t level, ssi_size_t index) {
	return level < _n_level && index < nLevel(level);
}

ssi_size_t BinTree::toPos(ssi_size_t level, ssi_size_t index) {
	return mypow (2, level) - 1 + index;
}

void BinTree::init(ssi_size_t n_level) {

	if (n_level == 0) {
		return;
	}

	_n_level = n_level;
	_n_nodes = nTree(_n_level - 1);
	_nodes = new Node[_n_nodes];
	
	for (ssi_size_t i = 0; i < _n_level; i++) {
		for (ssi_size_t j = 0; j < nLevel(i); j++) {
			Node &e = _nodes[toPos(i, j)];
			e.index = j;
			e.level = i;
			e.ptr = 0;
			e.top = i == 0 ? 0 : get(i - 1, j / 2);
			e.left = i == _n_level - 1 ? 0 : get(i + 1, j * 2);
			e.right = i == _n_level - 1 ? 0 : get(i + 1, j * 2 + 1);
		}
	}
}

void BinTree::clear() {

	delete[] _nodes; _nodes = 0;
	_n_nodes = 0;
	_n_level = 0;
}

BinTree::Node *BinTree::get(ssi_size_t level, ssi_size_t index) {

	if (!isValid(level, index)) {
		return 0;
	}

	return get (toPos(level, index));
}

BinTree::Node *BinTree::get(ssi_size_t index) {

	if (index >= _n_nodes) {
		return 0;
	}

	return _nodes + index;
}

BinTree::Node *BinTree::next(Node *node) {

	ssi_size_t pos = toPos(node->level, node->index) + 1;

	if (pos == _n_nodes) {
		return 0;
	}

	if (_nodes[pos].ptr) {
		return _nodes + pos;
	}

	return next(_nodes + pos);
}

BinTree::Node *BinTree::root() {
	return _nodes;
}

void BinTree::printIndent(FILE *fp, ssi_size_t level) {

	for (ssi_size_t i = 0; i <= level; i++) {
		ssi_fprint(fp, ".");
	}
}

void BinTree::print(void(*toString) (ssi_size_t size, ssi_char_t *str, void *node_ptr), bool full, FILE *fp) {

	ssi_char_t string[SSI_MAX_CHAR];

	if (full) {
		for (ssi_size_t i = 0; i < _n_nodes; i++) {
			printIndent(fp, _nodes[i].level);
			ssi_fprint(fp, " [");
			if (_nodes[i].ptr) {
				toString(SSI_MAX_CHAR, string, _nodes[i].ptr);
				ssi_fprint(fp, "%s", string);
			}
			ssi_fprint(fp, "] @ [%u,%u]\n", _nodes[i].level, _nodes[i].index);
		} 
	} else {
		Node *node = root();
		if (node && node->ptr) {
			while (node) {
				printIndent(fp, node->level);
				toString(SSI_MAX_CHAR, string, node->ptr);
				ssi_fprint(fp, " [%s] @ [%u,%u]\n", string, node->level, node->index);
				node = next(node);
			}
		}
	}
}

bool BinTree::save(const ssi_char_t *filepath, void(*toFile) (const ssi_char_t *filepath, FILE *fp, void *node_ptr)) {

	FILE *fp = fopen(filepath, "wb");
	if (!fp) {
		ssi_wrn("could not open file '%s'", filepath);
		return false;
	}

	bool result = false;
	result = save(filepath, fp, toFile);
	fclose(fp);

	return result;
}

bool BinTree::save(const ssi_char_t *filepath, FILE *fp, void(*toFile) (const ssi_char_t *filepath, FILE *fp, void *node_ptr)) {

	fwrite(&_n_level, sizeof(int), 1, fp);

	STATE::List empty = STATE::EMPTY;
	STATE::List filled = STATE::FILLED;
	for (ssi_size_t i = 0; i < _n_nodes; i++) {
		if (_nodes[i].ptr) {
			fwrite(&filled, sizeof(STATE::List), 1, fp);
			if (filled == STATE::FILLED) {
				toFile(filepath, fp, _nodes[i].ptr);
			}
		}
		else {
			fwrite(&empty, sizeof(STATE::List), 1, fp);
		}
	}

	return true;
}

BinTree *BinTree::Load(const ssi_char_t *filepath, void(*fromFile) (const ssi_char_t *filepath, FILE *fp, void **node_ptr)) {

	FILE *fp = fopen(filepath, "rb");
	if (!fp) {
		ssi_wrn("could not open file '%s'", filepath);
        return nullptr;
	}

	BinTree *q = Load(filepath, fp, fromFile);
	fclose(fp);
	return q;
}

BinTree *BinTree::Load(const ssi_char_t *filepath, FILE *fp, void(*fromFile) (const ssi_char_t *filepath, FILE *fp, void **node_ptr)) {

	ssi_size_t n = 0;
	fread(&n, 1, sizeof(ssi_size_t), fp);

	BinTree *t = new BinTree(n);

	STATE::List state = STATE::EMPTY;
	for (ssi_size_t i = 0; i < t->_n_nodes; i++) {
		fread(&state, sizeof(STATE::List), 1, fp);
		if (state == STATE::FILLED) {
			fromFile(filepath, fp, &(t->_nodes[i].ptr));
		}
	}

	return t;
}

}


