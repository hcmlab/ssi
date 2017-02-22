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

#include "IOBin.h"

namespace Torch {

void IOBin::saveSequence(XFile *file, Sequence *sequence)
{
  file->write(&sequence->n_frames, sizeof(int), 1);
  file->write(&sequence->frame_size, sizeof(int), 1);

  for(int i = 0; i < sequence->n_frames; i++)
    file->write(sequence->frames[i], sizeof(real), sequence->frame_size);
}

IOBin::IOBin(const char *filename_, bool one_file_is_one_sequence_, int max_load_, bool is_sequential_)
{
  // Boaf...
  one_file_is_one_sequence = one_file_is_one_sequence_;
  max_load = max_load_;
  is_sequential = is_sequential_;

  filename = (char *)allocator->alloc(strlen(filename_)+1);
  strcpy(filename, filename_);

  // Read the header...
  DiskXFile f(filename, "r");

  f.read(&n_total_frames, sizeof(int), 1);
  f.read(&frame_size, sizeof(int), 1);

  if( (max_load > 0) && (max_load < n_total_frames) && (!one_file_is_one_sequence) )
  {
    n_total_frames = max_load;
    message("IOBin: loading only %d frames", n_total_frames);
  }

  if(one_file_is_one_sequence)
    n_sequences = 1;
  else
    n_sequences = n_total_frames;
  
  file = NULL;
  current_frame_index = -1;  
}

void IOBin::getSequence(int t, Sequence *sequence)
{
  // Cas simple: on lit tout le bordel
  if(one_file_is_one_sequence)
  {
    file = new(allocator) DiskXFile(filename, "r");
    int murielle;
    file->read(&murielle, sizeof(int), 1); // fseek non car marche pas dans pipes
    file->read(&murielle, sizeof(int), 1);
    for(int i = 0; i < n_total_frames; i++)
      file->read(sequence->frames[i], sizeof(real), frame_size);
    allocator->free(file);
  }
  else
  {
    // Sequentiel ?
    if(is_sequential)
    {
      if(t != current_frame_index+1)
        error("IOBin: sorry, data are accessible only in a sequential way");
      
      // Doit-on ouvrir le putain de fichier ?
      if(current_frame_index < 0)
      {
        file = new(allocator) DiskXFile(filename, "r");
        int murielle;
        file->read(&murielle, sizeof(int), 1); // fseek non car marche pas dans pipes
        file->read(&murielle, sizeof(int), 1);
      }
    }
    else
    {
      file = new(allocator) DiskXFile(filename, "r");
      if(file->seek(t*frame_size*sizeof(real)+2*sizeof(int), SEEK_CUR) != 0)
        error("IOBin: cannot seek in your file!");
    }

    // Lis la frame mec
    file->read(sequence->frames[0], sizeof(real), frame_size);

    if(is_sequential)
    {
      // Si je suis a la fin du fichier, je le zigouille.
      current_frame_index++;
      if(current_frame_index == n_total_frames-1)
      {
        allocator->free(file);
        current_frame_index = -1;
      }
    }
    else
      allocator->free(file);
  }
}

int IOBin::getNumberOfFrames(int t)
{
  if(one_file_is_one_sequence)
    return n_total_frames;
  else
    return 1;
}

int IOBin::getTotalNumberOfFrames()
{
  return n_total_frames;
}

IOBin::~IOBin()
{
}

}
