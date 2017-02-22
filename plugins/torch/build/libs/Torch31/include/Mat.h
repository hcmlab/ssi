// Copyright (C) 2003--2004 Ronan Collobert (collober@idiap.ch)
//                
// This file is part of Torch 3.1.
//
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. The name of the author may not be used to endorse or promote products
//    derived from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef MAT_INC
#define MAT_INC

#include "Object.h"
#include "Vec.h"

namespace Torch {

/** Matrix object.

    @author Ronan Collobert (collober@idiap.ch)
*/
class Mat : public Object
{
  public:    
    /// Size of the matrix
    int m, n;
    
    /// Data of the matrix
    real **ptr;
    
    /** NULL if not allocated by Mat.
        (when you're using the first constructor of Mat, or for
        the matrix returned by subMat)
    */
    real *base;
    
    /** Create a matrix from values in #ptr_#.
        (No memory copy).
    */
    Mat(real ** ptr_, int n_rows, int n_cols);

    /** Create a matrix from values in #ptr_#.
        (No memory copy).
    */
    Mat(real * ptr_, int n_rows, int n_cols);
    
    /// Create a new matrix
    Mat(int n_rows, int n_cols);
    
    /// Copy the matrix #mat#
    void copy(Mat * mat);
    
    /// Zero the matrix
    void zero();
    
    /// Compute the norm1
    real norm1();
    
    /// Compute the Frobenius norm
    real normFrobenius();
    
    /// Compute the norm inf
    real normInf();
    
    /** Return the row #row# of the matrix.
        If #vec# is NULL, return a new vector.
        Else copy the row in #vec#.
    */
    Vec *getRow(int row, Vec * vec = NULL);
    
    /** Return the column #col# of the matrix.
        If #vec# is NULL, return a new vector.
        Else copy the column in #vec#.
    */
    Vec *getCol(int col, Vec * vec = NULL);
    
    /// Set the row #row# to values in #vec#
    void setRow(int row, Vec * vec);
    
    /// Set the column #col# to values in #vec#
    void setCol(int row, Vec * vec);
    
    /** Return a sub-matrix.
        Note that the memory is shared with the original
        matrix, so *be carefull*.
        You have to destroy the returned matrix.
    */
    Mat *subMat(int row1, int col1, int row2, int col2);

   ~Mat();
};

}

#endif
