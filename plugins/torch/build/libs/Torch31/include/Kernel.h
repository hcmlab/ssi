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

#ifndef KERNEL_INC
#define KERNEL_INC

#include "Object.h"
#include "Sequence.h"

namespace Torch {

/** General Kernel class.
    Kernels are applied on two sequences with the method #eval()#.
    Actually, usual kernels (given here) don't use sequences,
    but the first frame of each sequences...

    @author Ronan Collobert (collober@idiap.ch)
*/
class Kernel : public Object
{
  public:

    ///
    Kernel();

    /// Compute kernel between the example #x# and #y#.
    virtual real eval(Sequence *x, Sequence *y) = 0;

    //-----

    virtual ~Kernel();
};

/** DotProduct Kernel.
    k(x,y) = s*(x.y).

    @author Ronan Collobert (collober@idiap.ch)
 */
class DotKernel : public Kernel
{
  public:
    real s;

    ///
    DotKernel(real s_=1.);

    virtual real eval(Sequence *x, Sequence *y);
    virtual ~DotKernel();
};


/** Polynomial Kernel $k(x,y) = (s*x.y+r)^d$.
    
    @author Ronan Collobert (collober@idiap.ch)
*/
class PolynomialKernel : public Kernel
{
  public:
    int d;
    real s, r;

    ///
    PolynomialKernel(int degree, real s_=1., real r_=1.);

    virtual real eval(Sequence *x, Sequence *y);
    virtual ~PolynomialKernel();
};

/** Gaussian Kernel $k(x,y) = exp(-g * ||x-y||^2)$
    
    @author Ronan Collobert (collober@idiap.ch)
*/
class GaussianKernel : public Kernel
{
  public:
    real g;

    ///
    GaussianKernel(real g_);

    virtual real eval(Sequence *x, Sequence *y);
    virtual ~GaussianKernel();
};

/** Sigmoid Kernel $k(x,y) = tanh(s*x.y+r)$
    
    @author Ronan Collobert (collober@idiap.ch)
*/
class SigmoidKernel : public Kernel
{
  public:
    real s, r;

    ///
    SigmoidKernel(real s_, real r_);

    virtual real eval(Sequence *x, Sequence *y);
    virtual ~SigmoidKernel();
};

}

#endif
