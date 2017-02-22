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

#include "MLP.h"
#include "Linear.h"
#include "Tanh.h"
#include "Sigmoid.h"
#include "SoftMax.h"
#include "LogSoftMax.h"
#include "Exp.h"
#include "SoftPlus.h"

namespace Torch {

MLP::MLP(int n_layers_, int n_inputs_, ...)
{
  n_layers = n_layers_;
  layers = (GradientMachine **)allocator->alloc(sizeof(GradientMachine *)*n_layers);
  is_linear = (bool *)allocator->alloc(sizeof(bool)*n_layers);
  for(int i = 0; i < n_layers; i++)
    is_linear[i] = false;

  va_list args;
  va_start(args, n_inputs_);  
  for(int i = 0; i < n_layers; i++)
  {
    char *layer_type = va_arg(args, char *);
    int n_outputs_ = va_arg(args, int);
    bool is_valid = false;

    if(!strcmp(layer_type, "linear"))
    {
      layers[i] = new(allocator) Linear(n_inputs_, n_outputs_);
      is_linear[i] = true;
      is_valid = true;
    }

    if(!strcmp(layer_type, "tanh"))
    {
      layers[i] = new(allocator) Tanh(n_outputs_);
      is_valid = true;
    }

    if(!strcmp(layer_type, "sigmoid"))
    {
      layers[i] = new(allocator) Sigmoid(n_outputs_);
      is_valid = true;
    }

    if(!strcmp(layer_type, "softmax"))
    {
      layers[i] = new(allocator) SoftMax(n_outputs_);
      is_valid = true;
    }

    if(!strcmp(layer_type, "log-softmax"))
    {
      layers[i] = new(allocator) LogSoftMax(n_outputs_);
      is_valid = true;
    }

    if(!strcmp(layer_type, "exp"))
    {
      layers[i] = new(allocator) Exp(n_outputs_);
      is_valid = true;
    }

    if(!strcmp(layer_type, "softplus"))
    {
      layers[i] = new(allocator) SoftPlus(n_outputs_);
      is_valid = true;
    }

    if(!is_valid)
      error("MLP: unknow layer type <%s>", layer_type);
    
    this->addFCL(layers[i]);
    n_inputs_ = n_outputs_;
  }
  build();
  va_end(args);
}

void MLP::setWeightDecay(real weight_decay)
{
  for(int i = 0; i < n_layers; i++)
  {
    if(is_linear[i])
      layers[i]->setROption("weight decay", weight_decay);
  }
}

MLP::~MLP()
{
}

}
