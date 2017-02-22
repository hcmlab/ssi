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

#ifndef MULTINOMIAL_INC
#define MULTINOMIAL_INC

#include "Distribution.h"

namespace Torch {

/** This class can be used to model Multinomial Distributions.
    They can be trained using either EM (with EMTrainer) or gradient descent
    (with GMTrainer).

    @author Samy Bengio (bengio@idiap.ch)
*/
class Multinomial : public Distribution
{
  public:
    /// number of values this multinomial can take
    int n_values;

    /// the prior weight given to each value. kind of smoother
    real prior_weights;

    /// if true then does equal initialization of the weights
    bool equal_initialization;

    /// the pointers to the parameters
    real* log_weights;

    /// the pointers to the d_parameters
    real* dlog_weights;

    /// accumulators for EM
    real*  weights_acc;

    Multinomial(int n_values_);

    virtual void setDataSet(DataSet* data_);

    virtual void eMIterInitialize();
    virtual void iterInitialize();

    virtual real frameLogProbability(int t, real *inputs);

    virtual void sequenceInitialize(Sequence* inputs);
    virtual void eMSequenceInitialize(Sequence* inputs);
    virtual void frameEMAccPosteriors(int t, real *inputs, real log_posterior);
    virtual void eMUpdate();
    virtual void update();

    virtual void frameBackward(int t, real *f_inputs, real *beta_, real *f_outputs, real *alpha_);
    virtual void frameDecision(int t, real *decision);

    virtual ~Multinomial();
};


}

#endif
