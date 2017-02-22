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

#ifndef PARZEN_DISTRIBUTION_INC
#define PARZEN_DISTRIBUTION_INC

#include "Distribution.h"

namespace Torch {

/** This class can be used to model a Parzen density estimator with
    a Gaussian kernel:

    $ p(x) = \frac{1}{N}\sum_i \frac{1}{(2 \Pi var)^{d/2}} \exp(- \frac{||x - x_i||^2}{2 var})$

    where the sum is done on the whole training set.

    @author Samy Bengio (bengio@idiap.ch)
*/
class ParzenDistribution : public Distribution
{
  public:
    /// the variance used
    real var;

    /// the dataset
    DataSet* data;

    /// the indices of the training examples
    int *train_examples_index;
    int n_train_examples_index;

    /** in order to faster the computation, we can do some "pre-computation"
        pre-computed sum_log_var + n_obs * log_2_pi
    */
    real sum_log_var_plus_n_obs_log_2_pi;
    /// pre-computed -0.5 / var
    real minus_half_over_var;

    ParzenDistribution(int n_inputs_, real var_);

    virtual void setDataSet(DataSet* dataset_);

    virtual void setVar(real var_);

    virtual real frameLogProbability(int t, real *inputs);
    virtual real frameLogProbabilityOneFrame(real *inputs, real *mean);

    virtual void eMSequenceInitialize(Sequence* inputs);
    virtual void sequenceInitialize(Sequence* inputs);

    virtual ~ParzenDistribution();
};


}

#endif
