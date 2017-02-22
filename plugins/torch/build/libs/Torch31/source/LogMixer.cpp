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

#include "LogMixer.h"
#include "log_add.h"

namespace Torch {

LogMixer::LogMixer(int n_experts_, int n_outputs_per_expert) : GradientMachine(n_experts_*(n_outputs_per_expert+1), n_outputs_per_expert)
{
  n_experts = n_experts_;
}

void LogMixer::frameForward(int t, real *f_inputs, real *f_outputs)
{
  for(int i = 0; i < n_outputs; i++)
    f_outputs[i] = LOG_ZERO;

  real *log_weight_ = f_inputs;
  real *log_src_ = f_inputs+n_experts;
  for(int i = 0; i < n_experts; i++)
  {
    real z = log_weight_[i];
    for(int j = 0; j < n_outputs; j++)
      f_outputs[j] = logAdd(f_outputs[j], z + *log_src_++);
  }
}

void LogMixer::frameBackward(int t, real *f_inputs, real *beta_, real *f_outputs, real *alpha_)
{
  if(partial_backprop)
    return;

  real *beta__ = beta_+n_experts;
  real *log_weight_ = f_inputs;
  real *log_src_ = f_inputs+n_experts;
  for(int i = 0; i < n_experts; i++)
  {
    real z = log_weight_[i];
    for(int j = 0; j < n_outputs; j++)
      *beta__++ = alpha_[j] * exp(z + *log_src_++ - f_outputs[j]);
  }
    
  beta__ = beta_+n_experts;
  for(int i = 0; i < n_experts; i++)
  {
    real sum = 0;
    for(int j = 0; j < n_outputs; j++)
      sum += *beta__++;
    beta_[i] = sum;
  }
}

LogMixer::~LogMixer()
{
}

}
