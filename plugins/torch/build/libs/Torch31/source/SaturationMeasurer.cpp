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

#include "SaturationMeasurer.h"

namespace Torch {

SaturationMeasurer::SaturationMeasurer(GradientMachine *machine_, DataSet *data_, XFile *file_) : Measurer(data_, file_)
{
  machine = machine_;
  reset_();
}

void SaturationMeasurer::measureExample()
{
  Sequence *outputs_ = machine->outputs;
  real sum = 0;
  for(int i = 0; i < outputs_->n_frames; i++)
  {
    real *frame_ = outputs_->frames[i];
    for(int j = 0; j < outputs_->frame_size; j++)
      sum += fabs(frame_[j]);
    n_sum_out += outputs_->frame_size;
  }
  outputs_sum += sum;
  
  Parameters *der_params_ = machine->der_params;
  sum = 0;
  for(int i = 0; i < der_params_->n_data; i++)
  {
    real *data_ = der_params_->data[i];
    for(int j = 0; j < der_params_->size[i]; j++)
      sum += fabs(data_[j]);
    n_sum_der += der_params_->size[i];
  }
  derivatives_sum += sum;
}

void SaturationMeasurer::measureIteration()
{
  if(n_sum_out != 0)
    outputs_sum /= n_sum_out;
  if(n_sum_der != 0)
    derivatives_sum /= n_sum_der;

  if(binary_mode)
  {
    file->write(&outputs_sum, sizeof(real), 1);
    file->write(&derivatives_sum, sizeof(real), 1);
  }
  else
    file->printf("%g %g\n", outputs_sum, derivatives_sum);
  file->flush();
  reset_();
}

void SaturationMeasurer::reset()
{
  reset_();
}

void SaturationMeasurer::reset_()
{
  derivatives_sum = 0;
  outputs_sum = 0;
  n_sum_out = 0;
  n_sum_der = 0;
}

SaturationMeasurer::~SaturationMeasurer()
{
}

}
