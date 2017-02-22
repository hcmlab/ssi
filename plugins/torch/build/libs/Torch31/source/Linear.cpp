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

#include "Linear.h"
#include "Random.h"

namespace Torch {

Linear::Linear(int n_inputs_, int n_outputs_) : GradientMachine(n_inputs_, n_outputs_, (n_inputs_+1)*n_outputs_)
{
  addROption("weight decay", &weight_decay, 0, "weight decay");
  weights = params->data[0];
  bias = params->data[0]+n_inputs*n_outputs;
  der_weights = der_params->data[0];
  der_bias = der_params->data[0]+n_inputs*n_outputs;
  reset_();
}

void Linear::reset()
{
  reset_();
}

void Linear::reset_()
{
  // Note: just to be compatible with "Torch II Dev"
  real *weights_ = weights;
  real bound = 1./sqrt((real)n_inputs);

  for(int i = 0; i < n_outputs; i++)
  {
    for(int j = 0; j < n_inputs; j++)
      weights_[j] = Random::boundedUniform(-bound, bound);
    weights_ += n_inputs;
    bias[i] = Random::boundedUniform(-bound, bound);
  }
}

void Linear::frameForward(int t, real *f_inputs, real *f_outputs)
{
  real *weights_ = weights;
  for(int i = 0; i < n_outputs; i++)
  {
    real out = bias[i];

    for(int j = 0; j < n_inputs; j++)
      out += weights_[j] * f_inputs[j];
    weights_ += n_inputs;

    f_outputs[i] = out;
  }
}

void Linear::frameBackward(int t, real *f_inputs, real *beta_, real *f_outputs, real *alpha_)
{
  if(!partial_backprop)
  {
    for(int i = 0; i < n_inputs; i++)
      beta_[i] = 0;
    
    real *weights_ = weights;
    for(int i = 0; i < n_outputs; i++)
    {
      real z = alpha_[i];
      for(int j = 0; j < n_inputs; j++)
        beta_[j] += z * weights_[j];
      weights_ += n_inputs;
    }
  }

  real *der_weights_ = der_weights;
  for(int i = 0; i < n_outputs; i++)
  {
    real z = alpha_[i];
    for(int j = 0; j < n_inputs; j++)
      der_weights_[j] += z * f_inputs[j];
    der_weights_ += n_inputs;

    der_bias[i] += z;
  }

  if(weight_decay != 0)
  {
    real *src_ = params->data[0];
    real *dest_ = der_params->data[0];
    // Note: pas de weight decay sur les biais.
    for(int i = 0; i < n_inputs*n_outputs; i++)
      dest_[i] += weight_decay * src_[i];
  }
}

Linear::~Linear()
{
}

}
