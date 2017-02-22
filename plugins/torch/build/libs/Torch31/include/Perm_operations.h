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

#ifndef PERM_OPERATIONS_INC
#define PERM_OPERATIONS_INC

#include "Perm.h"
#include "Mat.h"
#include "Vec.h"

namespace Torch {

/** Collection of permutations operation functions.
    Based on the "Meschach Library", available at the
    anonymous ftp site thrain.anu.edu.au in the directory
    pub/meschach.

    @author David E. Stewart (david.stewart@anu.edu.au)
    @author Zbigniew Leyk (zbigniew.leyk@anu.edu.au)
    @author Ronan Collobert (collober@idiap.ch)
*/
//@{
/** Invert permutation -- in situ.
    Taken from ACM Collected Algorithms 250. */
void mxPermInv(Perm * px, Perm * out);

/** Permutation multiplication (composition) -- not in-situ */
void mxPermMulPerm(Perm * px1, Perm * px2, Perm * out);

/** Permute vector -- can be in-situ */
void mxPermVec(Perm * px, Vec * vec, Vec * out);

/** Apply the inverse of px to x, returning the result in out.
    Can be in-situ, but "oh booooy!"... */
void mxPermInvVec(Perm * px, Vec * x, Vec * out);

/** Transpose elements of permutation.
    Really multiplying a permutation by a transposition.
    i1 and i2 are the elements to transpose. */
void mxTrPerm(Perm * px, int i1, int i2);

/** Permute columns of matrix A; out = A.px'. -- May NOT be in situ */
void mxPermColsMat(Perm * px, Mat * mat, Mat * out);

/** Permute rows of matrix A; out = px.A. -- May NOT be in situ */
void mxPermRowsMat(Perm * px, Mat * mat, Mat * out);

//@}


}

#endif
