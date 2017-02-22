// Copyright (C) 2003--2004 Johnny Mariethoz (Johnny.Mariethoz@idiap.ch)
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

#ifndef MAP_DIAGONAL_GMM_INC
#define MAP_DIAGONAL_GMM_INC

#include "DiagonalGMM.h"

namespace Torch {

/** This class is a special case of a DiagonalGMM that implements the
    MAP algorithm instead of the EM algorithm. This means that the
    mean parameters will be changed according to the Maximum A Posteriori
    algorithm, given a prior value of the means (through a prior DiagonalGMM
    given in the constructor). Moreover, the variances and weights are
    not changed, as experimental results tend to show that there is no
    effects when they are changed.

    @author Samy Bengio (bengio@idiap.ch)
    @author Johnny Mariethoz (Johnny.Mariethoz@idiap.ch)
*/
class MAPDiagonalGMM : public DiagonalGMM
{
  public:

    /// The prior distribution used in MAP
    DiagonalGMM* prior_distribution;

    /// The weight to give to the prior parameters during update
    real weight_on_prior;

		/// update Gaussian's weights
		bool learn_weights;
		
		/// update Gaussian's variances
		bool learn_variances;

   /// update Gaussian's means 
		bool learn_means;

    ///
    MAPDiagonalGMM(DiagonalGMM* prior_distribution_);


		/// The backward step of Viterbi for a frame
    virtual void frameViterbiAccPosteriors(int t, real* inputs, real log_posterior);

		/// The backward step of EM for a frame
    virtual void frameEMAccPosteriors(int t, real *inputs, real log_posterior);

		/// The update after each iteration for EM
    virtual void eMUpdate();

		/**
       Copy the parameters of the prior distribution
		 */
		virtual void setDataSet(DataSet* data_);

    virtual ~MAPDiagonalGMM();
};


}

#endif
