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

#include "DiskDataSet.h"

namespace Torch {

DiskDataSet::DiskDataSet()
{
  io_inputs = NULL;
  io_targets = NULL;

  pre_processes = new(allocator) PreProcessingList;
}

void DiskDataSet::init(IOSequence *io_inputs_, IOSequence *io_targets_)
{
  io_inputs = io_inputs_;
  io_targets = io_targets_;

  int n_examples_ = 0;
  if(io_inputs)
    n_examples_ = io_inputs->n_sequences;
  if(io_targets)
    n_examples_ = io_targets->n_sequences;

  if(io_inputs && io_targets)
  {
    if(io_inputs->n_sequences != io_targets->n_sequences)
      error("DiskDataSet: inputs IO and targets IO don't have the same number of sequences!");
  }

  DataSet::init(n_examples_, (io_inputs ? io_inputs->frame_size : 0), (io_targets ? io_targets->frame_size : 0));

  if(n_inputs > 0)
    inputs = new(allocator) Sequence(0, n_inputs);
  if(n_targets > 0)
    targets = new(allocator) Sequence(0, n_targets);
}

void DiskDataSet::setRealExample(int t, bool set_inputs, bool set_targets)
{
  // Rq: marche car dataset par avec un etat indetermine...
  if(t == real_current_example_index)
    return;

  if( (n_inputs > 0) && set_inputs )
  {
    int n_frames = io_inputs->getNumberOfFrames(t);
    inputs->resize(n_frames);
    io_inputs->getSequence(t, inputs);

    if(pre_processes)
    {
      for(int i = 0; i < pre_processes->n_nodes; i++)
        pre_processes->nodes[i]->preProcessInputs(inputs);
    }
  }

  if( (n_targets > 0) && set_targets )
  {
    int n_frames = io_targets->getNumberOfFrames(t);
    targets->resize(n_frames);
    io_targets->getSequence(t, targets);

    if(pre_processes)
    {
      for(int i = 0; i < pre_processes->n_nodes; i++)
        pre_processes->nodes[i]->preProcessTargets(targets);
    }
  }

  real_current_example_index = t;
}

void DiskDataSet::pushExample()
{
  pushed_examples->push(&inputs, sizeof(Sequence *));
  pushed_examples->push(&targets, sizeof(Sequence *));
  pushed_examples->push(&real_current_example_index, sizeof(int));

  if(n_inputs > 0)
    inputs = new(allocator) Sequence(0, n_inputs);
  if(n_targets > 0)
    targets = new(allocator) Sequence(0, n_targets);

  real_current_example_index = -1;
}

void DiskDataSet::popExample()
{
  allocator->free(inputs);
  allocator->free(targets);

  pushed_examples->pop();
  pushed_examples->pop();
  pushed_examples->pop();
}

void DiskDataSet::getNumberOfFrames(int t, int *n_input_frames, int *n_target_frames)
{
  if( (n_inputs > 0) && n_input_frames )
    *n_input_frames = io_inputs->getNumberOfFrames(selected_examples[t]);
  if( (n_targets > 0) && n_target_frames )
    *n_target_frames = io_targets->getNumberOfFrames(selected_examples[t]);
}

void DiskDataSet::preProcess(PreProcessing *pre_processing)
{
  pre_processes->addNode(pre_processing);
}

DiskDataSet::~DiskDataSet()
{
}

}
