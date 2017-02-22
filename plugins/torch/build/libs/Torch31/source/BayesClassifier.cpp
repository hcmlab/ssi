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


#include "BayesClassifier.h"
#include "log_add.h"

namespace Torch {


BayesClassifier::BayesClassifier(BayesClassifierMachine* machine_) : Trainer(machine_)
{

  bayesmachine = (BayesClassifierMachine*)machine;
  n_classes = bayesmachine->n_trainers;
  
  classes = (int**) allocator->alloc(n_classes * sizeof(int*));
  for(int i = 0;i < n_classes ;i++)
    classes[i] = (int*) allocator->alloc(1 * sizeof(int));
  classes_n = (int*)allocator->alloc(n_classes * sizeof(int));
}

BayesClassifier::~BayesClassifier()
{
}

void BayesClassifier::train(DataSet *data, MeasurerList *measurers)
{
  message("BayesClassifier: Training");

  // attribute the classes
  for (int i=0;i<n_classes;i++) {
    classes_n[i] = 0;
    classes[i] = (int*) allocator->realloc(classes[i],data->n_examples * sizeof(int));
  }

  machine->setDataSet(data);

  for (int i=0;i<data->n_examples;i++) {
    data->setExample(i);
    int c = bayesmachine->class_format->getClass(data->targets->frames[0]);
    classes[c][classes_n[c]++] = i;
  }

  // eventually compute prior given training set
  if (bayesmachine->allocated_log_priors) {
    real log_n = log((real)data->n_examples);
    for (int i=0;i<n_classes;i++)
      if (classes_n[i] > 0)
        bayesmachine->log_priors[i] = log((real)classes_n[i]) - log_n;
      else
        bayesmachine->log_priors[i] = LOG_ZERO;
  }
  
  for(int c = 0;c < n_classes;c++) {
    data->pushSubset(classes[c],classes_n[c]);
    bayesmachine->trainers[c]->machine->reset();
    if (data->n_examples > 0) {
      if (bayesmachine->trainers_measurers)
        bayesmachine->trainers[c]->train(data,bayesmachine->trainers_measurers[c]);
      else
        bayesmachine->trainers[c]->train(data,NULL);
    } else {
      warning("BayesClassifier: there was no examples to train class %d",c);
    }
    data->popSubset();
  }

  if (measurers) {
    test(measurers);
  }
}


}

