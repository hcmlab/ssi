// Copyright (C) 2003--2004 Ronan Collobert (collober@idiap.ch)
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

#ifndef KFOLD_INC
#define KFOLD_INC

#include "Trainer.h"

namespace Torch {

/** Provides an interface to sample data, for use by methods such
    as cross-validation

    @author Samy Bengio (bengio@idiap.ch)
    @author Ronan Collobert (collober@idiap.ch)
*/

class KFold : public Object
{
  public:
    /// Training examples for each fold
    int** train_subsets;

    /// Test examples for each fold
    int** test_subsets;

    /// Number of training examples for each fold...
    int* n_train_subsets;

    /// Number of test examples for each fold...
    int* n_test_subsets;

    // Trainer used to do KFold
    Trainer* trainer;

    // Number of folds!
    int kfold;

    ///
    KFold(Trainer* trainer_, int kfold_);

    /** Do a cross-validation over #data#.
        #train_measurers# are called in each "train pass" for each fold.
        #test_measurers# are called in each "test pass" for each fold.
        #cross_valid_measurers# are called during the cross-validation loop.
    */
    virtual void crossValidate(DataSet *data, MeasurerList *train_measurers=NULL, MeasurerList *test_measurers=NULL, MeasurerList *cross_valid_measurers=NULL);

    /** Prepare the sample.
        Given #n_examples#, puts the right examples-indices in #train_subsets#
        and #test_subsets#, and updates #n_train_subsets# and #n_test_subsets#.
        (You don't have to allocate these arrays).
        The provided sample function is a standard sample for cross-validation,
        but you could imagine what you want!
    */
    virtual void sample(int n_examples);

    virtual ~KFold();
};

}

#endif
