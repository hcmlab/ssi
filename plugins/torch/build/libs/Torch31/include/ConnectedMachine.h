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

#ifndef CONNECTED_MACHINE_INC
#define CONNECTED_MACHINE_INC

#include "GradientMachine.h"

namespace Torch {

struct ConnectedNode
{
    GradientMachine *machine;
    Sequence **input_links;
    Sequence **alpha_links;
    int *alpha_links_offset;
    int n_input_links;
    int n_alpha_links;
    int n_inputs_check;

    Sequence *inputs;
    Sequence *alpha;
};

/** Easy connections between several #GradientMachine#.
    GradientMachine has "layers" on which you can
    add some #GradientMachine#.

    The inputs of the machine on the first layer will
    be the inputs of the #ConnectedMachine#.

    The outputs of the #ConnectedMachine# are the union
    (in the order of adding) of all machines on the
    last layer.

    @author Ronan Collobert (collober@idiap.ch)
*/
class ConnectedMachine : public GradientMachine
{
  private:
    Sequence *start_alpha;
    int current_alpha_offset;
    int current_layer;
    int current_machine;
    void checkInternalLinks();

  public:
    ConnectedNode ***machines;
    int *n_machines_on_layer;
    int n_layers;

    //-----

    ///
    ConnectedMachine();

    /** Add a Full Connected Layer. The #machine# is fully connected
        to the previous layer. If necessary, a layer is added before
        adding the machine.
    */
    void addFCL(GradientMachine *machine);
    
    /// Add a #machine# on the current layer
    void addMachine(GradientMachine *machine);

    /** Connect the last added machine on #machine#.
        Note that #machine# \emph{must} be in a previous layer.
    */
    void connectOn(GradientMachine *machine);

    /// Add a layer (you don't have to call that for the first layer)
    void addLayer();

    /** Contruct the machine... you need to call that after adding and
        connecting all the machines. */
    void build();

    //-----

    virtual void reset();
    virtual void iterInitialize();
    virtual void forward(Sequence *inputs);
    virtual void backward(Sequence *inputs, Sequence *alpha);
    virtual void loadXFile(XFile *file);
    virtual void saveXFile(XFile *file);
    virtual void setPartialBackprop(bool flag=true);
    virtual void setDataSet(DataSet *dataset_);

    virtual ~ConnectedMachine();
};

}

#endif
