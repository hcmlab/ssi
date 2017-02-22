// MatrixOps.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2007/10/11
// Copyright (C) 2007-14 University of Augsburg, Lab for Human Centered Multimedia
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

Provides some static functions for basic matrix operations.

*/

#pragma once

#ifndef SSI_SIGNAL_MATRIXOPS_H
#define SSI_SIGNAL_MATRIXOPS_H

#include "Matrix.h"

#include "ioput/file/File.h"

namespace ssi {

template <class T>
class MatrixOps {

public:

	/// PRINT ///
	
	//! \brief Prints matrix to console
	//
	//! \param data			the matrix
	//
	static void Print (const Matrix<T> *const matrix);

	//! \brief Prints matrix to a file
	//
	//! \param file			the output file
	//! \param data			the matrix
	//
	static void Print (File *file, const Matrix<T> *const matrix);

	//! \brief Prints signal to a file
	//
	//! \param file			the output file
	//! \param data			the matrix
	//
	static void Print (File *file, const Matrix<T> *const matrix, ssi_time_t sample_rate);

	//! \brief Reads matrix from file
	//
	//! \return				the matrix
	//
	static Matrix<T> *Read (File *file);

	/// CHECK ///
	static bool IsEmpty (const Matrix<T> *const matrix) {
		return matrix->rows == 0 || matrix->cols == 0;
	}
	static bool IsVector (const Matrix<T> *const matrix) {
		return matrix->rows == 1 || matrix->cols == 1;
	}
	static bool IsScalar (const Matrix<T> *const matrix) {
		return matrix->rows == 1 && matrix->cols == 1;
	}



	/// CREATE ///

	static Matrix<T> *Zeros (ssi_size_t rows, ssi_size_t cols);
	static Matrix<T> *Ones (ssi_size_t rows, ssi_size_t cols);
	static Matrix<T> *Rand (ssi_size_t rows, ssi_size_t cols, T maxval);
	static Matrix<T> *Array (T start, T delta, T end, MATRIX_DIMENSION dimension);
	static Matrix<ssi_size_t> *IndArray (ssi_size_t start, ssi_size_t delta, ssi_size_t end, MATRIX_DIMENSION dimension);
	static Matrix<T> *Concat (const Matrix<T> *const matrix_1, const Matrix<T> *const matrix_2, MATRIX_DIMENSION dimension);	
	static Matrix<T> *Clone (const Matrix<T> *const matrix);
	static Matrix<T> *Repmat (const Matrix<T> *const matrix, ssi_size_t vertical, ssi_size_t horizontal);
	static void Clone (const Matrix<T> *const matrix_in, const Matrix<T> *const matrix_out);



	/// RESHAPE ///

	static void Transpose (Matrix<T> *const matrix);
	static void Flip (const Matrix<T> *const matrix, MATRIX_DIMENSION dimension);





	/// ACCESS ///
	

	//! \brief Returns matrix element
	//
	//! \param matrix		the matrix
	//! \param index		the index
	//! \return				value
	//
	static T Get (const Matrix<T> *const matrix, ssi_size_t index);

	//! \brief Returns matrix element
	//
	//! \param matrix		the matrix
	//! \param row			row index
	//! \param col			column index
	//! \return				value
	//
	static T Get (const Matrix<T> *const matrix, ssi_size_t row, ssi_size_t col);

	//! \brief Sets matrix element
	//
	//! \param matrix		the matrix
	//! \param index		the index
	//! \param value		the value
	//
	static void Set (const Matrix<T> *const matrix, ssi_size_t index, T value);

	//! \brief Sets matrix element
	//
	//! \param matrix		the matrix
	//! \param row			row index
	//! \param col			column index
	//! \param value		the value
	//
	static void Set (const Matrix<T> *const matrix, ssi_size_t row, ssi_size_t col, T value);

	//! \brief Returns sub matrix
	//
	//! \param matrix		the matrix
	//! \param indices		matrix with indices
	//! \return				sub matrix
	//
	static Matrix<T> *GetSubMatrix (const Matrix<T> *const matrix, const Matrix<ssi_size_t> *const indices);

	//! \brief Replaces sub matrix
	//
	//! \param matrix		the matrix
	//! \param indices		matrix with indices
	//! \param values		matrix with values
	//
	static void SetSubMatrix (const Matrix<T> *const matrix, const Matrix<ssi_size_t> *const indices, const Matrix<T> *const submatrix);

	//! \brief Returns connected sub matrix
	//
	//! \param matrix		the matrix
	//! \param row_start    first row index
	//! \param row_stop		last row index
	//! \param col_start	first column index
	//! \param col_stop		last column index
	//! \return				sub matrix
	//
	static Matrix<T> *GetSubMatrix (const Matrix<T> *const matrix, ssi_size_t row_start, ssi_size_t row_stop, ssi_size_t col_start, ssi_size_t col_stop);

	//! \brief Replaces sub matrix of the target matrix with the source matrix.
	//
	//! \param matrix		target matrix
	//! \param row			row index
	//! \param col			column index
	//! \param values		soucre matrix
	//
	static void SetSubMatrix (const Matrix<T> *const matrix, ssi_size_t row, ssi_size_t col, const Matrix<T> *const submatrix);

	//! \brief Replaces sub matrix of the target matrix with sub matrix of the source matrix.
	//
	//! \param matrix_dst	target matrix
	//! \param row_dst		first row index in target matrix
	//! \param col_dst		first column index in target matrix
	//! \param row_src		first row index in source matrix
	//! \param col_src		first column index in target matrix
	//! \param row_number	number of rows to be copied
	//! \param col_number	number of colums to be copied
	//! \param values		source matrix
	//
	static void SetSubMatrix (const Matrix<T> *const matrix_dst, ssi_size_t row_dst, ssi_size_t col_dst, ssi_size_t row_src, ssi_size_t col_src, ssi_size_t row_number, ssi_size_t col_number, const Matrix<T> *const matrix_src);

	//! \brief Returns sub matrix
	//
	//! \param matrix		the matrix
	//! \param start		first index
	//! \param stop			last index
	//! \param dimension	dimension
	//! \return				sub matrix
	//
	static Matrix<T> *GetSubMatrix (const Matrix<T> *const matrix, ssi_size_t start, ssi_size_t stop, MATRIX_DIMENSION dimension);

	//! \brief Replaces sub matrix of the target matrix with the source matrix.
	//
	//! \param matrix		target matrix
	//! \param start		first index
	//! \param col			last index
	//! \param dimension	dimension
	//! \param values		source matrix
	//
	static void SetSubMatrix (const Matrix<T> *const matrix, ssi_size_t start, MATRIX_DIMENSION dimension, const Matrix<T> *const submatrix);

	//! \brief Replaces sub matrix of the target matrix with sub matrix of the source matrix.
	//
	//! \param matrix		the matrix
	//! \param start		first index
	//! \param col			last index
	//! \param dimension	dimension
	//! \param values		matrix with values
	//
	static void SetSubMatrix (const Matrix<T> *const matrix_dst, ssi_size_t start_dst, ssi_size_t start_src, MATRIX_DIMENSION dimension, ssi_size_t number, const Matrix<T> *const src_matrix);

	//! \brief Returns sub matrix along a dimension
	//
	//! \param matrix		the matrix
	//! \param indices		vector with indices
	//! \param dimension	dimension
	//! \return				sub matrix
	//
	static Matrix<T> *GetSubMatrix (const Matrix<T> *const matrix, const Matrix<ssi_size_t> *indices, MATRIX_DIMENSION dimension);

	//! \brief Returens sub matrix with indices
	//
	static Matrix<ssi_size_t> *Find (const Matrix<T> *const matrix, T value);






	/// ARITHMETIC ///


	//! \brief Caculates sum of all elements
	//
	//! \param matrix		the matrix
	//! \return				sum value
	//
	static T Sum (const Matrix<T> *const matrix);

	//! \brief Caculates sum along a dimension
	//
	//! \param matrix		the matrix
	//! \param dimension	the dimension
	//! \return				sum matrix
	//
	static Matrix<T> *Sum (const Matrix<T> *const matrix, MATRIX_DIMENSION dimension);

	//! \brief Caculates sum along a dimension
	//
	//! \param matrix		the matrix
	//! \param dimension	the dimension
	//! \return				sum matrix
	//
	static void Sum (const Matrix<T> *const matrix, const Matrix<T> *const result, MATRIX_DIMENSION dimension);

	//! \brief Caculates square sum of all elements
	//
	//! \param matrix		the matrix
	//! \return				sum value
	//
	static T SumSqr (const Matrix<T> *const matrix);

	//! \brief Caculates square sum along a dimension
	//
	//! \param matrix		the input matrix
	//! \param dimension	dimension
	//! \return				result matrix
	//
	static Matrix<T> *SumSqr (const Matrix<T> *const matrix, MATRIX_DIMENSION dimension);

	//! \brief Caculates square sum along a dimension
	//
	//! \param matrix		the input matrix
	//! \param result		the result matrix
	//! \param dimension	dimension
	//
	static void SumSqr (const Matrix<T> *const matrix, const Matrix<T> *const result, MATRIX_DIMENSION dimension);

	//! \brief Finds the minimum
	//
	//! \param matrix		the matrix
	//! \return				minimum
	//
	static T Min (const Matrix<T> *const matrix);

	//! \brief Finds the minimum
	//
	//! \param matrix		the matrix
	//! \param minval		reference to minimum value
	//! \param vinind		reference to minimum index (consecutive indexed)
	//
	static void Min (const Matrix<T> *const matrix, T &minval, ssi_size_t &minind);

	//! \brief Finds the minimum along a dimension
	//
	//! \param matrix		the matrix
	//! \param dimension	the dimension
	//! \return				matrix with minimum indeces (indexed along dimension)
	//
	static Matrix<ssi_size_t> *Min (const Matrix<T> *const matrix, MATRIX_DIMENSION dimension);

	//! \brief Finds the maximum
	//
	//! \param matrix		the matrix
	//! \return				maximum
	//
	static T Max (const Matrix<T> *const matrix);

	//! \brief Finds the maximum
	//
	//! \param matrix		the matrix
	//! \param minval		reference to maximum value
	//! \param vinind		reference to maximum index (consecutive indexed)
	//
	static void Max (const Matrix<T> *const matrix, T &maxval, ssi_size_t &maxind);

	//! \brief Finds the maximum along a dimension
	//
	//! \param matrix		the matrix
	//! \param dimension	the dimension
	//! \return				matrix with maximum indeces (indexed along dimension)
	//
	static Matrix<ssi_size_t> *Max (const Matrix<T> *const matrix, MATRIX_DIMENSION dimension);

	//! \brief Calculate mean of all elements
	//
	//! \param matrix		the matrix
	//! \return				mean value
	//
	static double Mean (const Matrix<T> *const matrix);

	//! \brief Calculate mean along a dimension
	//
	//! \param matrix		the matrix
	//! \param dimension	the dimension
	//! \return				matrix with mean values
	//
	static Matrix<double> *Mean (const Matrix<T> *const matrix, MATRIX_DIMENSION dimension);

	//! \brief Calculate variance all elements
	//
	//! \param matrix		the matrix
	//! \return				variance value
	//
	static double Var (const Matrix<T> *const matrix);

	//! \brief Sets all elements to zero
	//
	//! \param matrix		the matrix
	//
	static void Empty (const Matrix<T> *const matrix);

	//! \brief Sets all elements to value
	//
	//! \param matrix		the matrix
	//! \param value		the value
	//
	static void Fill (const Matrix<T> *const matrix, T value);

	//! \brief Adds a scalar to each element
	//
	//! \param matrix		the matrix
	//! \param value		the value
	//
	static void Plus (const Matrix<T> *const matrix, T scalar);

	//! \brief Adds the second matrix to the first matrix
	//
	//! \param matrix1		the first matrix
	//! \param matrix2		the second matrix
	//
	static void Plus (const Matrix<T> *const matrix1, const Matrix<T> *const matrix2);

	//! \brief Substracts a scalar from each element
	//
	//! \param matrix		the matrix
	//! \param value		the value
	//
	static void Minus (const Matrix<T> *const matrix, T scalar);

	//! \brief Subtracts the second matrix from the first matrix
	//
	//! \param matrix1		the first matrix
	//! \param matrix2		the second matrix
	//
	static void Minus (const Matrix<T> *const matrix1, const Matrix<T> *const matrix2);

	//! \brief Multiplies each element with a scalar
	//
	//! \param matrix		the matrix
	//! \param value		the value
	//
	static void Mult (const Matrix<T> *const matrix, T scalar);

	//! \brief Multiplies elementwise the first matrix with the second matrix
	//
	//! \param matrix1		the first matrix
	//! \param matrix2		the second matrix
	//
	static void Mult (const Matrix<T> *const matrix1, const Matrix<T> *const matrix2);

	//! \brief Matrix multiplication
	//
	//! \param matrix1		the first matrix
	//! \param matrix2		the second matrix
	//! \return				the result matrix
	//
	static Matrix<T> *MultM (const Matrix<T> *const matrix1, const Matrix<T> *const matrix2);

	//! \brief Vector multiplication
	//
	//! \param vector1		the first vector
	//! \param vector2		the second vector
	//! \return				the result value
	//
	static T MultV (const Matrix<T> *const vector1, const Matrix<T> *const vector2);

	//! \brief Matrix multiplication
	//	
	//! \param matrix1		the first matrix
	//! \param matrix2		the second matrix
	//! \param result		the result matrix
	//
	static void MultM (const Matrix<T> *const matrix1, const Matrix<T> *const matrix2, const Matrix<T> *const result);

	//! \brief Vector convolution
	//	
	//! \param matrix1		the first vector
	//! \param matrix2		the second vector
	//! \return				the result vector
	//
	static Matrix<T> *Conv (const Matrix<T> *const matrix1, const Matrix<T> *const matrix2, MATRIX_DIMENSION dimension);

	//! \brief Vector convolution
	//	
	//! \param matrix1		the first vector
	//! \param matrix2		the second vector
	//! \param result		the result vector
	//
	static void Conv (const Matrix<T> *const vector1, const Matrix<T> *const vector2, const Matrix<T> *const result);

	//! \brief Divides each element by a scalar
	//
	//! \param matrix		the matrix
	//! \param value		the value
	//
	static void Div (const Matrix<T> *const matrix, T scalar);

	//! \brief Divides elementwise the first matrix by the second matrix
	//
	//! \param matrix1		the first matrix
	//! \param matrix2		the second matrix
	//
	static void Div (const Matrix<T> *const matrix1, const Matrix<T> *const matrix2);

	//! \brief Calculates the power for each element
	//
	//! \param matrix		the matrix
	//! \param value		the value
	//
	static void Pow (const Matrix<T> *const matrix, T value);

	//! \brief Calculates the square root for each element
	//
	//! \param matrix		the matrix
	//
	static void Sqrt (const Matrix<T> *const matrix);

	//! \brief Calculates the exponential for each element
	//
	//! \param matrix		the matrix
	//
	static void Exp (const Matrix<T> *const matrix);

	//! \brief Calculates the sine for each element
	//
	//! \param matrix		the matrix
	//
	static void Sin (const Matrix<T> *const matrix);

	//! \brief Calculates the cosine for each element
	//
	//! \param matrix		the matrix
	//
	static void Cos (const Matrix<T> *const matrix);

	//! \brief Calculates natural logarithm for each element
	//
	//! \param matrix		the matrix
	//
	static void Log (const Matrix<T> *const matrix);

	//! \brief Calculates base-10 logarithm for each element
	//
	//! \param matrix		the matrix
	//
	static void Log10 (const Matrix<T> *const matrix);

	//! \brief Calculates the absolute value for each element
	//
	//! \param matrix		the matrix
	//
	static void Abs (const Matrix<T> *const matrix);

	//! \brief Calculates the absolute value for complex element
	//
	//! \param matrix		the matrix
	//
	static Matrix<T> *Abs (const Matrix<std::complex<T>> *const matrix);

	//! \brief Calculates the absolute value for complex elements
	//
	//! \param matrix_in	the matrix_in (complex)
	//! \param matrix_out	the matrix_out (non complex)
	//
	static void Abs (const Matrix<std::complex<T>> *const matrix_in, const Matrix<T> *const matrix_out);

	//! \brief Sorts matrix elements along dimension
	//
	//! \param matrix		the matrix
	//! \param dimension	dimension
	//
	static void Sort (const Matrix<T> *const matrix, MATRIX_DIMENSION dimension);
	//static void Sort (Matrix<std::complex<T>> *matrix, MATRIX_DIMENSION dimension);
	static int compare (const void *a, const void *b);
	//static int compare_complex (const void *a, const void *b);

};

}

#include "MatrixOps.cpp"

#endif // _MATRIXOPS_H
