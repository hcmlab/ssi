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

#include "SpatialSubSampling.h"
#include "Random.h"

namespace Torch {

SpatialSubSampling::SpatialSubSampling(int n_input_planes_, int width_, int height_, int k_w_, int d_x_, int d_y_)
  : GradientMachine(0, 0)
{
  n_input_planes = n_input_planes_;
  input_width = width_;
  input_height = height_;
  k_w = k_w_;
  d_x = d_x_;
  d_y = d_y_;

  n_inputs = n_input_planes * input_height * input_width;
  output_height = (input_height - k_w) / d_y + 1;
  output_width = (input_width - k_w) / d_x + 1;
  n_outputs = n_input_planes * output_height * output_width;

  if(input_height < k_w)
    error("SpatialConvolution: input image height is too small (height = %d < k_w = %d) ", input_height, k_w);
  if(input_width < k_w)
    error("SpatialConvolution: input image width is too small (width = %d < k_w = %d) ", input_width, k_w);

  outputs = new(allocator) Sequence(1, n_outputs);
  beta = new(allocator) Sequence(1, n_inputs);

  int n_params_ = 2*n_input_planes;
  params = new(allocator) Parameters(n_params_);
  der_params = new(allocator) Parameters(n_params_);

  weights = params->data[0];
  biases = params->data[0] + n_input_planes;

  der_weights = der_params->data[0];
  der_biases = der_params->data[0] + n_input_planes;

  message("SpatialSubSampling: output image is <%d x %d>", output_width, output_height);

  reset_();
}

void SpatialSubSampling::reset_()
{
  real bound = 1./sqrt((real)(k_w*k_w));

  real *params_ = params->data[0];
  for(int i = 0; i < params->n_params; i++)
    params_[i] = Random::boundedUniform(-bound, bound);
}

void SpatialSubSampling::reset()
{
  reset_();
}

void SpatialSubSampling::frameForward(int t, real *f_inputs, real *f_outputs)
{
  for(int k = 0; k < n_input_planes; k++)
  {
    // Initialize to the bias
    real z = biases[k];
    for(int i = 0; i < output_width*output_height; i++)
      f_outputs[i] = z;

    // Go!

    // Get the good mask for (k,i) (k out, i in)
    real the_weight = weights[k];
      
    // For all output pixels...
    real *outputs_ = f_outputs;
    for(int yy = 0; yy < output_height; yy++)
    {
      for(int xx = 0; xx < output_width; xx++)
      {
        // Compute the mean of the input image...
        real *ptr_img_in = f_inputs+yy*d_y*input_width+xx*d_x;
        real sum = 0;
        for(int ky = 0; ky < k_w; ky++)
        {
          for(int kx = 0; kx < k_w; kx++)
            sum += ptr_img_in[kx];
          ptr_img_in += input_width; // next input line
        }
        
        // Update output
        *outputs_++ += the_weight*sum;
      }
    }

    // Next input/output plane
    f_outputs += output_width*output_height;
    f_inputs += input_width*input_height;
  }
}

void SpatialSubSampling::frameBackward(int t, real *f_inputs, real *beta_, real *f_outputs, real *alpha_)
{
  // NOTE: boucle *necessaire* avec "partial backprop"

  real *alpha__ = alpha_;
  for(int k = 0; k < n_input_planes; k++)
  {
    real sum = 0;
    for(int i = 0; i < output_width*output_height; i++)
      sum += alpha__[i];
    der_biases[k] += sum;

    real *alpha___ = alpha__;
    sum = 0;
    for(int yy = 0; yy < output_height; yy++)
    {
      for(int xx = 0; xx < output_width; xx++)
      {
        real *ptr_img_in = f_inputs+yy*d_y*input_width+xx*d_x;
        real z = *alpha___++;
        for(int ky = 0; ky < k_w; ky++)
        {
          for(int kx = 0; kx < k_w; kx++)
            sum += z * ptr_img_in[kx];
          ptr_img_in += input_width;
        }    
      }
    }
    der_weights[k] += sum;
    alpha__ += output_width*output_height;
    f_inputs += input_width*input_height;
  }

  if(partial_backprop)
    return;

  // NOTE: boucle *non-necessaire* avec "partial backprop"

  for(int k = 0; k < n_inputs; k++)
    beta_[k] = 0;
  
  alpha__ = alpha_;
  for(int k = 0; k < n_input_planes; k++)
  {
    real the_weight = weights[k];
    real *alpha___ = alpha__;
    for(int yy = 0; yy < output_height; yy++)
    {
      for(int xx = 0; xx < output_width; xx++)
      {
        real *beta__ = beta_+yy*d_y*input_width+xx*d_x;
        real z = *alpha___++ * the_weight;
        for(int ky = 0; ky < k_w; ky++)
        {
          for(int kx = 0; kx < k_w; kx++)
            beta__[kx] += z;
          beta__ += input_width;
        }    
      }
    }
    alpha__ += output_width*output_height;
    beta_ += input_width*input_height;
  }
}

SpatialSubSampling::~SpatialSubSampling()
{
}

}
