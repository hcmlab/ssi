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

#include "Mat_operations.h"
#include "mx_low_level.h"

namespace Torch {

/* m_add -- matrix addition -- may be in-situ */
void mxMatAddMat(Mat * mat1, Mat * mat2, Mat * out)
{
  int m, n;

  m = mat1->m;
  n = mat1->n;
  for (int i = 0; i < m; i++)
    mxAdd__(mat1->ptr[i], mat2->ptr[i], out->ptr[i], n);
}

/* m_sub -- matrix subtraction -- may be in-situ */
void mxMatSubMat(Mat * mat1, Mat * mat2, Mat * out)
{
  int m, n;

  m = mat1->m;
  n = mat1->n;
  for (int i = 0; i < m; i++)
    mxSub__(mat1->ptr[i], mat2->ptr[i], out->ptr[i], n);
}

/* m_mlt -- matrix-matrix multiplication */
void mxMatMulMat(Mat * mat1, Mat * mat2, Mat * out)
{
  int m, n, p;
  real **mat1_v, **mat2_v;

  m = mat1->m;
  n = mat1->n;
  p = mat2->n;
  mat1_v = mat1->ptr;
  mat2_v = mat2->ptr;

  out->zero();
  for (int i = 0; i < m; i++)
  {
    for (int k = 0; k < n; k++)
    {
      if (mat1_v[i][k] != 0.0)
	mxRealMulAdd__(out->ptr[i], mat2_v[k], mat1_v[i][k], p);
    }
  }
}

/* mmtr_mlt -- matrix-matrix transposed multiplication
	-- A.B^T is returned, and stored in OUT */
void mxMatMulTrMat(Mat * mat1, Mat * mat2, Mat * out)
{
  int limit = mat1->n;
  for (int i = 0; i < mat1->m; i++)
  {
    for (int j = 0; j < mat2->m; j++)
      out->ptr[i][j] = mxIp__(mat1->ptr[i], mat2->ptr[j], limit);
  }
}

/* mtrm_mlt -- matrix transposed-matrix multiplication
	-- A^T.B is returned, result stored in OUT */
void mxTrMatMulMat(Mat * mat1, Mat * mat2, Mat * out)
{
  int limit = mat2->n;
  out->zero();
  for (int k = 0; k < mat1->m; k++)
    for (int i = 0; i < mat1->n; i++)
    {
      if (mat1->ptr[k][i] != 0.0)
	mxRealMulAdd__(out->ptr[i], mat2->ptr[k], mat1->ptr[k][i], limit);
    }
}

/* mv_mlt -- matrix-vector multiplication 
		-- Note: b is treated as a column vector */
void mxMatMulVec(Mat * mat, Vec * b, Vec * out)
{
  int m, n;
  real **mat_v, *b_v;

  m = mat->m;
  n = mat->n;
  mat_v = mat->ptr;
  b_v = b->ptr;
  for (int i = 0; i < m; i++)
    out->ptr[i] = mxIp__(mat_v[i], b_v, n);
}

/* sm_mlt -- scalar-matrix multiply -- may be in-situ */
void mxRealMulMat(real scalar, Mat * matrix, Mat * out)
{
  int m, n;

  m = matrix->m;
  n = matrix->n;
  for (int i = 0; i < m; i++)
    mxRealMul__(matrix->ptr[i], scalar, out->ptr[i], n);
}

/* vm_mlt -- vector-matrix multiplication 
		-- Note: b is treated as a row vector */
void mxVecMulMat(Vec * b, Mat * mat, Vec * out)
{
  int m, n;

  m = mat->m;
  n = mat->n;

  out->zero();
  for (int j = 0; j < m; j++)
    if (b->ptr[j] != 0.0)
      mxRealMulAdd__(out->ptr, mat->ptr[j], b->ptr[j], n);
}

/* m_transp -- transpose matrix */
void mxTrMat(Mat * in, Mat * out)
{
  if (in != out)
  {
    for (int i = 0; i < in->m; i++)
    {
      for (int j = 0; j < in->n; j++)
	out->ptr[j][i] = in->ptr[i][j];
    }
  }
  else
  {
    for (int i = 1; i < in->m; i++)
    {
      for (int j = 0; j < i; j++)
      {
	real tmp = in->ptr[i][j];
	in->ptr[i][j] = in->ptr[j][i];
	in->ptr[j][i] = tmp;
      }
    }
  }
}

/* swap_rows -- swaps rows i and j of matrix A upto column lim */
// lo and hi -1 if you want to swap all
void mxSwapRowsMat(Mat * mat, int i, int j, int lo, int hi)
{
  if (lo < 0)
    lo = 0;

  if (hi < 0)
    hi = mat->n - 1;

  real **mat_ptr = mat->ptr;
  for (int k = lo; k <= hi; k++)
  {
    real tmp = mat_ptr[k][i];
    mat_ptr[k][i] = mat_ptr[k][j];
    mat_ptr[k][j] = tmp;
  }
}

/* swap_cols -- swap columns i and j of matrix A upto row lim */
// lo and hi -1 if you want to swap all
void mxSwapColsMat(Mat * mat, int i, int j, int lo, int hi)
{
  if (lo < 0)
    lo = 0;

  if (hi < 0)
    hi = mat->m - 1;

  real **mat_ptr = mat->ptr;
  for (int k = lo; k <= hi; k++)
  {
    real tmp = mat_ptr[i][k];
    mat_ptr[i][k] = mat_ptr[j][k];
    mat_ptr[j][k] = tmp;
  }
}

/* ms_mltadd -- matrix-scalar multiply and add
	-- may be in situ
	-- returns out == A1 + s*A2 */
void mxMatAddRealMulMat(Mat * mat1, Mat * mat2, real s, Mat * out)
{
  int m, n;

  if (s == 0.0)
  {
    out->copy(mat1);
    return;
  }

  if (s == 1.0)
  {
    mxMatAddMat(mat1, mat2, out);
    return;
  }

  out->copy(mat1);
  m = mat1->m;
  n = mat1->n;
  for (int i = 0; i < m; i++)
    mxRealMulAdd__(out->ptr[i], mat2->ptr[i], s, n);
}

/* mv_mltadd -- matrix-vector multiply and add
	-- may not be in situ
	-- returns out == v1 + alpha*A*v2 */
void
mxVecAddRealMulMatMulVec(Vec * v1, real alpha, Mat * mat, Vec * v2,
			 Vec * out)
{
  int m, n;
  real *v2_ptr, *out_ptr;

  out->copy(v1);
  if (alpha == 0.0)
    return;

  v2_ptr = v2->ptr;
  out_ptr = out->ptr;
  m = mat->m;
  n = mat->n;
  for (int i = 0; i < m; i++)
    out_ptr[i] += alpha * mxIp__(mat->ptr[i], v2_ptr, n);
}

/* vm_mltadd -- vector-matrix multiply and add
	-- may not be in situ
	-- returns out' == v1' + alpha * v2'*A */
void
mxVecAddRealMulVecMulMat(Vec * v1, real alpha, Vec * v2, Mat * mat,
			 Vec * out)
{
  int m, n;
  real *out_ptr;

  out->copy(v1);

  out_ptr = out->ptr;
  m = mat->m;
  n = mat->n;
  for (int j = 0; j < m; j++)
  {
    real tmp = v2->ptr[j] * alpha;
    if (tmp != 0.0)
      mxRealMulAdd__(out_ptr, mat->ptr[j], tmp, n);
  }
}

}

