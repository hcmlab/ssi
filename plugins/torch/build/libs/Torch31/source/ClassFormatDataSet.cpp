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

#include "ClassFormatDataSet.h"

namespace Torch {

ClassFormatDataSet::ClassFormatDataSet(DataSet *data_, Sequence *class_labels_)
{
  data = data_;
  class_labels = class_labels_;

  DataSet::init(data->n_examples, data->n_inputs, class_labels->frame_size);

  inputs = NULL;
  if(n_targets > 0)
    targets = new(allocator) Sequence(0, n_targets);
}

ClassFormatDataSet::ClassFormatDataSet(DataSet *data_, int n_classes)
{
  data = data_;
  
  if(n_classes <= 0)
  {
    n_classes = 0;
    for(int t = 0; t < data->n_examples; t++)
    {
      data->setExample(t, false, true);
      for(int i = 0; i < data->targets->n_frames; i++)
      {
        int z = (int)data->targets->frames[i][0];
        if(z > n_classes)
          n_classes = z;
      }
    }
    n_classes++;
    message("ClassFormatDataSet: %d classes detected", n_classes);
  }

  class_labels = new(allocator) Sequence(n_classes, n_classes);
  for(int cl = 0; cl < n_classes; cl++)
  {
    memset(class_labels->frames[cl], 0, sizeof(real)*n_classes);
    class_labels->frames[cl][cl] = 1.;
  }
  
  DataSet::init(data->n_examples, data->n_inputs, n_classes);

  inputs = NULL;
  if(n_targets > 0)
    targets = new(allocator) Sequence(0, n_targets);
}

void ClassFormatDataSet::getNumberOfFrames(int t_, int *n_input_frames_, int *n_target_frames_)
{
  int t = selected_examples[t_];
  if( (n_inputs > 0) && n_input_frames_ )
    data->getNumberOfFrames(t, n_input_frames_, NULL);

  if( (n_targets > 0) && n_target_frames_ )
    data->getNumberOfFrames(t, NULL, n_target_frames_);
}

void ClassFormatDataSet::setRealExample(int t, bool set_inputs, bool set_targets)
{
  data->setExample(t, set_inputs, set_targets);
  inputs = data->inputs;

  if(set_targets)
  {
    targets->resize(data->targets->n_frames);
    for(int i = 0; i < data->targets->n_frames; i++)
    {
      int cl = (int)data->targets->frames[i][0];
      memcpy(targets->frames[i], class_labels->frames[cl], sizeof(real)*n_targets);
    }
  }
  real_current_example_index = t;
}

void ClassFormatDataSet::preProcess(PreProcessing *pre_processing)
{
  error("ClassFormatDataSet: pre-processing not supported");
}

void ClassFormatDataSet::pushExample()
{
  data->pushExample();
  pushed_examples->push(&inputs, sizeof(Sequence *));
  pushed_examples->push(&targets, sizeof(Sequence *));
  pushed_examples->push(&real_current_example_index, sizeof(int));
  if(n_targets > 0)
    targets = new(allocator) Sequence(0, n_targets);
  real_current_example_index = -1;
}

void ClassFormatDataSet::popExample()
{
  allocator->free(targets);
  pushed_examples->pop();
  pushed_examples->pop();
  pushed_examples->pop();
  data->popExample();
}

ClassFormatDataSet::~ClassFormatDataSet()
{
}

}
