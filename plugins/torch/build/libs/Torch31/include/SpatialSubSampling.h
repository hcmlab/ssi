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

#ifndef SPATIAL_SUB_SAMPLING_INC
#define SPATIAL_SUB_SAMPLING_INC

#include "GradientMachine.h"

namespace Torch {

/** Class for doing sub-sampling over images.
    
    Suppose you put #n_input_planes# images in each input frame.
    The images are in one big vector: each input frame has a size of
    #n_input_planes*input_height*input_width#. (image after image).
    Thus, #n_inputs = n_input_planes*input_height*input_width#.

    Then, for each output planes, it takes its associated input plane
    and it computes the convolution of the input image with a kernel
    of size #k_w*k_w#, where the weights of the kernel are equals.

    The output image size is computed in the constructor and
    put in #output_height# and #output_width#.
    #n_outputs = n_input_planes*output_height*output_width#.

    Note that, depending of the size of your kernel, several (last) input columns
    or rows of the image could be lost.

    Note also that \emph{no} non-linearity is applied in this layer.

    @author Ronan Collobert (collober@idiap.ch)
*/
class SpatialSubSampling : public GradientMachine
{
  public:
    /// Kernel size (height and width).
    int k_w;

    /// 'x' translation \emph{in the input image} after each application of the kernel.
    int d_x;

    /// 'y' translation \emph{in the input image} after each application of the kernel.
    int d_y;

    /// Number of input images. The number of output images in sub-sampling is the same.
    int n_input_planes;

    /// Height of each input image.
    int input_height;

    /// Width of each input image.
    int input_width;

    /// Height of each output image.
    int output_height;

    /// Width of each output image.
    int output_width;

    /** #weights[i]# means kernel-weight for output plane #i#.
        #weights[i]# contains only one weight.
    */
    real *weights;

    /// Derivatives associated to #weights#.
    real *der_weights;

    /// #biases[i]# is the bias for output plane #i#.
    real *biases;

    /// Derivatives associated to #biases#.
    real *der_biases;

    /// Create a sub-sampling layer...
    SpatialSubSampling(int n_input_planes_, int width_, int height_, int k_w_=2, int d_x_=2, int d_y_=2);

    //-----

    void reset_();
    virtual void reset();
    virtual void frameForward(int t, real *f_inputs, real *f_outputs);
    virtual void frameBackward(int t, real *f_inputs, real *beta_, real *f_outputs, real *alpha_);

    virtual ~SpatialSubSampling();
};

}

#endif
