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

#include "Sequence.h"
#include "XFile.h"

namespace Torch {

Sequence::Sequence()
{
  // General
  frames = NULL;
  n_real_frames = 0;
  n_frames = 0;
  frame_size = 0;
}

Sequence::Sequence(real **frames_, int n_frames_, int frame_size_)
{
  frames = frames_;
  n_real_frames = n_frames_;
  n_frames = n_frames_;
  frame_size = frame_size_;
}

Sequence::Sequence(int n_frames_, int frame_size_)
{
  // General
  n_real_frames = n_frames_;
  n_frames = n_frames_;
  frame_size = frame_size_;
  if(n_frames > 0)    
    frames = (real **)allocator->alloc(sizeof(real *)*n_frames);
  else
  {
    frames = NULL;
    n_frames = 0;
  }

  if(frame_size <= 0)
    error("Sequence: try to create a sequence with a negative size");

  if(n_frames > 0)
  {
    real *data_buffer = NULL;
    data_buffer = (real *)allocator->alloc(sizeof(real)*n_frames*frame_size);
    for(int i = 0; i < n_frames; i++)
      frames[i] = data_buffer+i*frame_size;
  }
}

void Sequence::resize(int n_frames_, bool allocate_new_frames)
{
  // Do we have already this frames in memory ? OOooOOh cooOOool...
  if(n_real_frames >= n_frames_)
  {
    n_frames = n_frames_;
    return;
  }

  // Allocate array of frames and possibly the frames...
  // If frame_size is 0 (or user explicit specification), frames won't be allocated  
  reallocFramesArray(n_frames_);
  if( (frame_size == 0) || (!allocate_new_frames) )
  {
    for(int i = n_real_frames; i < n_frames_; i++)
      frames[i] = NULL;
  }
  else
  {
    real *data_buffer = (real *)allocator->alloc(sizeof(real)*(n_frames_-n_real_frames)*frame_size);
    for(int i = n_real_frames; i < n_frames_; i++)
      frames[i] = data_buffer+(i-n_real_frames)*frame_size;
  }

  n_frames = n_frames_;
  n_real_frames = n_frames_;
}

void Sequence::addFrame(real *frame, bool do_copy)
{
  if(!frame)
    return;

  reallocFramesArray(n_real_frames+1);
  if(do_copy)
  {
    frames[n_real_frames] = (real *)allocator->alloc(sizeof(real)*frame_size);
    real *frame_dest = frames[n_real_frames];
    for(int i = 0; i < frame_size; i++)
      frame_dest[i] = frame[i];
  }
  else
    frames[n_real_frames] = frame;

  if(n_real_frames == n_frames)
    n_frames++;
  n_real_frames++;
}

void Sequence::add(Sequence *sequence, bool do_copy)
{
  if(!sequence)
    return;

  if(frame_size != sequence->frame_size)
    error("Sequence: try to add a sequence with a wrong frame size");

  reallocFramesArray(n_real_frames+sequence->n_frames);
  if(do_copy)
  {
    for(int i = 0; i < sequence->n_frames; i++)
    {
      frames[n_real_frames+i] = (real *)allocator->alloc(sizeof(real)*frame_size);
      real *frame_dest = frames[n_real_frames+i];
      real *frame_src = sequence->frames[i];
      for(int i = 0; i < frame_size; i++)
        frame_dest[i] = frame_src[i];
    }
  }
  else
  {
    for(int i = 0; i < sequence->n_frames; i++)
      frames[n_real_frames+i] = sequence->frames[i];
  }

  if(n_real_frames == n_frames)
    n_frames += sequence->n_frames;

  n_real_frames += sequence->n_frames;
}

void Sequence::copy(Sequence *from)
{
  int src_frame_size = from->frame_size;
  int src_frame_number = 0;
  int src_index_in_frame = 0;

  real *src_frame = from->frames[0];
  for(int i = 0; i < n_frames; i++)
  {
    real *dest_frame = frames[i];
    for(int j = 0; j < frame_size; j++)
    {
      if(src_index_in_frame == src_frame_size)
      {
        src_index_in_frame = 0;
        src_frame = from->frames[++src_frame_number];
      }
      dest_frame[j] = src_frame[src_index_in_frame++];
    }
  }
}

void Sequence::reallocFramesArray(int n_frames_)
{
  if(allocator->isMine(frames))
    frames = (real **)allocator->realloc(frames, sizeof(real *)*(n_frames_));
  else
  {
    real **frames_ = (real **)allocator->alloc(sizeof(real *)*(n_frames_));
    for(int i = 0; i < n_real_frames; i++)
      frames_[i] = frames[i];
    frames = frames_;
  }
}

void Sequence::copyFrom(real *vec)
{
  for(int i = 0; i < n_frames; i++)
  {
    real *frame_ = frames[i];
    for(int j = 0; j < frame_size; j++)
      frame_[j] = vec[j];
    vec += frame_size;
  }
}

void Sequence::copyTo(real *vec)
{
  for(int i = 0; i < n_frames; i++)
  {
    real *frame_ = frames[i];
    for(int j = 0; j < frame_size; j++)
      vec[j] = frame_[j];
    vec += frame_size;
  }
}

int Sequence::getSequenceSpace()
{
  return(sizeof(Sequence));
}

int Sequence::getFramesSpace()
{
  return(frame_size*n_frames*sizeof(real)+n_frames*sizeof(real *));
}

Sequence *Sequence::clone(Allocator *allocator_, void *sequence_memory, void *frames_memory)
{
  if(!allocator_)
    allocator_ = allocator;

  real **frames_ = (real **)frames_memory;
  if(frames_memory)
  {
    real *buffer = (real *)(frames_ + n_frames);
    for(int i = 0; i < n_frames; i++)
      frames_[i] = buffer+i*frame_size;
  }

  Sequence *the_clone = NULL;
  if(sequence_memory)
  {
    if(frames_memory)
      the_clone = new(allocator_, sequence_memory) Sequence(frames_, n_frames, frame_size);
    else
      the_clone = new(allocator_, sequence_memory) Sequence(n_frames, frame_size);
  }
  else
  {
    if(frames_memory)
      the_clone = new(allocator_) Sequence(frames_, n_frames, frame_size);
    else
      the_clone = new(allocator_) Sequence(n_frames, frame_size);
  }

  the_clone->copy(this);

  return the_clone;
}

void Sequence::loadXFile(XFile *file)
{
  for(int i = 0; i < n_frames; i++)
    file->taggedRead(frames[i], sizeof(real), frame_size, "FRAME");
}

void Sequence::saveXFile(XFile *file)
{
  for(int i = 0; i < n_frames; i++)
    file->taggedWrite(frames[i], sizeof(real), frame_size, "FRAME");
}

Sequence::~Sequence()
{
}

IMPLEMENT_NEW_LIST(SequenceList, Sequence)

}
