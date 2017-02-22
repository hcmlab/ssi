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

#ifndef MEAN_VAR_NORM_INC
#define MEAN_VAR_NORM_INC

#include "PreProcessing.h"
#include "DataSet.h"

namespace Torch {

/** In the constructor, it computes the mean and the standard deviation
    over all the frames in the given DataSet (by default, only for inputs).
    Then, when calling pre-processing methods, it normalizes each column
    by this computed mean and stdv. (substracts the mean, then divides
    by the standard deviation).

    As a result, the mean of the full set of frames given by the concatenation
    of all calls to #setExample()# will be 0, and the variance will be 1.

    @author Ronan Collobert (collober@idiap.ch)
*/
class MeanVarNorm : public PreProcessing
{
  private:
    void normalizeSequence(Sequence *sequence, real *mean, real *stdv);

  public:
    /// Input frame size
    int n_inputs;

    /// Target frame size
    int n_targets;

    /// Inputs means array
    real *inputs_mean;

    /// Targets means array
    real *targets_mean;

    /// Inputs standard deviations array
    real *inputs_stdv;

    /// Targets standard deviations array
    real *targets_stdv;

    ///
    MeanVarNorm(DataSet *data, bool norm_inputs=true, bool norm_targets=false);

	// added 2007/10/11
	// Johannes Wagner <wagner@hcm-lab.de>
	MeanVarNorm (int n_inputs_, int n_targets_);

    virtual void preProcessInputs(Sequence *inputs);
    virtual void preProcessTargets(Sequence *targets);

    /// Load means and standard deviations
    virtual void loadXFile(XFile *file);
    /// Save means and standard deviations
    virtual void saveXFile(XFile *file);

    virtual ~MeanVarNorm();
};

}

#endif
