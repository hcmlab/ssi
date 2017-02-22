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

#ifndef MEMORY_DATA_SET_INC
#define MEMORY_DATA_SET_INC

#include "DataSet.h"
#include "IOSequence.h"

namespace Torch {

/** DataSet where data is fully loaded in memory.

    Inputs and targets are put in two arrays: #inputs_array# and #targets_array#.
    You can fill these fields by using the #init()# method (if you are using
    IOSequence). But you could imagine a MemoryDataSet where you fill these fields
    by hand (based on the #init()# method).
    MatDataSet is a good example if you plan to do a new MemoryDataSet.

    @see MatDataSet
    @author Ronan Collobert (collober@idiap.ch)
*/
class MemoryDataSet : public DataSet
{
  private:
    virtual void allocData(IOSequence *io_torch, Sequence **sequences_array);

  public:

    /// Inputs array.
    Sequence **inputs_array;

    /// Targets array.
    Sequence **targets_array;

    ///
    MemoryDataSet();

    /** May help you to initialize the DataSet if you're using IOSequence.
        You should call this method in the constructor of your subsclasses.
        Initialize #n_examples#, #n_real_examples#, #n_inputs# and #n_targets#.
        #inputs_array# and #targets_array# filled with sequences given by the IOSequence classes.
    */
    void init(IOSequence *io_inputs, IOSequence *io_outputs);

    /// Override current inputs array.
    void setInputs(Sequence **inputs_, int n_sequences_);

    /// Override current targets array.
    void setTargets(Sequence **targets_, int n_sequences_);

    virtual void getNumberOfFrames(int t, int *n_input_frames_, int *n_target_frames_);
    virtual void preProcess(PreProcessing *pre_processing);
    virtual void setRealExample(int t, bool set_inputs=true, bool set_targets=true);
    virtual void pushExample();
    virtual void popExample();

    //-----

    virtual ~MemoryDataSet();
};

}

#endif
