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

#include "OutputMeasurer.h"

namespace Torch {

OutputMeasurer::OutputMeasurer(DataSet *data_, XFile *file_) : Measurer(data_, file_)
{
  sequences = new(allocator) SequenceList;
  total_frame_size = 0;
}

void OutputMeasurer::addSequence(Sequence *sequence)
{
  sequences->addNode(sequence);
  total_frame_size += sequence->frame_size;
}

void OutputMeasurer::measureExample()
{
  if(sequences->n_nodes == 0)
    return;
  
  int n_frames = sequences->nodes[0]->n_frames;
  for(int i = 0; i < sequences->n_nodes; i++)
  {
    if(sequences->nodes[i]->n_frames != n_frames)
      error("OutputMeasurer: sorry, sequences don't have the same number of frames");
  }

  if(binary_mode)
  {
    file->write(&n_frames, sizeof(int), 1);
    file->write(&total_frame_size, sizeof(int), 1);
    for(int i = 0; i < n_frames; i++)
    {
      for(int j = 0; j < sequences->n_nodes; j++)
        file->write(sequences->nodes[j]->frames[i], sizeof(real), sequences->nodes[j]->frame_size);
    }
  }
  else
  {
    file->printf("%d %d\n", n_frames, total_frame_size);
    for(int i = 0; i < n_frames; i++)
    {
      for(int j = 0; j < sequences->n_nodes; j++)
      {
        real *src = sequences->nodes[j]->frames[i];
        for(int k = 0; k < sequences->nodes[j]->frame_size; k++)
          file->printf("%g ", src[k]);
      }
      file->printf("\n");
    }
  }
}

OutputMeasurer::~OutputMeasurer()
{
}

}
