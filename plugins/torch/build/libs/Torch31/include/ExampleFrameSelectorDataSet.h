// Copyright (C) 2003--2004 Johnny Mariethoz (Johnny.Mariethoz@idiap.ch)
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

#ifndef EXAMPLE_FRAME_SELECTOR_DATA_SET_INC
#define EXAMPLE_FRAME_SELECTOR_DATA_SET_INC

#include "DataSet.h"

namespace Torch {

struct InternalAMoi{
	int data_index;
	int start_inputs_frame;
	int start_targets_frame;
	int n_selected_inputs_frames;
	int n_selected_targets_frames;
};


/**
	This dataset is empty at the begining. Each subsequence of the original dataset
	can be added/remove by the methods:
	\begin{itemize}
	\item addExample
	\item removeExample
	\end{itemize}
	@see FrameSelectorDataSet
	@author Johnny Mariethoz (Johnny.Mariethoz@idiap.ch)
	*/

class ExampleFrameSelectorDataSet : public DataSet
{
  private:
    int n_pushed_examples;
    int n_max_pushed_examples;
		InternalAMoi* internal;
		int n_max_internal;
		int selected_example_size;

  public:
    DataSet *data;
    int *n_selected_input_frames;
    int *n_selected_target_frames;
    int **input_frames_indices;
    int **target_frames_indices;

    ///
    ExampleFrameSelectorDataSet(DataSet *data_);

		void addExample(int t, int inputs_start_indices_, int n_inputs_frames_, 
		 												int targets_start_indices_, int n_targets_frames_);

		void removeExample(int t);
		virtual void getNumberOfFrames(int t_, int *n_input_frames_, int *n_target_frames_);
    virtual void setRealExample(int t, bool set_inputs, bool set_targets);
    virtual void pushExample();
    virtual void popExample();

virtual void preProcess(PreProcessing *pre_processing);
    //-----

    virtual ~ExampleFrameSelectorDataSet();
};

}

#endif
