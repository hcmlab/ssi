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

#include "WeightedMSECriterion.h"

namespace Torch {

WeightedMSECriterion::WeightedMSECriterion(DataSet *data_) : Criterion(data_->n_targets)
{
  addBOption("average frame size", &average_frame_size, true, "divided by the frame size");
  data = data_;
  weights = (real *)allocator->alloc(sizeof(real)*data->n_real_examples);
  for(int i = 0; i < data->n_real_examples; i++)
    weights[i] = 1;
}

WeightedMSECriterion::WeightedMSECriterion(DataSet *data_, real *weights_) : Criterion(data_->n_targets)
{
  data = data_;
  weights = weights_;
}

void WeightedMSECriterion::setDataSet(DataSet *data_)
{
  if(data_->n_real_examples != data->n_real_examples)
    error("WeightedMSECriterion: trying to use a wrong DataSet");
  data = data_;
}

void WeightedMSECriterion::frameForward(int t, real *f_inputs, real *f_outputs)
{
  real *desired = data->targets->frames[t];
  real err = 0;
  
  for(int i = 0; i < n_inputs; i++)
  {
      real z = desired[i] - f_inputs[i];
      err += z*z;
  }

  f_outputs[0] = err*weights[data->real_current_example_index];

  if(average_frame_size)
    f_outputs[0] /= n_inputs;
}

void WeightedMSECriterion::frameBackward(int t, real *f_inputs, real *beta_, real *f_outputs, real *alpha_)
{
  real z = 2.*weights[data->real_current_example_index];
  if(average_frame_size)
    z /= n_inputs;
  real *desired = data->targets->frames[t];
  for(int i = 0; i < n_inputs; i++)
    beta_[i] = z*(f_inputs[i] - desired[i]);
}

WeightedMSECriterion::~WeightedMSECriterion()
{
}

}
