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

#ifndef STOCHASTIC_GRADIENT_INC
#define STOCHASTIC_GRADIENT_INC

#include "Trainer.h"
#include "GradientMachine.h"
#include "DataSet.h"
#include "Criterion.h"

namespace Torch {

/** Trainer for GradientMachine.
    Given a machine and a criterion, train the machine using
    a stochastic gradient descent.

    Options:
    \begin{tabular}{lcll}
      "end accuracy"         & real  &  end accuracy                   & [0.0001]\\
      "learning rate"        & real  &  learning rate                  & [0.01]\\
      "learning rate decay"  & real  &  learning rate decay            & [0]\\
      "max iter"             & int   &  maximum number of iterations   & [-1]\\
      "shuffle"              & bool  &  shuffle the dataset            & [true]
    \end{tabular}

    @author Ronan Collobert (collober@idiap.ch)
*/
class StochasticGradient : public Trainer
{
  public:
    Criterion *criterion;
    real learning_rate;
    real learning_rate_decay;
    real end_accuracy;
    int max_iter;
    bool do_shuffle;

    //-----

    /** Take the #machine_# to train, the train #data_#, the #criterion_#
        to use, and the #optimizer_# to use.
    */
    StochasticGradient(GradientMachine *machine_, Criterion *criterion_);

    //-----

    virtual void train(DataSet *data, MeasurerList *measurers);
    virtual ~StochasticGradient();
};

}

#endif
