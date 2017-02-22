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

#ifndef GRADIENT_MACHINE_INC
#define GRADIENT_MACHINE_INC

#include "Machine.h"
#include "Parameters.h"

namespace Torch {

/** Gradient machine: machine which can
    be trained with a gradient descent.

    Gradient machines take in inputs sequences which have always the same
    frame size, given by #n_inputs#, and outputs sequences which have always
    the same frame size too, given by #n_outputs#.

    @see StochasticGradient
    @author Ronan Collobert (collober@idiap.ch)
*/
class GradientMachine : public Machine
{
  public:
    /* Internal flag to know if we do the backprop with respect to
       the inputs */
    bool partial_backprop;

    /// Frame size of inputs sequences.
    int n_inputs;

    /// Frame size of outputs sequences.
    int n_outputs;

    /** Contains all parameters which will be
        updated with the gradient descent.
        Almost all machines will have only one
        node in params.
    */
    Parameters *params;

    /** Contains the derivatives for all parameters.
        Warning: #params# and #der_params#
        must have the same structure.
    */
    Parameters *der_params;

    /// Contains the derivative with respect to the inputs.
    Sequence *beta;

    //-----

    /** Initialize a gradient machine with #n_inputs_# for the input frame size,
        #n_outputs_# for the output frame size and #n_params_# parameters.
        If #n_inputs_# is 0, no #beta# sequence will be allocated.
        If #n_outputs_# is 0, no #outputs# sequence will be allocated.
    */
    GradientMachine(int n_inputs_, int n_outputs_, int n_params_=0);

    /** This function is called before each
        training iteration.
        By default, do nothing.
    */
    virtual void iterInitialize();

    /** Given a sequence, update #outputs#.
        By default, it uses #frameForward()#, to update each output frame
        given each input frame. It supposes by default the number of input
        and output frames is the same.
    */
    virtual void forward(Sequence *inputs);

    /** Given a sequence, update the derivative with respect to the input (#beta#)
        and #der_params#. If #partial_backprop# is false, don't update #beta#.
        By default, it uses #frameBackward()#, to update each beta frame
        given each input and alpha frame. It supposes by default the number of input
        and output frames is the same.
    */
    virtual void backward(Sequence *inputs, Sequence *alpha);

    /// Set the partial backprop flag...
    virtual void setPartialBackprop(bool flag=true);

    /** Given a frame #f_inputs#, updates #f_outputs#. Used to easily create new classes.
        It is called by the default #forward()#, and it does nothing by default.
        If your machine needs to do special things on sequence (if input sequence do not
        have the same size as the output sequence), don't overload this function, but
        overload #forward()#. #t# is the current frame to be forwarded.
    */
    virtual void frameForward(int t, real *f_inputs, real *f_outputs);

    /** Given the #f_inputs# and the derivatives #alpha_# with
        respect to the outputs, updates the derivative with respect to the inputs (#beta_#)
        and #der_params#.
        It is called by the default #backward()#, and it does nothing by default.
        If your machine needs to do special things on sequence (if input sequence do not
        have the same size as the output sequence), don't overload this function, but
        overload #backward()#. #t# is the current frame to be back-propagated.
    */
    virtual void frameBackward(int t, real *f_inputs, real *beta_, real *f_outputs, real *alpha_);

    /// By default, load the #params# field.
    virtual void loadXFile(XFile *file);

    /// By default, save the #params# field.
    virtual void saveXFile(XFile *file);

    //-----

    virtual ~GradientMachine();
};

}

#endif
