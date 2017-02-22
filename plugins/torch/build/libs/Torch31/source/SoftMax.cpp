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

#include "SoftMax.h"

namespace Torch {

SoftMax::SoftMax(int n_units) : GradientMachine(n_units, n_units)
{
  addROption("shift", &shift, 0, "shift to avoid overflow");
  addBOption("compute shift", &calc_shift, false, "compute shift to avoid overflow");
}

void SoftMax::frameForward(int t, real *f_inputs, real *f_outputs)
{
  if(calc_shift)
  {
    shift = f_inputs[0];
    for(int i = 1; i < n_inputs; i++)
    {
      if(f_inputs[i] > shift)
          shift = f_inputs[i];
    }
  }

  real sum = 0;
  for(int i = 0; i < n_outputs; i++)
  {
    real z = exp(f_inputs[i] - shift);
    f_outputs[i] = z;
    sum += z;
  }

  for(int i = 0; i < n_outputs; i++)
    f_outputs[i] /= sum;
}

void SoftMax::frameBackward(int t, real *f_inputs, real *beta_, real *f_outputs, real *alpha_)
{
  if(partial_backprop)
    return;

  real sum = 0;
  for(int i = 0; i < n_outputs; i++)
    sum += alpha_[i] * f_outputs[i];

  for(int i = 0; i < n_outputs; i++)
    beta_[i] = f_outputs[i] * (alpha_[i] - sum);
}

SoftMax::~SoftMax()
{
}

}
