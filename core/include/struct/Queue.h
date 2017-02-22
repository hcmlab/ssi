// Queue.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/09/08
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

#ifndef SSI_STRUCT_QUEUE_H
#define SSI_STRUCT_QUEUE_H

#include "thread/Mutex.h"

namespace ssi {

class Queue {

public:

	Queue (ssi_size_t n);
	virtual ~Queue ();

	bool enqueue (void *x);
	bool dequeue (void **x);
	bool empty ();
	bool full ();
	void print(void(*toString) (ssi_size_t size, ssi_char_t *str, void *x), FILE *fp = ssiout);

	bool save(const ssi_char_t *filepath, void(*toFile) (const ssi_char_t *filepath, FILE *fp, void *x));
	bool save(const ssi_char_t *filepath, FILE *fp, void(*toFile) (const ssi_char_t *filepath, FILE *fp, void *x));
	static Queue *Load(const ssi_char_t *filepath, void(*fromFile) (const ssi_char_t *filepath, FILE *fp, void **x));
	static Queue *Load(const ssi_char_t *filepath, FILE *fp, void(*fromFile) (const ssi_char_t *filepath, FILE *fp, void **x));

protected:

	struct STATE {
		enum List {
			EMPTY = 0,
			FILLED = 1
		};
	};

	void **_q;			// queue
	int _n;				// max elements
	int _first;			// position of first element
	int _last;			// position of last element
	int _count;			// number of queue elements
	Mutex _mutex;
};

}

#endif

