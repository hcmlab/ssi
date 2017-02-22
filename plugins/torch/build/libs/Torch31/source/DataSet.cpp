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

#include "DataSet.h"

namespace Torch {

DataSet::DataSet()
{
  n_targets = 0;
  targets = NULL;
  n_inputs = 0;
  inputs = NULL;
  n_examples = 0;
  n_real_examples = 0;
  select_examples = false;  
  selected_examples = NULL;
  subsets = NULL;
  n_examples_subsets = NULL;
  n_subsets = 0;

  // Indeterminated state...
  real_current_example_index = -1;

  // Pushed Examples...
  pushed_examples = new(allocator) Stack;
}

void DataSet::init(int n_examples_, int n_inputs_, int n_targets_)
{
  n_examples = n_examples_;
  n_real_examples = n_examples_;
  n_inputs = n_inputs_;
  n_targets = n_targets_;

  select_examples = false;
  selected_examples = (int *)allocator->alloc(sizeof(int)*n_examples);
  for(int i = 0; i < n_examples; i++)
    selected_examples[i] = i;
}

void DataSet::pushSubset(int *subset_, int n_examples_)
{
  subsets = (int **)allocator->realloc(subsets, sizeof(int *)*(n_subsets+1));
  n_examples_subsets = (int *)allocator->realloc(n_examples_subsets, sizeof(int)*(n_subsets+1));
  subsets[n_subsets] = subset_;
  n_examples_subsets[n_subsets] = n_examples_;
  n_subsets++;

  selected_examples = (int *)allocator->realloc(selected_examples, sizeof(int)*n_examples_);
  for(int t = 0; t < n_examples_; t++)
  {
    int index = subset_[t];
    for(int s = n_subsets-2; s >= 0; s--)
      index = subsets[s][index];
    selected_examples[t] = index;
  }

  select_examples = true;
  n_examples = n_examples_;  
}

void DataSet::popSubset()
{
  // Rq: realloc renvoie NULL si freed
  subsets = (int **)allocator->realloc(subsets, sizeof(int *)*(n_subsets-1));
  n_examples_subsets = (int *)allocator->realloc(n_examples_subsets, sizeof(int)*(n_subsets-1));
  n_subsets--;
    
  if(n_subsets == 0)
  {
    select_examples = false;
    n_examples = n_real_examples;

    selected_examples = (int *)allocator->realloc(selected_examples, sizeof(int)*n_examples);
    for(int i = 0; i < n_examples; i++)
      selected_examples[i] = i;
  }
  else
  {
    int n_examples_ = n_examples_subsets[n_subsets-1];
    int *subset_ = subsets[n_subsets-1];
    selected_examples = (int *)allocator->realloc(selected_examples, sizeof(int)*n_examples_);
    for(int t = 0; t < n_examples_; t++)
    {
      int index = subset_[t];
      for(int s = n_subsets-2; s >= 0; s--)
        index = subsets[s][index];
      selected_examples[t] = index;
    }
    n_examples = n_examples_;
  }
}

void DataSet::setExample(int t, bool set_inputs, bool set_targets)
{  
  int t_ = (select_examples ? selected_examples[t] : t);
  setRealExample(t_, set_inputs, set_targets);
}

DataSet::~DataSet()
{
}

}
