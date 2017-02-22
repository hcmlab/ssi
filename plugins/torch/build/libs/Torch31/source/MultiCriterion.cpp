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

#include "MultiCriterion.h"

namespace Torch {

MultiCriterion::MultiCriterion(Criterion** criterions_,int n_criterions_, real* weights_) : Criterion(criterions_[0]->n_inputs)
{
  n_criterions = n_criterions_;
  criterions = criterions_;
  weights = weights_;
  for(int i = 0; i < n_criterions; i++)
  {
    if(n_inputs != criterions[i]->n_inputs)
      error("MultiCriterion: given criterions don't have the same number of inputs");
  }

  if(!weights)
  {
    weights = (real *)allocator->alloc(sizeof(real)*n_criterions);
    for(int i = 0; i < n_criterions; i++)
      weights[i] = 1.;
  }
}

void MultiCriterion::setDataSet(DataSet *data_)
{
  data = data_;
  for (int i=0;i<n_criterions;i++)
    criterions[i]->setDataSet(data_);
}

void MultiCriterion::iterInitialize()
{
  for (int i=0;i<n_criterions;i++)
    criterions[i]->iterInitialize();
}

void MultiCriterion::reset()
{
  for (int i=0;i<n_criterions;i++)
    criterions[i]->reset();
}

void MultiCriterion::forward(Sequence *inputs)
{
  for(int i = 0; i < n_criterions; i++)
    criterions[i]->forward(inputs);

  int n_frames_ = criterions[0]->outputs->n_frames;
  outputs->resize(n_frames_);

  for(int i = 0; i < n_frames_; i++)
  {
    real sum = 0;
    for(int j = 0; j < n_criterions; j++)
      sum += criterions[j]->outputs->frames[i][0] * weights[j];
    outputs->frames[i][0] = sum;
  }
}

void MultiCriterion::backward(Sequence *inputs, Sequence *alpha)
{
  int n_frames_ = inputs->n_frames;
  beta->resize(n_frames_);

  for(int i = 0; i < n_criterions; i++)
    criterions[i]->backward(inputs, alpha);

  for(int i = 0; i < n_frames_; i++)
  {
    real *beta_ = beta->frames[i];
    for(int j = 0; j < n_inputs; j++)
      beta_[j] = 0;

    for(int j = 0; j < n_criterions; j++)
    {
      real *src_ = criterions[j]->beta->frames[i];
      real w_ = weights[j];
      for(int k = 0; k < n_inputs; k++)
        beta_[k] += src_[k] * w_;
    }
  }
}

MultiCriterion::~MultiCriterion()
{
}

}
