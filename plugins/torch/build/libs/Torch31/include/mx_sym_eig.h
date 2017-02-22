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

#ifndef MX_SYM_EIG_INC
#define MX_SYM_EIG_INC

#include "Vec.h"
#include "Mat.h"
#include "mx_givens.h"
#include "mx_hessenberg.h"

namespace Torch {

/**
	Routines for symmetric eigenvalue problems.

  Based on the "Meschach Library", available at the
  anonymous ftp site thrain.anu.edu.au in the directory
  pub/meschach.
  
  @author David E. Stewart (david.stewart@anu.edu.au)
  @author Zbigniew Leyk (zbigniew.leyk@anu.edu.au)
  @author Ronan Collobert (collober@idiap.ch)
*/
//@{

/** Finds eigenvalues of symmetric tridiagonal matrices.
    The matrix is represented by a pair of vectors #a# (diag entries)
		and #b# (sub-diag and super-diag entries).
    Eigenvalues in #a# on return, and eigenvectors in #mat_q#, if this one
    is not #NULL#.
*/
void mxTriEig(Vec * a, Vec * b, Mat * mat_q);

/** Computes eigenvalues of a dense symmetric matrix.
	  #mat_a# \emph{must} be symmetric on entry.
    Eigenvalues stored in #out#.
    #mat_q# contains orthogonal matrix of eigenvectors if not #NULL#.
*/
void mxSymEig(Mat * mat_a, Mat * mat_q, Vec * out);

//@}


}

#endif
