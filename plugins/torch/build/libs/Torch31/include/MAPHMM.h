// Copyright (C) 2003--2004 Johnny Mariethoz (marietho@idiap.ch)
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

#ifndef MAP_HMM_INC
#define MAP_HMM_INC

#include "HMM.h"

namespace Torch {

/** This class is a special case of a HMM that implements the
    MAP algorithm for HMM transitions probabilities.
    
    @author Samy Bengio (bengio@idiap.ch)
    @author Johnny Mariethoz (marietho@idiap.ch)
*/
class MAPHMM : public HMM
{
  public:
     
    /// The prior distribution used in MAP
    HMM* prior_distribution;
     
    /// The weight to give to the prior parameters during update
    real weight_on_prior;
     
    ///log(weight_on_prior)
    real log_weight_on_prior;
     
    ///log(1-weight_on_prior_
    real log_1_weight_on_prior;
    
    ///
    MAPHMM(int n_states_, Distribution **states_, real** transitions_, HMM* prior_distribution_);

		void setWeightOnPrior(real weight_on_prior_);

    /// map adaptation method for transitions probabilities
    virtual void eMUpdate();

    virtual ~MAPHMM();
    
};


}

#endif
