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

#include "mx_givens.h"

namespace Torch {

/*
		Files for matrix computations

	Givens operations file. Contains routines for calculating and
	applying givens rotations for/to vectors and also to matrices by
	row and by column.
*/

/* givens -- returns c,s parameters for Givens rotation to
		eliminate y in the vector [ x y ]' */
void mx_givens(real x, real y, real * c, real * s)
{
  real norm = sqrt(x * x + y * y);
  if (norm == 0.0)
  {
    *c = 1.0;
    *s = 0.0;
  }				/* identity */
  else
  {
    *c = x / norm;
    *s = y / norm;
  }
}

/* rot_vec -- apply Givens rotation to x's i & k components */
void mx_rot_vec(Vec * x, int i, int k, real c, real s, Vec * out)
{
  out->copy(x);

  real temp = c * out->ptr[i] + s * out->ptr[k];
  out->ptr[k] = -s * out->ptr[i] + c * out->ptr[k];
  out->ptr[i] = temp;
}

/* rot_rows -- premultiply mat by givens rotation described by c,s */
void mx_rot_rows(Mat * mat, int i, int k, real c, real s, Mat * out)
{
  out->copy(mat);

  for (int j = 0; j < mat->n; j++)
  {
    real temp = c * out->ptr[i][j] + s * out->ptr[k][j];
    out->ptr[k][j] = -s * out->ptr[i][j] + c * out->ptr[k][j];
    out->ptr[i][j] = temp;
  }
}

/* rot_cols -- postmultiply mat by givens rotation described by c,s */
void mx_rot_cols(Mat * mat, int i, int k, real c, real s, Mat * out)
{
  out->copy(mat);

  for (int j = 0; j < mat->m; j++)
  {
    real temp = c * out->ptr[j][i] + s * out->ptr[j][k];
    out->ptr[j][k] = -s * out->ptr[j][i] + c * out->ptr[j][k];
    out->ptr[j][i] = temp;
  }
}

}

