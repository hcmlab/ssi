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

#ifndef LINEAR_INC
#define LINEAR_INC

#include "GradientMachine.h"

namespace Torch {

/** Linear layer for #GradientMachine#.
    Formally speaking, $ouputs[i] = \sum_j w_{ij} inputs[i] + b_i$.\\
    $w_{ij}$ and $b_j$ are in #params#, with the following structure:\\
    $w_00... w_0n, b_0, w_10... w_1n, b_1, ...$\\

    If you want, you can add a weight decay which looks like
    $\sum_i,j w_{ij}^2 + sum_i b_i^2$.

    Options:
    \begin{tabular}{lcll}
      "weight decay"  &  real  &  the weight decay & [0]
    \end{tabular}

    @author Ronan Collobert (collober@idiap.ch)
*/
class Linear : public GradientMachine
{
  public:
    real weight_decay;
    real *weights;
    real *bias;
    real *der_weights;
    real *der_bias;
    void reset_();

    //-----

    ///
    Linear(int n_inputs_, int n_outputs_);

    //-----

    virtual void frameForward(int t, real *f_inputs, real *f_outputs);
    virtual void frameBackward(int t, real *f_inputs, real *beta_, real *f_outputs, real *alpha_);

    /// Computes new random values for weights.
    virtual void reset();

    virtual ~Linear();
};


}

#endif
