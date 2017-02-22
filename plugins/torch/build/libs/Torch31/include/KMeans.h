// Copyright (C) 2003--2004 Samy Bengio (bengio@idiap.ch)
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

#ifndef KMEANS_INC
#define KMEANS_INC

#include "DiagonalGMM.h"

namespace Torch {

/** This class can be used to do a "kmeans" on a given set of data.
    It has been implemented in the framework of a Distribution that can
    be trained with EM. This means that the kmeans distance is in fact
    returned by the method logProbability.

    Note that as KMeans is a subclass of DiagonalGMM, they share the same
    parameter structure. Hence, a DiagonalGMM can be easily initialized by
    a KMeans.

    @author Samy Bengio (bengio@idiap.ch)
*/
class KMeans : public DiagonalGMM
{
  public:

    /// for each example, keep the index of the neirest cluster
    Sequence* min_cluster;

    /// initialize the parameters from the data set to false if you
		/// load the data.
    bool initialize_parameters;
    ///
    KMeans(int n_inputs, int n_gaussians_);

    virtual void setDataSet(DataSet* data_);

    virtual void eMIterInitialize();
    virtual void frameEMAccPosteriors(int t, real *inputs, real log_posterior);
    virtual void eMUpdate();
    virtual void frameBackward(int t, real *f_inputs, real *beta_, real *f_outputs, real *alpha_);


    virtual void eMSequenceInitialize(Sequence* inputs);

    /** note that this method returns in fact the euclidean distance between 
        the observation and the neirest cluster
    */
    virtual real frameLogProbability(int t, real *inputs);

		/** similarly to frameLogProbability, this method returns the euclidean
			  distance between cluster g and the given observation
		*/
		virtual real frameLogProbabilityOneGaussian(int g, real *inputs);


    virtual ~KMeans();
};


}

#endif
