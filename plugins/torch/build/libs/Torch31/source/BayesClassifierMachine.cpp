// Copyright (C) 2003--2004 Samy Bengio (bengio@idiap.ch)
//                and Bison Ravi (francois.belisle@idiap.ch)
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


#include "BayesClassifierMachine.h"

namespace Torch {

BayesClassifierMachine::BayesClassifierMachine(EMTrainer** trainers_, int n_trainers_, MeasurerList** trainers_measurers_, ClassFormat* class_format_,real* log_priors_)
{
  allocated_log_priors = false;

  trainers = trainers_;
  n_trainers = n_trainers_;
  trainers_measurers = trainers_measurers_;
  class_format = class_format_;

  log_probabilities = new(allocator) Sequence(1,n_trainers);

  //if we are not given any log_prior class probabilities, 
  //then we will assume training set proportions.
  
  if(log_priors_ != NULL)
    log_priors = log_priors_;
  else {
    allocated_log_priors = true;
    log_priors = (real*) allocator->alloc (n_trainers * sizeof(real));
    //as a first approximation
    for(int i = 0;i < n_trainers;i++)
      log_priors[i] = -log((real)n_trainers);
  }

  n_outputs = class_format->getOutputSize();
  outputs = new(allocator)Sequence(1,n_outputs);
}
 
BayesClassifierMachine::~BayesClassifierMachine()
{
}

void BayesClassifierMachine::forward(Sequence *inputs)
{

  for(int trainer = 0;trainer < n_trainers;trainer++) {
    trainers[trainer]->machine->forward(inputs);
    log_probabilities->frames[0][trainer] = 
      trainers[trainer]->distribution->log_probability + log_priors[trainer];
  }
  //transform the output from one_hot representation to class_format
  class_format->fromOneHot(outputs->frames[0],log_probabilities->frames[0]);
}

void BayesClassifierMachine::reset()
{
/* I think this is not necessary as it is done in the train method...
  for(int i = 0;i < n_trainers;i++)
    trainers[i]->machine->reset();
*/
}

void BayesClassifierMachine::loadXFile(XFile* file)
{
  for(int i = 0;i < n_trainers;i++)
    trainers[i]->loadXFile(file);
}

void BayesClassifierMachine::saveXFile(XFile* file)
{
  for(int i = 0;i < n_trainers;i++)
    trainers[i]->saveXFile(file);
}


}

