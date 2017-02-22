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

#include "Multinomial.h"
#include "log_add.h"
#include "Random.h"

namespace Torch {

Multinomial::Multinomial(int n_values_) : Distribution(1,n_values_)
{
  n_values = n_values_;
	addROption("prior weights", &prior_weights , 1e-3, "minimum weights for each gaussians");
  addBOption("equal initialization", &equal_initialization , false, "equal initialization");

  log_weights = (real*)params->data[0];
  dlog_weights = (real*)der_params->data[0];
  weights_acc = (real*)allocator->alloc(sizeof(real)*n_values);
}

void Multinomial::setDataSet(DataSet* data_)
{
  // here, initialize the parameters somehow...

	real sum = 0.;
  if (equal_initialization) {
    // initialize the weights with equal values
      for (int i=0;i<n_values;i++)  {
        log_weights[i] = 1.0 / (float) n_values;
      sum += log_weights[i];
    }
  } else {
	  // initialize randomly the weights
	  for (int i=0;i<n_values;i++) {
		  log_weights[i] = Random::boundedUniform(0.1,1);
		  sum += log_weights[i];
	  }
  }
	for (int i=0;i<n_values;i++) {
		log_weights[i] = log(log_weights[i]/sum);
	}
}

void Multinomial::eMSequenceInitialize(Sequence* inputs)
{
  if (!inputs)
    return;
	log_probabilities->resize(inputs->n_frames);
}

void Multinomial::sequenceInitialize(Sequence* inputs)
{
  eMSequenceInitialize(inputs);
}

real Multinomial::frameLogProbability(int t, real *inputs)
{
  int obs = (int)inputs[0];
  if (obs < 0 || obs >= n_values)
    error("Multinomial::frameLogProbability observed an non-realistic value: %d\n",obs);
  real log_prob = log_weights[obs];
  log_probabilities->frames[t][0] = log_prob;
  return log_prob;
}

void Multinomial::frameEMAccPosteriors(int t, real *inputs, real log_posterior)
{
  int obs = (int)inputs[0];
  if (obs < 0 || obs >= n_values)
    error("Multinomial::frameEMAccPosteriors observed an non-realistic value: %d\n",obs);
  weights_acc[obs] += exp(log_posterior);
}

void Multinomial::eMUpdate()
{
  real* p_weights_acc = weights_acc;
  real sum_weights_acc = 0;
  for (int i=0;i<n_values;i++)
    sum_weights_acc += *p_weights_acc++;
  real *p_log_weights = log_weights;
  real log_sum = log(sum_weights_acc);
  p_weights_acc = weights_acc;
  for (int i=0;i<n_values;i++)
    *p_log_weights++ = log(*p_weights_acc++) - log_sum;
}

void Multinomial::eMIterInitialize()
{
  // initialize the accumulators to 0 and compute pre-computed value
  for (int i=0;i<n_values;i++) {
    weights_acc[i] = prior_weights;
  }
}

void Multinomial::iterInitialize()
{
}

void Multinomial::frameBackward(int t, real *f_inputs, real *beta_, real *f_outputs, real *alpha_)
{
  int obs = (int)f_inputs[0];
  if (obs < 0 || obs >= n_values)
    error("Multinomial::frameBackward observed an non-realistic value: %d\n",obs);
  dlog_weights[obs] += *alpha_;
  for (int i=0;i<n_values;i++) {
    dlog_weights[i] -= *alpha_ * exp(log_weights[i]);
    if (isnan(dlog_weights[i]))
      error("Multinomial::frameBackward dlog_weights (%d) is nan!",i);
  }
}

void Multinomial::update()
{
  // normalize log_weights
  real log_sum = LOG_ZERO;
  for (int i=0;i<n_values;i++)
    log_sum = logAdd(log_sum,log_weights[i]);
  for (int i=0;i<n_values;i++)
    log_weights[i] -= log_sum;
}

void Multinomial::frameDecision(int t, real *decision)
{
  for(int i = 0 ; i < n_values ; i++) {
    real class_target = (real)i;
    decision[i] = frameLogProbability(t, &class_target);
  }
}


Multinomial::~Multinomial()
{
}

}

