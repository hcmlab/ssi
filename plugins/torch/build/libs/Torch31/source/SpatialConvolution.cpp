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

#include "SpatialConvolution.h"
#include "Random.h"

namespace Torch {

/* Bon. Pour info, j'ai essaye de coder une premiere version degeulasse, ou
   j'essayais de prendre en compte le cache de la becane. Ca m'a pris une demi
   journee, plus quelques heures de debuggage, et c'etait du code horrible.
   J'ai recode ce truc en 10 minutes. Ca a marche du premier coup. Et c'est
   plus rapide! Bordel! Alors vous prenez pas la tete...
*/

SpatialConvolution::SpatialConvolution(int n_input_planes_, int n_output_planes_, int width_, int height_, int k_w_, int d_x_, int d_y_)
  : GradientMachine(0, 0)
{
  n_input_planes = n_input_planes_;
  n_output_planes = n_output_planes_;
  input_width = width_;
  input_height = height_;
  k_w = k_w_;
  d_x = d_x_;
  d_y = d_y_;

  n_inputs = n_input_planes * input_height * input_width;
  output_height = (input_height - k_w) / d_y + 1;
  output_width = (input_width - k_w) / d_x + 1;
  n_outputs = n_output_planes * output_height * output_width;

  if(input_height < k_w)
    error("SpatialConvolution: input image height is too small (height = %d < k_w = %d) ", input_height, k_w);
  if(input_width < k_w)
    error("SpatialConvolution: input image width is too small (width = %d < k_w = %d) ", input_width, k_w);

  outputs = new(allocator) Sequence(1, n_outputs);
  beta = new(allocator) Sequence(1, n_inputs);

  int n_params_ = k_w*k_w*n_input_planes*n_output_planes+n_output_planes;
  params = new(allocator) Parameters(n_params_);
  der_params = new(allocator) Parameters(n_params_);

  weights = (real **)allocator->alloc(sizeof(real *)*n_output_planes);
  for(int i = 0; i < n_output_planes; i++)
    weights[i] = params->data[0] + i*k_w*k_w*n_input_planes;
  biases = params->data[0] + k_w*k_w*n_input_planes*n_output_planes;

  der_weights = (real **)allocator->alloc(sizeof(real *)*n_output_planes);
  for(int i = 0; i < n_output_planes; i++)
    der_weights[i] = der_params->data[0] + i*k_w*k_w*n_input_planes;
  der_biases = der_params->data[0] + k_w*k_w*n_input_planes*n_output_planes;

  message("SpatialConvolution: output image is <%d x %d>", output_width, output_height);

  reset_();
}

void SpatialConvolution::reset_()
{
  real bound = 1./sqrt((real)(k_w*k_w*n_input_planes));

  real *params_ = params->data[0];
  for(int i = 0; i < params->n_params; i++)
    params_[i] = Random::boundedUniform(-bound, bound);
}

void SpatialConvolution::reset()
{
  reset_();
}

void SpatialConvolution::frameForward(int t, real *f_inputs, real *f_outputs)
{
  for(int k = 0; k < n_output_planes; k++)
  {
    // Initialize to the bias
    real z = biases[k];
    for(int i = 0; i < output_width*output_height; i++)
      f_outputs[i] = z;

    // Go!

    for(int i = 0; i < n_input_planes; i++)
    {
      // Get the good mask for (k,i) (k out, i in)
      real *ptr_w = weights[k]+i*k_w*k_w;
      
      // Get the input image
      real *ptr_img_in = f_inputs+i*input_width*input_height;
      
      // For all output pixels...
      real *outputs_ = f_outputs;
      for(int yy = 0; yy < output_height; yy++)
      {
        for(int xx = 0; xx < output_width; xx++)
        {
          // Dot product in two dimensions... (between input image and the mask)
          real *ptr_img_in_ = ptr_img_in+yy*d_y*input_width+xx*d_x;
          real *ptr_w_ = ptr_w;
          real sum = 0;
          for(int ky = 0; ky < k_w; ky++)
          {
            for(int kx = 0; kx < k_w; kx++)
              sum += ptr_img_in_[kx]*ptr_w_[kx];
            ptr_img_in_ += input_width; // next input line
            ptr_w_ += k_w; // next mask line
          }
          
          // Update output
          *outputs_++ += sum;
        }
      }
    }

    // Next output plane
    f_outputs += output_width*output_height;
  }
}

void SpatialConvolution::frameBackward(int t, real *f_inputs, real *beta_, real *f_outputs, real *alpha_)
{
  //NOTE: boucle *necessaire* avec "partial backprop"

  real *alpha__ = alpha_;
  for(int k = 0; k < n_output_planes; k++)
  {
    real sum = 0;
    for(int i = 0; i < output_width*output_height; i++)
      sum += alpha__[i];
    der_biases[k] += sum;

    for(int i = 0; i < n_input_planes; i++)
    {
      real *der_ptr_w = der_weights[k] + i*k_w*k_w;
      real *ptr_img_in = f_inputs+i*input_width*input_height;
      real *alpha___ = alpha__;
      for(int yy = 0; yy < output_height; yy++)
      {
        for(int xx = 0; xx < output_width; xx++)
        {
          real *ptr_img_in_ = ptr_img_in+yy*d_y*input_width+xx*d_x;
          real *der_ptr_w_ = der_ptr_w;

          real z = *alpha___++;
          for(int ky = 0; ky < k_w; ky++)
          {
            for(int kx = 0; kx < k_w; kx++)
              der_ptr_w_[kx] += z * ptr_img_in_[kx];
            ptr_img_in_ += input_width;
            der_ptr_w_ += k_w;
          }
        }
      }
    }
    alpha__ += output_width*output_height;
  }


  if(partial_backprop)
    return;

  // NOTE: boucle *non-necessaire* avec "partial backprop"

  for(int k = 0; k < n_inputs; k++)
    beta_[k] = 0;
  
  alpha__ = alpha_;
  for(int k = 0; k < n_output_planes; k++)
  {
    for(int i = 0; i < n_input_planes; i++)
    {
      real *ptr_w = weights[k]+i*k_w*k_w;
      real *beta__ = beta_+i*input_width*input_height;
      real *alpha___ = alpha__;
      for(int yy = 0; yy < output_height; yy++)
      {
        for(int xx = 0; xx < output_width; xx++)
        {
          real *beta___ = beta__+yy*d_y*input_width+xx*d_x;
          real *ptr_w_ = ptr_w;

          real z = *alpha___++;
          for(int ky = 0; ky < k_w; ky++)
          {
            for(int kx = 0; kx < k_w; kx++)
              beta___[kx] += z * ptr_w_[kx];
            beta___ += input_width;
            ptr_w_ += k_w;
          }
        }
      }
    }
    alpha__ += output_width*output_height;
  }
}

SpatialConvolution::~SpatialConvolution()
{
}

}
