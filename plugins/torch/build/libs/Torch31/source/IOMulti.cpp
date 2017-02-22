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

#include "IOMulti.h"

namespace Torch {

IOMulti::IOMulti(IOSequence **io_files_, int n_files_)
{
  io_files = io_files_;
  n_files = n_files_;

  if(n_files <= 0)
    error("IOMulti: check the number of files!");

  n_sequences = 0;
  for(int i = 0; i < n_files; i++)
    n_sequences += io_files[i]->n_sequences;
  frame_size = io_files[0]->frame_size;

  indices = (int *)allocator->alloc(sizeof(int)*n_sequences);
  offsets = (int *)allocator->alloc(sizeof(int)*n_sequences);

  int *ptr_indices = indices;
  int *ptr_offsets = offsets;
  for(int i = 0; i < n_files; i++)
  {
    if(frame_size != io_files[i]->frame_size)
      error("IOMulti: provided IO have incompatible frame sizes");

    for(int j = 0; j < io_files[i]->n_sequences; j++)
    {
      *ptr_indices++ = i;
      *ptr_offsets++ = j;
    }
  }
}

void IOMulti::getSequence(int t, Sequence *sequence)
{
  io_files[indices[t]]->getSequence(offsets[t], sequence);
}

int IOMulti::getNumberOfFrames(int t)
{
  return io_files[indices[t]]->getNumberOfFrames(offsets[t]);
}

int IOMulti::getTotalNumberOfFrames()
{
  int n_total_frames_ = 0;
  for(int i = 0; i < n_files; i++)
    n_total_frames_ += io_files[i]->getTotalNumberOfFrames();
  return n_total_frames_;
}

IOMulti::~IOMulti()
{
}

}
