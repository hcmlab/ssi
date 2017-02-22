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

#include "MSEMeasurer.h"

namespace Torch {

MSEMeasurer::MSEMeasurer(Sequence *inputs_, DataSet *data_, XFile *file_) : Measurer(data_, file_)
{
  inputs = inputs_;
  internal_error = 0;
  addBOption("average examples", &average_examples, true, "divided by the number of examples");
  addBOption("average frame size", &average_frame_size, true, "divided by the frame size");
  addBOption("average frames", &average_frames, true, "divided by the number of frames");
}

void MSEMeasurer::measureExample()
{
  Sequence *desired = data->targets;

  real sum = 0;
  for(int i = 0; i < inputs->n_frames; i++)
  {
    real *src_1 = desired->frames[i];
    real *src_2 = inputs->frames[i];
    for(int j = 0; j < inputs->frame_size; j++)
    {
      real z = src_2[j] - src_1[j];
      sum += z*z;
    }
  }
  if(average_frames)
    sum /= inputs->n_frames;
  if(average_frame_size)
    sum /= inputs->frame_size;
  internal_error += sum;
}

void MSEMeasurer::measureIteration()
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

void MSEMeasurer::reset()
{
  internal_error = 0;
}

MSEMeasurer::~MSEMeasurer()
{
}

}
