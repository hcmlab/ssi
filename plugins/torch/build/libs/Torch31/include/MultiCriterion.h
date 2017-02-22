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

#ifndef MULTI_CRITERION_INC
#define MULTI_CRITERION_INC

#include "Criterion.h"

namespace Torch {

/** MultiCriterion can be used to handle multiple criterions.

    The #MultiCriterion# is a #Criterion# which is the weighted
    sum of given criterions. (By default, the weights are equal to #1#).

    @author Samy Bengio (bengio@idiap.ch)
*/
class MultiCriterion : public Criterion
{
  public:

    /// the number of criterions
    int n_criterions;

    /// the criterions
    Criterion** criterions;

    /// the relative weight of each criterion
    real* weights;

    //-----

    ///
    MultiCriterion(Criterion** criterions_, int n_criterions_, real* weights_ = NULL);

    //-----

    virtual void forward(Sequence *inputs);
    virtual void backward(Sequence *inputs, Sequence *alpha);
    virtual void setDataSet(DataSet *data_);
    virtual void iterInitialize();
    virtual void reset();

    virtual ~MultiCriterion();
};

}

#endif
