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

#ifndef CRITERION_INC
#define CRITERION_INC

#include "GradientMachine.h"
#include "DataSet.h"

namespace Torch {

/** #Criterion# class for #StochasticGradient#.
    A #Criterion# computes an error with its inputs
    and a dataset #data#.

    It knows how to backpropagate this error.

    A #Criterion# shouldn't be connected to the inputs of
    another #GradientMachine#.
    ne doit pas se connecter sur l'entree d'une

    By default, the number of outputs for a #Criterion#
    is fixed to one. It \emph{must} contain the error:
    This one is used by the #StochasticGradient# for the
    stopping criterion.

    @author Ronan Collobert (collober@idiap.ch)
*/
class Criterion : public GradientMachine
{
  public:

    /// #DataSet# used to compute the error.
    DataSet *data;

    ///
    Criterion(int n_inputs_, int n_params_=0);

    /** Set #data# to #data_#. The criterion
        should be able to react to this function.
    */
    virtual void setDataSet(DataSet *data_);

    //-----

    virtual ~Criterion();
};


}

#endif
