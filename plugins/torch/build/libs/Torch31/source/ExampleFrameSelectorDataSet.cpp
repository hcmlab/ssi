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

#include "ExampleFrameSelectorDataSet.h"

namespace Torch {

ExampleFrameSelectorDataSet::ExampleFrameSelectorDataSet(DataSet *data_)
{
  data = data_;
  if(data->n_examples == 0)
    error("ExampleFrameSelectorDataSet: cannot handle DataSet with no examples");

	internal = NULL;
	
	DataSet::init(data->n_examples, data->n_inputs, data->n_targets);
	n_max_internal = n_examples;
	selected_example_size = n_examples;

  data->setExample(0);
  if(n_inputs > 0)
    inputs = new(allocator) Sequence(0, n_inputs);
  else
    inputs = NULL;

  if(n_targets > 0)
    targets = new(allocator) Sequence(0, n_targets);
	else
		targets = NULL;
  
  n_pushed_examples = 0;
  n_max_pushed_examples = 0;
  pushed_examples = NULL;
	n_max_internal = 0;
	n_examples = 0;
	n_real_examples = n_examples;
}

void ExampleFrameSelectorDataSet::addExample(int t, int inputs_start_indices_, int n_inputs_frames_, int targets_start_indices_, int n_targets_frames_)
{
	int index = n_examples++;
	n_real_examples = n_examples;
	if(n_examples > n_max_internal){
		n_max_internal++;
		internal = (InternalAMoi *)allocator->realloc(internal, sizeof(InternalAMoi)*(n_max_internal));
	}
	internal[index].data_index = t;
	internal[index].start_inputs_frame = inputs_start_indices_;
	internal[index].start_targets_frame = targets_start_indices_;
	internal[index].n_selected_inputs_frames = n_inputs_frames_;
	internal[index].n_selected_targets_frames = n_targets_frames_;
  
	if(n_examples >= selected_example_size){
		selected_example_size *= 2;
		allocator->free(selected_examples);
		selected_examples = (int *)allocator->alloc(sizeof(int)*selected_example_size);
		for(int i = 0; i < selected_example_size; i++)
			selected_examples[i] = i;
	}
}

void ExampleFrameSelectorDataSet::preProcess(PreProcessing *pre_processing)
{
	error("ExampleFrameSelectorDataSet: preProcess() not supported !!");
}

void ExampleFrameSelectorDataSet::removeExample(int t){

	InternalAMoi* ptr = internal+t;
	InternalAMoi* to_copy_ptr = internal+t+1;
	for(int i=t;i<n_examples;i++){
		*ptr++ = *to_copy_ptr++;
	}
	n_examples--;
	n_real_examples = n_examples;
}

void ExampleFrameSelectorDataSet::getNumberOfFrames(int t_, int *n_input_frames_, int *n_target_frames_)
{
  if (n_input_frames_)
	  *n_input_frames_ = internal[t_].n_selected_inputs_frames;
  if (n_target_frames_)
	  *n_target_frames_ = internal[t_].n_selected_targets_frames;
}

void ExampleFrameSelectorDataSet::setRealExample(int t, bool set_inputs, bool set_targets)
{
	InternalAMoi* ptr = internal + t;
  data->setExample(ptr->data_index, set_inputs, set_targets);

  if(set_inputs){
		inputs->resize(ptr->n_selected_inputs_frames, false);
		for(int i = 0; i < ptr->n_selected_inputs_frames; i++)
			inputs->frames[i] = data->inputs->frames[ptr->start_inputs_frame+i];
	}
	
  if(set_targets)
  {
		targets->resize(ptr->n_selected_targets_frames, false);
		for(int i = 0; i < ptr->n_selected_targets_frames; i++)
			targets->frames[i] = data->targets->frames[ptr->start_targets_frame+i];
	}

  real_current_example_index = t;
}

void ExampleFrameSelectorDataSet::pushExample()
{
  data->pushExample();
 
  pushed_examples->push(&inputs, sizeof(Sequence *));
  pushed_examples->push(&targets, sizeof(Sequence *));
  pushed_examples->push(&real_current_example_index, sizeof(int));

  if(n_inputs > 0)
    inputs = new(allocator) Sequence(0, n_inputs);
  if(n_targets > 0)
    targets = new(allocator) Sequence(0, n_targets);
  real_current_example_index = -1;
}

void ExampleFrameSelectorDataSet::popExample()
{
  allocator->free(inputs);
  allocator->free(targets);

  pushed_examples->pop();
  pushed_examples->pop();
  pushed_examples->pop();
  data->popExample();
}


ExampleFrameSelectorDataSet::~ExampleFrameSelectorDataSet()
{
}

}
