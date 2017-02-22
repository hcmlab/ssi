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

#include "Vec.h"
#include "mx_low_level.h"

namespace Torch {

Vec::Vec(real * ptr_, int n_dim)
{
  ptr = ptr_;
  n = n_dim;
}

Vec::Vec(int n_dim)
{
  n = n_dim;
  ptr = (real *)allocator->alloc(sizeof(real) * n);
}

void Vec::copy(Vec * vec, int start_i)
{
  if (vec == this)
    return;

  real *ptr_r = vec->ptr + start_i;
  real *ptr_w = ptr + start_i;
  for (int i = 0; i < n - start_i; i++)
    *ptr_w++ = *ptr_r++;
}

void Vec::zero()
{
  real *ptr_w = ptr;
  for (int i = 0; i < n; i++)
    *ptr_w++ = 0.;
}

real Vec::norm1(Vec * weights)
{
  real sum = 0.0;
  real *ptr_x = ptr;
  if (weights)
  {
    real *ptr_w = weights->ptr;
    for (int i = 0; i < n; i++)
      sum += *ptr_w++ * fabs(*ptr_x++);
  }
  else
  {
    for (int i = 0; i < n; i++)
      sum += fabs(*ptr_x++);
  }

  return sum;
}

real Vec::norm2(Vec * weights)
{
  real sum = 0.0;
  real *ptr_x = ptr;
  if (weights)
  {
    real *ptr_w = weights->ptr;
    for (int i = 0; i < n; i++)
    {
      real z = *ptr_x++;
      sum += *ptr_w++ * z * z;
    }
  }
  else
  {
    for (int i = 0; i < n; i++)
    {
      real z = *ptr_x++;
      sum += z * z;
    }
  }

  return sqrt(sum);
}

real Vec::normInf()
{
  real *ptr_x = ptr;
  real max_val = fabs(*ptr_x++);

  for (int i = 1; i < n; i++)
  {
    real z = fabs(*ptr_x);
    if (max_val < z)
      max_val = z;
    ptr_x++;
  }

  return max_val;
}

real Vec::iP(Vec * vec, int start_i)
{
  return (mxIp__(ptr + start_i, vec->ptr + start_i, n - start_i));
}

Vec *Vec::subVec(int dim1, int dim2)
{
  Vec *vec = new Vec(ptr + dim1, dim2 - dim1 + 1);

  return (vec);
}

Vec::~Vec()
{
}

}

