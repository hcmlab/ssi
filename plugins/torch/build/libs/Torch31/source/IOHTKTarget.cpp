// Copyright (C) 2003--2004 Johnny Mariethoz (Johnny.Mariethoz@idiap.ch)
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

#include "IOHTKTarget.h"
#include "DiskXFile.h"

namespace Torch {

#ifdef USE_DOUBLE
#define REAL_FORMAT "%lf"
#else
#define REAL_FORMAT "%f"
#endif


IOHTKTarget::IOHTKTarget(const char *filename_, LexiconInfo* lex_, int n_per_frame_, bool words_)
{
  // Boaf...
  lexicon = lex_;
  n_per_frame = n_per_frame_;
  words = words_;

  one_file_is_one_sequence = true;
  
  filename = (char *)allocator->alloc(strlen(filename_)+1);
  strcpy(filename, filename_);

  // Read the file...
  file = new(allocator) DiskXFile(filename, "r");

    
  char buffer[80];
  file->gets(buffer,80);
  char* elements[10];
  elements[0] = strtok(buffer," \t");
  int n_element;
  for (n_element=1;(elements[n_element]=strtok(NULL," \t"));n_element++);
  if(n_element == 1){
    frame_size = 1;
  }else if (n_element == 3){
    frame_size = 2;
  }else 
    error("IOHTKTarget: this target file contains more nor 1 or 3 columns (%d)...",n_element);
  n_total_frames = 1;
  while(file->gets(buffer,80)){
    n_total_frames++;
  }
  n_sequences = 1;
  allocator->free(file);
}

int IOHTKTarget::getNumberOfFrames(int t)
{
    return n_total_frames;
}

int IOHTKTarget::getTotalNumberOfFrames()
{
  return n_total_frames;
}

void IOHTKTarget::saveSequence(XFile *file_, Sequence* sequence, LexiconInfo* lex_, int n_per_frame_, bool words_)
{
  for(int i = 0; i < sequence->n_frames; i++) {
    if (sequence->frame_size == 2) {
      int begin = i == 0 ? 0 : (int)(sequence->frames[i-1][1] * n_per_frame_);
      int end = (int)(sequence->frames[i][1] *n_per_frame_);

      file_->printf("%d",begin);
      file_->printf("%d",end);
    }
    if(words_)
      file_->printf("%s\n",lex_->vocabulary->words[(int)sequence->frames[i][0]]);
    else
      file_->printf("%s\n",lex_->phone_info->phone_names[(int)sequence->frames[i][0]]);
  } 
}

void IOHTKTarget::getSequence(int t, Sequence* sequence)
{
  // Read the file...
  char buffer[80];
  int begin;
  int end;
  file = new(allocator) DiskXFile(filename, "r");

  for(int i = 0; i < n_total_frames; i++) {
    real *dest_ = sequence->frames[i];
    if(frame_size == 2){
      file->scanf("%d",&begin);
      file->scanf("%d",&end);
      dest_[1] = (real)(end/n_per_frame); 
    }
    file->scanf("%s",&buffer);
    if(words)
      dest_[0] = lexicon->vocabulary->getIndex(buffer);
    else
      dest_[0] = lexicon->phone_info->getIndex(buffer);
		if (dest_[0] < 0)
			error("IOHTKTarget::getSequence: \"%s\" not found in %s", buffer, words? "vocabulary" : "phone set");
  }

  allocator->free(file);
}

IOHTKTarget::~IOHTKTarget()
{
}

}
