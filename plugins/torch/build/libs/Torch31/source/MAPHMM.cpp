// Copyright (C) 2003--2004 Johnny Mariethoz (marietho@idiap.ch)
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

#include "MAPHMM.h"
#include "log_add.h"

namespace Torch {

MAPHMM::MAPHMM(int n_states_, Distribution **states_, real** transitions_,HMM* prior_distribution_) : 
	HMM(n_states_,states_, transitions_){
  prior_distribution = prior_distribution_;
	setWeightOnPrior(0.5);
}

void MAPHMM::setWeightOnPrior(real weight_on_prior_){
	weight_on_prior = weight_on_prior_;
  log_weight_on_prior = log(weight_on_prior);
  log_1_weight_on_prior = log(1-weight_on_prior);
}

void MAPHMM::eMUpdate()
{
  // first the states
  for (int i=1;i<n_states-1;i++) {
    states[i]->eMUpdate();
  }
  // then the transitions;
  for (int i=0;i<n_states-1;i++) {
    real sum_trans_acc = 0;
    for (int j=0;j<n_states;j++) {
      if (log_transitions[j][i] == LOG_ZERO)
        continue;
      sum_trans_acc += transitions_acc[j][i];
    }
    real log_sum = log(sum_trans_acc);
    for (int j=0;j<n_states;j++) {
      if (log_transitions[j][i] == LOG_ZERO)
        continue;
      log_transitions[j][i] = logAdd(log_weight_on_prior + prior_distribution->log_transitions[j][i],
	                              log_1_weight_on_prior + log(transitions_acc[j][i]) - log_sum);
    }
  }
}

MAPHMM::~MAPHMM()
{
}

}

