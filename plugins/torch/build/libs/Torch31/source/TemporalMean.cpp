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

#include "TemporalMean.h"
#include "Random.h"

namespace Torch {

TemporalMean::TemporalMean(int input_frame_size)
  : GradientMachine(input_frame_size, input_frame_size)
{
  outputs->resize(1);
}

void TemporalMean::forward(Sequence *inputs)
{
  real *outputs_ = outputs->frames[0];
  for(int i = 0; i < n_outputs; i++)
    outputs_[i] = 0;

  for(int i = 0; i < inputs->n_frames; i++)
  {
    real *input_frame_ = inputs->frames[i];
    for(int j = 0; j < n_outputs; j++)
      outputs_[j] += input_frame_[j];
  }

  real z = 1./((real)inputs->n_frames);
  for(int i = 0; i < n_outputs; i++)
    outputs_[i] *= z;
}

void TemporalMean::backward(Sequence *inputs, Sequence *alpha)
{
  if(partial_backprop)
    return;
  
  beta->resize(inputs->n_frames);
  real *alpha_ = alpha->frames[0];

  real z = 1./((real)inputs->n_frames);
  for(int i = 0; i < beta->n_frames; i++)
  {
    real *beta_frame_ = beta->frames[i];
    for(int j = 0; j < n_outputs; j++)
      beta_frame_[j] = alpha_[j]*z;
  }
}

TemporalMean::~TemporalMean()
{
}

}
