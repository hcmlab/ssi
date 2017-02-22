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

#include "DiagonalGMM.h"
#include "MeanVarNorm.h"
#include "log_add.h"
#include "Random.h"

namespace Torch {

	DiagonalGMM::DiagonalGMM(int n_inputs_, int n_gaussians_, EMTrainer* initial_kmeans_trainer_) : Distribution(n_inputs_,(n_inputs_*n_gaussians_)*2+n_gaussians_)
	{
		n_gaussians = n_gaussians_;
		initial_kmeans_trainer = initial_kmeans_trainer_;

		initial_kmeans_trainer_measurers = NULL;

		var_threshold = (real*)allocator->alloc(sizeof(real)*n_inputs);
		for (int i=0;i<n_inputs;i++)
			var_threshold[i] = 1e-10; 

		addOOption("initial kmeans trainer measurers", (Object**) &initial_kmeans_trainer_measurers, NULL, "initial kmeans trainer measurers");
		addROption("prior weights", &prior_weights , 1e-3, "minimum weights for each gaussians");

		log_probabilities_g = new(allocator)Sequence(1,n_gaussians);
		real* p = (real*)params->data[0];
		real* dp = (real*)der_params->data[0];
		log_weights = p;
		dlog_weights = dp;
		p += n_gaussians;
		dp += n_gaussians;
		means = (real**)allocator->alloc(sizeof(real*)*n_gaussians);
		dmeans = (real**)allocator->alloc(sizeof(real*)*n_gaussians);
		var = (real**)allocator->alloc(sizeof(real*)*n_gaussians);
		dvar = (real**)allocator->alloc(sizeof(real*)*n_gaussians);
		means_acc = (real**)allocator->alloc(sizeof(real*)*n_gaussians);
		var_acc = (real**)allocator->alloc(sizeof(real*)*n_gaussians);
		weights_acc = (real*)allocator->alloc(sizeof(real)*n_gaussians);
		minus_half_over_var = (real**)allocator->alloc(sizeof(real*)*n_gaussians);
		for (int i=0;i<n_gaussians;i++) {
			means[i] = p;
			dmeans[i] = dp;
			p += n_inputs;
			dp += n_inputs;
			var[i] = p;
			dvar[i] = dp;
			p += n_inputs;
			dp += n_inputs;
			means_acc[i] = (real*)allocator->alloc(sizeof(real)*n_inputs);
			var_acc[i] = (real*)allocator->alloc(sizeof(real)*n_inputs);
			minus_half_over_var[i] = (real*)allocator->alloc(sizeof(real)*n_inputs);
		}
		sum_log_var_plus_n_obs_log_2_pi = (real*)allocator->alloc(sizeof(real)*n_gaussians);

		best_gauss = -1;
		best_gauss_per_frame = new(allocator)Sequence(1,1);
	}

	void DiagonalGMM::setVarThreshold(real* var_threshold_){
		for (int i=0;i<n_inputs;i++)
			var_threshold[i] = var_threshold_[i];
	}

	void DiagonalGMM::reset(){
		// initialize randomly
		// first the weights
		if (!initial_kmeans_trainer) {
			real sum = 0.;
			for (int i=0;i<n_gaussians;i++) {
				log_weights[i] = Random::boundedUniform(0.1,1);
				sum += log_weights[i];
			}
			for (int i=0;i<n_gaussians;i++) {
				log_weights[i] = log(log_weights[i]/sum);
			}

			// then the means and variances
			for (int i=0;i<n_gaussians;i++) {
				for (int j=0;j<n_inputs;j++) {
					means[i][j] = Random::boundedUniform(0,1);
					var[i][j] = Random::boundedUniform(var_threshold[j],var_threshold[j]*10);
				}
			}
		}
	}


	void DiagonalGMM::setDataSet(DataSet* data_)
	{
		// here, initialize the parameters somehow...
		if (initial_kmeans_trainer) {
			initial_kmeans_trainer->train(data_,initial_kmeans_trainer_measurers);
			params->copy(initial_kmeans_trainer->distribution->params);
		}
		//check variance flooring
		for (int i=0;i<n_gaussians;i++) {
			real* p_var_i = var[i];
			for (int j=0;j<n_inputs;j++,p_var_i++)
				if(*p_var_i < var_threshold[j])
					*p_var_i = var_threshold[j];
		}
		// check the weights
		real log_sum = LOG_ZERO;
		for (int i=0;i<n_gaussians;i++)
			log_sum = logAdd(log_sum,log_weights[i]);
		for (int i=0;i<n_gaussians;i++)
			log_weights[i] -= log_sum;
	}

	void DiagonalGMM::eMSequenceInitialize(Sequence* inputs)
	{
		if (!inputs)
			return;
		best_gauss_per_frame->resize(inputs->n_frames);
		log_probabilities_g->resize(inputs->n_frames);
		log_probabilities->resize(inputs->n_frames);
	}

	void DiagonalGMM::display()
	{
		for(int i=0;i<n_gaussians;i++){
			printf("Mixture %d %.3g\n",i+1,exp(log_weights[i]));
			printf("Mean %d\n",i);
			for(int j=0;j<n_inputs;j++){
				printf("%.3g ",means[i][j]);
			}
			printf("\nVar\n");
			for(int j=0;j<n_inputs;j++){
				printf("%.3g ",var[i][j]);
			}
			printf("\n"); 
		}
	}

	void DiagonalGMM::sequenceInitialize(Sequence* inputs)
	{
		// initialize the accumulators to 0 and compute pre-computed value
		eMSequenceInitialize(inputs);
		for (int i=0;i<n_gaussians;i++) {
			real *sum = &sum_log_var_plus_n_obs_log_2_pi[i];
			*sum = n_inputs * LOG_2_PI;
			real *mh_i = minus_half_over_var[i];
			real *v = var[i];
			real *vt = var_threshold;
			for (int j=0;j<n_inputs;j++,vt++) {
				if (*v < *vt)
					*v = *vt;
				*mh_i++ = -0.5 / *v;
				*sum += log(*v++);
			}
			*sum *= -0.5;
		}
	}

	void DiagonalGMM::generateSequence(Sequence* sequence)
	{
		for(int i=0;i<sequence->n_frames;i++)
			generateObservation(sequence->frames[i]);
	}

	void DiagonalGMM::generateObservation(real* observation)
	{
		real v_tot,v_partial;
		v_tot = Random::uniform();
		v_partial = 0.;
		real* lw = log_weights; 
		int j;
		for (j=0;j<n_gaussians;j++) {
			v_partial += exp(*lw++);
			if (v_partial > v_tot) break;
		}
		if(j>=n_gaussians)
			j = n_gaussians - 1;
		real* v = var[j];
		real* m = means[j];
		real* obs = observation;

		for (int i=0;i<n_inputs;i++) {
			*obs++ = Random::normal(*m++, sqrt(*v++));
		}
	}


	real DiagonalGMM::frameLogProbabilityOneGaussian(int g, real *inputs)
	{
		real* means_g = means[g];
		real* mh_g = minus_half_over_var[g];
		real sum_xmu = 0.;
		real *x = inputs;
		for(int j = 0; j < n_inputs; j++) {
			real xmu = (*x++ - *means_g++);
			sum_xmu += xmu*xmu * *mh_g++;
		}
		real lp = sum_xmu + sum_log_var_plus_n_obs_log_2_pi[g];
		return lp;
	}


	real DiagonalGMM::viterbiFrameLogProbability(int t, real *inputs)
	{
		real *p_log_w = log_weights;
		real *lpg = log_probabilities_g->frames[t];
		real max_lpg = LOG_ZERO;
		real max_lp = LOG_ZERO;
		real lpg_w = 0;
		for (int i=0;i<n_gaussians;i++,lpg++) {
			*lpg = frameLogProbabilityOneGaussian(i, inputs);
			lpg_w = *lpg + *p_log_w++;
			if(lpg_w > max_lpg){
				max_lpg = lpg_w;
				best_gauss = i;
				max_lp = *lpg;
		 }
		}
		log_probabilities->frames[t][0] = max_lp;
		best_gauss_per_frame->frames[t][0] = (real)best_gauss;
		return max_lp;
	}

	real DiagonalGMM::frameLogProbability(int t, real *inputs)
	{
		real *p_log_w = log_weights;
		real *lpg = log_probabilities_g->frames[t];
		real log_prob = LOG_ZERO;
		for (int i=0;i<n_gaussians;i++) {
			*lpg = frameLogProbabilityOneGaussian(i, inputs);
			log_prob = logAdd(log_prob, *lpg++ + *p_log_w++);
		}
		log_probabilities->frames[t][0] = log_prob;
		return log_prob;
	}

	void DiagonalGMM::frameViterbiAccPosteriors(int t, real *inputs, real log_posterior)
	{
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
		real* var_acc_i = var_acc[best_g];
		real *x = inputs;
		for(int j = 0; j < n_inputs; j++) {
			*var_acc_i++ +=  *x * *x;
			*means_acc_i++ +=  *x++;
		}
	}

	void DiagonalGMM::frameEMAccPosteriors(int t, real *inputs, real log_posterior)
	{
		real log_prob = log_probabilities->frames[t][0];
		real *p_weights_acc = weights_acc;
		real *lp_i = log_probabilities_g->frames[t];
		real *log_w_i = log_weights;
		for (int i=0;i<n_gaussians;i++) {
			real post_i = exp(log_posterior + *log_w_i++ + *lp_i++ - log_prob);
			*p_weights_acc++ += post_i;
			real* means_acc_i = means_acc[i];
			real* var_acc_i = var_acc[i];
			real *x = inputs;
			for(int j = 0; j < n_inputs; j++) {
				*var_acc_i++ += post_i * *x * *x;
				*means_acc_i++ += post_i * *x++;
			}
		}
	}

	void DiagonalGMM::eMUpdate()
	{
		// first the gaussians
		real* p_weights_acc = weights_acc;
		for (int i=0;i<n_gaussians;i++,p_weights_acc++) {
			if (*p_weights_acc == 0) {
				warning("Gaussian %d of GMM is not used in EM",i);
			} else {
				real* p_means_i = means[i];
				real* p_var_i = var[i];
				real* p_means_acc_i = means_acc[i];
				real* p_var_acc_i = var_acc[i];
				for (int j=0;j<n_inputs;j++) {
					*p_means_i = *p_means_acc_i++ / *p_weights_acc;
					real v = *p_var_acc_i++ / *p_weights_acc - *p_means_i * *p_means_i++;
					*p_var_i++ = v >= var_threshold[j] ? v : var_threshold[j];
				}
			}
		}
		// then the weights
		real sum_weights_acc = 0;
		p_weights_acc = weights_acc;
		for (int i=0;i<n_gaussians;i++)
			sum_weights_acc += *p_weights_acc++;
		real *p_log_weights = log_weights;
		real log_sum = log(sum_weights_acc);
		p_weights_acc = weights_acc;
		for (int i=0;i<n_gaussians;i++)
			*p_log_weights++ = log(*p_weights_acc++) - log_sum;
	}

	void DiagonalGMM::update()
	{
		// normalize log_weights
		real log_sum = LOG_ZERO;
		for (int i=0;i<n_gaussians;i++)
			log_sum = logAdd(log_sum,log_weights[i]);
		for (int i=0;i<n_gaussians;i++)
			log_weights[i] -= log_sum;
	}

	void DiagonalGMM::eMIterInitialize()
	{
		// initialize the accumulators to 0 and compute pre-computed value
		for (int i=0;i<n_gaussians;i++) {
			real *pm = means_acc[i];
			real *ps = var_acc[i];
			real *v = var[i];
			real *sum = &sum_log_var_plus_n_obs_log_2_pi[i];
			*sum = n_inputs * LOG_2_PI;
			real *mh_i = minus_half_over_var[i];
			for (int j=0;j<n_inputs;j++) {
				*pm++ = 0.;
				*ps++ = 0.;
				*mh_i++ = -0.5 / *v;
				*sum += log(*v++);
			}
			*sum *= -0.5;
			weights_acc[i] = prior_weights;
		}
	}

	void DiagonalGMM::iterInitialize()
	{
	}

	void DiagonalGMM::frameBackward(int t, real *f_inputs, real *beta_, real *f_outputs, real *alpha_)
	{
		real log_prob = log_probabilities->frames[t][0];
		real *lp_i = log_probabilities_g->frames[t];
		real *lw = log_weights;
		real* dlw = dlog_weights;
		for (int i=0;i<n_gaussians;i++,lw++,lp_i++) {
			real post_i =  *alpha_ * exp(*lw + *lp_i - log_prob);
			*dlw++ += post_i;
			real *dlw2 = dlog_weights;
			real *lw2 = log_weights;
			for (int j=0;j<n_gaussians;j++) {
				*dlw2++ -= post_i * exp(*lw2++);
			}
			real* obs = f_inputs;
			real* means_i = means[i];
			real* dmeans_i = dmeans[i];
			real* var_i = var[i];
			real* dvar_i = dvar[i];
			for (int j=0;j<n_inputs;j++,var_i++,obs++,means_i++,dmeans_i++,dvar_i++) {
				real xmuvar = (*obs - *means_i) / *var_i;
				//real xmuvar = (*obs - *means_i);
				real dm = post_i * 2. * xmuvar;
				*dmeans_i += dm;
				*dvar_i += post_i * 0.5 * (xmuvar*xmuvar - 1./ *var_i);
			}
		}
	}

	void DiagonalGMM::frameDecision(int t, real *decision)
	{
		real *obs = decision;
		for (int i=0;i<n_inputs;i++) {
			*obs++ = 0;
		}
		real *lw = log_weights;
		for (int i=0;i<n_gaussians;i++) {
			obs = decision;
			real *means_i = means[i];
			real w = exp(*lw++);
			for (int j=0;j<n_inputs;j++) {
				*obs++ += w * *means_i++;
			}
		}
	}

	void DiagonalGMM::setNGaussians(int n_gaussians_)
	{
		n_gaussians = n_gaussians_;
	}

	DiagonalGMM::~DiagonalGMM()
	{
	}

}

