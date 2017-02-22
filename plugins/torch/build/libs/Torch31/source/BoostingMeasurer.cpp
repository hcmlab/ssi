// Copyright (C) 2003--2004 Ronan Collobert (collober@idiap.ch)
//                and Samy Bengio (bengio@idiap.ch)
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

#include "BoostingMeasurer.h"

namespace Torch {

BoostingMeasurer::BoostingMeasurer(ClassFormat *class_format_, XFile *file_) : Measurer(NULL, file_)
{
  weights = NULL;
  inputs = NULL;
  status = NULL;

  class_format = class_format_;

  internal_error = 0;
  current_example = 0;
}

void BoostingMeasurer::setDataSet(DataSet *data_)
{
  data = data_;
  status = (int *)allocator->realloc(status, sizeof(int)*data->n_examples);
}

void BoostingMeasurer::setWeights(real *weights_)
{
  weights = weights_;
}

void BoostingMeasurer::setInputs(Sequence *inputs_)
{
  inputs = inputs_;
}

void BoostingMeasurer::measureExample()
{
  int c_obs = class_format->getClass(inputs->frames[0]);
  int c_des = class_format->getClass(data->targets->frames[0]);

  if(c_obs != c_des)
  {
    internal_error += weights[current_example];
    status[current_example++] = -1;
  }
  else
    status[current_example++] = 1;
}

void BoostingMeasurer::measureIteration()
{
  beta = internal_error/(1. - internal_error);

  if(binary_mode)
  {
    file->write(&internal_error, sizeof(real), 1);
    file->write(&beta, sizeof(real), 1);
  }
  else
    file->printf("%g ==> %g for beta\n", internal_error, beta);
  file->flush();
  reset();
}

void BoostingMeasurer::reset()
{
  internal_error = 0;
  current_example = 0;
}

BoostingMeasurer::~BoostingMeasurer()
{
}

}
