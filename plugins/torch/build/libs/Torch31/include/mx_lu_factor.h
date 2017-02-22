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

#ifndef MX_LU_FACTOR_INC
#define MX_LU_FACTOR_INC

#include "Mat.h"
#include "Vec.h"
#include "Perm.h"

namespace Torch {

/** Collection of matrix factorisation operation functions.
    Based on the "Meschach Library", available at the
    anonymous ftp site thrain.anu.edu.au in the directory
    pub/meschach.

    Most matrix factorisation routines are in-situ
    unless otherwise specified.

    @author David E. Stewart (david.stewart@anu.edu.au)
    @author Zbigniew Leyk (zbigniew.leyk@anu.edu.au)
    @author Ronan Collobert (collober@idiap.ch)
*/
//@{

/** Gaussian elimination with scaled partial pivoting.
    -- Note: returns LU matrix which is #A#. */
void mxLUFactor(Mat * mat, Perm * pivot);

/**
   Given an LU factorisation in #A#, solve #Ax=b# */
void mxLUSolve(Mat * mat, Perm * pivot, Vec * b, Vec * x);

/**
   Given an LU factorisation in #A#, solve #A^T.x=b# */
void mxLUTSolve(Mat * mat, Perm * pivot, Vec * b, Vec * x);

/**
   Returns inverse of #A#, provided #A# is not too rank deficient.
   Uses LU factorisation. */
void mxInverse(Mat * mat, Mat * out);

/**
   Given #A# and #b#, solve #A.x=b# */
void mxSolve(Mat *mat, Vec *b, Vec *x);

//@}


}

#endif
