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

#ifndef TEMPORAL_SUB_SAMPLING_INC
#define TEMPORAL_SUB_SAMPLING_INC

#include "GradientMachine.h"

namespace Torch {

/** Class for doing sub-sampling over a sequence.
    
    Then, for each component of output frames, it takes its associated input component
    and it computes the convolution of the input sequence with a kernel
    of size #k_w#, over the time, where the weights of the kernel are equals.

    Note that, depending of the size of your kernel, several (last) frames
    of the input seqience could be lost.

    Note also that \emph{no} non-linearity is applied in this layer.

    @author Ronan Collobert (collober@idiap.ch)
*/
class TemporalSubSampling : public GradientMachine
{
  public:
    /// Kernel size.
    int k_w;

    /// Time translation after one application of the kernel.
    int d_t;
    
    /** #weights[i]# means kernel-weights for the #i#-th component of output frames.
        #weights[i]# contains only one weight.
    */
    real *weights;

    /// Derivatives associated to #weights#.
    real *der_weights;

    /// #biases[i]# is the bias for the #i#-th component of output frames.
    real *biases;

    /// Derivatives associated to #biases#.
    real *der_biases;
    
    /// Create a sub-sampling layer...
    TemporalSubSampling(int input_frame_size, int k_w_=2, int d_t_=2);

    //-----
    
    void reset_();
    virtual void reset();
    virtual void forward(Sequence *inputs);
    virtual void backward(Sequence *inputs, Sequence *alpha);

    virtual ~TemporalSubSampling();
};

}

#endif
