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

#ifndef BOOSTING_MEASURER_INC
#define BOOSTING_MEASURER_INC

#include "Measurer.h"
#include "DataSet.h"
#include "ClassFormat.h"

namespace Torch {

/** Compute the classification weighted error (in %) for #BoostingMachine#
    of the #inputs# with respect to the #targets# of #data#.
    The weights are given by #setWeights()#.

    Needed by #Boosting#. (Designed just for it).

    The format of the class is given with a #ClassFormat#.

    @author Ronan Collobert (collober@idiap.ch)
    @author Samy Bengio (bengio@idiap.ch)
    @see Boosting
*/
class BoostingMeasurer : public Measurer
{
  public:
    // General
    Sequence *inputs;
    real *weights;
    real beta;
    int *status;

    real internal_error;
    int current_example;
    ClassFormat *class_format;

    ///
    BoostingMeasurer(ClassFormat *class_format_, XFile *file_);

    /// Set the current working dataset.
    virtual void setDataSet(DataSet *data_);

    /// Set the current weights of training examples.
    virtual void setWeights(real *weights_);

    /// Gives the pointer where the measure will be done.
    virtual void setInputs(Sequence *inputs_);

    //-----
    
    void init_();
    virtual void reset();
    virtual void measureExample();
    virtual void measureIteration();

    virtual ~BoostingMeasurer();
};


}

#endif
