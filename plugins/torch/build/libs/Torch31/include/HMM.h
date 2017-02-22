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

#ifndef HMM_INC
#define HMM_INC

#include "Distribution.h"
#include "Trainer.h"

namespace Torch {

/** This class implements a Hidden Markov Model distribution. It can be trained
    either by EM, Viterbi, or Gradient Descent.

    Note that this kind of HMM always contain one initial state and
    one final state. Both are non-emitting.

		Note that the log_probabilities is the average over all frames of the 
		log_probability of the example.

    @author Samy Bengio (bengio@idiap.ch)
*/
class HMM : public Distribution
{
  public:

    /** The number of states of the HMM.
        the first model is the initial state,
        the last model is the final (absorbing) state,
        (neither of them are emitting).
        hence, n_states > 2
    */
    int n_states;

    /// a prior on the transition probabilities
    real prior_transitions;
    
    /// keep the emission distributions
    Distribution** states;

    /// if the states are in fact shared in some way or another, the original ones are in shared_states
    Distribution** shared_states;
    int n_shared_states;

		bool linear_segmentation;

    /// the initial transitions between states are kept as a matrix
    real** transitions;
    /// in fact, we keep the transitions in log
    real** log_transitions;
    /// the derivative of the log transitions for gradient descent
    real** dlog_transitions;
    /// the accumulators of the transitions for EM
    real** transitions_acc;
    
    /// accumulator used in the forward phase to compute log likelihood 
    Sequence* log_alpha;
    /// accumulator used in the backward phase to compute log likelihood 
    Sequence* log_beta;
    /// for each state, for each time step, keep the best predecessor 
    Sequence* arg_viterbi;
    /// arg_viterbi of the finishing state
    int last_arg_viterbi;
    /// for each time step, keep the best state 
    Sequence* viterbi_sequence;

    /// keep for each time step and each model its emission log probability
    Sequence* log_probabilities_s;

    /// do we need to initialize the model?
    bool initialize;

    HMM(int n_states_, Distribution **states_, real** transitions_, int n_shared_states = 0, Distribution **shared_states_ = NULL);

    virtual void setDataSet(DataSet* data_);
    virtual void loadXFile(XFile *file);
    virtual void saveXFile(XFile *file);

	virtual void copy(HMM* other);

    /// this method can be used for debugging purpose to see the transitions
    virtual void printTransitions(bool real_values=false,bool transitions_only=false);

    /// computes the log_alpha during forward phase of EM
    virtual void logAlpha(Sequence* inputs);
    /// computes the log_beta during backward phase of EM
    virtual void logBeta(Sequence* inputs);
    /// computes the log_viterbi during forward phase of Viterbi
    virtual void logViterbi(Sequence* inputs);

    /// this method returns the state sequence associated to the input
    virtual void decode(Sequence* input);

    /** computes for each state and each time step of the sequence #inputs#
        its associated emission probability.
    */
    virtual void logProbabilities(Sequence *inputs);

    virtual real logProbability(Sequence *inputs);
    virtual real viterbiLogProbability(Sequence *inputs);

    virtual void  iterInitialize();
    virtual void  eMIterInitialize();
    virtual void  eMSequenceInitialize(Sequence* inputs);
    virtual void  sequenceInitialize(Sequence* inputs);
    virtual void  eMAccPosteriors(Sequence *inputs, real log_posterior);
    virtual void  viterbiAccPosteriors(Sequence *inputs, real log_posterior);
    virtual void  eMUpdate();
    virtual void  update();

    virtual void backward(Sequence *inputs, Sequence *alpha);
    virtual void viterbiBackward(Sequence *inputs, Sequence *alpha);

    virtual ~HMM();
};


}

#endif
