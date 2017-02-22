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

#ifndef DIAGONAL_GMM_INC
#define DIAGONAL_GMM_INC

#include "Distribution.h"
#include "EMTrainer.h"

namespace Torch {

/** This class can be used to model Diagonal Gaussian Mixture Models.
    They can be trained using either EM (with EMTrainer) or gradient descent
    (with GMTrainer).

    @author Samy Bengio (bengio@idiap.ch)
    @author Johnny Mariethoz (Johnny.Mariethoz@idiap.ch)
*/
class DiagonalGMM : public Distribution
{

  public:
    /// number of Gaussians in the mixture
    int n_gaussians;

    /** prior weights of the Gaussians, used in EM to give 
        a small prior on each Gaussian
    */
    real prior_weights;

    /** optional initializations
        if nothing is given, then random calling by reset(), 
				at your own risks or model loaded by the user...
        one can give a initial trainer containing a kmeans
    */
    EMTrainer* initial_kmeans_trainer;

    /// as well as a measurer of this trainer
    MeasurerList* initial_kmeans_trainer_measurers;

    /// the pointers to the parameters
    real* log_weights;
    real** means;
    real** var;

    /// the pointers to the derivative of the parameters
    real* dlog_weights;
    real** dmeans;
    real** dvar;

    /// this contains the minimal value of each variance
    real* var_threshold;

    /// for each frame, for each gaussian, keep its log probability
    Sequence* log_probabilities_g;


		/// gaussian that maximize the observed frame
		int best_gauss;


		/// gaussian that maximize the observed frame
		Sequence* best_gauss_per_frame; 

    /** in order to faster the computation, we can do some "pre-computation"
        pre-computed sum_log_var + n_obs * log_2_pi
    */
    real* sum_log_var_plus_n_obs_log_2_pi;

    /// pre-computed -0.5 / var
    real** minus_half_over_var;

    /// accumulators for EM
    real** means_acc;
    real** var_acc;
    real*  weights_acc;

    ///
		DiagonalGMM(int n_inputs_, int n_gaussians_, EMTrainer* initial_kmeans_trainer_ = NULL);

    void generateObservation(real* inputs_);
    void generateSequence(Sequence* sequence);

		/**
       This methods have to call by the user to initialized the
			 random parameters.
		*/
		virtual void reset();


		/**
		   If the KmeanTrainer has been given to the constructor, Kmeans algorithm
			 is perform.
		*/
		virtual void setDataSet(DataSet* data_);

		/**
		   Set the minimum value for the variances
		*/
		virtual void setVarThreshold(real* var_threshold_);

    virtual void display();

    /** Methods used to initialize the model at the beginning of each
        EM iteration
    */
    virtual void eMIterInitialize();

    /** Methods used to initialize the model at the beginning of each
        gradient descent iteration
    */
    virtual void iterInitialize();

    /// Returns the log probability of a frame of a sequence
    virtual real frameLogProbability(int t, real *inputs);
    
		/// Returns the log probability of a frame of a sequence on viterbi mode
		virtual real viterbiFrameLogProbability(int t, real *inputs);

    /// this method returns the log probability of the "g" Gaussian
    virtual real frameLogProbabilityOneGaussian(int g, real *inputs);

    /** Methods used to initialize the model at the beginning of each
        example during gradient descent training
    */
    virtual void sequenceInitialize(Sequence* inputs);

    /** Methods used to initialize the model at the beginning of each
        example during EM training
    */
    virtual void eMSequenceInitialize(Sequence* inputs);
    
		/// The backward step of EM for a frame
    virtual void frameEMAccPosteriors(int t, real *inputs, real log_posterior);
    
		/// The backward step of Viterbi for a frame
		virtual void frameViterbiAccPosteriors(int t, real *inputs, real log_posterior);
    
		/// The update after each iteration for EM
    virtual void eMUpdate();
		/// The update after each iteration for gradient
    virtual void update();

    /// Same as backward, but for one frame only
    virtual void frameBackward(int t, real *f_inputs, real *beta_, real *f_outputs, real *alpha_);
    
		/// Returns the expected value in #decision#
    virtual void frameDecision(int t, real *decision);

    virtual void setNGaussians(int n_gaussians_);

    virtual ~DiagonalGMM();
};


}

#endif
