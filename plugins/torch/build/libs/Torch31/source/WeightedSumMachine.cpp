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

#include "WeightedSumMachine.h"

namespace Torch {

WeightedSumMachine::WeightedSumMachine(Trainer **trainers_, int n_trainers_, MeasurerList** trainers_measurers_, real *weights_)
{
  // Boaf
  trainers = trainers_;
  trainers_measurers = trainers_measurers_;
  n_trainers = n_trainers_;

  n_outputs = trainers[0]->machine->outputs->frame_size;
  for(int i = 0; i < n_trainers; i++)
  {
    if(n_outputs != trainers[i]->machine->outputs->frame_size)
      error("WeightedSumMachine: provided machines don't have the same output size!");
  }
  outputs = new(allocator) Sequence(0, n_outputs);

  if(weights_)
    weights = weights_;
  else
  {
    weights = (real *)allocator->alloc(n_trainers*sizeof(real));
    for(int i = 0; i < n_trainers; i++)
      weights[i] = 0;
  }

  n_trainers_trained = 0;
}

void WeightedSumMachine::reset()
{
  for (int i=0;i<n_trainers;i++)
    trainers[i]->machine->reset();
  n_trainers_trained = 0;
}

void WeightedSumMachine::forward(Sequence *inputs)
{
  for(int i = 0; i < n_trainers_trained; i++)
    trainers[i]->machine->forward(inputs);

  int n_frames = trainers[0]->machine->outputs->n_frames;
  outputs->resize(n_frames);  
  for(int i = 0; i < n_frames; i++)
  {
    real *dest_ = outputs->frames[i];
    for(int j = 0; j < n_outputs; j++)
      dest_[j] = 0;
    
    for(int j = 0; j < n_trainers_trained; j++)
    {
      real z = weights[j];
      real *src_ = trainers[j]->machine->outputs->frames[i];
      for(int k = 0; k < n_outputs; k++)
        dest_[k] += z * src_[k];
    }
  }
}

void WeightedSumMachine::loadXFile(XFile *file)
{
  file->taggedRead(&n_trainers_trained, sizeof(int), 1, "NTRAINERS");
  file->taggedRead(weights, sizeof(real), n_trainers, "WEIGHTS");  
  for (int i = 0; i < n_trainers; i++)
    trainers[i]->loadXFile(file);
}

void WeightedSumMachine::saveXFile(XFile *file)
{
  file->taggedWrite(&n_trainers_trained, sizeof(int), 1, "NTRAINERS");
  file->taggedWrite(weights, sizeof(real), n_trainers, "WEIGHTS");
  for (int i = 0; i < n_trainers; i++)
    trainers[i]->saveXFile(file);
}

WeightedSumMachine::~WeightedSumMachine()
{
}

}
