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

#ifndef DISK_DATA_SET_INC
#define DISK_DATA_SET_INC

#include "DataSet.h"
#include "IOSequence.h"
#include "List.h"

namespace Torch {

/** Provides an interface to manipulate all kind of data which are
    kept on disk, and not fully loaded in memory.
    It uses #IOSequence#.
    Usefull for large databases.
    DiskMatDataSet is a good example if you plan to code a new DiskDataSet.

    @see DiskMatDataSet
    @see IOSequence
    @author Ronan Collobert (collober@idiap.ch)
 */
class DiskDataSet : public DataSet
{
  public:
    /// List if pre processes to do
    PreProcessingList *pre_processes;

    /// IOMatrix which provides inputs.
    IOSequence *io_inputs;

    /// IOMatrix which provides targets.
    IOSequence *io_targets;

    ///
    DiskDataSet();

    /** This function \emph{has to be called} by your sub-classes.
        You give here the IOMatrix which handle the inputs
        and the targets of your dataset.
        Should be called in the constructor of all yoyr sub-classes.
     */
    void init(IOSequence *io_inputs_, IOSequence *io_targets_);

    virtual void getNumberOfFrames(int t, int *n_input_frames, int *n_target_frames);
    virtual void preProcess(PreProcessing *pre_processing);
    virtual void setRealExample(int t, bool set_inputs=true, bool set_targets=true);
    virtual void pushExample();
    virtual void popExample();

    //-----

    virtual ~DiskDataSet();
};

}

#endif
