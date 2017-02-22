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

#ifndef MX_HESSENBERG_INC
#define MX_HESSENBERG_INC

#include "Mat.h"
#include "Vec.h"

namespace Torch {

/**
   Routines for determining Hessenberg factorisations.
   
   Based on the "Meschach Library", available at the
   anonymous ftp site thrain.anu.edu.au in the directory
   pub/meschach.
   
   @author David E. Stewart (david.stewart@anu.edu.au)
   @author Zbigniew Leyk (zbigniew.leyk@anu.edu.au)
   @author Ronan Collobert (collober@idiap.ch)
*/
//@{

/** Compute Hessenberg factorisation in compact form.
   Factorisation performed in situ.
*/
void mxHFactor(Mat * mat, Vec * diag, Vec * beta);


/** Construct the Hessenberg orthogonalising matrix Q.
    i.e. Hess M = Q.M.Q'.
*/
void mxMakeHQ(Mat * h_mat, Vec * diag, Vec * beta, Mat * q_out);

/** Construct actual Hessenberg matrix.
 */
void mxMakeH(Mat * h_mat, Mat * h_out);

//@}


}

#endif
