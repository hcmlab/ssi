// BinTree.h
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

#ifndef SSI_STRUCT_BINTREE_H
#define SSI_STRUCT_BINTREE_H

#include "SSI_Cons.h"

namespace ssi {

class BinTree {

public:

	struct Node {
		Node *top;
		Node *left;
		Node *right;
		void *ptr;
		ssi_size_t index;
		ssi_size_t level;		
	};

public:

	BinTree(ssi_size_t n_level);
	virtual ~BinTree();	

	// get node by index
	ssi_size_t getSize() { return _n_nodes; }
	Node *get(ssi_size_t index);

	// get node by level and relative index on level
	ssi_size_t getSize(ssi_size_t level) { return nLevel(level); }
	Node *get (ssi_size_t level, ssi_size_t index);

	// quickly browse non empty nodes
	Node *root();
	Node *next(Node *node);
	
	void print(void(*toString) (ssi_size_t size, ssi_char_t *str, void *node_ptr), bool full = false, FILE *fp = ssiout);

	bool save(const ssi_char_t *filepath, void(*toFile) (const ssi_char_t *filepath, FILE *fp, void *node_ptr));
	bool save(const ssi_char_t *filepath, FILE *fp, void(*toFile) (const ssi_char_t *filepath, FILE *fp, void *node_ptr));
	static BinTree *Load(const ssi_char_t *filepath, void(*fromFile) (const ssi_char_t *filepath, FILE *fp, void **node_ptr));
	static BinTree *Load(const ssi_char_t *filepath, FILE *fp, void(*fromFile) (const ssi_char_t *filepath, FILE *fp, void **node_ptr));
	
protected:

	struct STATE {
		enum List {
			EMPTY = 0,
			FILLED = 1
		};
	};

	ssi_size_t _n_level;
	ssi_size_t _n_nodes;
	Node *_nodes;

	void init(ssi_size_t n_level);
	void clear();

	ssi_size_t mypow(ssi_size_t node_ptr, ssi_size_t y);
	bool isValid(ssi_size_t level, ssi_size_t index);
	ssi_size_t toPos(ssi_size_t level, ssi_size_t index);
	ssi_size_t nLevel(ssi_size_t level);
	ssi_size_t nTree(ssi_size_t level);

	void printIndent(FILE *fp, ssi_size_t level);
};

}

#endif
