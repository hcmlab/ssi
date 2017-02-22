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

#include "TwoClassFormat.h"

namespace Torch {

TwoClassFormat::TwoClassFormat(DataSet *data)
{
  if(data->n_targets != 1)
    warning("TwoClassFormat: the data has %d ouputs", data->n_targets);
  
  int n_set = 0;
  for(int i = 0; i < data->n_examples; i++)
  {
    data->setExample(i);
    
    bool flag = false;
    for(int k = 0; k < n_set; k++)
    {
      if(data->targets->frames[0][0] == tabclasses[k])
        flag = true;
    }

    if(!flag)
    {
      if(n_set == 2)
        error("TwoClassFormat: you have more than two classes");

      tabclasses[n_set++] = data->targets->frames[0][0];
    }
  }

  switch(n_set)
  {
    case 0:
      warning("TwoClassFormat: you have no examples");
      tabclasses[0] = 0;
      tabclasses[1] = 0;
      break;
    case 1:
      warning("TwoClassFormat: you have only one class [%g]", tabclasses[0]);
      tabclasses[1] = tabclasses[0];
      break;
    case 2:
      if(tabclasses[0] > tabclasses[1])
      {
        real z = tabclasses[1];
        tabclasses[1] = tabclasses[0];
        tabclasses[0] = z;
      }
      message("TwoClassFormat: two classes detected [%g and %g]", tabclasses[0], tabclasses[1]);
      break;
  }

  // He He He...
  n_classes = 2;
  class_labels = (real **)allocator->alloc(sizeof(real *)*n_classes);
  for(int i = 0; i < n_classes; i++)
    class_labels[i] = tabclasses+i;
}

TwoClassFormat::TwoClassFormat(real class_1, real class_2)
{
  tabclasses[0] = class_1;
  tabclasses[1] = class_2;
  n_classes = 2;
  class_labels = (real **)allocator->alloc(sizeof(real *)*n_classes);
  for(int i = 0; i < n_classes; i++)
    class_labels[i] = tabclasses+i;
}

int TwoClassFormat::getOutputSize()
{
  return 1;
}

void TwoClassFormat::fromOneHot(real *outputs, real *one_hot_outputs)
{
  outputs[0] = one_hot_outputs[0] - one_hot_outputs[1];
  if(tabclasses[1] > tabclasses[0]) 
    outputs[0] = one_hot_outputs[1] - one_hot_outputs[0];
  else
    outputs[0] = one_hot_outputs[0] - one_hot_outputs[1];
}

void TwoClassFormat::toOneHot(real *outputs, real *one_hot_outputs)
{
  int maxclass = (tabclasses[1]>tabclasses[0]);
  int minclass = (tabclasses[0]>tabclasses[1]);
  one_hot_outputs[0] = fabs(outputs[0] - tabclasses[maxclass]);
  one_hot_outputs[1] = fabs(outputs[0] - tabclasses[minclass]);
}

int TwoClassFormat::getClass(real *vector)
{
  real out = vector[0];
  
  return(fabs(out - tabclasses[0]) > fabs(out - tabclasses[1]) ? 1 : 0);
}

TwoClassFormat::~TwoClassFormat()
{
}

}
