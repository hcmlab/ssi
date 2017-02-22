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

#ifndef MIXER_INC
#define MIXER_INC

#include "GradientMachine.h"

namespace Torch {

/** Mixer useful for experts mixtures.
    Formally speaking, it computes:
    $outputs[i] = \sum_j a_j * inputs_j[i]$

    where
    \begin{itemize}
      \item ${a_1,...,a_n}$ are in the table
      of the first node of the #inputs# list,
      when you call #forward()#.
      
      \item $inputs_j$ are the inputs of the j-th expert.
      Therefore, the #inputs# list has the structure
      ${a, inputs_1, inputs_2, ...}$.
      Only $a$ must be alone in one node.
    \end{itemize}

    @author Ronan Collobert (collober@idiap.ch)
*/
class Mixer : public GradientMachine
{
  public:
    /// Number of experts.
    int n_experts;

    //-----

    ///
    Mixer(int n_inputs_, int n_outputs_per_expert);

    //-----

    virtual void frameForward(int t, real *f_inputs, real *f_outputs);
    virtual void frameBackward(int t, real *f_inputs, real *beta_, real *f_outputs, real *alpha_);

    virtual ~Mixer();
};


}

#endif
