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

#ifndef MX_SOLVE_INC
#define MX_SOLVE_INC

#include "Mat.h"
#include "Vec.h"

namespace Torch {

/*  Collection of factorisations functions.
    Based on the "Meschach Library", available at the
    anonymous ftp site thrain.anu.edu.au in the directory
    pub/meschach.

    @author David E. Stewart (david.stewart@anu.edu.au)
    @author Zbigniew Leyk (zbigniew.leyk@anu.edu.au)
    @author Ronan Collobert (collober@idiap.ch)
*/

/* Back substitution with optional over-riding diagonal.
    Can be in-situ but doesn't need to be. */
void mxUSolve(Mat * matrix, Vec * b, Vec * out, real diag);

// Forward elimination with (optional) default diagonal value.
void mxLSolve(Mat * matrix, Vec * b, Vec * out, real diag);

/* Forward elimination with (optional) default diagonal value.
    Use UPPER triangular part of matrix. */
void mxUTSolve(Mat * mat, Vec * b, Vec * out, real diag);

/* Solves Dx=b where D is the diagonal of A.
    May be in-situ. */
void mxDSolve(Mat * mat, Vec * b, Vec * x);

/* Back substitution with optional over-riding diagonal.
   Use the LOWER triangular part of matrix.
   Can be in-situ but doesn't need to be. */
void mxLTSolve(Mat * mat, Vec * b, Vec * out, real diag);


}

#endif
