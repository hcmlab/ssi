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

#ifndef QC_MACHINE_INC
#define QC_MACHINE_INC

#include "Machine.h"
#include "QCCache.h"

namespace Torch {

/** "Quadratic Constrained Machine".
    It's a machine with #alpha# parameters which
    are the minimum of a constrained quadratic problem.

    The problem:

    minimize $QP(alpha) = alpha' * Q * alpha + beta'*alpha$
    
    with the following constraints:
         \begin{itemize}
           \item $\sum_j alpha_j y_j = 1$
           \item $Cdown_j <= alpha_j <= Cup_j$
         \end{itemize}

    where $y$, $Cdown$, $Cup$ are given by the user
    and \emph{must be} $y_j = +- 1$.

    The number of #alpha# variable is determined here by #l#.
    (Therefore, it determines the size of de Cup, Cdown et y...)

    Note: if $alpha_j$ is closed to $Cup_j$ [ou $Cdown_j$] with
          the accuracy "eps bounds", $alpha_j$ is considered
          to be equal to $Cup_j$ [ou $Cdown_j$].

    Options:
    \begin{tabular}{lcll}
      "eps bounds"  &  real  &  bound accuracy: & [1E-4 in float, 1E-12 in double]
    \end{tabular}

    @author Ronan Collobert (collober@idiap.ch)
*/
class QCMachine : public Machine
{
  public:

    ///
    real *Cup;

    ///
    real *Cdown;

    ///
    real bound_eps;

    ///
    int n_alpha;

    ///
    real *alpha;

    ///
    real *grad;

    ///
    real *y;

    ///
    QCCache *cache;

    //-----

    ///
    QCMachine();

    /** Function called by #QCTrainer# after the optimization.
        @see QCTrainer
    */
    virtual void checkSupportVectors() = 0;
    void reInit(int n_alpha_);

    //-----

    virtual ~QCMachine();    
};


}

#endif
