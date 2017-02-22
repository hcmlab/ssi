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

#include "Bagging.h"
#include "Random.h"

namespace Torch {

Bagging::Bagging(WeightedSumMachine* w_machine_) : Trainer(w_machine_)
{
  w_machine = w_machine_;

  n_trainers = w_machine->n_trainers;
  n_unselected_examples = (int *)allocator->alloc(sizeof(int)*n_trainers);
  unselected_examples = (int **)allocator->alloc(sizeof(int*)*n_trainers);
  selected_examples = (int **)allocator->alloc(sizeof(int*)*n_trainers);
  is_selected_examples = NULL;
}

void Bagging::bootstrapData(int* selected, int* is_selected, int n_examples)
{
  for (int j=0;j<n_examples;j++) {
    selected[j] = (int)floor(Random::boundedUniform(0,n_examples));
    is_selected[selected[j]] = 1;
  }
}

void Bagging::train(DataSet *data, MeasurerList* measurers)
{
  // Misc Initializations
  int n = data->n_examples;
  is_selected_examples = (int *)allocator->realloc(is_selected_examples, sizeof(int)*n);
  for (int i = 0; i < n_trainers; i++)
  {
    unselected_examples[i] = (int *)allocator->realloc(unselected_examples[i], sizeof(int)*n);
    selected_examples[i] = (int *)allocator->realloc(selected_examples[i], sizeof(int)*n);
  } 

  for(int i = 0; i < n_trainers; i++)
    w_machine->weights[i] = 1./((real)n_trainers);

  message("Bagging: training");
  w_machine->n_trainers_trained = 0;

  for (int i=0;i<n_trainers;i++) {
    // initialization
    for (int j=0;j<n;j++) {
      is_selected_examples[j]=0;
    }

    // select a bootstrap
    bootstrapData(selected_examples[i],is_selected_examples,n);
    data->pushSubset(selected_examples[i],n);

    // keep in mind examples not used by trainers[i]
    int k=0;
    for (int j=0;j<n;j++) {
      if (!is_selected_examples[j])
        unselected_examples[i][k++] = j;
    }
    n_unselected_examples[i] = k;
    

    // train the trainer
    w_machine->trainers[i]->machine->reset();
    w_machine->trainers[i]->train(data, w_machine->trainers_measurers ? w_machine->trainers_measurers[i] : NULL);

    // put back the selected_examples
    data->popSubset();

    w_machine->n_trainers_trained = i+1;

    // if measurers is given, call the test method by fooling it
    // with the number of trainers
    if (measurers)
      test(measurers);
  }
}

Bagging::~Bagging()
{
}

}
