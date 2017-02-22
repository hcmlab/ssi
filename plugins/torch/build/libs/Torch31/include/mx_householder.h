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

#ifndef MX_HOUSEHOLDER_INC
#define MX_HOUSEHOLDER_INC

#include "Mat.h"
#include "Vec.h"

namespace Torch {

/**
   Householder transformation routines.
   Contains routines for calculating householder transformations,
   applying them to vectors and matrices by both row and column.

   Based on the "Meschach Library", available at the
   anonymous ftp site thrain.anu.edu.au in the directory
   pub/meschach.
   
   @author David E. Stewart (david.stewart@anu.edu.au)
   @author Zbigniew Leyk (zbigniew.leyk@anu.edu.au)
   @author Ronan Collobert (collober@idiap.ch)
*/
//@{

/** Calulates Householder vector.
    To eliminate all entries after the i0 entry of the vector vec.
    It is returned as out. May be in-situ.
*/
void mxHhVec(Vec * vec, int i0, real * beta, Vec * out, real * newval);


/** Apply Householder transformation to vector. 
    May be in-situ. (#hh# is the Householder vector).
*/
void mxHhTrVec(Vec * hh, real beta, int i0, Vec * in, Vec * out);

/** Transform a matrix by a Householder vector by rows.
    Start at row i0 from column j0. In-situ.
*/
void mxHhTrRows(Mat * mat, int i0, int j0, Vec * hh, real beta);

/* Transform a matrix by a Householder vector by columns.
   Start at row i0 from column j0. In-situ.
*/
void mxHhTrCols(Mat * mat, int i0, int j0, Vec * hh, real beta);

//@}


}

#endif
