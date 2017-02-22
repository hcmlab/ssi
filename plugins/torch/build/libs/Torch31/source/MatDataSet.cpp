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

#include "IOBufferize.h"
#include "MatDataSet.h"
#include "IOAscii.h"
#include "IOMulti.h"
#include "IOBin.h"
#include "IOSub.h"

namespace Torch {

MatDataSet::MatDataSet(const char *filename, int n_inputs_, int n_targets_,
                       bool one_file_is_one_sequence, int max_load, bool binary_mode)
{
  io_allocator = new Allocator;
  if( (n_inputs_ < 0) && (n_targets < 0) )
    error("MatDataSet: cannot guess n_inputs <and> n_targets!");

  IOSequence *io_file = NULL;
  if(binary_mode)
    io_file = new(io_allocator) IOBin(filename, one_file_is_one_sequence, max_load);
  else
    io_file = new(io_allocator) IOAscii(filename, one_file_is_one_sequence, max_load);

  init_(io_file, n_inputs_, n_targets_);
}

MatDataSet::MatDataSet(char **filenames, int n_files_, int n_inputs_, int n_targets_,
                       bool one_file_is_one_sequence, int max_load, bool binary_mode)
{
  io_allocator = new Allocator;
  if(n_files_ <= 0)
    error("MatDataSet: check the number of files!");

  IOSequence **io_files = (IOSequence **)io_allocator->alloc(sizeof(IOSequence *)*n_files_);
  if(max_load > 0)
  {
    int i = 0;
    while( (max_load > 0) && (i < n_files_) )
    {
      if(binary_mode)
        io_files[i] = new(io_allocator) IOBin(filenames[i], one_file_is_one_sequence, max_load);
      else
        io_files[i] = new(io_allocator) IOAscii(filenames[i], one_file_is_one_sequence, max_load);
      max_load -= io_files[i]->n_sequences;
      i++;
    }
    n_files_ = i;
  }
  else
  {
    if(binary_mode)
    {
      for(int i = 0; i < n_files_; i++)
        io_files[i] = new(io_allocator) IOBin(filenames[i], one_file_is_one_sequence);
    }
    else
    {
      for(int i = 0; i < n_files_; i++)
        io_files[i] = new(io_allocator) IOAscii(filenames[i], one_file_is_one_sequence);
    }
  }

  IOMulti *io_file = new(io_allocator) IOMulti(io_files, n_files_);
  init_(io_file, n_inputs_, n_targets_);
}

MatDataSet::MatDataSet(char **input_filenames, char **target_filenames, int n_files_,
                       int max_load, bool binary_mode)
{
  IOSequence *io_inputs = NULL;
  IOSequence *io_targets = NULL;
  io_allocator = new Allocator;

  if(n_files_ <= 0)
    error("MatDataSet: check the number of files!");

  if(input_filenames)
  {
    IOSequence **input_io_files = (IOSequence **)io_allocator->alloc(sizeof(IOSequence *)*n_files_);
    int max_load_ = max_load;
    int n_files__ = 0;
    if(max_load_ > 0)
    {
      int i = 0;
      while( (max_load_ > 0) && (i < n_files_) )
      {
        if(binary_mode)
          input_io_files[i] = new(io_allocator) IOBin(input_filenames[i], true, max_load_);
        else
          input_io_files[i] = new(io_allocator) IOAscii(input_filenames[i], true, max_load_);
        max_load_ -= input_io_files[i]->n_sequences;
        i++;
      }
      n_files__ = i;
    }
    else
    {
      if(binary_mode)
      {
        for(int i = 0; i < n_files_; i++)
          input_io_files[i] = new(io_allocator) IOBin(input_filenames[i], true);
      }
      else
      {
        for(int i = 0; i < n_files_; i++)
          input_io_files[i] = new(io_allocator) IOAscii(input_filenames[i], true);
      }
      n_files__ = n_files_;
    }
    io_inputs = new(io_allocator) IOMulti(input_io_files, n_files__);
  }

  if(target_filenames)
  {
    IOSequence **target_io_files = (IOSequence **)io_allocator->alloc(sizeof(IOSequence *)*n_files_);
    int max_load_ = max_load;
    int n_files__ = 0;
    if(max_load_ > 0)
    {
      int i = 0;
      while( (max_load_ > 0) && (i < n_files_) )
      {
        if(binary_mode)
          target_io_files[i] = new(io_allocator) IOBin(target_filenames[i], true, max_load_);
        else
          target_io_files[i] = new(io_allocator) IOAscii(target_filenames[i], true, max_load_);
        max_load_ -= target_io_files[i]->n_sequences;
        i++;
      }
      n_files__ = i;
    }
    else
    {
      if(binary_mode)
      {
        for(int i = 0; i < n_files_; i++)
          target_io_files[i] = new(io_allocator) IOBin(target_filenames[i], true);
      }
      else
      {
        for(int i = 0; i < n_files_; i++)
          target_io_files[i] = new(io_allocator) IOAscii(target_filenames[i], true);
      }
      n_files__ = n_files_;
    }
    io_targets = new(io_allocator) IOMulti(target_io_files, n_files__);
  }

  MemoryDataSet::init(io_inputs, io_targets);
  message("MatDataSet: %d examples loaded [%d inputs and %d targets detected]", n_examples, n_inputs, n_targets);
  delete io_allocator;
}

void MatDataSet::init_(IOSequence *io_file, int n_inputs_, int n_targets_)
{
  IOSequence *io_inputs = NULL;
  IOSequence *io_targets = NULL;

  if( (n_inputs_ > io_file->frame_size) || (n_targets_ > io_file->frame_size) )
    error("MatDataSet: n_inputs (%d) or n_targets (%d) too large (> %d) !", n_inputs_, n_targets_, io_file->frame_size);

  if(n_inputs_ < 0)
    n_inputs_ = io_file->frame_size - n_targets_;

  if(n_targets_ < 0)
    n_targets_ = io_file->frame_size - n_inputs_;

  if(io_file->frame_size != (n_inputs_ + n_targets_))
    error("MatDataSet: %d columns in the file != %d inputs + %d targets", io_file->frame_size, n_inputs_, n_targets_);

  IOBufferize *io_buffer = NULL;
  if( (n_inputs_ > 0) && (n_targets_ > 0) )
    io_buffer = new(io_allocator) IOBufferize(io_file);

  if(n_inputs_ > 0)
  {
    if(n_targets_ > 0)
      io_inputs = new(io_allocator) IOSub(io_buffer, 0, n_inputs_);
    else
      io_inputs = io_file;
  }
  if(n_targets_ > 0)
  {
    if(n_inputs_ > 0)
      io_targets = new(io_allocator) IOSub(io_buffer, n_inputs_, n_targets_);
    else
      io_targets = io_file;
  }

  MemoryDataSet::init(io_inputs, io_targets);
  message("MatDataSet: %d examples loaded", n_examples);
  delete io_allocator;
}

MatDataSet::~MatDataSet()
{
}

}
