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

#include "FrameSelectorDataSet.h"

namespace Torch {

FrameSelectorDataSet::FrameSelectorDataSet(DataSet *data_)
{
  data = data_;
  if(data->n_examples == 0)
    error("FrameSelectorDataSet: cannot handle DataSet with no examples");

  n_selected_input_frames = (int *)allocator->alloc(sizeof(int)*data->n_examples);
  n_selected_target_frames = (int *)allocator->alloc(sizeof(int)*data->n_examples);
  input_frames_indices = (int **)allocator->alloc(sizeof(int *)*data->n_examples);
  target_frames_indices = (int **)allocator->alloc(sizeof(int *)*data->n_examples);
  for(int i = 0; i < data->n_examples; i++)
  {
    n_selected_input_frames[i] = 0;
    input_frames_indices[i] = NULL;
    n_selected_target_frames[i] = 0;
    target_frames_indices[i] = NULL;
  }

  DataSet::init(data->n_examples, data->n_inputs, data->n_targets);

  data->setExample(0);
  if(n_inputs > 0)
    inputs_buffer = new(allocator) Sequence(0, n_inputs);
  else
    inputs_buffer = NULL;
  if(n_targets > 0)
    targets_buffer = new(allocator) Sequence(0, n_targets);
  else
    targets_buffer = NULL;
}

void FrameSelectorDataSet::selectInputFrames(int t_, int *frames_indices_, int n_frames_)
{
  int t = selected_examples[t_];
  if(input_frames_indices[t])
    allocator->free(input_frames_indices[t]);
  input_frames_indices[t] = (int *)allocator->alloc(sizeof(int)*n_frames_);
  for(int i = 0; i < n_frames_; i++)
    input_frames_indices[t][i] = frames_indices_[i];
  n_selected_input_frames[t] = n_frames_;
}

void FrameSelectorDataSet::selectTargetFrames(int t_, int *frames_indices_, int n_frames_)
{
  int t = selected_examples[t_];
  if(target_frames_indices[t])
    allocator->free(target_frames_indices[t]);
  target_frames_indices[t] = (int *)allocator->alloc(sizeof(int)*n_frames_);
  for(int i = 0; i < n_frames_; i++)
    target_frames_indices[t][i] = frames_indices_[i];
  n_selected_target_frames[t] = n_frames_;
}

void FrameSelectorDataSet::unselectInputFrames(int t_)
{
  int t = selected_examples[t_];
  if(input_frames_indices[t])
    allocator->free(input_frames_indices[t]);
  input_frames_indices[t] = NULL;
  n_selected_input_frames[t] = 0;
}

void FrameSelectorDataSet::unselectTargetFrames(int t_)
{
  int t = selected_examples[t_];
  if(target_frames_indices[t])
    allocator->free(target_frames_indices[t]);
  target_frames_indices[t] = NULL;
  n_selected_target_frames[t] = 0;
}

void FrameSelectorDataSet::getNumberOfFrames(int t_, int *n_input_frames_, int *n_target_frames_)
{
  int t = selected_examples[t_];
  if( (n_inputs > 0) && n_input_frames_ )
  {
    if(input_frames_indices[t])
      *n_input_frames_ = n_selected_input_frames[t];
    else
      data->getNumberOfFrames(t, n_input_frames_, NULL);
  }
  if( (n_targets > 0) && n_target_frames_ )
  {
    if(target_frames_indices[t])
      *n_target_frames_ = n_selected_target_frames[t];
    else
      data->getNumberOfFrames(t, NULL, n_target_frames_);
  }
}

void FrameSelectorDataSet::setRealExample(int t, bool set_inputs, bool set_targets)
{
  data->setExample(t);

  if(set_inputs)
  {
    if(input_frames_indices[t])
    {
      inputs_buffer->resize(n_selected_input_frames[t], false);
      for(int i = 0; i < n_selected_input_frames[t]; i++)
        inputs_buffer->frames[i] = data->inputs->frames[input_frames_indices[t][i]];
      inputs = inputs_buffer;
    }
    else
      inputs = data->inputs;
  }

  if(set_targets)
  {
    if(target_frames_indices[t])
    {
      targets_buffer->resize(n_selected_target_frames[t], false);
      for(int i = 0; i < n_selected_target_frames[t]; i++)
        targets_buffer->frames[i] = data->targets->frames[target_frames_indices[t][i]];
      targets = targets_buffer;
    }
    else
      targets = data->targets;
  }  
  real_current_example_index = t;
}

void FrameSelectorDataSet::preProcess(PreProcessing *pre_processing)
{
  error("FrameSelectorDataSet: pre-processing not supported");
}

void FrameSelectorDataSet::pushExample()
{
  data->pushExample();
  pushed_examples->push(&inputs_buffer, sizeof(Sequence *));
  pushed_examples->push(&targets_buffer, sizeof(Sequence *));
  pushed_examples->push(&inputs, sizeof(Sequence *));
  pushed_examples->push(&targets, sizeof(Sequence *));
  pushed_examples->push(&real_current_example_index, sizeof(int));

  if(n_inputs > 0)
    inputs_buffer = new(allocator) Sequence(0, n_inputs);
  if(n_targets > 0)
    targets_buffer = new(allocator) Sequence(0, n_targets);
  real_current_example_index = -1;
}

void FrameSelectorDataSet::popExample()
{
  allocator->free(inputs_buffer);
  allocator->free(targets_buffer);

  pushed_examples->pop();
  pushed_examples->pop();
  pushed_examples->pop();
  pushed_examples->pop();
  pushed_examples->pop();
  data->popExample();
}

FrameSelectorDataSet::~FrameSelectorDataSet()
{
}

}
