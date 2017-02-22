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

#ifndef SVM_INC
#define SVM_INC

#include "QCMachine.h"
#include "Kernel.h"
#include "DataSet.h"
#include "IOSequenceArray.h"

namespace Torch {

/** Support Vector Machine.
    
    The Q matrix of #QCMachine# is in this case
    $q_{ij} = k(x_i, x_j)$, where $k$ is a kernel
    and $x_i$ is the i-th example of #data#/
    
    The goal is to looking for the #alpha# and #b#
    which are the best in a SVM-sense.

    The learning function is
    $f(x) = \sum_j y_j alpha_j k(x_i, x) + b$
    
    @author Ronan Collobert (collober@idiap.ch)
*/
class SVM : public QCMachine
{
  public:
    /// To allocate all stuff related to support vectors.
    Allocator *sv_allocator;

    /// Give the sequence-format.
    IOSequenceArray *io_sequence_array;

    /** The dataset associated to the SVM.
    */
    DataSet *data;

    /// The kernel associated to the SVM.
    Kernel *kernel;

    ///
    real b;

    /// The support vectors
    int *support_vectors;

    /// sv_alpha[i] is the weight of the SV i.
    real *sv_alpha;

    /// SV sequences.
    Sequence **sv_sequences;

    /// The number of support vectors
    int n_support_vectors;

    /// The number of support vectors which are at the bound "C"
    int n_support_vectors_bound;

    //-----
    
    ///
    SVM(Kernel *kernel_, IOSequenceArray *io_sequence_array_=NULL);

    /// Computes the #b#.
    bool bCompute();

    //-----

    virtual void forward(Sequence *inputs);
    virtual void loadXFile(XFile *file);
    virtual void saveXFile(XFile *file);

    virtual ~SVM();    
};

}

#endif
