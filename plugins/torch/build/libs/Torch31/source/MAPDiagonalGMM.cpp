// Copyright (C) 2003--2004 Johnny Mariethoz (Johnny.Mariethoz@idiap.ch)
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

#include "MAPDiagonalGMM.h"
#include "log_add.h"

namespace Torch {

MAPDiagonalGMM::MAPDiagonalGMM(DiagonalGMM* prior_distribution_) : DiagonalGMM(prior_distribution_->n_inputs, prior_distribution_->n_gaussians)
{
  prior_distribution = prior_distribution_;
	addROption("weight on prior", &weight_on_prior , 0.5, "weight for the prior distribution for MAP adaptation");
  addBOption("learn weights", &learn_weights, false, "learn the weights of gaussians");
  addBOption("learn variances", &learn_variances, false, "learn the variances of gaussians");
  addBOption("learn means", &learn_means, false, "learn the variances of gaussians");
}

void MAPDiagonalGMM::setDataSet(DataSet* data_)
{
  // here, initialize the parameters to the parameters of the prior
  // distribution
  if (prior_distribution)
		params->copy(prior_distribution->params);
  else
    DiagonalGMM::reset();
}

void MAPDiagonalGMM::frameViterbiAccPosteriors(int t, real *inputs, real log_posterior)
{
	if(learn_variances){
		DiagonalGMM::frameViterbiAccPosteriors(t, inputs, log_posterior);
		return;
	}
	real *p_weights_acc = weights_acc;
	real *lp_i = log_probabilities_g->frames[t];
	real *log_w_i = log_weights;
	real max_lpg = LOG_ZERO;
	int best_g = 0;
	//findmax
	for (int i=0;i<n_gaussians;i++) {
		real post_i =  *log_w_i++ + *lp_i++;
		if(post_i > max_lpg){
			best_g = i;
			max_lpg = post_i;
		}
	}
	p_weights_acc[best_g] += 1;
	real* means_acc_i = means_acc[best_g];
	real *x = inputs;
	for(int j = 0; j < n_inputs; j++) {
		*means_acc_i++ +=  *x++;
	}
}


void MAPDiagonalGMM::frameEMAccPosteriors(int t, real *inputs, real log_posterior)
{
	if(learn_variances){
	  DiagonalGMM::frameEMAccPosteriors(t, inputs, log_posterior);
	  return;
	}

  real log_prob = log_probabilities->frames[t][0];
  real *p_weights_acc = weights_acc;
  real *lp_i = log_probabilities_g->frames[t];
  real *log_w_i = log_weights;
  for (int i=0;i<n_gaussians;i++) {
    real post_i = exp(log_posterior + *log_w_i++ + *lp_i++ - log_prob);
    *p_weights_acc++ += post_i;
    real* means_acc_i = means_acc[i];
    real *x = inputs;
    for(int j = 0; j < n_inputs; j++) {
      *means_acc_i++ += post_i * *x++;
    }
  }
}

void MAPDiagonalGMM::eMUpdate()
{
  // just the means
	real epsilon = 10*REAL_EPSILON;
  //real* p_weights_acc = weights_acc;
	//real sum = .0;
	 //for (int i=0;i<n_gaussians;i++)
		// sum += *p_weights_acc++;
	 real* p_weights_acc = weights_acc;
	 if(learn_means)
		 for (int i=0;i<n_gaussians;i++,p_weights_acc++) {
				 //printf("%g ",*p_weights_acc);
			 if (*p_weights_acc <= (prior_weights + epsilon)){
				 //warning("Gaussian %d of GMM is not used in EM (w=%g)",i,*p_weights_acc);
				 real* p_means_prior_i = prior_distribution->means[i];
				 real* p_means_i = means[i];
				 for (int j=0;j<n_inputs;j++) 
					 *p_means_i++ = *p_means_prior_i++;
			 }else{
				 real* p_means_prior_i = prior_distribution->means[i];
				 real* p_means_i = means[i];
				 real* p_means_acc_i = means_acc[i];
				 for (int j=0;j<n_inputs;j++) {
					 *p_means_i++ = (weight_on_prior * *p_means_prior_i++) + ((1 - weight_on_prior) * *p_means_acc_i++ / *p_weights_acc);
				 }
			 }
		 }

  p_weights_acc = weights_acc;
	if(learn_variances)
		for (int i=0;i<n_gaussians;i++,p_weights_acc++) {
			if (*p_weights_acc <= (prior_weights + epsilon)) {
				warning("Gaussian %d of GMM is not used in EM",i);
			} else {
				real* p_var_i = var[i];
				real* p_means_acc_i = means_acc[i];
				real* p_var_acc_i = var_acc[i];
				real* p_means_prior_i = prior_distribution->means[i];
				real* p_var_prior_i = prior_distribution->var[i];
				for (int j=0;j<n_inputs;j++) {
					real means_ml = *p_means_acc_i++ / *p_weights_acc;
					real means_map = weight_on_prior * *p_means_prior_i +
						                        (1 - weight_on_prior) * means_ml;
					real var_ml = *p_var_acc_i++ / *p_weights_acc - means_map * means_map;
					real map_prior_2 = (means_map - *p_means_prior_i) * (means_map - *p_means_prior_i++);
          real map_ml_2 = (means_map - means_ml) * (means_map - means_ml);
					real var_map = weight_on_prior * (*p_var_prior_i++ + map_prior_2) +
					          (1 - weight_on_prior) * (var_ml + map_ml_2);
					*p_var_i++ = var_map >= var_threshold[j] ? var_map : var_threshold[j];
				}
			}
		}

	if(learn_weights){
		// then the weights
		real sum_weights_acc = 0;
		p_weights_acc = weights_acc;
		for (int i=0;i<n_gaussians;i++)
			sum_weights_acc += *p_weights_acc++;
		real *p_log_weights = log_weights;
		real *prior_log_weights = prior_distribution->log_weights;
		real log_sum = log(sum_weights_acc);
		p_weights_acc = weights_acc;
		for (int i=0;i<n_gaussians;i++)
			*p_log_weights++ = log(weight_on_prior * exp( *prior_log_weights++) + (1-weight_on_prior) * exp(log(*p_weights_acc++) - log_sum));
	}
}


/*
void MAPDiagonalGMM::frameBackward(real *observations, real *alpha, real *inputs, int t)
{
  real log_prob = log_probabilities[t];
  real *lp_i = log_probabilities_g[t];
  real *lw = log_weights;
  for (int i=0;i<n_gaussians;i++,lw++,lp_i++) {
    real post_i =  *alpha * exp(*lw + *lp_i - log_prob);
    real* obs = observations;
    real* means_i = means[i];
    real* dmeans_i = dmeans[i];
    real* var_i = var[i];
    for (int j=0;j<n_observations;j++,var_i++,obs++,means_i++,dmeans_i++) {
      real xmuvar = (*obs - *means_i) / *var_i;
      real dm = post_i * 2. * xmuvar;
      *dmeans_i += dm;
    }
  }
}
*/
MAPDiagonalGMM::~MAPDiagonalGMM()
{
}

}

