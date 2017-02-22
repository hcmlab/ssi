// Copyright (C) 2003--2004 Samy Bengio (bengio@idiap.ch)
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

#include "ParzenMachine.h"

namespace Torch {

ParzenMachine::ParzenMachine(int n_inputs_,int n_outputs_,real var_)
{
  data = NULL;
  setVar(var_);
  n_outputs = n_outputs_;
  n_inputs = n_inputs_;
  outputs = new(allocator) Sequence(1,n_outputs);
  n_real_examples = 0;
  real_examples = NULL;
}

void ParzenMachine::setVar(real var_)
{
  var = var_;
}

void ParzenMachine::setDataSet(DataSet *dataset_)
{
  data = dataset_;
  n_real_examples = data->n_examples;
  real_examples = (int*)allocator->realloc(real_examples,n_real_examples*sizeof(int));
  for (int i=0;i<data->n_examples;i++) {
    real_examples[i] = data->selected_examples[i];
  }
}

void ParzenMachine::forward(Sequence* inputs)
{
  // keep current example in order to restore at the end
  data->pushExample();

  real* out = outputs->frames[0];
  denominator = 0.;
  for (int j=0;j<n_outputs;j++)
    *out++ = 0;
  int *i_ptr = real_examples;
  for (int i=0;i<n_real_examples;i++) {
    data->setRealExample(*i_ptr++);
    real* in = inputs->frames[0];
    real* in_i = data->inputs->frames[0];
    real dist = 0;
    for (int j=0;j<n_inputs;j++) {
      real z = *in++ - *in_i++;
      dist += z*z;
    }
    real e = exp(-dist / (2.*var));
    denominator += e;
    out = outputs->frames[0];
    real* targ_i = data->targets->frames[0];
    for (int j=0;j<n_outputs;j++) {
      *out++ += *targ_i++ * e;
    }
  }

  out = outputs->frames[0];
  for (int j=0;j<n_outputs;j++)
    *out++ /= denominator;

  // restore current_example
  data->popExample();
}

ParzenMachine::~ParzenMachine()
{
}

}

