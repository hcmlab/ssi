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

#ifndef LOG_RBF_INC
#define LOG_RBF_INC

#include "GradientMachine.h"
#include "EMTrainer.h"

namespace Torch {

/** LogRBF layer for #GradientMachine#.
    Formally speaking, $ouputs[i] = -0.5 \sum_j gamma_ij^2 * (inputs[j] - mu_ij)^2$.\\
    $mu_ij$ and $gamma_ij$ are in #params#, with the following structure:\\
    $mu_00... mu_0n, gamma_00.. gamma_0n,..., $\\

    For a better initialization, one can provide a #EMTrainer# using a
    #Kmeans# distribution that will be used to initialize the means and
    gamma.

    @author Ronan Collobert (collober@idiap.ch)
*/
class LogRBF : public GradientMachine
{
  public:
    real *gamma;
    real *mu;
    real *der_gamma;
    real *der_mu;

    /// optional initialization using a Kmeans
    EMTrainer* initial_kmeans_trainer;

    ///
    LogRBF(int n_inputs_, int n_outputs_, EMTrainer* kmeans_trainer=NULL);

    //-----

    virtual void setDataSet(DataSet* data_);
    virtual void frameForward(int t, real *f_inputs, real *f_outputs);
    virtual void frameBackward(int t, real *f_inputs, real *beta_, real *f_outputs, real *alpha_);
    virtual ~LogRBF();
};

}

#endif
