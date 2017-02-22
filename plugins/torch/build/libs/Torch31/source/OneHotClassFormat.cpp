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

#include "OneHotClassFormat.h"

namespace Torch {

OneHotClassFormat::OneHotClassFormat(DataSet *data)
{
  n_classes = data->n_targets;
  class_labels_buffer = (real *)allocator->alloc(sizeof(real)*n_classes*n_classes);
  class_labels = (real **)allocator->alloc(sizeof(real *)*n_classes);
  for(int i = 0; i < n_classes; i++)
  {
    class_labels[i] = class_labels_buffer+i*n_classes;
    for(int j = 0; j < n_classes; j++)
      class_labels[i][j] = 0.;
    class_labels[i][i] = 1.;
  }
  message("OneHotClassFormat: %d classes detected", n_classes);
}

OneHotClassFormat::OneHotClassFormat(int n_targets)
{
  n_classes = n_targets;
  class_labels_buffer = (real *)allocator->alloc(sizeof(real)*n_classes*n_classes);
  class_labels = (real **)allocator->alloc(sizeof(real *)*n_classes);
  for(int i = 0; i < n_classes; i++)
  {
    class_labels[i] = class_labels_buffer+i*n_classes;
    for(int j = 0; j < n_classes; j++)
      class_labels[i][j] = 0.;
    class_labels[i][i] = 1.;
  }
  message("OneHotClassFormat: %d classes detected", n_classes);
}

int OneHotClassFormat::getOutputSize()
{
  return n_classes;
}

void OneHotClassFormat::fromOneHot(real *outputs, real *one_hot_outputs)
{
  for(int i = 0; i < n_classes; i++)
    outputs[i] = one_hot_outputs[i];
}

void OneHotClassFormat::toOneHot(real *outputs, real *one_hot_outputs)
{
  for(int i = 0; i < n_classes; i++)
    one_hot_outputs[i] = outputs[i];
}

int OneHotClassFormat::getClass(real *vector)
{
  real z = vector[0];
  int index = 0;
  for(int i = 1; i < n_classes; i++)
  {
    if(vector[i] > z)
    {
      index = i;
      z  = vector[i];
    }
  }
  return(index);
}

OneHotClassFormat::~OneHotClassFormat()
{
}

}

