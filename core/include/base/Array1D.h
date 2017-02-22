// Array1D.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/10/18
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

#ifndef SSI_BASE_ARRAY1D_H
#define SSI_BASE_ARRAY1D_H

#include "SSI_Cons.h"

namespace ssi {

template <class T>
class Array1D {

public:
 	
	Array1D () 
		: _ptr (0), _ptrc (0), _end (0), _size (0), _count (0) {
	}
	~Array1D () {
		release ();
	}

	void release () {
		delete[] _ptr;	
		_end = _ptrc = _ptr = 0;
		_size = 0;
		_count = 0;
	}

	void clear () {		
		for (T *ptr = _ptr; ptr < _end; ptr++) {	
			*ptr = 0;
		}
		reset ();
	}

	void init (ssi_size_t n) {		
		release ();
		_size = n;
		if (_size > 0) {
			_ptr = _ptrc = new T[_size];
			_end = _ptr + _size;
			clear ();
		}		
	}

	T &operator[] (int i) { 
		return _ptr[i]; 
	}

	ssi_size_t size () {
		return _size;
	}

	ssi_size_t count () {
		return _count;
	}

	T *end () {
		return _end;
	}

	T *ptr () {
		return _ptr;
	}

	T *next () {
		if (_ptrc >= _end) {
			return 0;
		}
		++_count;
		return _ptrc++;
	}

	void reset () {
		_ptrc = _ptr;
		_count = 0;
	}

	void print (void (*toString) (ssi_size_t str_n, ssi_char_t *str, T x), FILE *file = ssiout, ssi_size_t from = 0, ssi_size_t to = 0) {
		ssi_char_t str[SSI_MAX_CHAR];
		ssi_fprint (file, "[ ");		
		to = to == 0 ? _size : to + 1;
		for (ssi_size_t i = from; i < to; i++) {
			T x = _ptr[i];
			toString (SSI_MAX_CHAR, str, x);
			ssi_fprint (file, "%s ", str);
		}		
		ssi_fprint (file, "]\n");
	}

	static void ToString (ssi_size_t size, ssi_char_t *string, char x) {
		ssi_sprint (string, "%c", x);
	}

	static void ToString (ssi_size_t size, ssi_char_t *string, unsigned char x) {		
		ssi_sprint (string, "%d", ssi_cast (int, x));
	}

	static void ToString (ssi_size_t size, ssi_char_t *string, int x) {		
		ssi_sprint (string, "%d", x);
	}

	static void ToString (ssi_size_t size, ssi_char_t *string, unsigned int x) {		 
		ssi_sprint (string, "%u", x);
	}

	static void ToString (ssi_size_t size, ssi_char_t *string, float x) {
		ssi_sprint (string, "%.2f", x);
	}

	static void ToString (ssi_size_t size, ssi_char_t *string, double x) {		
		ssi_sprint (string, "%.2f", x);
	}

protected:
		
	ssi_size_t _size;
	ssi_size_t _count;
	T *_end;	
	T *_ptr;
	T *_ptrc;
};

}

#endif
