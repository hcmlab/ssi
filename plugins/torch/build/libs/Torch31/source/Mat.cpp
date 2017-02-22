// Copyright (C) 2003--2004 Ronan Collobert (collober@idiap.ch)
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

#include "Mat.h"

namespace Torch {

Mat::Mat(real ** ptr_, int n_rows, int n_cols)
{
  ptr = ptr_;
  m = n_rows;
  n = n_cols;
  base = NULL;
}

Mat::Mat(real * ptr_, int n_rows, int n_cols)
{
  m = n_rows;
  n = n_cols;
  base = NULL;
  ptr = (real **)allocator->alloc(sizeof(real *) * m);
  for (int i = 0; i < m; i++)
    ptr[i] = ptr_ + i * n;
}

Mat::Mat(int n_rows, int n_cols)
{
  m = n_rows;
  n = n_cols;
  base = (real *)allocator->alloc(sizeof(real) * m * n);
  ptr = (real **)allocator->alloc(sizeof(real *) * m);
  for (int i = 0; i < m; i++)
    ptr[i] = base + i * n;
}

void Mat::copy(Mat * mat)
{
  if (mat == this)
    return;

  for (int i = 0; i < m; i++)
  {
    real *ptr_r = mat->ptr[i];
    real *ptr_w = ptr[i];
    for (int j = 0; j < n; j++)
      *ptr_w++ = *ptr_r++;
  }
}

void Mat::zero()
{
  for (int i = 0; i < m; i++)
  {
    real *ptr_w = ptr[i];
    for (int j = 0; j < n; j++)
      *ptr_w++ = 0.;
  }
}

real Mat::norm1()
{
  real max_val = 0.;
  for (int j = 0; j < n; j++)
  {
    real sum = 0.0;
    for (int i = 0; i < m; i++)
      sum += fabs(ptr[i][j]);

    if (max_val < sum)
      max_val = sum;
  }

  return max_val;
}

real Mat::normFrobenius()
{
  real sum = 0.;
  for (int i = 0; i < m; i++)
  {
    real *ptr_x = ptr[i];
    for (int j = 0; j < n; j++)
    {
      real z = *ptr_x++;
      sum += z * z;
    }
  }

  return sqrt(sum);
}

real Mat::normInf()
{
  real max_val = 0.;
  for (int i = 0; i < m; i++)
  {
    real sum = 0.0;
    real *ptr_x = ptr[i];
    for (int j = 0; j < n; j++)
      sum += fabs(*ptr_x++);

    if (max_val < sum)
      max_val = sum;
  }

  return max_val;
}

Vec *Mat::getRow(int row, Vec * vec)
{
  if (!vec)
    vec = new Vec(n);

  real *ptr_w = vec->ptr;
  real *ptr_r = ptr[row];
  for (int i = 0; i < n; i++)
    *ptr_w++ = *ptr_r++;

  return (vec);
}

Vec *Mat::getCol(int col, Vec * vec)
{
  if (!vec)
    vec = new Vec(m);

  real *ptr_w = vec->ptr;
  for (int i = 0; i < m; i++)
    *ptr_w++ = ptr[i][col];

  return (vec);
}

void Mat::setRow(int row, Vec * vec)
{
  real *ptr_w = ptr[row];
  real *ptr_r = vec->ptr;
  for (int i = 0; i < n; i++)
    *ptr_w++ = *ptr_r++;
}

void Mat::setCol(int col, Vec * vec)
{
  real *ptr_r = vec->ptr;
  for (int i = 0; i < m; i++)
    ptr[i][col] = *ptr_r++;
}

Mat *Mat::subMat(int row1, int col1, int row2, int col2)
{
  real **ptr_ = (real **)Allocator::sysAlloc(sizeof(real *) * (row2 - row1 + 1));
  for (int i = row1; i <= row2; i++)
    ptr_[i - row1] = &ptr[i][col1];

  Mat *mat = new Mat(ptr_, row2 - row1 + 1, col2 - col1 + 1);
  mat->allocator->retain(ptr_);

  return (mat);
}

Mat::~Mat()
{
}

}
