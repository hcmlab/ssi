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

#ifndef BAGGING_INC
#define BAGGING_INC

#include "Trainer.h"
#include "Measurer.h"
#include "DataSet.h"
#include "WeightedSumMachine.h"

namespace Torch {

/** This class represents a #Trainer# that implements the well-known
    Bagging algorithm (Breiman, 1996). A "bagger" contains a series
    of trainers, each trained on a bootstrap of the original dataset.
    The output of the bagging is then the average of the output of
    each trainer.

    It is implemented using a #WeightedSumMachine# that performs the combination.

    @author Samy Bengio (bengio@idiap.ch)
    @see WeightedSumMachine
*/
class Bagging : public Trainer
{
  public:

    /// This machine performs the combination. It contains many trainers.
    WeightedSumMachine* w_machine;

    /// The number of trainers in the bagging.
    int n_trainers;

    /// for each trainer, keep the indices of examples not used during training
    int** unselected_examples;
    /// for each trainer, keep the indices of examples used during training
    int** selected_examples;
    /// for each trainer, keep the number of examples not used during training
    int* n_unselected_examples;
    /// for each trainer, keep the number of examples used during training
    int* is_selected_examples;
 
    ///
    Bagging(WeightedSumMachine *w_machine);

    /// create a boostrap of the data and put in in selected
    virtual void bootstrapData(int* selected, int* is_selected, int n_examples);

    virtual void train(DataSet *data, MeasurerList* measurers);

    virtual ~Bagging();
};


}

#endif
