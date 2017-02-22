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

#include "ClassMeasurer.h"

namespace Torch {

ClassMeasurer::ClassMeasurer(Sequence *inputs_, DataSet *data_, ClassFormat *class_format_, XFile *file_,
                             bool calc_confusion_at_each_iter_) : Measurer(data_, file_)
{
  inputs = inputs_;
  class_format = class_format_;
  calc_confusion_at_each_iter = calc_confusion_at_each_iter_;
  confusion = NULL;

  n_classes = class_format->n_classes;
  if(calc_confusion_at_each_iter)
  {
    confusion   = (int **)allocator->alloc(sizeof(int*)*n_classes);

    for(int i = 0; i < n_classes; i++)
      confusion[i]   = (int *)allocator->alloc(sizeof(int)*n_classes);
  }
  reset_();
}

void ClassMeasurer::measureExample()
{
  for(int i = 0; i < inputs->n_frames; i++)
  {
    int c_obs = class_format->getClass(inputs->frames[i]);
    int c_des = class_format->getClass(data->targets->frames[i]);
    
    if(c_obs != c_des)
      internal_error += 1.;
    
    if(calc_confusion_at_each_iter)
      confusion[c_obs][c_des]++;
    n_examples++;
  }
}

void ClassMeasurer::measureIteration()
{
  internal_error /= n_examples;

  if(binary_mode)
    file->write(&internal_error, sizeof(real), 1);
  else
    file->printf("%g\n", internal_error);
  file->flush();

  if(calc_confusion_at_each_iter)
    printConfusionMatrix();

  reset();
}

void ClassMeasurer::printConfusionMatrix()
{
  if(binary_mode)
  {
    for(int i = 0; i < n_classes; i++)
      file->write(confusion[i], sizeof(real), n_classes);
  }
  else
  {
    file->printf("# Labels of classes:\n");
    for(int i = 0; i < n_classes; i++)
    {
      for(int j = 0; j < class_format->getOutputSize(); j++)
        file->printf("%g ", class_format->class_labels[i][j]);
      file->printf("\n");
    }
  
    file->printf("# Confusion matrix [rows: observed, colums: desired]:\n");
    for(int i = 0; i < n_classes; i++)
    {
      for(int j = 0; j < n_classes; j++)
        file->printf("%d ", confusion[i][j]);
      file->printf("\n");
    }
  }
  file->flush();
}

void ClassMeasurer::reset_()
{
  n_examples = 0;
  internal_error = 0;
  if(calc_confusion_at_each_iter)
  {
    for(int i = 0; i < n_classes; i++)
    {
      for(int j = 0; j < n_classes; j++)
        confusion[i][j] = 0;
    }
  }
}

void ClassMeasurer::reset()
{
  reset_();
}

ClassMeasurer::~ClassMeasurer()
{
}

}
