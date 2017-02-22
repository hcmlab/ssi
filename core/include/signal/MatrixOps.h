// MatrixOps.h
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
	static Matrix<T> *Abs (const Matrix< std::complex<T> > *const matrix);

	//! \brief Calculates the absolute value for complex elements
	//
	//! \param matrix_in	the matrix_in (complex)
	//! \param matrix_out	the matrix_out (non complex)
	//
	static void Abs (const Matrix< std::complex<T> > *const matrix_in, const Matrix<T> *const matrix_out);

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

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////
///                                IMPLEMENTATION STARTS HERE
/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////


/// PRINT ///

template <class T>
void MatrixOps<T>::Print (const Matrix<T> *const matrix) {

        File *file = File::CreateAndOpen (File::ASCII, File::WRITE, 0);
        Print (file, matrix);
        delete file;
}

template <class T>
void MatrixOps<T>::Print (File *file, const Matrix<T> *const matrix) {

        bool status;

        if (file->getType () == File::ASCII) {
                file->setType (ssi_gettype<T> (matrix->data[0]));
                status = file->write (matrix->data, matrix->cols, matrix->cols * matrix->rows);
        } else {
                status = file->write (matrix->data, sizeof (T), matrix->cols * matrix->rows);
        }

        if (!status) {
                ssi_err ("Print failed ()");
        }
}

template <class T>
void MatrixOps<T>::Print (File *file, const Matrix<T> *const matrix, double sample_rate) {

        // add header:
        // sample_rate sample_dimension sample_bytes
        // sample_time sample_number
        if (file->getType () == File::ASCII) {
                ssi_char_t string[1024];
                sprintf (string, "%Lf %u %u", sample_rate, matrix->cols, sizeof (T));
                file->writeLine (string);
                sprintf (string, "%Lf %u", 0.0, matrix->rows);
                file->writeLine (string);
        } else {
                file->write (&sample_rate, sizeof (ssi_time_t), 1);
                file->write (&matrix->cols, sizeof (ssi_size_t), 1);
                ssi_size_t sample_bytes = sizeof (T);
                file->write (&sample_bytes, sizeof (ssi_size_t), 1);
                ssi_time_t consume_time = 0.0;
                file->write (&consume_time, sizeof (ssi_time_t), 1);
                file->write (&matrix->rows, sizeof (ssi_size_t), 1);
        }

        Print (file, matrix);
}

template <class T>
Matrix<T> *MatrixOps<T>::Read (File *file) {

        ssi_time_t sample_rate;
        ssi_size_t sample_dimension;
        ssi_size_t sample_bytes;
        ssi_time_t time;
        ssi_size_t sample_number;
        Matrix<T> *matrix;

        // header
        if (file->getType () == File::ASCII) {
                ssi_char_t string[1024];
                file->readLine (1024, string);
                sscanf (string, "%Lf %u %u", &sample_rate, &sample_dimension, &sample_bytes);
        } else {
                file->read (&sample_rate, sizeof (ssi_time_t), 1);
                file->read (&sample_dimension, sizeof (ssi_size_t), 1);
                file->read (&sample_bytes, sizeof (ssi_size_t), 1);
        }

        // allocate matrix
        if (file->getType () == File::ASCII) {
                ssi_char_t string[1024];
                file->readLine (1024, string);
                ssi_time_t time;
                sscanf (string, "%Lf %u", &time, &sample_number);
                matrix = new Matrix<T> (sample_number, sample_dimension);
        } else {
                file->read (&time, sizeof (ssi_time_t), 1);
                file->read (&sample_number, sizeof (ssi_size_t), 1);
                matrix = new Matrix<T> (sample_number, sample_dimension);
        }

        // read data
        if (file->getType () == File::ASCII) {
                file->read (matrix->data, sample_dimension, sample_number * sample_dimension);
        } else {
                file->read (matrix->data, sample_bytes, sample_number * sample_dimension);
        }

        return matrix;
}



/// RESHAPE ///

template <class T>
void MatrixOps<T>::Transpose (Matrix<T> *const matrix) {

        if (MatrixOps<T>::IsEmpty (matrix)) {
                return;
        }

        if (matrix->rows > 1 && matrix->cols > 1) {

                Matrix<T> *tmp = MatrixOps<T>::Clone (matrix);

                T *dstptr = matrix->data;
                ssi_size_t srcrows = tmp->rows;
                ssi_size_t srccols = tmp->cols;
                for (ssi_size_t i = 0; i < srccols; i++) {
                        T *srcptr = tmp->data + i;
                        for (ssi_size_t j = 0; j < srcrows; j++) {
                                *dstptr = *srcptr;
                                srcptr += srccols;
                                dstptr++;
                        }
                }

                delete tmp;
        }

        ssi_size_t tmp = matrix->cols;
        matrix->cols = matrix->rows;
        matrix->rows = tmp;
}

template <class T>
void MatrixOps<T>::Flip (const Matrix<T> *const matrix, MATRIX_DIMENSION dimension) {

        if (MatrixOps<T>::IsEmpty (matrix)) {
                return;
        }

        ssi_size_t cols = matrix->cols;
        ssi_size_t rows = matrix->rows;
        ssi_size_t steps;
        T *ptr1, *ptr2, *ptr1tmp, *ptr2tmp;
        T tmp;

        switch (dimension) {
                case MATRIX_DIMENSION_COL:

                        steps = rows >> 1;
                        ptr1 = matrix->data;
                        ptr2 = matrix->data + (rows - 1) * cols;
                        for (ssi_size_t i = 0; i < cols; i++) {
                                ptr1tmp = ptr1;
                                ptr2tmp = ptr2;
                                for (ssi_size_t j = 0; j < steps; j++) {
                                        tmp = *ptr2tmp;
                                        *ptr2tmp = *ptr1tmp;
                                        *ptr1tmp = tmp;
                                        ptr1tmp += cols;
                                        ptr2tmp -= cols;
                                }
                                ptr1++;
                                ptr2++;
                        }

                        break;
                case MATRIX_DIMENSION_ROW:

                        steps = cols >> 1;
                        ptr1 = matrix->data;
                        ptr2 = matrix->data + cols - 1;
                        for (ssi_size_t i = 0; i < rows; i++) {
                                ptr1tmp = ptr1;
                                ptr2tmp = ptr2;
                                for (ssi_size_t j = 0; j < steps; j++) {
                                        tmp = *ptr2tmp;
                                        *ptr2tmp-- = *ptr1tmp;
                                        *ptr1tmp = tmp;
                                        ptr1tmp++;
                                }
                                ptr1 += cols;
                                ptr2 += cols;
                        }

                        break;
        }

}









/// CREATE ///

template <class T>
Matrix<T> *MatrixOps<T>::Zeros (ssi_size_t rows, ssi_size_t cols) {

        Matrix<T> *matrix = new Matrix<T> (rows, cols);
        ssi_size_t elems = rows * cols;

        T *dataptr = matrix->data;
        for (ssi_size_t i = 0; i < elems; i++) {
                *dataptr = 0;
                dataptr++;
        }

        return matrix;
}

template <class T>
Matrix<T> *MatrixOps<T>::Ones (ssi_size_t rows, ssi_size_t cols) {

        Matrix<T> *matrix = new Matrix<T> (rows, cols);
        ssi_size_t elems = rows * cols;

        T *dataptr = matrix->data;
        for (ssi_size_t i = 0; i < elems; i++) {
                *dataptr = 1;
                dataptr++;
        }

        return matrix;
}

template <class T>
Matrix<T> *MatrixOps<T>::Rand (ssi_size_t rows, ssi_size_t cols, T maxval) {

        Matrix<T> *matrix = new Matrix<T> (rows, cols);
        ssi_size_t elems = rows * cols;

        T *dataptr = matrix->data;
        double number;
        for (ssi_size_t i = 0; i < elems; i++) {
                // generate random number in interval [0..1)
                number = static_cast<double>(rand ()) / (static_cast<double>(RAND_MAX) + 1.0);
                *dataptr = ssi_cast (T, number * maxval);
                dataptr++;
        }

        return matrix;
}

template <class T>
Matrix<T> *MatrixOps<T>::Array (T start, T delta, T end, MATRIX_DIMENSION dimension) {

        //cout << (end - start) / static_cast<double> (delta) + 1.0 << endl;
        //cout << static_cast<ssi_size_t> ((end - start) / static_cast<double> (delta) + + 1.001) << endl;
        ssi_size_t steps = static_cast<ssi_size_t> (((end - start) / static_cast<double> (delta)) + 1.001);
        Matrix<T> *matrix;

        if (steps <= 0) {
                matrix = new Matrix<T> (0,0,0);
                return matrix;
        }

        switch (dimension) {
                case MATRIX_DIMENSION_ROW:
                        matrix = new Matrix<T> (1, steps);
                        break;
                case MATRIX_DIMENSION_COL:
                        matrix = new Matrix<T> (steps, 1);
                        break;
        }

        T *dataptr = matrix->data;
        *dataptr = start;
        for (ssi_size_t i = 0; i < steps-1; i++) {
                *(dataptr+1) = *dataptr + delta;
                dataptr++;
        }

        return matrix;
}

template <class T>
Matrix<ssi_size_t> *MatrixOps<T>::IndArray (ssi_size_t start, ssi_size_t delta, ssi_size_t end, MATRIX_DIMENSION dimension) {

        bool flip = start > end;
        if (flip) {
                ssi_size_t tmp = start;
                start = end;
                end = tmp;
        }
        ssi_size_t steps = (end - start) / delta + 1;
        Matrix<ssi_size_t> *matrix;

        switch (dimension) {
                case MATRIX_DIMENSION_ROW:
                        matrix = new Matrix<ssi_size_t> (1, steps);
                        break;
                case MATRIX_DIMENSION_COL:
                        matrix = new Matrix<ssi_size_t> (steps, 1);
                        break;
        }

        ssi_size_t *dataptr = matrix->data;
        if (flip) {
                *dataptr = end;
                for (ssi_size_t i = 0; i < steps-1; i++) {
                        *(dataptr+1) = *dataptr - delta;
                        dataptr++;
                }
        } else {
                *dataptr = start;
                for (ssi_size_t i = 0; i < steps-1; i++) {
                        *(dataptr+1) = *dataptr + delta;
                        dataptr++;
                }
        }

        return matrix;
}

template <class T>
Matrix<T> *MatrixOps<T>::Concat (const Matrix<T> *const matrix_1, const Matrix<T> *const matrix_2, MATRIX_DIMENSION dimension) {

        Matrix<T> *resmat;

        switch (dimension) {

                case MATRIX_DIMENSION_ROW:

                        SSI_ASSERT (matrix_1->cols == matrix_2->cols);

                        resmat = new Matrix<T> (matrix_1->rows + matrix_2->rows, matrix_1->cols);
                        memcpy (resmat->data, matrix_1->data, sizeof (T) * matrix_1->rows * matrix_1->cols);
                        memcpy (resmat->data + (matrix_1->rows * matrix_1->cols), matrix_2->data, sizeof (T) * matrix_2->rows * matrix_2->cols);

                        break;

                case MATRIX_DIMENSION_COL:

                        SSI_ASSERT (matrix_1->rows == matrix_2->rows);

                        resmat = new Matrix<T> (matrix_1->rows, matrix_1->cols + matrix_2->cols);
                        T *srcptr_1 = matrix_1->data;
                        T *srcptr_2 = matrix_2->data;
                        T *dstptr = resmat->data;
                        for (ssi_size_t i = 0; i < matrix_1->rows; i++) {
                                memcpy (dstptr, srcptr_1, sizeof (T) * matrix_1->cols);
                                memcpy (dstptr + matrix_1->cols, srcptr_2, sizeof (T) * matrix_2->cols);
                                srcptr_1 += matrix_1->cols;
                                srcptr_2 += matrix_2->cols;
                                dstptr += resmat->cols;
                        }

                        break;
        }

        return resmat;
}

template <class T>
Matrix<T> *MatrixOps<T>::Clone (const Matrix<T> *const matrix) {

        if (MatrixOps<T>::IsEmpty (matrix)) {
                return new Matrix<T> (0,0,0);
        }

        Matrix<T> *resmat = new Matrix<T> (matrix->rows, matrix->cols);
        memcpy (resmat->data, matrix->data, sizeof (T) * matrix->cols * matrix->rows);

        return resmat;
}

template <class T>
void MatrixOps<T>::Clone (const Matrix<T> *const matrix_in, const Matrix<T> *const matrix_out) {

        SSI_ASSERT (matrix_in->cols == matrix_out->cols && matrix_in->rows == matrix_out->rows);

        memcpy (matrix_out->data, matrix_in->data, sizeof (T) * matrix_in->cols * matrix_in->rows);
}

template <class T>
Matrix<T> *MatrixOps<T>::Repmat (const Matrix<T> *const matrix, ssi_size_t vert, ssi_size_t horz) {

        if (MatrixOps<T>::IsEmpty (matrix)) {
                return new Matrix<T> (0,0,0);
        }

        Matrix<T> *resmat = new Matrix<T> (matrix->rows * vert, matrix->cols * horz);

        T *srcptr = matrix->data;
        ssi_size_t rows = matrix->rows;
        ssi_size_t cols = matrix->cols;
        T *dstptr = resmat->data;

        // copy rowwise in horizontal direction
        for (ssi_size_t i = 0; i < rows; ++i) {
                for (ssi_size_t j = 0; j < horz; ++j) {
                        memcpy (dstptr, srcptr, sizeof (T) * cols);
                        dstptr += cols;
                }
                srcptr += cols;
        }

        // now clone in vertical direction
        srcptr = resmat->data;
        ssi_size_t elems = rows * cols * horz;
        for (ssi_size_t i = 1; i < vert; ++i) {
                memcpy (dstptr, srcptr, sizeof (T) * elems);
                dstptr += elems;
        }

        return resmat;
}





/// ACCESS ///

template <class T>
T MatrixOps<T>::Get (const Matrix<T> *const matrix, ssi_size_t index) {

        SSI_ASSERT (!MatrixOps<T>::IsEmpty (matrix));

        return matrix->data[index];
}

template <class T>
T MatrixOps<T>::Get (const Matrix<T> *const matrix, ssi_size_t row, ssi_size_t col) {

        SSI_ASSERT (!MatrixOps<T>::IsEmpty (matrix));

        return matrix->data[row * matrix->cols + col];
}

template <class T>
void MatrixOps<T>::Set (const Matrix<T> *const matrix, ssi_size_t index, T value) {

        SSI_ASSERT (!MatrixOps<T>::IsEmpty (matrix));

        matrix->data[index] = value;
}

template <class T>
void MatrixOps<T>::Set (const Matrix<T> *const matrix, ssi_size_t row, ssi_size_t col, T value) {

        SSI_ASSERT (!MatrixOps<T>::IsEmpty (matrix));

        matrix->data[row * matrix->cols + col] = value;
}

template <class T>
Matrix<T> *MatrixOps<T>::GetSubMatrix (const Matrix<T> *const matrix, const Matrix<ssi_size_t> *const indices) {

        Matrix<T> *resmat = new Matrix<T> (indices->rows, indices->cols);
        T *resmatptr = resmat->data;
        ssi_size_t resrows = resmat->rows;
        ssi_size_t rescols = resmat->cols;
        ssi_size_t *indmatptr = indices->data;
        for (ssi_size_t i = 0; i < resrows; i++) {
                for (ssi_size_t j = 0; j < rescols; j++) {
                        *resmatptr = matrix->data[*indmatptr];
                        resmatptr++;
                        indmatptr++;
                }
        }

        return resmat;
}

template <class T>
void MatrixOps<T>::SetSubMatrix (const Matrix<T> *const matrix, const Matrix<ssi_size_t> *const indices, const Matrix<T> *const values) {

        SSI_ASSERT (indices->rows == values->rows && indices->cols == values->cols);

        T *srcptr = values->data;
        T *dstptr = matrix->data;
        ssi_size_t *indptr = indices->data;
        ssi_size_t elems = indices->rows * indices->cols;
        for (ssi_size_t i = 0; i < elems; i++) {
                *(dstptr + *(indptr)) = *(srcptr);
                indptr++;
                srcptr++;
        }
}

template <class T>
Matrix<T> *MatrixOps<T>::GetSubMatrix (const Matrix<T> *const matrix, ssi_size_t row_start, ssi_size_t row_stop, ssi_size_t col_start, ssi_size_t col_stop) {

        SSI_ASSERT (row_start <= row_stop && col_start <= col_stop && row_stop < matrix->rows && col_stop < matrix->cols);

        ssi_size_t dstrows = row_stop - row_start + 1;
        ssi_size_t dstcols = col_stop - col_start + 1;

        Matrix<T> *resmat = new Matrix<T> (dstrows, dstcols);
        T *srcptr = matrix->data + row_start * matrix->cols + col_start;
        ssi_size_t srccols = matrix->cols;
        T *dstptr = resmat->data;
        for (ssi_size_t i = 0; i < dstrows; i++) {
                memcpy (dstptr, srcptr, sizeof (T) * dstcols);
                srcptr += srccols;
                dstptr += dstcols;
        }

        return resmat;
}

template <class T>
void MatrixOps<T>::SetSubMatrix (const Matrix<T> *const matrix, ssi_size_t row, ssi_size_t col, const Matrix<T> *const submatrix) {

        SetSubMatrix (matrix, row, col, 0, 0, submatrix->rows, submatrix->cols, submatrix);
}

template <class T>
void MatrixOps<T>::SetSubMatrix (const Matrix<T> *const matrix_dst, ssi_size_t row_dst, ssi_size_t col_dst, ssi_size_t row_src, ssi_size_t col_src, ssi_size_t row_number, ssi_size_t col_number, const Matrix<T> *const matrix_src) {

        SSI_ASSERT (row_dst + row_number <= matrix_dst->rows && col_dst + col_number <= matrix_dst->cols &&
                        row_src + row_number <= matrix_src->rows && col_src + col_number <= matrix_src->cols);

        ssi_size_t srccols = matrix_src->cols;
        T *srcptr = matrix_src->data + row_src * srccols + col_src;
        ssi_size_t dstcols = matrix_dst->cols;
        T *dstptr = matrix_dst->data + row_dst * dstcols + col_dst;
        for (ssi_size_t i = 0; i < row_number; i++) {
                memcpy (dstptr, srcptr, sizeof (T) * col_number);
                srcptr += srccols;
                dstptr += dstcols;
        }
}


template <class T>
Matrix<T> *MatrixOps<T>::GetSubMatrix (const Matrix<T> *const matrix, ssi_size_t start, ssi_size_t stop, MATRIX_DIMENSION dimension) {

        switch (dimension) {
                case MATRIX_DIMENSION_ROW:
                        return GetSubMatrix (matrix, start, stop, 0, matrix->cols - 1);
                case MATRIX_DIMENSION_COL:
                        return GetSubMatrix (matrix, 0, matrix->rows - 1, start, stop);
        }

        return NULL;
}

template <class T>
void MatrixOps<T>::SetSubMatrix (const Matrix<T> *const matrix, ssi_size_t start, MATRIX_DIMENSION dimension, const Matrix<T> *const submatrix) {

        switch (dimension) {
                case MATRIX_DIMENSION_ROW:
                        SSI_ASSERT (matrix->cols == submatrix->cols);
                        SetSubMatrix (matrix, start, 0, submatrix);
                        break;
                case MATRIX_DIMENSION_COL:
                        SSI_ASSERT (matrix->rows == submatrix->rows);
                        SetSubMatrix (matrix, 0, start, submatrix);
                        break;
        }
}

template <class T>
void MatrixOps<T>::SetSubMatrix (const Matrix<T> *const matrix_dst, ssi_size_t start_dst, ssi_size_t start_src, MATRIX_DIMENSION dimension, ssi_size_t number, const Matrix<T> *const matrix_src) {

        switch (dimension) {
                case MATRIX_DIMENSION_ROW:
                        SSI_ASSERT (matrix_dst->cols == matrix_src->cols);
                        SetSubMatrix (matrix_dst, start_dst, 0, start_src, 0, number, matrix_dst->cols, matrix_src);
                        break;
                case MATRIX_DIMENSION_COL:
                        SSI_ASSERT (matrix_dst->rows == matrix_src->rows);
                        SetSubMatrix (matrix_dst, 0, start_dst, 0, start_src, matrix_dst->rows, number, matrix_src);
                        break;
        }
}

template <class T>
Matrix<T> *MatrixOps<T>::GetSubMatrix (const Matrix<T> *const matrix, const Matrix<ssi_size_t> *const indices, MATRIX_DIMENSION dimension) {

        SSI_ASSERT (MatrixOps<ssi_size_t>::IsVector (indices));

        if (IsEmpty (matrix)) {
                return new Matrix<T> (0, 0);
        }

        Matrix<T> *resmat;

        switch (dimension) {

                case MATRIX_DIMENSION_ROW:
                        {
                                ssi_size_t resrows = indices->rows * indices->cols;
                                ssi_size_t rescols = matrix->cols;
                                resmat = new Matrix<T> (resrows, rescols);
                                T *resmatptr = resmat->data;
                                ssi_size_t *indmatptr = indices->data;
                                T *srcmatptr = matrix->data;
                                ssi_size_t srcrows = matrix->rows;
                                ssi_size_t srccols = matrix->cols;
                                size_t cpysize = srccols * sizeof (T);
                                for (ssi_size_t i = 0; i < resrows; i++) {
                                        memcpy (resmatptr, srcmatptr + *indmatptr * srccols, cpysize);
                                        resmatptr += srccols;
                                        indmatptr++;
                                }
                        }
                        break;

                case MATRIX_DIMENSION_COL:
                        {
                                ssi_size_t resrows = matrix->rows;
                                ssi_size_t rescols = indices->rows * indices->cols;
                                resmat = new Matrix<T> (resrows, rescols);
                                T *resmatptr;
                                ssi_size_t *indmatptr = indices->data;
                                T *srcmatptr;
                                ssi_size_t srcrows = matrix->rows;
                                ssi_size_t srccols = matrix->cols;
                                for (ssi_size_t i = 0; i < rescols; i++) {
                                        srcmatptr = matrix->data + *indmatptr;
                                        resmatptr = resmat->data + i;
                                        for (ssi_size_t j = 0; j < srcrows; j++) {
                                                *resmatptr = *srcmatptr;
                                                srcmatptr += srccols;
                                                resmatptr += rescols;
                                        }
                                        indmatptr++;
                                }
                        }
                        break;
        }

        return resmat;
}

template <class T>
Matrix<ssi_size_t> *MatrixOps<T>::Find (const Matrix<T> *const matrix, T value) {

        if (IsEmpty (matrix)) {
                return new Matrix<ssi_size_t> (0, 0);
        }

        ssi_size_t elems = matrix->rows * matrix->cols;
        T *dataptr = matrix->data;

        ssi_size_t *tmpinds = new ssi_size_t[elems];
        ssi_size_t counter = 0;
        for (ssi_size_t i = 0; i < elems; i++) {
                if (*dataptr == value) {
                        tmpinds[counter] = i;
                        counter++;
                }
                dataptr++;
        }

        Matrix<ssi_size_t> *result = new Matrix<ssi_size_t> (counter, 1);
        memcpy (result->data, tmpinds, counter * sizeof (ssi_size_t));

        delete tmpinds;

        return result;
}





/// ARITHMETIC ///

template <class T>
T MatrixOps<T>::Sum (const Matrix<T> *const matrix) {

        T sum = 0;
        ssi_size_t elems = matrix->rows * matrix->cols;

        T* dataptr = matrix->data;
        for (ssi_size_t i = 0; i < elems; i++) {
                sum += *dataptr;
                dataptr++;
        }

        return sum;
}

template <class T>
Matrix<T> *MatrixOps<T>::Sum (const Matrix<T> *const matrix, MATRIX_DIMENSION dimension) {

        Matrix<T> *resmat;

        switch (dimension) {

                case MATRIX_DIMENSION_ROW:

                        resmat = new Matrix<T> (matrix->rows, 1);
                        break;

                case MATRIX_DIMENSION_COL:

                        resmat = new Matrix<T> (1, matrix->cols);
                        break;

        }

        Sum (matrix, resmat, dimension);

        return resmat;
}

template <class T>
void MatrixOps<T>::Sum (const Matrix<T> *const matrix, const Matrix<T> *const result, MATRIX_DIMENSION dimension) {

        T *srcptr, *dstptr;

        switch (dimension) {

                case MATRIX_DIMENSION_ROW:

                        SSI_ASSERT (result->rows * result->cols == matrix->rows);

                        srcptr = matrix->data;
                        dstptr = result->data;
                        for (ssi_size_t i = 0; i < matrix->rows; i++) {
                                *dstptr = 0;
                                for (ssi_size_t j = 0; j < matrix->cols; j++) {
                                        *dstptr += *srcptr;
                                    srcptr++;
                                }
                                dstptr++;
                        }

                        break;


                case MATRIX_DIMENSION_COL:

                        SSI_ASSERT (result->rows * result->cols == matrix->cols);

                        dstptr = result->data;
                        for (ssi_size_t i = 0; i < matrix->cols; i++) {
                                *dstptr = 0;
                                dstptr++;
                        }
                        srcptr = matrix->data;
                        for (ssi_size_t i = 0; i < matrix->rows; i++) {
                                dstptr = result->data;
                                for (ssi_size_t j = 0; j < matrix->cols; j++) {
                                        *dstptr += *srcptr;
                                        dstptr++;
                                        srcptr++;
                                }
                        }

                        break;
        }
}

template <class T>
T MatrixOps<T>::SumSqr (const Matrix<T> *const matrix) {

        T sum = 0;
        ssi_size_t elems = matrix->rows * matrix->cols;

        T* dataptr = matrix->data;
        for (ssi_size_t i = 0; i < elems; i++) {
                sum += *dataptr * *dataptr;
                dataptr++;
        }

        return sum;
}

template <class T>
Matrix<T> *MatrixOps<T>::SumSqr (const Matrix<T> *const matrix, MATRIX_DIMENSION dimension) {

        Matrix<T> *resmat;

        switch (dimension) {

                case MATRIX_DIMENSION_ROW:

                        resmat = new Matrix<T> (matrix->rows, 1);
                        break;

                case MATRIX_DIMENSION_COL:

                        resmat = new Matrix<T> (1, matrix->cols);
                        break;

        }

        SumSqr (matrix, resmat, dimension);

        return resmat;
}

template <class T>
void MatrixOps<T>::SumSqr (const Matrix<T> *const matrix, const Matrix<T> *const result, MATRIX_DIMENSION dimension) {

        T *srcptr, *dstptr;

        switch (dimension) {

                case MATRIX_DIMENSION_ROW:

                        SSI_ASSERT (result->rows * result->cols == matrix->rows);

                        srcptr = matrix->data;
                        dstptr = result->data;
                        for (ssi_size_t i = 0; i < matrix->rows; i++) {
                                *dstptr = 0;
                                for (ssi_size_t j = 0; j < matrix->cols; j++) {
                                        *dstptr += *srcptr * *srcptr;
                                        srcptr++;
                                }
                                dstptr++;
                        }

                        break;


                case MATRIX_DIMENSION_COL:

                        SSI_ASSERT (result->rows * result->cols == matrix->cols);

                        dstptr = result->data;
                        for (ssi_size_t i = 0; i < matrix->cols; i++) {
                                *dstptr++ = 0;
                        }
                        srcptr = matrix->data;
                        for (ssi_size_t i = 0; i < matrix->rows; i++) {
                                dstptr = result->data;
                                for (ssi_size_t j = 0; j < matrix->cols; j++) {
                                        *dstptr += *srcptr * *srcptr;
                                        dstptr++;
                                        srcptr++;
                                }
                        }

                        break;
        }
}


template <class T>
T MatrixOps<T>::Min (const Matrix<T> *const matrix) {

        T minval;
        ssi_size_t minind;
        Max (matrix, minval, minind);
        return minval;
}

template <class T>
void MatrixOps<T>::Min (const Matrix<T> *const matrix, T &minval, ssi_size_t &minind) {

        T* dataptr = matrix->data;
        ssi_size_t elems =  matrix->rows * matrix->cols;

        minval = *(dataptr);
        dataptr++;
        minind = 0;
        for (ssi_size_t i = 1; i < elems; i++) {
                if (*dataptr < minval) {
                        minval = *dataptr;
                        minind = i;
                }
                dataptr++;
        }
}

template <class T>
Matrix<ssi_size_t> *MatrixOps<T>::Min (const Matrix<T> *const matrix, MATRIX_DIMENSION dimension) {

        Matrix<ssi_size_t> *resmat;
        T val;
        T* dataptr;

        switch (dimension) {

                case MATRIX_DIMENSION_ROW:

                        resmat = new Matrix<ssi_size_t> (matrix->rows, 1);
                        dataptr = matrix->data;
                        for (ssi_size_t i = 0; i < matrix->rows; i++) {
                                val = *dataptr;
                                resmat->data[i] = 0;
                                dataptr++;
                                for (ssi_size_t j = 1; j < matrix->cols; j++) {
                                        if (val > *dataptr) {
                                                val = *dataptr;
                                                resmat->data[i] = j;
                                        }
                                        dataptr++;
                                }
                        }
                        break;

                case MATRIX_DIMENSION_COL:

                        resmat = new Matrix<ssi_size_t> (1, matrix->cols);
                        for (ssi_size_t i = 0; i < matrix->cols; i++) {
                                dataptr = matrix->data + i;
                                val = *dataptr;
                                resmat->data[i] = 0;
                                dataptr += matrix->cols;
                                for (ssi_size_t j = 1; j < matrix->rows; j++) {
                                        if (val > *dataptr) {
                                                val = *dataptr;
                                                resmat->data[i] = j;
                                        }
                                        dataptr += matrix->cols;
                                }
                        }
                        break;
        }

        return resmat;
}


template <class T>
T MatrixOps<T>::Max (const Matrix<T> *const matrix) {

        T maxval;
        ssi_size_t maxind;
        Max (matrix, maxval, maxind);
        return maxval;
}

template <class T>
void MatrixOps<T>::Max (const Matrix<T> *const matrix, T &maxval, ssi_size_t &maxind) {

        T* dataptr = matrix->data;
        ssi_size_t elems  = matrix->rows * matrix->cols;

        maxval = *(dataptr);
        dataptr++;
        maxind = 0;
        for (ssi_size_t i = 1; i < elems; i++) {
                if (*dataptr > maxval) {
                        maxval = *dataptr;
                        maxind = i;
                }
                dataptr++;
        }
}

template <class T>
Matrix<ssi_size_t> *MatrixOps<T>::Max (const Matrix<T> *const matrix, MATRIX_DIMENSION dimension) {

        Matrix<ssi_size_t> *resmat;
        T val;
        T* dataptr;

        switch (dimension) {

                case MATRIX_DIMENSION_ROW:

                        dataptr = matrix->data;
                        resmat = new Matrix<ssi_size_t> (matrix->rows, 1);
                        for (ssi_size_t i = 0; i < matrix->rows; i++) {
                                val = *dataptr;
                                resmat->data[i] = 0;
                                dataptr++;
                                for (ssi_size_t j = 1; j < matrix->cols; j++) {
                                        if (val < *dataptr) {
                                                val = *dataptr;
                                                resmat->data[i] = j;
                                        }
                                        dataptr++;
                                }
                        }
                        break;

                case MATRIX_DIMENSION_COL:

                        resmat = new Matrix<ssi_size_t> (1, matrix->cols);
                        for (ssi_size_t i = 0; i < matrix->cols; i++) {
                                dataptr = matrix->data + i;
                                val = *dataptr;
                                resmat->data[i] = 0;
                                dataptr += matrix->cols;
                                for (ssi_size_t j = 1; j < matrix->rows; j++) {
                                        if (val < *dataptr) {
                                                val = *dataptr;
                                                resmat->data[i] = j;
                                        }
                                        dataptr += matrix->cols;
                                }
                        }
                        break;

        }

        return resmat;
}

template <class T>
double MatrixOps<T>::Mean (const Matrix<T> *const matrix) {

        SSI_ASSERT (!MatrixOps<T>::IsEmpty (matrix));

        double sum = 0;
        ssi_size_t elems  = matrix->rows * matrix->cols;

        return ssi_cast (double, Sum (matrix)) / elems;
}

template <class T>
Matrix<double> *MatrixOps<T>::Mean (const Matrix<T> *const matrix, MATRIX_DIMENSION dimension) {

        SSI_ASSERT (!MatrixOps<T>::IsEmpty (matrix));

        Matrix<T> *summat = Sum (matrix, dimension);
        Matrix<double> *resmat = ToDouble (summat);
        delete summat;

        switch (dimension) {

                case MATRIX_DIMENSION_ROW:
                        MatrixOps<double>::Mult (resmat, 1.0 / matrix->cols);
                        break;

                case MATRIX_DIMENSION_COL:
                        MatrixOps<double>::Mult (resmat, 1.0 / matrix->rows);
                        break;
        }

        return resmat;
}

template <class T>
double MatrixOps<T>::Var (const Matrix<T> *const matrix) {

        SSI_ASSERT (!MatrixOps<T>::IsEmpty (matrix));

        double res  = 0.0;
        double mean = Mean (matrix);
        ssi_size_t elems  = matrix->rows * matrix->cols;

        T *dataptr = matrix->data;
        for (ssi_size_t i = 0; i < elems; i++) {
                res += (*dataptr - mean) * (*dataptr - mean);
                dataptr++;
        }
        return res / (elems - 1);
}

template <class T>
void MatrixOps<T>::Plus (const Matrix<T> *const matrix, T scalar) {

        if (MatrixOps<T>::IsEmpty (matrix)) {
                return;
        }

        T *dataptr = matrix->data;
        ssi_size_t elems  = matrix->rows * matrix->cols;

        for (ssi_size_t i = 0; i < elems; i++) {
                (*dataptr) += scalar;
                dataptr++;
        }
}

template <class T>
void MatrixOps<T>::Plus (const Matrix<T> *const matrix1, const Matrix<T> *const matrix2) {

        // SSI_ASSERT that matrices are of same size
        SSI_ASSERT ((matrix1->rows == matrix2->rows) && (matrix1->cols == matrix2->cols));

        if (MatrixOps<T>::IsEmpty (matrix1)) {
                return;
        }

        T *dataptr1 = matrix1->data;
        T *dataptr2 = matrix2->data;
        ssi_size_t elems  = matrix1->rows * matrix1->cols;

        for (ssi_size_t i = 0; i < elems; i++) {
                *dataptr1 += *dataptr2;
                dataptr1++;
                dataptr2++;
        }
}

template <class T>
void MatrixOps<T>::Minus (const Matrix<T> *const matrix, T scalar) {

        if (MatrixOps<T>::IsEmpty (matrix)) {
                return;
        }

        T *dataptr = matrix->data;
        ssi_size_t elems  = matrix->rows * matrix->cols;

        for (ssi_size_t i = 0; i < elems; i++) {
                (*dataptr) -= scalar;
                dataptr++;
        }
}

template <class T>
void MatrixOps<T>::Minus (const Matrix<T> *const matrix1, const Matrix<T> *const matrix2) {

        // SSI_ASSERT that matrices are of same size
        SSI_ASSERT ((matrix1->rows == matrix2->rows) && (matrix1->cols == matrix2->cols));

        if (MatrixOps<T>::IsEmpty (matrix1)) {
                return;
        }

        T *dataptr1 = matrix1->data;
        T *dataptr2 = matrix2->data;
        ssi_size_t elems  = matrix1->rows * matrix1->cols;

        for (ssi_size_t i = 0; i < elems; i++) {
                *dataptr1 -= *dataptr2;
                dataptr1++;
                dataptr2++;
        }
}

template <class T>
void MatrixOps<T>::Mult (const Matrix<T> *const matrix, T scalar) {

        if (MatrixOps<T>::IsEmpty (matrix)) {
                return;
        }

        T *dataptr = matrix->data;
        ssi_size_t elems  = matrix->rows * matrix->cols;

        for (ssi_size_t i = 0; i < elems; i++) {
                (*dataptr) *= scalar;
                dataptr++;
        }
}

template <class T>
void MatrixOps<T>::Mult (const Matrix<T> *const matrix1, const Matrix<T> *const matrix2) {

        // SSI_ASSERT that matrices are of same size
        SSI_ASSERT ((matrix1->rows == matrix2->rows) && (matrix1->cols == matrix2->cols));

        if (MatrixOps<T>::IsEmpty (matrix1)) {
                return;
        }

        T *dataptr1 = matrix1->data;
        T *dataptr2 = matrix2->data;
        ssi_size_t elems  = matrix1->rows * matrix1->cols;

        for (ssi_size_t i = 0; i < elems; i++) {
                *dataptr1 *= *dataptr2;
                dataptr1++;
                dataptr2++;
        }
}

template <class T>
T MatrixOps<T>::MultV (const Matrix<T> *const vector1, const Matrix<T> *const vector2) {

        SSI_ASSERT (MatrixOps<T>::IsVector (vector1) && MatrixOps<T>::IsVector (vector2) && vector1->cols * vector1->rows == vector2->cols * vector2->rows);

        T result = 0;
        ssi_size_t len = vector1->cols * vector1->rows;
        T *vecptr1 = vector1->data;
        T *vecptr2 = vector2->data;
        for (ssi_size_t i = 0; i < len; i++) {
                result += *vecptr1 * *vecptr2;
                vecptr1++;
                vecptr2++;
        }

        return result;
}

template <class T>
Matrix<T> *MatrixOps<T>::Conv (const Matrix<T> *const vector1, const Matrix<T> *const vector2, MATRIX_DIMENSION dimension) {

        Matrix<T> *resvec;

        if (dimension == MATRIX_DIMENSION_COL) {
                resvec = new Matrix<T> (vector1->rows * vector1->cols + vector2->rows * vector2->cols - 1, 1);
        } else {
                resvec = new Matrix<T> (1, vector1->rows * vector1->cols + vector2->rows * vector2->cols - 1);
        }

        Conv (vector1, vector2, resvec);

        return resvec;
}

template <class T>
void MatrixOps<T>::Conv (const Matrix<T> *const vector1, const Matrix<T> *const vector2, const Matrix<T> *const result) {

        SSI_ASSERT (MatrixOps<T>::IsVector (vector1) && MatrixOps<T>::IsVector (vector2) && MatrixOps<T>::IsVector (result));

        /*

        xx = [zeros(1,length(y)-1) x zeros(1,length(y)-1)];
        yy = fliplr(y)';
        zz = zeros (1, length (x) + length(y) - 1);
        for	i = 1:length (zz)
                zz(i) = xx(i:i+length(y)-1) * yy;
        end

        */

        ssi_size_t vector1size = vector1->rows * vector1->cols;
        ssi_size_t vector2size = vector2->rows * vector2->cols;
        ssi_size_t dstsize = result->rows * result->cols;

        SSI_ASSERT (vector1size + vector2size - 1 == dstsize);

        T *filtptr;
        ssi_size_t filtsize;
        Matrix<T> *source;

        if (vector1size >= vector2size) {
                source = MatrixOps<T>::Zeros (vector1size + (vector2size << 1) - 2, 1);
                memcpy (source->data + vector2size - 1, vector1->data, sizeof (T) * vector1size);
                filtptr = vector2->data + vector2size - 1;
                filtsize = vector2size;
        } else {
                source = MatrixOps<T>::Zeros (vector2size + (vector1size << 1) - 2, 1);
                memcpy (source->data + vector1size - 1, vector2->data, sizeof (T) * vector2size);
                filtptr = vector1->data + vector1size - 1;
                filtsize = vector1size;
        }

        T *dstptr = result->data;
        T *filtptrtmp;
        T *srcptr = source->data;
        T *srcptrtmp;
        for (ssi_size_t i = 0; i < dstsize; i++) {
                filtptrtmp = filtptr;
                srcptrtmp = srcptr;
                srcptr++;
                *dstptr = *filtptrtmp * *srcptrtmp;
                filtptrtmp--;
                srcptrtmp++;

                for (ssi_size_t j = 1; j < filtsize; j++) {
                        *dstptr += *filtptrtmp * *srcptrtmp;
                    filtptrtmp--;
                    srcptrtmp++;
                }
                dstptr++;
        }

        delete source;
}

template <class T>
Matrix<T> *MatrixOps<T>::MultM (const Matrix<T> *const matrix1, const Matrix<T> *const matrix2) {

        Matrix<T> *resmat = new Matrix<T> (matrix1->rows, matrix2->cols);

        MultM (matrix1, matrix2, resmat);

        return resmat;
}

template <class T>
void MatrixOps<T>::MultM (const Matrix<T> *const matrix1, const Matrix<T> *const matrix2, const Matrix<T> *const matrix) {

        SSI_ASSERT (matrix1->cols == matrix2->rows && matrix->rows == matrix1->rows && matrix->cols == matrix2->cols);

        MatrixOps<T>::Empty (matrix);

        T *src1ptr = matrix1->data;
        ssi_size_t src1rows = matrix1->rows;
        T *src2ptr = matrix2->data;
        T *src2ptr2;
        ssi_size_t src2rows = matrix2->rows;
        ssi_size_t src2cols = matrix2->cols;
        T *dstptr = matrix->data;
        T *dstptr2;
        for (ssi_size_t i = 0; i < src1rows; i++) {
                src2ptr2 = src2ptr;
                for (ssi_size_t j = 0; j < src2rows; j++) {
                        dstptr2 = dstptr;
                        for (ssi_size_t k = 0; k < src2cols; k++) {
                                *dstptr2 +=  (*src2ptr2) * (*src1ptr);
                                src2ptr2++;
                                dstptr2++;
                        }
                        src1ptr++;
                }
                dstptr = dstptr2;
        }
}

template <class T>
void MatrixOps<T>::Div (const Matrix<T> *const matrix, T scalar) {

        if (MatrixOps<T>::IsEmpty (matrix)) {
                return;
        }

        T *dataptr = matrix->data;
        ssi_size_t elems  = matrix->rows * matrix->cols;

        for (ssi_size_t i = 0; i < elems; i++) {
                *dataptr /= scalar;
                dataptr++;
        }
}

template <class T>
void MatrixOps<T>::Div (const Matrix<T> *const matrix1, const Matrix<T> *const matrix2) {

        // SSI_ASSERT that matrices are of same size
        SSI_ASSERT ((matrix1->rows == matrix2->rows) && (matrix1->cols == matrix2->cols));

        if (MatrixOps<T>::IsEmpty (matrix1)) {
                return;
        }

        T *dataptr1 = matrix1->data;
        T *dataptr2 = matrix2->data;
        ssi_size_t elems  = matrix1->rows * matrix1->cols;

        for (ssi_size_t i = 0; i < elems; i++) {
                *dataptr1 /= *dataptr2;
                dataptr1++;
                dataptr2++;
        }
}

template <class T>
void MatrixOps<T>::Pow (const Matrix<T> *const matrix, T value) {

        if (MatrixOps<T>::IsEmpty (matrix)) {
                return;
        }

        T *dataptr = matrix->data;
        ssi_size_t elems  = matrix->rows * matrix->cols;

        for (ssi_size_t i = 0; i < elems; i++) {
                *dataptr = pow (*dataptr, value);
                dataptr++;
        }
}

template <class T>
void MatrixOps<T>::Sqrt (const Matrix<T> *const matrix) {

        if (MatrixOps<T>::IsEmpty (matrix)) {
                return;
        }

        T *dataptr = matrix->data;
        ssi_size_t elems  = matrix->rows * matrix->cols;

        for (ssi_size_t i = 0; i < elems; i++) {
                *dataptr = sqrt (*dataptr);
                dataptr++;
        }
}

template <class T>
void MatrixOps<T>::Exp (const Matrix<T> *const matrix) {

        if (MatrixOps<T>::IsEmpty (matrix)) {
                return;
        }

        T *dataptr = matrix->data;
        ssi_size_t elems  = matrix->rows * matrix->cols;

        for (ssi_size_t i = 0; i < elems; i++) {
                *dataptr = exp (*dataptr);
                dataptr++;
        }
}

template <class T>
void MatrixOps<T>::Sin (const Matrix<T> *const matrix) {

        if (MatrixOps<T>::IsEmpty (matrix)) {
                return;
        }

        T *dataptr = matrix->data;
        ssi_size_t elems  = matrix->rows * matrix->cols;

        for (ssi_size_t i = 0; i < elems; i++) {
                *dataptr = sin (*dataptr);
                dataptr++;
        }
}

template <class T>
void MatrixOps<T>::Cos (const Matrix<T> *const matrix) {

        if (MatrixOps<T>::IsEmpty (matrix)) {
                return;
        }

        T *dataptr = matrix->data;
        ssi_size_t elems  = matrix->rows * matrix->cols;

        for (ssi_size_t i = 0; i < elems; i++) {
                *dataptr = cos (*dataptr);
                dataptr++;
        }
}

template <class T>
void MatrixOps<T>::Log (const Matrix<T> *const matrix) {

        if (MatrixOps<T>::IsEmpty (matrix)) {
                return;
        }

        T *dataptr = matrix->data;
        ssi_size_t elems  = matrix->rows * matrix->cols;

        for (ssi_size_t i = 0; i < elems; i++) {
                *dataptr = log (*dataptr);
                dataptr++;
        }
}

template <class T>
void MatrixOps<T>::Log10 (const Matrix<T> *const matrix) {

        if (MatrixOps<T>::IsEmpty (matrix)) {
                return;
        }

        T *dataptr = matrix->data;
        ssi_size_t elems  = matrix->rows * matrix->cols;

        for (ssi_size_t i = 0; i < elems; i++) {
                *dataptr = *dataptr <= 0 ? 0 : log10 (*dataptr);
                dataptr++;
        }
}

template <class T>
void MatrixOps<T>::Abs (const Matrix<T> *const matrix) {

        if (MatrixOps<T>::IsEmpty (matrix)) {
                return;
        }

        T *dataptr = matrix->data;
        ssi_size_t elems  = matrix->rows * matrix->cols;

        for (ssi_size_t i = 0; i < elems; i++) {
                *dataptr = fabs (*dataptr);
                dataptr++;
        }

}

template <class T>
Matrix<T> *MatrixOps<T>::Abs (const Matrix< std::complex<T> > *const matrix) {

        Matrix<T> *resmat = new Matrix<T> (matrix->rows, matrix->cols);

        Abs (matrix, resmat);

        return resmat;
}

template <class T>
void MatrixOps<T>::Abs (const Matrix< std::complex<T> > *const matrix1, const Matrix<T> *const matrix2) {

        // SSI_ASSERT that matrices are of same size
        SSI_ASSERT ((matrix1->rows == matrix2->rows) && (matrix1->cols == matrix2->cols));

        if (MatrixOps<T>::IsEmpty (matrix2)) {
                return;
        }

        std::complex<T> *srcptr = matrix1->data;
        T *dstptr = matrix2->data;
        ssi_size_t elems  = matrix1->rows * matrix1->cols;

        T real, imag;
        for (ssi_size_t i = 0; i < elems; i++) {
                real = (*srcptr).real ();
                imag = (*srcptr).imag ();
                srcptr++;
                *dstptr = sqrt (real * real + imag * imag);
                dstptr++;
        }
}



template <class T>
void MatrixOps<T>::Empty (const Matrix<T> *const matrix) {

        Fill (matrix, 0);
}

template <class T>
void MatrixOps<T>::Fill (const Matrix<T> *const matrix, T value) {

        if (MatrixOps<T>::IsEmpty (matrix)) {
                return;
        }

        ssi_size_t elems = matrix->rows * matrix->cols;

        T *dataptr = matrix->data;
        for (ssi_size_t i = 0; i < elems; i++) {
                *dataptr = value;
                dataptr++;
        }
}

template <class T>
void MatrixOps<T>::Sort (const Matrix<T> *const matrix, MATRIX_DIMENSION dimension) {

        if (dimension == MATRIX_DIMENSION_COL) {
                MatrixOps<T>::Transpose (const_cast<Matrix<T> *const> (matrix));
        }

        T *dataptr = matrix->data;
        ssi_size_t rows = matrix->rows;
        ssi_size_t cols = matrix->cols;

        for (ssi_size_t i = 0; i < rows; i++) {
                qsort (dataptr, cols, sizeof (T), compare);
                dataptr += cols;
        }

        if (dimension == MATRIX_DIMENSION_COL) {
                MatrixOps<T>::Transpose (const_cast<Matrix<T> *const> (matrix));
        }
};

template <class T>
int MatrixOps<T>::compare (const void *a, const void *b) {

        if (*ssi_pcast (const T, a) < *ssi_pcast (const T, b))
                return -1;
        else
                return 1;
}

/*template <class T>
ssi_size_t MatrixOps<T>::compare_complex (const void *a, const void *b) {

        return (*ssi_cast (T *> (a) - *static_cast<T *, b));
}
*/

}



#endif // _MATRIXOPS_H
