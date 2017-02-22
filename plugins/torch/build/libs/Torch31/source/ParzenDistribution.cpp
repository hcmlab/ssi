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

#include "ParzenDistribution.h"
#include "log_add.h"
#include "DataSet.h"

namespace Torch {

ParzenDistribution::ParzenDistribution(int n_inputs_, real var_) : Distribution(n_inputs_,0)
{
  data = NULL;

  setVar(var_);

  n_train_examples_index = 0;
  train_examples_index = NULL;
}

void ParzenDistribution::setVar(real var_)
{
  var = var_;

  sum_log_var_plus_n_obs_log_2_pi = -0.5 * n_inputs*(LOG_2_PI + log(var));
  minus_half_over_var = -0.5 / var;
}

void ParzenDistribution::setDataSet(DataSet* dataset_)
{
  data = dataset_;
  n_train_examples_index = data->n_examples;
  train_examples_index = (int*)allocator->realloc(train_examples_index,n_train_examples_index*sizeof(int));
  for (int i=0;i<data->n_examples;i++) {
    train_examples_index[i] = data->selected_examples[i];
  }
}

void ParzenDistribution::eMSequenceInitialize(Sequence* inputs)
{
  if (!inputs)
    return;
	log_probabilities->resize(inputs->n_frames);
}

void ParzenDistribution::sequenceInitialize(Sequence* inputs)
{
  eMSequenceInitialize(inputs);
}

real ParzenDistribution::frameLogProbability(int t, real *inputs)
{
  // first keep the current pointers...
  Sequence *current_seq = data->inputs;
  data->pushExample();

  // then compute the likelihood...
  real lp = 0;
  int tot_n_frames = 0;
  int *i_ptr = train_examples_index;
  for (int i=0;i<n_train_examples_index;i++) {
    data->setRealExample(*i_ptr++);
    Sequence* seq = data->inputs;
    tot_n_frames += seq->n_frames;
    for (int j=0;j<seq->n_frames;j++) {
      real lp_ij = frameLogProbabilityOneFrame(seq->frames[j],current_seq->frames[t]);
      lp += lp_ij;
    }
  }
  lp -= log((real)tot_n_frames);
	log_probabilities->frames[t][0] = lp;

  // restore the dataset status
  data->popExample();

  return lp;
}

real ParzenDistribution::frameLogProbabilityOneFrame(real *inputs, real *mean)
{
  real sum_xmu = 0.;
  real *x = inputs;
  real *m = mean;
  for(int j = 0; j < n_inputs; j++) {
    real xmu = (*x++ - *m++);
    sum_xmu += xmu*xmu;
  }
  real lp = sum_xmu*minus_half_over_var + sum_log_var_plus_n_obs_log_2_pi;
  return lp;
}

ParzenDistribution::~ParzenDistribution()
{
}

}

