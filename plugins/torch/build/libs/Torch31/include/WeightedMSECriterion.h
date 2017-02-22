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

#ifndef WEIGHTED_MSE_CRITERION_INC
#define WEIGHTED_MSE_CRITERION_INC

#include "Criterion.h"

namespace Torch {

/** Similar to #MSECriterion#, but you
    can put a weight on each example.

    Options:
    \begin{tabular}{lcll}
      "average frame size"  &  bool  &  divided by the frame size & [true]
    \end{tabular}

    @author Ronan Collobert (collober@idiap.ch)
*/
class WeightedMSECriterion : public Criterion
{
  public:
    bool average_frame_size;

    /** Pointer on the weights.
        You can modify the weights when you want.
        
        The length of #weights# \emph{is} the
        #n_real_examples# of #data#.
    */
    real *weights;

    //-----

    /** With this constructor, the weights are allocated
        and initializated to 1.
    */
    WeightedMSECriterion(DataSet *data_);

    /** With this constructor, you provide the #weights#.

       The length of #weights# \emph{must be} the
       #n_real_examples# of #data#.       
    */
    WeightedMSECriterion(DataSet *data_, real *weights_);

    //-----

    virtual void setDataSet(DataSet *data_);
    virtual void frameForward(int t, real *f_inputs, real *f_outputs);
    virtual void frameBackward(int t, real *f_inputs, real *beta_, real *f_outputs, real *alpha_);

    virtual ~WeightedMSECriterion();
};

}

#endif
