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

#include "MeanVarNorm.h"
#include "XFile.h"

namespace Torch {

// added 2007/10/11
// Johannes Wagner <wagner@hcm-lab.de>
MeanVarNorm::MeanVarNorm(int n_inputs_, int n_targets_)
{
  inputs_mean = NULL;
  inputs_stdv = NULL;
  targets_mean = NULL;
  targets_stdv = NULL;
  n_inputs = n_inputs_;
  n_targets = n_targets_;

  if(n_inputs > 0)
  {
    inputs_mean = (real *)allocator->alloc(sizeof(real)*n_inputs);
    inputs_stdv = (real *)allocator->alloc(sizeof(real)*n_inputs);
    for(int i = 0; i < n_inputs; i++)
    {
      inputs_mean[i] = 0;
      inputs_stdv[i] = 0;
    }
  }

  if(n_targets > 0)
  {
    targets_mean = (real *)allocator->alloc(sizeof(real)*n_targets);
    targets_stdv = (real *)allocator->alloc(sizeof(real)*n_targets);
    for(int i = 0; i < n_targets; i++)
    {
      targets_mean[i] = 0;
      targets_stdv[i] = 0;
    }
  }
}

MeanVarNorm::MeanVarNorm(DataSet *data, bool norm_inputs, bool norm_targets)
{
  inputs_mean = NULL;
  inputs_stdv = NULL;
  targets_mean = NULL;
  targets_stdv = NULL;
  n_inputs = data->n_inputs;
  n_targets = data->n_targets;

  if(norm_inputs)
  {
    inputs_mean = (real *)allocator->alloc(sizeof(real)*n_inputs);
    inputs_stdv = (real *)allocator->alloc(sizeof(real)*n_inputs);
    for(int i = 0; i < n_inputs; i++)
    {
      inputs_mean[i] = 0;
      inputs_stdv[i] = 0;
    }
  }

  if(norm_targets)
  {
    targets_mean = (real *)allocator->alloc(sizeof(real)*n_targets);
    targets_stdv = (real *)allocator->alloc(sizeof(real)*n_targets);
    for(int i = 0; i < n_targets; i++)
    {
      targets_mean[i] = 0;
      targets_stdv[i] = 0;
    }
  }

  int n_total_input_frames = 0;
  int n_total_target_frames = 0;
  for(int t = 0; t < data->n_examples; t++)
  {
    data->setExample(t);

    // Les inputs
    if(norm_inputs)
    {
      for(int i = 0; i < data->inputs->n_frames; i++)
      {
        real *src_ = data->inputs->frames[i];
        for(int j = 0; j < n_inputs; j++)
        {
          real z = src_[j];
          inputs_mean[j] += z;
          inputs_stdv[j] += z*z;
        }
      }
      n_total_input_frames += data->inputs->n_frames;
    }

    // Les targets
    if(norm_targets)
    {
      for(int i = 0; i < data->targets->n_frames; i++)
      {
        real *src_ = data->targets->frames[i];
        for(int j = 0; j < n_targets; j++)
        {
          real z = src_[j];
          targets_mean[j] += z;
          targets_stdv[j] += z*z;
        }
      }
      n_total_target_frames += data->targets->n_frames;
    }
  }

  if(norm_inputs)
  {
    for(int i = 0; i < n_inputs; i++)
    {
      inputs_mean[i] /= (real)n_total_input_frames;
      inputs_stdv[i] /= (real)n_total_input_frames;
      inputs_stdv[i] -= inputs_mean[i]*inputs_mean[i];
      if(inputs_stdv[i] <= 0)
      {
        warning("MeanVarNorm: input column %d has a null stdv. Replaced by 1.", i);
        inputs_stdv[i] = 1.;
      }
      else
        inputs_stdv[i] = sqrt(inputs_stdv[i]);
    }
  }

  if(norm_targets)
  {
    for(int i = 0; i < n_targets; i++)
    {
      targets_mean[i] /= (real)n_total_target_frames;
      targets_stdv[i] /= (real)n_total_target_frames;
      targets_stdv[i] -= targets_mean[i]*targets_mean[i];
      if(targets_stdv[i] <= 0)
      {
        warning("MeanVarNorm: target column %d has a null stdv. Replaced by 1.", i);
        targets_stdv[i] = 1.;
      }
      else
        targets_stdv[i] = sqrt(targets_stdv[i]);
    }
  }
}

void MeanVarNorm::normalizeSequence(Sequence *sequence, real *mean, real *stdv)
{
  for(int i = 0; i < sequence->n_frames; i++)
  {
    real *ptr_ = sequence->frames[i];
    for(int k = 0; k < sequence->frame_size; k++)
      ptr_[k] = (ptr_[k] - mean[k])/stdv[k];
  }
}

void MeanVarNorm::preProcessInputs(Sequence *inputs)
{
  if(!inputs_mean)
    return;

  normalizeSequence(inputs, inputs_mean, inputs_stdv);
}

void MeanVarNorm::preProcessTargets(Sequence *targets)
{
  if(!targets_mean)
    return;

  normalizeSequence(targets, targets_mean, targets_stdv);
}

void MeanVarNorm::loadXFile(XFile *file)
{
  if(inputs_mean)
  {
    file->taggedRead(inputs_mean, sizeof(real), n_inputs, "IMEANS");
    file->taggedRead(inputs_stdv, sizeof(real), n_inputs, "ISTDVS");
  }
  if(targets_mean)
  {
    file->taggedRead(targets_mean, sizeof(real), n_targets, "TMEANS");
    file->taggedRead(targets_stdv, sizeof(real), n_targets, "TSTDVS");
  }
}

void MeanVarNorm::saveXFile(XFile *file)
{
  if(inputs_mean)
  {
    file->taggedWrite(inputs_mean, sizeof(real), n_inputs, "IMEANS");
    file->taggedWrite(inputs_stdv, sizeof(real), n_inputs, "ISTDVS");
  }
  if(targets_mean)
  {
    file->taggedWrite(targets_mean, sizeof(real), n_targets, "TMEANS");
    file->taggedWrite(targets_stdv, sizeof(real), n_targets, "TSTDVS");
  }
}

MeanVarNorm::~MeanVarNorm()
{
}

}
