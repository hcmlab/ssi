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

#include "MSECriterion.h"

namespace Torch {

MSECriterion::MSECriterion(int n_inputs_) : Criterion(n_inputs_)
{
  addBOption("average frame size", &average_frame_size, true, "divided by the frame size");
}

void MSECriterion::frameForward(int t, real *f_inputs, real *f_outputs)
{
  real *desired = data->targets->frames[t];
  real err = 0;

  for(int i = 0; i < n_inputs; i++)
  {
      real z = desired[i] - f_inputs[i];
      err += z*z;
  }

  if(average_frame_size)
    err /= n_inputs;

  f_outputs[0] = err;
}

void MSECriterion::frameBackward(int t, real *f_inputs, real *beta_, real *f_outputs, real *alpha_)
{
  real *desired = data->targets->frames[t];

  if(average_frame_size)
  {
    real norm_ = 2./n_inputs;
    for(int i = 0; i < n_inputs; i++)
      beta_[i] = norm_*(f_inputs[i] - desired[i]);
  }
  else
  {
    for(int i = 0; i < n_inputs; i++)
      beta_[i] = 2.*(f_inputs[i] - desired[i]);
  }
}

MSECriterion::~MSECriterion()
{
}

}
