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

#ifndef SVM_CLASSIFICATION_INC
#define SVM_CLASSIFICATION_INC

#include "SVM.h"

namespace Torch {


/** SVM in classification.

    Try to find the hyperplane $w.x+b = 0$
    as
    $(w,b)$ minimize $0.5*||w||^2 + \sum_j C_j |1- y_j*(w.x_j+b)|_+$

    (where $|x|_+ = x$ if $x > 0$, else $0$)

    (in fact, we use a #kernel# instead of a dot product)

    The $C_j$ coefficients are given in #C_# when you
    call the constructor. If this one is NULL, then
    the value given by the "C" option is used for
    all $C_j$.
    (The size of #C_# \emph{must be} #data->n_real_examples#)

    Options:
    \begin{tabular}{lcll}
      "C"          &  real &  trade off between the weight decay and the error & [100] \\
      "cache size" & real  &  cache size (in Mo)                               & [50]
    \end{tabular}

    @author Ronan Collobert (collober@idiap.ch)
*/
class SVMClassification : public SVM
{
  private:
    char *sequences_buffer;
    char *frames_buffer;

  public:
    real cache_size_in_megs;
    real *Cuser;
    real C_cst;

    //-----

    ///
    SVMClassification(Kernel *kernel_, real *C_=NULL, IOSequenceArray *io_sequence_array_=NULL);

    //-----

    virtual void setDataSet(DataSet *dataset_);
    virtual void checkSupportVectors();
    virtual ~SVMClassification();
};

}

#endif
