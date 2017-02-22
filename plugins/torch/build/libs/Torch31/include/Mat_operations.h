// Copyright (C) 2003--2004 Zbigniew Leyk (zbigniew.leyk@anu.edu.au)
//                and David E. Stewart (david.stewart@anu.edu.au)
//                and Ronan Collobert (collober@idiap.ch)
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

#ifndef MAT_OPERATIONS_INC
#define MAT_OPERATIONS_INC

#include "Mat.h"
#include "Vec.h"

namespace Torch {

/** Collection of matrix operation functions.
    Based on the "Meschach Library", available at the
    anonymous ftp site thrain.anu.edu.au in the directory
    pub/meschach.

    @author David E. Stewart (david.stewart@anu.edu.au)
    @author Zbigniew Leyk (zbigniew.leyk@anu.edu.au)
    @author Ronan Collobert (collober@idiap.ch)
*/
//@{
/// Matrix addition -- may be in-situ
void mxMatAddMat(Mat * mat1, Mat * mat2, Mat * out);
/// Matrix subtraction -- may be in-situ
void mxMatSubMat(Mat * mat1, Mat * mat2, Mat * out);
/// Matrix-matrix multiplication
void mxMatMulMat(Mat * mat1, Mat * mat2, Mat * out);
/** Matrix-matrix transposed multiplication.
    -- #A.B^T# is stored in out */
void mxMatMulTrMat(Mat * mat1, Mat * mat2, Mat * out);
/** Matrix transposed-matrix multiplication.
    -- #A^T.B# is stored in out */
void mxTrMatMulMat(Mat * mat1, Mat * mat2, Mat * out);
/** Matrix-vector multiplication.
    -- Note: #b# is treated as a column vector */
void mxMatMulVec(Mat * mat, Vec * b, Vec * out);
/// Scalar-matrix multiply -- may be in-situ
void mxRealMulMat(real scalar, Mat * matrix, Mat * out);
/** Vector-matrix multiplication. 
    -- Note: #b# is treated as a row vector */
void mxVecMulMat(Vec * b, Mat * mat, Vec * out);
/// Transpose matrix
void mxTrMat(Mat * in, Mat * out);
/** Swaps rows i and j of matrix A upto column lim.
    #lo# and #hi# to -1 if you want to swap all */
void mxSwapRowsMat(Mat * mat, int i, int j, int lo, int hi);
/** Swap columns i and j of matrix A upto row lim.
    #lo# and #hi# to -1 if you want to swap all */
void mxSwapColsMat(Mat * mat, int i, int j, int lo, int hi);
/** Matrix-scalar multiply and add.
    -- may be in situ.
    -- returns out == A1 + s*A2 */
void mxMatAddRealMulMat(Mat * mat1, Mat * mat2, real s, Mat * out);
/** Matrix-vector multiply and add.
    -- may not be in situ
    -- returns out == v1 + alpha*A*v2 */
void mxVecAddRealMulMatMulVec(Vec * v1, real alpha, Mat * mat, Vec * v2,
			      Vec * out);
/** Vector-matrix multiply and add
    -- may not be in situ
    -- returns out' == v1' + alpha * v2'*A */
void mxVecAddRealMulVecMulMat(Vec * v1, real alpha, Vec * v2, Mat * mat,
			      Vec * out);
//@}


}

#endif
