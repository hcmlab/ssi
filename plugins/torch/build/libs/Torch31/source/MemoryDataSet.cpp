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

#include "MemoryDataSet.h"

namespace Torch {

MemoryDataSet::MemoryDataSet()
{
  inputs_array = NULL;
  targets_array = NULL;
}

void MemoryDataSet::setRealExample(int t, bool set_inputs, bool set_targets)
{
  real_current_example_index = t;
  if(inputs_array && set_inputs)
    inputs = inputs_array[t];
  if(targets_array && set_targets)
    targets = targets_array[t];
}

void MemoryDataSet::pushExample()
{
  pushed_examples->push(&inputs, sizeof(Sequence *));
  pushed_examples->push(&targets, sizeof(Sequence *));
  pushed_examples->push(&real_current_example_index, sizeof(int));
}

void MemoryDataSet::popExample()
{
  pushed_examples->pop();
  pushed_examples->pop();
  pushed_examples->pop();
}

void MemoryDataSet::init(IOSequence *io_inputs, IOSequence *io_targets)
{
  int n_examples_ = 0;
  if(io_inputs)
    n_examples_ = io_inputs->n_sequences;
  if(io_targets)
    n_examples_ = io_targets->n_sequences;

  if(io_inputs && io_targets)
  {
    if(io_inputs->n_sequences != io_targets->n_sequences)
      error("MemoryDataSet: inputs IO and targets IO don't have the same number of sequences!");
  }

  // The parent
  DataSet::init(n_examples_, (io_inputs ? io_inputs->frame_size : 0), (io_targets ? io_targets->frame_size : 0));
  
  // Yeah.
  if(n_inputs > 0)
  {
    inputs_array = (Sequence **)allocator->alloc(sizeof(Sequence *)*n_examples);
    allocData(io_inputs, inputs_array);
  }

  if(n_targets > 0)
  {
    targets_array = (Sequence **)allocator->alloc(sizeof(Sequence *)*n_examples);
    allocData(io_targets, targets_array);
  }

  for(int t = 0; t < n_examples; t++)
  {
    if(n_inputs > 0)
      io_inputs->getSequence(t, inputs_array[t]);
    if(n_targets > 0)
      io_targets->getSequence(t, targets_array[t]);
  }
}

void MemoryDataSet::setInputs(Sequence **inputs_, int n_sequences_)
{
  if(n_sequences_ <= 0)
    error("MemoryDataSet: invalid number of sequences in provided inputs");

  // Deja alloue ?
  if(selected_examples)
  {
    if(n_sequences_ != n_real_examples)
      error("MemoryDataSet: invalid number of sequences in provided inputs");
  }
  else
    DataSet::init(n_sequences_, inputs_[0]->frame_size, 0);

  n_inputs = inputs_[0]->frame_size;
  for(int t = 0; t < n_sequences_; t++)
  {
    if(inputs_[t]->frame_size != n_inputs)
      error("MemoryDataSet: sorry, provided inputs sequences don't have the same frame size");
  }
  inputs_array = inputs_;
}

void MemoryDataSet::setTargets(Sequence **targets_, int n_sequences_)
{
  if(n_sequences_ <= 0)
    error("MemoryDataSet: invalid number of sequences in provided targets");

  // Deja alloue ?
  if(selected_examples)
  {
    if(n_sequences_ != n_real_examples)
      error("MemoryDataSet: invalid number of sequences in provided targets");
  }
  else
    DataSet::init(n_sequences_, 0, targets_[0]->frame_size);

  n_targets = targets_[0]->frame_size;
  for(int t = 0; t < n_sequences_; t++)
  {
    if(targets_[t]->frame_size != n_targets)
      error("MemoryDataSet: sorry, provided targets sequences don't have the same frame size");
  }
  targets_array = targets_;
}

void MemoryDataSet::getNumberOfFrames(int t, int *n_input_frames_, int *n_target_frames_)
{
  if( (n_inputs > 0) && n_input_frames_ )
    *n_input_frames_ = inputs_array[selected_examples[t]]->n_frames;
  if( (n_targets > 0) && n_target_frames_ )
    *n_target_frames_ = targets_array[selected_examples[t]]->n_frames;
}

void MemoryDataSet::allocData(IOSequence *io_torch, Sequence **sequences_array)
{
  int n_total_frames = io_torch->getTotalNumberOfFrames();
  int frame_size = io_torch->frame_size;

  Sequence *sequences_buffer = (Sequence *)allocator->alloc(sizeof(Sequence)*n_examples);
  real **frames_pointers_buffer = (real **)allocator->alloc(sizeof(real *)*n_total_frames);
  real *frames_buffer = (real *)allocator->alloc(sizeof(real)*n_total_frames*frame_size);
  for(int i = 0; i < n_total_frames; i++)
    frames_pointers_buffer[i] = frames_buffer+i*frame_size;

  for(int t = 0; t < io_torch->n_sequences; t++)
  {
    int n_frames = io_torch->getNumberOfFrames(t);
    sequences_array[t] = new(allocator, sequences_buffer) Sequence(frames_pointers_buffer, n_frames, frame_size);
    sequences_buffer++;
    frames_pointers_buffer += n_frames;
  }
}

void MemoryDataSet::preProcess(PreProcessing *pre_processing)
{
  for(int t = 0; t < n_examples; t++)
  {
    setExample(t);
    if(n_inputs > 0)
      pre_processing->preProcessInputs(inputs);
    if(n_targets > 0)
      pre_processing->preProcessTargets(targets);
  }
}

MemoryDataSet::~MemoryDataSet()
{
}

}
