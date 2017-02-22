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

#ifndef DISTRIBUTION_INC
#define DISTRIBUTION_INC

#include "GradientMachine.h"

namespace Torch {

/** This class is designed to handle generative distribution models
    such as Gaussian Mixture Models and Hidden Markov Models. As 
    distribution inherits from GradientMachine, they can be trained 
    by gradient descent or by Expectation Maximization (EM) or even
    Viterbi.

    Note that the output of a distribution is the negative log likelihood.

    @author Samy Bengio (bengio@idiap.ch)
*/
class Distribution : public GradientMachine
{
  public:

    /// the log likelihood
    real log_probability;

    /// the log likelihood for each frame when available
    Sequence* log_probabilities;

    /// 
    Distribution(int n_inputs_,int n_params_=0);

    /// Returns the log probability of a sequence represented by #inputs#
    virtual real logProbability(Sequence* inputs);
    /// Returns the viterbi score of a sequence represented by #inputs#
    virtual real viterbiLogProbability(Sequence* inputs);
    /// Returns the log probability of a frame of a sequence
    virtual real frameLogProbability(int t, real *f_inputs);
    /// Returns the log probability of a frame of a sequence on viterbi mode
		virtual real viterbiFrameLogProbability(int t, real *f_inputs);
    virtual void frameGenerate(int t, real *inputs);

    /** Methods used to initialize the model at the beginning of each
        EM iteration
    */
    virtual void eMIterInitialize();
    /** Methods used to initialize the model at the beginning of each
        gradient descent iteration
    */
    virtual void iterInitialize();
    /** Methods used to initialize the model at the beginning of each
        example during EM training
    */
    virtual void eMSequenceInitialize(Sequence* inputs);
    /** Methods used to initialize the model at the beginning of each
        example during gradient descent training
    */
    virtual void sequenceInitialize(Sequence* inputs);
    /// The backward step of EM for a sequence
    virtual void eMAccPosteriors(Sequence *inputs, real log_posterior);
    /// The backward step of EM for a frame
    virtual void frameEMAccPosteriors(int t, real *f_inputs, real log_posterior);
    /// The backward step of Viterbi learning for a sequence
    virtual void viterbiAccPosteriors(Sequence *inputs, real log_posterior);
    /// The backward step of Viterbi for a frame
    virtual void frameViterbiAccPosteriors(int t, real *f_inputs, real log_posterior);
    /// The update after each iteration for EM
    virtual void eMUpdate();
    /// The update after each gradient iteration
    virtual void update();

    /// For some distribution like SpeechHMM, decodes the most likely path
    virtual void decode(Sequence *inputs);

    virtual void forward(Sequence *inputs);


    /// Same as forward, but for EM
    virtual void eMForward(Sequence *inputs);
    /// Same as forward, but for Viterbi
    virtual void viterbiForward(Sequence *inputs);
    virtual void backward(Sequence *inputs, Sequence *alpha);
    /// Same as backward, but for one frame only
    virtual void frameBackward(int t, real *f_inputs, real *beta_, real *f_outputs, real *alpha_);
    /// Same as backward, but for Viterbi 
    virtual void viterbiBackward(Sequence *inputs, Sequence *alpha);
    virtual void loadXFile(XFile *file);

    /// Returns the decision of the distribution
    /// decision is expectation for regression, class likelihoods for classif
    virtual void decision(Sequence* decision);
    /// Returns the decision of a frame of a sequence
    virtual void frameDecision(int t, real *decision);

    virtual ~Distribution();
};


}

#endif

