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

#include "mx_householder.h"
#include "mx_low_level.h"

namespace Torch {

/*
  Files for matrix computations
  
  Householder transformation file. Contains routines for calculating
  householder transformations, applying them to vectors and matrices
  by both row & column.
*/


/* hhvec -- calulates Householder vector to eliminate all entries after the
	i0 entry of the vector vec. It is returned as out. May be in-situ */
void mxHhVec(Vec * vec, int i0, real * beta, Vec * out, real * newval)
{
  out->copy(out, i0);
  real norm = sqrt(out->iP(out, i0));
  if (norm <= 0.0)
  {
    *beta = 0.0;
    return;
  }
  *beta = 1.0 / (norm * (norm + fabs(out->ptr[i0])));
  if (out->ptr[i0] > 0.0)
    *newval = -norm;
  else
    *newval = norm;
  out->ptr[i0] -= *newval;
}

/* hhtrvec -- apply Householder transformation to vector -- may be in-situ */
/* hh = Householder vector */
void mxHhTrVec(Vec * hh, real beta, int i0, Vec * in, Vec * out)
{
  real scale = beta * hh->iP(in, i0);
  out->copy(in);
  mxRealMulAdd__(&out->ptr[i0], &hh->ptr[i0], -scale, in->n - i0);
}

/* hhtrrows -- transform a matrix by a Householder vector by rows
	starting at row i0 from column j0 -- in-situ */
void mxHhTrRows(Mat * mat, int i0, int j0, Vec * hh, real beta)
{
  real ip, scale;

  if (beta == 0.0)
    return;

  /* for each row ... */
  for (int i = i0; i < mat->m; i++)
  {				/* compute inner product */
    ip = mxIp__(&mat->ptr[i][j0], &hh->ptr[j0], mat->n - j0);

    scale = beta * ip;
    if (scale == 0.0)
      continue;

    /* do operation */
    mxRealMulAdd__(&mat->ptr[i][j0], &hh->ptr[j0], -scale, mat->n - j0);
  }
}


/* hhtrcols -- transform a matrix by a Householder vector by columns
	starting at row i0 from column j0 -- in-situ */
void mxHhTrCols(Mat * mat, int i0, int j0, Vec * hh, real beta)
{
  if (beta == 0.0)
    return;

  Vec *w = new Vec(mat->n);
  w->zero();

  for (int i = i0; i < mat->m; i++)
  {
    if (hh->ptr[i] != 0.0)
      mxRealMulAdd__(&w->ptr[j0], &mat->ptr[i][j0], hh->ptr[i],
		     mat->n - j0);
  }
  for (int i = i0; i < mat->m; i++)
  {
    if (hh->ptr[i] != 0.0)
      mxRealMulAdd__(&mat->ptr[i][j0], &w->ptr[j0], -beta * hh->ptr[i],
		     mat->n - j0);
  }

  delete w;
}

}

