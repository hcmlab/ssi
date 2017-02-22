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


#include "Distribution.h"
#include "log_add.h"

namespace Torch {

Distribution::Distribution(int n_inputs_,int n_params_) : GradientMachine(n_inputs_,1,n_params_)
{
  log_probabilities = new(allocator)Sequence(1,n_outputs);
  outputs->resize(n_outputs);
}


real Distribution::logProbability(Sequence *inputs)
{
  real ll = 0;
  for (int i=0;i<inputs->n_frames;i++) {
    ll += frameLogProbability(i,inputs->frames[i]);
  }
  return ll;
}

real Distribution::viterbiLogProbability(Sequence *inputs)
{
  real ll = 0;
  for (int i=0;i<inputs->n_frames;i++) {
    ll += viterbiFrameLogProbability(i,inputs->frames[i]);
  }
  return ll;
}

real Distribution::viterbiFrameLogProbability(int t, real *inputs)
{
	return	frameLogProbability(t, inputs);
}

real Distribution::frameLogProbability(int t, real *inputs)
{
  return LOG_ZERO;
}

void Distribution::frameGenerate(int t, real *inputs)
{
}

void Distribution::iterInitialize()
{
  eMIterInitialize();
}

void Distribution::eMIterInitialize()
{
}

void Distribution::eMSequenceInitialize(Sequence* inputs)
{
  log_probabilities->resize(inputs->n_frames);
}

void Distribution::sequenceInitialize(Sequence* inputs)
{
  log_probabilities->resize(inputs->n_frames);
}

void Distribution::eMAccPosteriors(Sequence *inputs, real log_posterior)
{
  for (int i=0;i<inputs->n_frames;i++) {
    frameEMAccPosteriors(i, inputs->frames[i], log_posterior);
  }
}

void Distribution::viterbiAccPosteriors(Sequence *inputs, real log_posterior)
{
  for (int i=0;i<inputs->n_frames;i++) {
    frameViterbiAccPosteriors(i, inputs->frames[i], log_posterior);
  }
}

void Distribution::frameEMAccPosteriors(int t, real *inputs, real log_posterior)
{
}

void Distribution::frameViterbiAccPosteriors(int t, real *inputs, real log_posterior)
{
}

void Distribution::eMUpdate()
{
}

void Distribution::update()
{
}

void Distribution::decode(Sequence *inputs)
{
}

void Distribution::forward(Sequence *inputs)
{
 sequenceInitialize(inputs);
 log_probability = logProbability(inputs);
 outputs->frames[0][0] = log_probability;
}


void Distribution::eMForward(Sequence *inputs)
{
  eMSequenceInitialize(inputs);
  log_probability = logProbability(inputs);
}

void Distribution::viterbiForward(Sequence *inputs)
{
   eMSequenceInitialize(inputs);
   log_probability = viterbiLogProbability(inputs);
}

void Distribution::backward(Sequence *inputs, Sequence *alpha)
{
  for (int i=0;i<inputs->n_frames;i++) {
    frameBackward(i, inputs->frames[i], NULL, NULL, alpha->frames[0]);
  }
}

void Distribution::viterbiBackward(Sequence *inputs, Sequence *alpha)
{
  backward(inputs,alpha);
}


void Distribution::frameBackward(int t, real *f_inputs, real *beta_, real *f_outputs, real *alpha_)
{
}

void Distribution::loadXFile(XFile *file)
{
  params->loadXFile(file);
  eMIterInitialize();
}

void Distribution::decision(Sequence* decision)
{
  for (int i=0;i<decision->n_frames;i++) {
    frameDecision(i,decision->frames[i]);
  }
}

void Distribution::frameDecision(int t, real *decision)
{
}

Distribution::~Distribution()
{
}

}

