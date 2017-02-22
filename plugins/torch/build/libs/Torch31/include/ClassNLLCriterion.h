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

#ifndef CLASS_NLL_CRITERION_INC
#define CLASS_NLL_CRITERION_INC

#include "Criterion.h"
#include "ClassFormat.h"

namespace Torch {

/** This criterion can be used to train *in classification* a #GradientMachine#
    object using the #StochasticGradient# trainer. It then maximizes the log 
    likelihood of the data.

    If we write $o_i$ for the output $i$ of the #GradientMachine#, it supposes that
    \begin{itemize}
      \item the outputs $o_i$ are log-probabilities.
      \item $exp(o_i)$ is the probability for the class $i$
      \item the predicted class follows a multinomial distribution with parameters
            $(exp(o_1), exp(o_2), exp(o_3)...)$
    \end{itemize}

    The number of target frames in #DataSet# must
    correspond to the number of input frames given
    to this criterion.

    @author Ronan Collobert (collober@idiap.ch)
*/
class ClassNLLCriterion : public Criterion
{
  public:
    ClassFormat *class_format;

    /// The ClassFormat is needed just to know how the targets are encoded in the dataset.
    ClassNLLCriterion(ClassFormat *class_format);

    virtual void frameForward(int t, real *f_inputs, real *f_outputs);
    virtual void frameBackward(int t, real *f_inputs, real *beta_, real *f_outputs, real *alpha_);
    virtual ~ClassNLLCriterion();
};


}

#endif
