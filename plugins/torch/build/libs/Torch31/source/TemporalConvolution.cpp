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

#include "TemporalConvolution.h"
#include "Random.h"

namespace Torch {

TemporalConvolution::TemporalConvolution(int input_frame_size, int output_frame_size, int k_w_, int d_t_)
  : GradientMachine(input_frame_size, output_frame_size, (k_w_*input_frame_size+1)*output_frame_size)
{
  k_w = k_w_;
  d_t = d_t_;

  weights = (real **)allocator->alloc(sizeof(real *)*n_outputs);
  for(int i = 0; i < n_outputs; i++)
    weights[i] = params->data[0] + i*k_w*n_inputs;
  biases = params->data[0] + k_w*n_inputs*n_outputs;

  der_weights = (real **)allocator->alloc(sizeof(real *)*n_outputs);
  for(int i = 0; i < n_outputs; i++)
    der_weights[i] = der_params->data[0] + i*k_w*n_inputs;
  der_biases = der_params->data[0] + k_w*n_inputs*n_outputs;

  reset_();
}

void TemporalConvolution::reset_()
{
  real bound = 1./sqrt((real)(k_w*n_inputs));

  real *params_ = params->data[0];
  for(int i = 0; i < params->n_params; i++)
    params_[i] = Random::boundedUniform(-bound, bound);
}

void TemporalConvolution::reset()
{
  reset_();
}

void TemporalConvolution::forward(Sequence *inputs)
{
  if(inputs->n_frames < k_w)
    error("TemporalSubSampling: input sequence too small! (n_frames = %d < k_w = %d)", inputs->n_frames, k_w);

  int n_output_frames = (inputs->n_frames - k_w) / d_t + 1;
  outputs->resize(n_output_frames);

  int current_input_frame = 0;
  for(int i = 0; i < n_output_frames; i++)
  {
    real *output_frame_ = outputs->frames[i];
    for(int j = 0; j < n_outputs; j++)
      output_frame_[j] = biases[j];

    // Sur le noyau...
    for(int j = 0; j < k_w; j++)
    {
      // Sur tous les "neurones" de sorties
      for(int k = 0; k < n_outputs; k++)
      {
        real *weights_ = weights[k]+j*n_inputs;
        real *input_frame_ = inputs->frames[current_input_frame+j];

        real sum = 0;
        for(int l = 0; l < n_inputs; l++)
          sum += weights_[l]*input_frame_[l];

        output_frame_[k] += sum;
      }
    }
    current_input_frame += d_t;
  }
}

void TemporalConvolution::backward(Sequence *inputs, Sequence *alpha)
{
  int n_output_frames = alpha->n_frames;

  // NOTE: boucle *necessaire* avec "partial backprop"

  int current_input_frame = 0;
  for(int i = 0; i < n_output_frames; i++)
  {
    real *alpha_frame_ = alpha->frames[i];
    for(int j = 0; j < n_outputs; j++)
      der_biases[j] += alpha_frame_[j];
    
    for(int j = 0; j < k_w; j++)
    {
      for(int k = 0; k < n_outputs; k++)
      {
        real *der_weights_ = der_weights[k]+j*n_inputs;
        real *input_frame_ = inputs->frames[current_input_frame+j];

        real alpha_ = alpha_frame_[k];
        for(int l = 0; l < n_inputs; l++)
          der_weights_[l] += alpha_*input_frame_[l];
      }
    }
    current_input_frame += d_t;
  }

  if(partial_backprop)
    return;

  // NOTE: boucle *non-necessaire* avec "partial backprop"

  beta->resize(inputs->n_frames);
  for(int i = 0; i < beta->n_frames; i++)
  {
    real *beta_frame_ = beta->frames[i];
    for(int j = 0; j < n_inputs; j++)
      beta_frame_[j] = 0;
  }

  int current_beta_frame = 0;
  for(int i = 0; i < n_output_frames; i++)
  {
    real *alpha_frame_ = alpha->frames[i];
    for(int j = 0; j < k_w; j++)
    {
      for(int k = 0; k < n_outputs; k++)
      {
        real *weights_ = weights[k]+j*n_inputs;
        real *beta_frame_ = beta->frames[current_beta_frame+j];

        real alpha_ = alpha_frame_[k];
        for(int l = 0; l < n_inputs; l++)
           beta_frame_[l] += weights_[l]*alpha_;
      }
    }
    current_beta_frame += d_t;
  }
}

TemporalConvolution::~TemporalConvolution()
{
}

}
