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

#include "GradientMachine.h"

namespace Torch {

GradientMachine::GradientMachine(int n_inputs_, int n_outputs_, int n_params_)
{
  n_inputs = n_inputs_;
  n_outputs = n_outputs_;
  if(n_outputs > 0)
    outputs = new(allocator) Sequence(0, n_outputs);
  else
    outputs = NULL;
  if(n_inputs > 0)
    beta = new(allocator) Sequence(0, n_inputs);
  else
    beta = NULL;
  params = new(allocator) Parameters(n_params_);
  der_params = new(allocator) Parameters(n_params_);
  partial_backprop = false;
}

void GradientMachine::setPartialBackprop(bool flag)
{
  partial_backprop = flag;
}

void GradientMachine::iterInitialize()
{
}

void GradientMachine::forward(Sequence *inputs)
{
  outputs->resize(inputs->n_frames);

  for(int i = 0; i < inputs->n_frames; i++)
    frameForward(i, inputs->frames[i], outputs->frames[i]);
}

void GradientMachine::backward(Sequence *inputs, Sequence *alpha)
{
  beta->resize(inputs->n_frames);

  if(alpha)
  {
    for(int i = 0; i < inputs->n_frames; i++)
      frameBackward(i, inputs->frames[i], beta->frames[i], outputs->frames[i], alpha->frames[i]);
  }
  else
  {
    for(int i = 0; i < inputs->n_frames; i++)
      frameBackward(i, inputs->frames[i], beta->frames[i], outputs->frames[i], NULL);
  }
}

void GradientMachine::frameForward(int t, real *f_inputs, real *f_outputs)
{
}

void GradientMachine::frameBackward(int t, real *f_inputs, real *beta_, real *f_outputs, real *alpha_)
{
}

void GradientMachine::loadXFile(XFile *file)
{
  if(params)
    params->loadXFile(file);
}

void GradientMachine::saveXFile(XFile *file)
{
  if(params)
    params->saveXFile(file);
}

GradientMachine::~GradientMachine()
{
}

}
