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

#ifndef PARZEN_MACHINE_INC
#define PARZEN_MACHINE_INC

#include "Machine.h"
#include "DataSet.h"

namespace Torch {

/** This machine implements the Parzen Window estimator.
    Given a dataset (in the constructor), the #forward# method returns
    for a given input the average of the outputs of the training set
    weighted by a Gaussian kernel distance:

    $ y(x) = \frac{\sum_i t_i \exp(- \frac{||x - x_i||^2}{2 var})}{\sum_i \exp(- \frac{||x - x_i||^2}{2 var})}$

    The only parameter #var# is given in the constructor.

    @author Samy Bengio (bengio@idiap.ch)
*/
class ParzenMachine : public Machine
{
  public:

    /// the variance used
    real var;

    /// The dataset that contains the training set
    DataSet* data;

    /// the indices of the training examples
    int *real_examples;
    int n_real_examples;

    /// keep the denominator
    real denominator;

    /// the size of the output vector
    int n_outputs;

    /// the size of the input vector
    int n_inputs;

    ///
    ParzenMachine(int n_inputs_,int n_outputs_,real var_);

    virtual void forward(Sequence *inputs);
    virtual void setDataSet(DataSet* dataset_);

    /// change the value of var
    virtual void setVar(real var_);

    virtual ~ParzenMachine();
};


}

#endif
