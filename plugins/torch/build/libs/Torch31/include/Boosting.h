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

#ifndef BOOSTING_INC
#define BOOSTING_INC

#include "Trainer.h"
#include "WeightedSumMachine.h"
#include "ClassFormat.h"

namespace Torch {

/** Boosting implementation.
    As the idea of boosting in regression hasn't been really well tested,
    this is boosting for *classification* only.
    
    This trainer will "boost" the machine given by the #WeightedSumMachine#,
    on the given #DataSet#.

    You have to provide a #ClassFormat# to know how the classes are encoded.

    @author Ronan Collobert (collober@idiap.ch)
    @see WeightedSumMachine
*/
class Boosting : public Trainer
{
  public:

    /// This machine performs the combination. It contains many trainers.
    WeightedSumMachine *w_machine;

    /// #ClassFormat# to know how the classes are encoded.
    ClassFormat *class_format;

    /// The number of trainers in the boosting.
    int n_trainers;

    /// The weights of each machine in the boosting.
    real *weights;

    ///
    Boosting(WeightedSumMachine *w_machine_, ClassFormat *class_format_);

    //-----
    
    virtual void train(DataSet *data, MeasurerList *measurers);

    virtual ~Boosting();
};


}

#endif
