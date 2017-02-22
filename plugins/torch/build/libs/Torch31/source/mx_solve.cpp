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

#include "mx_solve.h"
#include "mx_low_level.h"

namespace Torch {

/* Most matrix factorisation routines are in-situ unless otherwise specified */

/* Usolve -- back substitution with optional over-riding diagonal
		-- can be in-situ but doesn't need to be */
void mxUSolve(Mat * matrix, Vec * b, Vec * out, real diag)
{
  int dim;
  int i, i_lim;
  real **mat_ptr, *mat_row, *b_ptr, *out_ptr, *out_col, sum, tiny;

  dim = min(matrix->m, matrix->n);
  mat_ptr = matrix->ptr;
  b_ptr = b->ptr;
  out_ptr = out->ptr;

  tiny = 10.0 / INF;

  for (i = dim - 1; i >= 0; i--)
    if (b_ptr[i] != 0.0)
      break;
    else
      out_ptr[i] = 0.0;
  i_lim = i;

  for (; i >= 0; i--)
  {
    sum = b_ptr[i];
    mat_row = &mat_ptr[i][i + 1];
    out_col = &out_ptr[i + 1];
    sum -= mxIp__(mat_row, out_col, i_lim - i);
    if (diag == 0.0)
    {
      if (fabs(mat_ptr[i][i]) <= tiny * fabs(sum))
	error("USolve: sorry, singular problem.");
      else
	out_ptr[i] = sum / mat_ptr[i][i];
    }
    else
      out_ptr[i] = sum / diag;
  }
}

/* Lsolve -- forward elimination with (optional) default diagonal value */
void mxLSolve(Mat * matrix, Vec * b, Vec * out, real diag)
{
  int dim, i, i_lim;
  real **mat_ptr, *mat_row, *b_ptr, *out_ptr, *out_col, sum, tiny;

  dim = min(matrix->m, matrix->n);
  mat_ptr = matrix->ptr;
  b_ptr = b->ptr;
  out_ptr = out->ptr;

  for (i = 0; i < dim; i++)
    if (b_ptr[i] != 0.0)
      break;
    else
      out_ptr[i] = 0.0;
  i_lim = i;

  tiny = 10.0 / INF;

  for (; i < dim; i++)
  {
    sum = b_ptr[i];
    mat_row = &mat_ptr[i][i_lim];
    out_col = &out_ptr[i_lim];
    sum -= mxIp__(mat_row, out_col, i - i_lim);
    if (diag == 0.0)
    {
      if (fabs(mat_ptr[i][i]) <= tiny * fabs(sum))
	error("LSolve: sorry, singular problem.");
      else
	out_ptr[i] = sum / mat_ptr[i][i];
    }
    else
      out_ptr[i] = sum / diag;
  }
}


/* UTsolve -- forward elimination with (optional) default diagonal value
		using UPPER triangular part of matrix */
void mxUTSolve(Mat * mat, Vec * b, Vec * out, real diag)
{
  int dim, i, i_lim;
  real **mat_ptr, *b_ptr, *out_ptr, tmp, invdiag, tiny;

  dim = min(mat->m, mat->n);
  mat_ptr = mat->ptr;
  b_ptr = b->ptr;
  out_ptr = out->ptr;

  tiny = 10.0 / INF;

  for (i = 0; i < dim; i++)
  {
    if (b_ptr[i] != 0.0)
      break;
    else
      out_ptr[i] = 0.0;
  }
  i_lim = i;
  if (b != out)
  {
    mxZero__(out_ptr, out->n);
    real *ptr_r = &b_ptr[i_lim];
    real *ptr_w = &out_ptr[i_lim];
    for (int j = 0; j < dim - i_lim; j++)
      *ptr_w++ = *ptr_r++;
  }

  if (diag == 0.0)
  {
    for (; i < dim; i++)
    {
      tmp = mat_ptr[i][i];
      if (fabs(tmp) <= tiny * fabs(out_ptr[i]))
	error("UTSolve: sorry, singular problem.");
      out_ptr[i] /= tmp;
      mxRealMulAdd__(&out_ptr[i + 1], &mat_ptr[i][i + 1], -out_ptr[i],
		     dim - i - 1);
    }
  }
  else
  {
    invdiag = 1.0 / diag;
    for (; i < dim; i++)
    {
      out_ptr[i] *= invdiag;
      mxRealMulAdd__(&out_ptr[i + 1], &mat_ptr[i][i + 1], -out_ptr[i],
		     dim - i - 1);
    }
  }
}

/* Dsolve -- solves Dx=b where D is the diagonal of A -- may be in-situ */
void mxDSolve(Mat * mat, Vec * b, Vec * x)
{
  int dim, i;
  real tiny;

  dim = min(mat->m, mat->n);

  tiny = 10.0 / INF;

  dim = b->n;
  for (i = 0; i < dim; i++)
  {
    if (fabs(mat->ptr[i][i]) <= tiny * fabs(b->ptr[i]))
      error("DSolve: sorry, singular problem.");
    else
      x->ptr[i] = b->ptr[i] / mat->ptr[i][i];
  }
}

/* LTsolve -- back substitution with optional over-riding diagonal
		using the LOWER triangular part of matrix
		-- can be in-situ but doesn't need to be */
void mxLTSolve(Mat * mat, Vec * b, Vec * out, real diag)
{
  int dim;
  int i, i_lim;
  real **mat_ptr, *b_ptr, *out_ptr, tmp, invdiag, tiny;

  dim = min(mat->m, mat->n);
  mat_ptr = mat->ptr;
  b_ptr = b->ptr;
  out_ptr = out->ptr;

  tiny = 10.0 / INF;

  for (i = dim - 1; i >= 0; i--)
  {
    if (b_ptr[i] != 0.0)
      break;
  }
  i_lim = i;

  if (b != out)
  {
    mxZero__(out_ptr, out->n);
    real *ptr_r = b_ptr;
    real *ptr_w = out_ptr;
    for (int j = 0; j < i_lim + 1; j++)
      *ptr_w++ = *ptr_r++;
  }

  if (diag == 0.0)
  {
    for (; i >= 0; i--)
    {
      tmp = mat_ptr[i][i];
      if (fabs(tmp) <= tiny * fabs(out_ptr[i]))
	error("LTSolve: sorry, singular problem.");
      out_ptr[i] /= tmp;
      mxRealMulAdd__(out_ptr, mat_ptr[i], -out_ptr[i], i);
    }
  }
  else
  {
    invdiag = 1.0 / diag;
    for (; i >= 0; i--)
    {
      out_ptr[i] *= invdiag;
      mxRealMulAdd__(out_ptr, mat_ptr[i], -out_ptr[i], i);
    }
  }
}

}

