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

#include "KFold.h"
#include "Random.h"

namespace Torch {

KFold::KFold(Trainer* trainer_, int kfold_)
{
  kfold = kfold_;
  trainer = trainer_;

  train_subsets = (int**)allocator->alloc(sizeof(int*)*kfold);
  test_subsets = (int**)allocator->alloc(sizeof(int*)*kfold);
  n_train_subsets = (int*)allocator->alloc(sizeof(int)*kfold);
  n_test_subsets = (int*)allocator->alloc(sizeof(int)*kfold);

  for (int i=0;i<kfold;i++)
  {
    train_subsets[i] = NULL;
    test_subsets[i] = NULL;
  }
}

void KFold::sample(int n_examples)
{
  int *mix_subset = (int *)Allocator::sysAlloc(sizeof(int)*n_examples);
  Random::getShuffledIndices(mix_subset, n_examples);
  int fucking_hack_because_round_sucks = n_examples/kfold;
  int taille_subset = ( (((real)n_examples)/((real)kfold)) - (real)fucking_hack_because_round_sucks >= 0.5 ? fucking_hack_because_round_sucks+1 : fucking_hack_because_round_sucks );
  for(int i = 0; i < kfold; i++)
  {
    n_train_subsets[i] = 0;
    n_test_subsets[i] = 0;

    for(int j = 0; j < i*taille_subset; j++)
      train_subsets[i][n_train_subsets[i]++] = mix_subset[j];
    for(int j = i*taille_subset; j < min((i+1)*taille_subset, n_examples); j++)
      test_subsets[i][n_test_subsets[i]++] = mix_subset[j];
    if(i == kfold-1)
    {
      for(int j = min((i+1)*taille_subset, n_examples); j < n_examples; j++)
        test_subsets[i][n_test_subsets[i]++] = mix_subset[j];
    }
    else
    {
      for(int j = (i+1)*taille_subset; j < n_examples; j++)
        train_subsets[i][n_train_subsets[i]++] = mix_subset[j];
    }
  }
  free(mix_subset);
}

void KFold::crossValidate(DataSet *data, MeasurerList *train_measurers, MeasurerList *test_measurers, MeasurerList *cross_valid_measurers)
{
  for (int i=0;i<kfold;i++)
  {
    train_subsets[i] = (int*)allocator->realloc(train_subsets[i], sizeof(int)*data->n_examples);
    test_subsets[i] = (int*)allocator->realloc(test_subsets[i], sizeof(int)*data->n_examples);
  }

  sample(data->n_examples);

  if(cross_valid_measurers)
  {
    for(int i = 0; i < cross_valid_measurers->n_nodes; i++)
      cross_valid_measurers->nodes[i]->reset();
  }

  for(int i = 0; i < kfold; i++)
  {
    data->pushSubset(train_subsets[i], n_train_subsets[i]);
    trainer->machine->reset();
    trainer->train(data, train_measurers);
    data->popSubset();

    data->pushSubset(test_subsets[i], n_test_subsets[i]);
    trainer->test(test_measurers);
    data->popSubset();

    if(cross_valid_measurers)
    {
      for(int j = 0; j < cross_valid_measurers->n_nodes; j++)
        cross_valid_measurers->nodes[j]->measureExample();
    }
  }
  
  if(cross_valid_measurers)
  {
    for(int i = 0; i < cross_valid_measurers->n_nodes; i++)
    {
      cross_valid_measurers->nodes[i]->measureIteration();
      cross_valid_measurers->nodes[i]->measureEnd();
    }
  }
}

KFold::~KFold()
{
}

}
