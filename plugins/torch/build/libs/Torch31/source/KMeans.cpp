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

#include "KMeans.h"
#include "log_add.h"
#include "Random.h"

namespace Torch {

KMeans::KMeans(int n_inputs_, int n_gaussians_) : DiagonalGMM(n_inputs_, n_gaussians_)
{
  min_cluster = new (allocator)Sequence(1,1);
	addBOption("intitialize parameters", &initialize_parameters , true, "initialize the kmeans parameters from the data");
}

void KMeans::setDataSet(DataSet* data_)
{
  // initialize the parameters using some examples in the dataset randomly
	int tot_n_frames = 0;
	int* example_size = (int*) allocator->alloc(sizeof(int)*data_->n_examples);
	int* ex_s = example_size;
	for(int i=0; i<data_->n_examples; i++){
		data_->getNumberOfFrames(i, ex_s, NULL);
		tot_n_frames += *ex_s++;
	}
/*
	if(tot_n_frames < n_gaussians)
		error("The number of frame: %d is smaller than the number of gaussians: %d",tot_n_frames, n_gaussians);
*/
  int n_part = (int)(tot_n_frames/(real)n_gaussians);

	int sum = 0;
	int ex = 0;
  for (int i=0;i<n_gaussians;i++) {
    int from = (int)(i*n_part);
    int to = (int)((i+1)*n_part);
    int diff = max(to - from,1);
		int index = (int)from + (int)(Random::uniform()*(real)diff);
		while(sum <= index){
			sum += example_size[ex++];
		}
		sum -= example_size[--ex];
		data_->setExample(ex);
    real *x = data_->inputs->frames[index - sum];
    real *means_i = means[i];
    real *var_i = var[i];
    real *thresh = var_threshold;
    for(int j = 0; j < n_inputs; j++) {
      *means_i++ = *x++;
      *var_i++ = *thresh++;
    }
    log_weights[i] = log(1./n_gaussians);
  }
	allocator->free(example_size);
}


void KMeans::eMIterInitialize()
{
  // initialize the accumulators to 0
  for (int i=0;i<n_gaussians;i++) {
    real *pm = means_acc[i];
    real *ps = var_acc[i];
    for (int j=0;j<n_inputs;j++) {
      *pm++ = 0.;
      *ps++ = 0.;
    }
    weights_acc[i] = prior_weights;
  }
}

void KMeans::eMSequenceInitialize(Sequence* inputs)
{
	min_cluster->resize(inputs->n_frames);
  DiagonalGMM::eMSequenceInitialize(inputs);
}

real KMeans::frameLogProbability(int t, real *inputs)
{
  real min_dist = INF;
  int min_i = -1;
  for (int i=0;i<n_gaussians;i++) {
    real dist = 0;
    real* means_i = means[i];
    real *x = inputs;
    for(int j = 0; j < n_inputs; j++) {
      real diff = *x++ - *means_i++;
      dist += diff*diff;
    }
    if (dist < min_dist) {
      min_dist = dist;
      min_i = i;
    }
  }
  log_probabilities->frames[t][0] = -min_dist;
  min_cluster->frames[t][0] = (real)min_i;
  return -min_dist;
}

real KMeans::frameLogProbabilityOneGaussian(int g, real *inputs)
{ 
  real dist = 0;
  real* means_g = means[g];
  real *x = inputs;
  for(int j = 0; j < n_inputs; j++) {
    real diff = *x++ - *means_g++;
    dist += diff*diff;
  }
  return dist;
}

void KMeans::frameEMAccPosteriors(int t, real *inputs, real log_posterior)
{
  int min_i = (int)min_cluster->frames[t][0];
  real* means_acc_i = means_acc[min_i];
  real* var_acc_i = var_acc[min_i];
  real *x = inputs;
  for(int j = 0; j < n_inputs; j++) {
    *var_acc_i++ += *x * *x;
    *means_acc_i++ += *x++;
  }
  weights_acc[min_i] ++;
}

void KMeans::frameBackward(int t, real *f_inputs, real *beta_, real *f_outputs, real *alpha_)
{
  int min_i = (int)min_cluster->frames[t][0];
  real* min_means = means[min_i];
  real* min_dmeans = dmeans[min_i];
  for (int i=0;i<n_inputs;i++) {
    min_dmeans[i] += (f_inputs[i] - min_means[i]) * *alpha_;
  }
}

void KMeans::eMUpdate()
{
   // first the weights and var
  real* p_weights_acc = weights_acc;
  for (int i=0;i<n_gaussians;i++,p_weights_acc++) {
    if (*p_weights_acc == 0) {
      warning("Gaussian %d of KMeans is not used in EM",i);
    } else {
      real* p_means_i = means[i];
      real* p_var_i = var[i];
      real* p_means_acc_i = means_acc[i];
      real* p_var_acc_i = var_acc[i];
      for (int j=0;j<n_inputs;j++,p_means_i++) {
        *p_means_i = *p_means_acc_i++ / *p_weights_acc;
        real v = *p_var_acc_i++ / *p_weights_acc - *p_means_i * *p_means_i;
        *p_var_i++ = v >= var_threshold[j] ? v : var_threshold[j];
      }
    }
  }
  // then the weights
  real sum_weights_acc = 0;
  p_weights_acc = weights_acc;
  for (int i=0;i<n_gaussians;i++)
    sum_weights_acc += *p_weights_acc++;
  if (sum_weights_acc == 0)
    warning("the posteriors of weights of KMeans are not used");
  else {
    real *p_log_weights = log_weights;
    real log_sum = log(sum_weights_acc);
    p_weights_acc = weights_acc;
    for (int i=0;i<n_gaussians;i++)
      *p_log_weights++ = log(*p_weights_acc++) - log_sum;
  }
}

KMeans::~KMeans()
{
}

}

