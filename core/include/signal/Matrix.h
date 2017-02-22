// Matrix.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2007/10/11
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute itand/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

/**

A very basic matrix class.

Each sample consists of row/column size and a pointer to the data.

*/

#pragma once

#ifndef SSI_SIGNAL_MATRIX_H
#define SSI_SIGNAL_MATRIX_H

#include "signal/SignalCons.h"

#include <complex>

namespace ssi {

enum MATRIX_DIMENSION {

	//! rowwise
	MATRIX_DIMENSION_ROW = 0,
	//! columnwise
	MATRIX_DIMENSION_COL
};

template <class T>
class Matrix {

public:

	Matrix (ssi_size_t row_number, 
		ssi_size_t col_number) 
		: rows (row_number), 
		cols (col_number),
		data (0) {

		if (rows * cols > 0) {
			data = new T[rows*cols];
		}
	}

	Matrix (ssi_size_t row_number, 
		ssi_size_t col_number, 
		T* data_) 
		: rows (row_number), 
		cols (col_number), 
		data (data_) {
	};
	
	virtual ~Matrix (){
		delete[] data;
	}

	void reset (ssi_size_t row_number, 
		ssi_size_t col_number) {

		delete[] data;

		rows = row_number;
		cols = col_number;
		data = new T[rows*cols];
	}	
	
	ssi_size_t rows, cols;	// row and column size
	T *data;				// data pointer
};

}

#endif
