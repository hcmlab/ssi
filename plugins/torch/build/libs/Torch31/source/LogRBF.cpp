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

#include "LogRBF.h"
#include "Random.h"
#include "KMeans.h"

namespace Torch {

LogRBF::LogRBF(int n_inputs_, int n_outputs_, EMTrainer* kmeans_trainer_) : GradientMachine(n_inputs_, n_outputs_, 2*n_inputs_*n_outputs_)
{
  initial_kmeans_trainer = kmeans_trainer_;
  mu = params->data[0];
  gamma = params->data[0] + n_inputs*n_outputs;
  der_mu = der_params->data[0];
  der_gamma = der_params->data[0] + n_inputs*n_outputs;
}

void LogRBF::setDataSet(DataSet* data_)
{
  if(initial_kmeans_trainer)
  {
    initial_kmeans_trainer->train(data_, NULL);

    KMeans *kmeans = (KMeans *)initial_kmeans_trainer->distribution;
    for(int i = 0; i < n_outputs; i++)
    {
      real *src = kmeans->means[i];
      real *dest = mu + i*n_inputs;
      for(int j = 0; j < n_inputs; j++)
        dest[j] = src[j];
    }
  }
  else
  {  
    for(int i = 0; i < n_inputs*n_outputs; i++)
      mu[i] = Random::uniform();
  }

  for(int i = 0; i < n_inputs*n_outputs; i++)
    gamma[i] = 1./sqrt((real)n_inputs);
}

void LogRBF::frameForward(int t, real *f_inputs, real *f_outputs)
{
  real *mu_ = mu;
  real *gamma_ = gamma;
  for(int i = 0; i < n_outputs; i++)
  {
    real out = 0;
    for(int j = 0; j < n_inputs; j++)
    {
      real z = (f_inputs[j] - mu_[j]) * gamma_[j];
      out += z*z;
    }
    f_outputs[i] = -0.5*out;
    mu_ += n_inputs;
    gamma_ += n_inputs;
  }
}

void LogRBF::frameBackward(int t, real *f_inputs, real *beta_, real *f_outputs, real *alpha_)
{
  for(int i = 0; i < n_inputs; i++)
    beta_[i] = 0;

  real *mu_ = mu;
  real *gamma_ = gamma;
  real *der_mu_ = der_mu;
  real *der_gamma_ = der_gamma;
  for(int i = 0; i < n_outputs; i++)
  {
    real z = alpha_[i];
    for(int j = 0; j < n_inputs; j++)
    {
      real gamma__ = gamma_[j];
      real diff = f_inputs[j] - mu_[j];

      real zz = z * diff * gamma__ * gamma__;
      der_mu_[j] += zz;
      beta_[j] -= zz;

      der_gamma_[j] -= z * diff*diff * gamma__;
    }
    mu_ += n_inputs;
    gamma_ += n_inputs;
    der_mu_ += n_inputs;
    der_gamma_ += n_inputs;
  }
}

LogRBF::~LogRBF()
{
}

}
