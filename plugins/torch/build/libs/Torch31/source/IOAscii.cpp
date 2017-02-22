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

#include "IOAscii.h"

namespace Torch {

#ifdef USE_DOUBLE
#define REAL_FORMAT "%lf"
#else
#define REAL_FORMAT "%f"
#endif

void IOAscii::saveSequence(XFile *file, Sequence *sequence)
{
  file->printf("%d %d\n", sequence->n_frames, sequence->frame_size);    
  for(int i = 0; i < sequence->n_frames; i++)
  {
    real *z = sequence->frames[i];
    for(int j = 0; j < sequence->frame_size; j++)
      file->printf("%g ", z[j]);
    file->printf("\n");
  }
}

IOAscii::IOAscii(const char *filename_, bool one_file_is_one_sequence_, int max_load_)
{
  // Boaf...
  one_file_is_one_sequence = one_file_is_one_sequence_;
  max_load = max_load_;

  filename = (char *)allocator->alloc(strlen(filename_)+1);
  strcpy(filename, filename_);

  // Read the header...
  DiskXFile f(filename, "r");

  f.scanf("%d", &n_total_frames);
  f.scanf("%d", &frame_size);

  if( (max_load > 0) && (max_load < n_total_frames) && (!one_file_is_one_sequence) )
  {
    n_total_frames = max_load;
    message("IOAscii: loading only %d frames", n_total_frames);
  }

  if(one_file_is_one_sequence)
    n_sequences = 1;
  else
    n_sequences = n_total_frames;

  file = NULL;
  current_frame_index = -1;
}

void IOAscii::getSequence(int t, Sequence *sequence)
{
  // Cas simple: on lit tout le bordel
  if(one_file_is_one_sequence)
  {
    file = new(allocator) DiskXFile(filename, "r");
    int murielle;
    file->scanf("%d", &murielle);
    file->scanf("%d", &murielle);
    for(int i = 0; i < n_total_frames; i++)
    {
      real *dest_ = sequence->frames[i];
      for(int j = 0; j < frame_size; j++)
        file->scanf(REAL_FORMAT, &dest_[j]);
    }
    allocator->free(file);
  }
  else
  {
    // Sequentiel ?
    if(t != current_frame_index+1)
      error("IOAscii: sorry, data are accessible only in a sequential way");

    // Doit-on ouvrir le putain de fichier ?
    if(current_frame_index < 0)
    {
      file = new(allocator) DiskXFile(filename, "r");
      int murielle;
      file->scanf("%d", &murielle);
      file->scanf("%d", &murielle);
    }

    // Lis la frame mec
    real *dest_ = sequence->frames[0];
    for(int j = 0; j < frame_size; j++)
      file->scanf(REAL_FORMAT, &dest_[j]);
    
    // Si je suis a la fin du fichier, je le zigouille.
    current_frame_index++;
    if(current_frame_index == n_total_frames-1)
    {
      allocator->free(file);
      current_frame_index = -1;
    }
  }
}

int IOAscii::getNumberOfFrames(int t)
{
  if(one_file_is_one_sequence)
    return n_total_frames;
  else
    return 1;
}

int IOAscii::getTotalNumberOfFrames()
{
  return n_total_frames;
}

IOAscii::~IOAscii()
{
}

}
