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

#ifndef WEIGHTED_SUM_MACHINE_INC
#define WEIGHTED_SUM_MACHINE_INC

#include "Trainer.h"

namespace Torch {

/** Weighted-sum machine.
    This class contains a series of #Trainers#, and its forward method
    simply performs the average of the output of each machine associated to
    the trainer on the same input.

    @see Bagging
    @see Boosting
    @author Ronan Collobert (collober@idiap.ch)
*/
class WeightedSumMachine : public Machine
{
  public:
    /// Output frame size
    int n_outputs;

    /// The trainers used in the combination.
    Trainer **trainers;

    /// The corresponding measurers.
    MeasurerList **trainers_measurers;

    /// The number of trainers in the combination.
    int n_trainers;

    /** The number of trainers that have been already trained.
        After the initialization, it's zero.
        Note that the forward method depends on this value.
        (only the first #n_trainers_trained# trainers are used)
    */
    int n_trainers_trained;

    /// The weights of the combination.
    real *weights;

    /// True if the weights aren't given by the user, false otherwise.
    bool weights_is_allocated;

    /** #trainers_measurers# is an array which possibly specify the measurers which
        should be given to the corresponding trainer when training.
        If #weights_# isn't specified, weights are setted to zero.
    */
    WeightedSumMachine(Trainer **trainer_, int n_trainers_, MeasurerList **trainers_measurers, real *weights_=NULL);

    //-----

    virtual void forward(Sequence *inputs);
    virtual void reset();
    virtual void loadXFile(XFile *file);
    virtual void saveXFile(XFile *file);

    virtual ~WeightedSumMachine();
};

}

#endif
