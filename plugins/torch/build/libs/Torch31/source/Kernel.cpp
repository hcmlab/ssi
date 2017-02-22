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

#include "Kernel.h"

namespace Torch {

Kernel::Kernel()
{
}

Kernel::~Kernel()
{
}

//=====

DotKernel::DotKernel(real s_)
{
  s = s_;
}

real DotKernel::eval(Sequence *x, Sequence *y)
{
  real *x_ = x->frames[0];
  real *y_ = y->frames[0];

  real sum = 0;
  for(int i = 0; i < x->frame_size; i++)
    sum += x_[i]*y_[i];

  return s*sum;
}

DotKernel::~DotKernel()
{
}

//=====

PolynomialKernel::PolynomialKernel(int degree, real s_, real r_)
{
  d = degree;
  s = s_;
  r = r_;
}

real PolynomialKernel::eval(Sequence *x, Sequence *y)
{
  real *x_ = x->frames[0];
  real *y_ = y->frames[0];

  real sum = 0;
  for(int i = 0; i < x->frame_size; i++)
    sum += x_[i]*y_[i];

  sum = s*sum+r;

  // la fonction pow rame a donf
  real julie = sum;
  for(int t = 1; t < d; t++)
    julie *= sum;
    
  return(julie);
}

PolynomialKernel::~PolynomialKernel()
{
}

//=====

GaussianKernel::GaussianKernel(real g_)
{
  g = g_;
}

real GaussianKernel::eval(Sequence *x, Sequence *y)
{
  real *x_ = x->frames[0];
  real *y_ = y->frames[0];

  real sum = 0.;
  for(int i = 0; i < x->frame_size; i++)
  {
    real z = x_[i]-y_[i];
    sum -= z*z;
  }

  return exp(g*sum);
}

GaussianKernel::~GaussianKernel()
{
}

//=====

SigmoidKernel::SigmoidKernel(real s_, real r_)
{
  s = s_;
  r = r_;
}

real SigmoidKernel::eval(Sequence *x, Sequence *y)
{
  real *x_ = x->frames[0];
  real *y_ = y->frames[0];

  real sum = 0;
  for(int i = 0; i < x->frame_size; i++)
    sum += x_[i]*y_[i];

  return(tanh(s*sum+r));
}

SigmoidKernel::~SigmoidKernel()
{
}

//=====

}
