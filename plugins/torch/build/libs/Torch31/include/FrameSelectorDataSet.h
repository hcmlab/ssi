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

#ifndef FRAME_SELECTOR_DATA_SET_INC
#define FRAME_SELECTOR_DATA_SET_INC

#include "DataSet.h"

namespace Torch {

/** A dataset used to select some frames of another dataset.
    It takes a dataset in the constructor. Then you call
    select functions to select frames. After that, when
    you will do a #setExample()# the example of the previous
    dataset will be returned in #inputs# and #targets# fields,
    with the right frames...

    @author Ronan Collobert (collober@idiap.ch)
 */
class FrameSelectorDataSet : public DataSet
{
  private:
    Sequence *inputs_buffer;
    Sequence *targets_buffer;

  public:
    /// DataSet where we will select frames
    DataSet *data;

    /// Number of input selected frames, for each example
    int *n_selected_input_frames;

    /// Number of target selected frames, for each example
    int *n_selected_target_frames;

    /// Indices of input selected frames, for each example
    int **input_frames_indices;

    /// Indices of target selected frames, for each example
    int **target_frames_indices;

    ///
    FrameSelectorDataSet(DataSet *data_);

    /** Select input frames of the example #t#.
        Frames indices are given by #frames_indices_#.
        The size of #frames_indices_# is given by #n_frames_#.
        Takes in account pushed subsets.
    */
    void selectInputFrames(int t_, int *frames_indices_, int n_frames_);

    /// Same as #selectInputFrames()#, but for targets.
    void selectTargetFrames(int t_, int *frames_indices_, int n_frames_);

    /** Unselect inputs frames of the example #t#.
        Takes in account pushed subsets.
    */
    void unselectInputFrames(int t_);

    /** Unselect targets frames of the example #t#.
        Takes in account pushed subsets.
    */
    void unselectTargetFrames(int t_);

    virtual void preProcess(PreProcessing *pre_processing);
    virtual void getNumberOfFrames(int t_, int *n_input_frames_, int *n_target_frames_);
    virtual void setRealExample(int t, bool set_inputs=true, bool set_targets=true);
    virtual void pushExample();
    virtual void popExample();

    //-----

    virtual ~FrameSelectorDataSet();
};

}

#endif
