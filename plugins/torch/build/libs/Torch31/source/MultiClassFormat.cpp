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

#include "MultiClassFormat.h"

namespace Torch {

extern "C" int multiClassTriMelanie(const void *a, const void *b)
{
  real *ar = (real *)a;
  real *br = (real *)b;

  if(*ar < *br)
    return -1;
  else
    return  1;
}

MultiClassFormat::MultiClassFormat(DataSet *data)
{
  tabclasses = NULL;

  if(data->n_targets != 1)
    warning("MultiClassFormat: the data has %d ouputs", data->n_targets);
  
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
      tabclasses = (real *)allocator->realloc(tabclasses, sizeof(real)*(n_set+1));
      tabclasses[n_set++] = data->targets->frames[0][0];
    }
  }

  switch(n_set)
  {
    case 0:
      error("MultiClassFormat: you have no examples");
      break;
    case 1:
      warning("MultiClassFormat: you have only one class [%g]", tabclasses[0]);
      break;
    default:
      message("MultiClassFormat: %d classes detected", n_set);
      break;
  }

  // He He He...
  n_classes = n_set;
  qsort(tabclasses, n_classes, sizeof(real), multiClassTriMelanie);
  class_labels = (real **)allocator->alloc(sizeof(real *)*n_classes);
  for(int i = 0; i < n_classes; i++)
    class_labels[i] = tabclasses+i;
}

MultiClassFormat::MultiClassFormat(int n_classes_, real *class_labels_)
{
  n_classes = n_classes_;
  tabclasses = (real *)allocator->alloc(sizeof(real)*n_classes);

  if(class_labels_)
  {
    for(int i = 0; i < n_classes; i++)
      tabclasses[i] = class_labels_[i];
  }
  else
  {
    for(int i = 0; i < n_classes; i++)
      tabclasses[i] = (real)i;
  }

  class_labels = (real **)allocator->alloc(sizeof(real *)*n_classes);
  for(int i = 0; i < n_classes; i++)
    class_labels[i] = tabclasses+i;
}

int MultiClassFormat::getOutputSize()
{
  return 1;
}

void MultiClassFormat::fromOneHot(real *outputs, real *one_hot_outputs)
{
  real max = -INF;
  int index = -1;
  for(int i = 0; i < n_classes; i++)
  {
    if(one_hot_outputs[i] > max)
    {
      max = one_hot_outputs[i];
      index = i;
    }
  }
  outputs[0] = (real)index;
}

void MultiClassFormat::toOneHot(real *outputs, real *one_hot_outputs)
{
  real out = outputs[0];
  // heuristic: find the one or two labels that are closer to "out" and
  // attribute them the difference between out and their label. put 0 for
  // all the other values

  // first initialize one_hot_outputs with all zeros
  for(int i = 0; i < n_classes; i++)
    one_hot_outputs[i] = 0.;

  // then there are 3 different cases
  if(out > n_classes-1)
  {
    one_hot_outputs[n_classes-1] = fabs(out - tabclasses[n_classes-1]);
  }
  else if(out < 0)
  {
    one_hot_outputs[0] = fabs(out - tabclasses[0]);
  }
  else
  {
    int before = (int)floor(out);
    int after = (int)ceil(out);
    // the scores are reversed so the max score is given to the neirest
    real diff_before = after - out;
    real diff_after = out - before;
    if (before == after)
      diff_before = diff_after = 1.;
    
    one_hot_outputs[before] = diff_before;
    one_hot_outputs[after] = diff_after;
  }
}

int MultiClassFormat::getClass(real *vector)
{
  real out = vector[0];
  real dist = fabs(out - tabclasses[0]);
  int index = 0;

  for(int i = 1; i < n_classes; i++)
  {
    real z = fabs(out - tabclasses[i]);
    if(z < dist)
    {
      index = i;
      dist = z;
    }
  }
  
  return(index);
}

MultiClassFormat::~MultiClassFormat()
{
}

}
