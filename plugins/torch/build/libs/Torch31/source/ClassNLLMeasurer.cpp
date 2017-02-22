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

#include "ClassNLLMeasurer.h"

namespace Torch {

ClassNLLMeasurer::ClassNLLMeasurer(Sequence *inputs_, DataSet *data_, ClassFormat *class_format_, XFile *file_) : Measurer(data_, file_)
{
  class_format = class_format_;
  inputs = inputs_;
  internal_error = 0;

  addBOption("average examples", &average_examples, true, "divided by the number of examples");
  addBOption("average frames", &average_frames, true, "divided by the number of frames");
}

void ClassNLLMeasurer::measureExample()
{
  real sum = 0;
  for(int i = 0; i < inputs->n_frames; i++)
  {
    int the_class = class_format->getClass(data->targets->frames[i]);
    sum += inputs->frames[i][the_class];
  }
  if(average_frames)
    sum /= inputs->n_frames;

  internal_error -= sum;
}

void ClassNLLMeasurer::measureIteration()
{
  if(average_examples)
    internal_error /= data->n_examples;

  if(binary_mode)
    file->write(&internal_error, sizeof(real), 1);
  else
    file->printf("%g\n", internal_error);
  file->flush();
  reset();
}

void ClassNLLMeasurer::reset()
{
  internal_error = 0;
}

ClassNLLMeasurer::~ClassNLLMeasurer()
{
}

}
