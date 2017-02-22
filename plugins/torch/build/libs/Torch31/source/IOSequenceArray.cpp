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

#include "IOSequenceArray.h"
#include "XFile.h"

namespace Torch {

IOSequenceArray::IOSequenceArray()
{
}

void IOSequenceArray::write(XFile *file, Sequence **sequences_array, int n_sequences)
{
  if(n_sequences <= 0)
    return;

  int frame_size = sequences_array[0]->frame_size;
  int n_total_frames = 0;
  for(int i = 0; i < n_sequences; i++)
  {
    if(frame_size != sequences_array[i]->frame_size)
      error("IOSequenceArray: sorry, sequences don't have the same frame size");
    n_total_frames += sequences_array[i]->n_frames;
  }
  file->taggedWrite(&n_total_frames, sizeof(int), 1, "NTF");
  file->taggedWrite(&frame_size, sizeof(int), 1, "FS");

  for(int i = 0; i < n_sequences; i++)
  {
    file->taggedWrite(&sequences_array[i]->n_frames, sizeof(int), 1, "NF");
    sequences_array[i]->saveXFile(file);
  }
}

void IOSequenceArray::read(XFile *file, Sequence **sequences_array, int n_sequences, Allocator *allocator_)
{
  if(!allocator_)
    allocator_ = allocator;

  int n_total_frames, frame_size;
  file->taggedRead(&n_total_frames, sizeof(int), 1, "NTF");
  file->taggedRead(&frame_size, sizeof(int), 1, "FS");

  Sequence *sequences_buffer = (Sequence *)allocator_->alloc(sizeof(Sequence)*n_sequences);
  real **frames_buffer = (real **)allocator_->alloc(sizeof(real *)*n_total_frames);
  real *buffer = (real *)allocator_->alloc(sizeof(real)*n_total_frames*frame_size);
  for(int i = 0; i < n_total_frames; i++)
    frames_buffer[i] = buffer+i*frame_size;

  for(int i = 0; i < n_sequences; i++)
  {
    int n_frames_;
    file->taggedRead(&n_frames_, sizeof(int), 1, "NF");
    sequences_array[i] = new(allocator_, sequences_buffer) Sequence(frames_buffer, n_frames_, frame_size);
    sequences_array[i]->loadXFile(file);
    frames_buffer += n_frames_;
    sequences_buffer++;
  }
}

IOSequenceArray::~IOSequenceArray()
{
}

}
