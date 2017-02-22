// Copyright (C) 2003--2004 Samy Bengio (bengio@idiap.ch)
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

#include "FrameSegMeasurer.h"

namespace Torch {

FrameSegMeasurer::FrameSegMeasurer(FrameSeg* frameseg_, DataSet *data_, XFile *file_,char** file_list_,int n_file_list_) : Measurer(data_, file_)
{
  frameseg = frameseg_;
	file_list = file_list_;
	n_file_list = n_file_list_;
  addBOption("print timing", &print_timing, false, "print timing");
  addBOption("print phoneme timing", &print_phoneme_timing, false, "print phoneme timing");
  addBOption("print desired timing", &print_desired_timing, false, "print timing for desired");
  addBOption("print with words", &print_with_words, false, "if timing, print with words");
  addBOption("print frame err", &print_frame_err, false, "print frame err");
  addBOption("print soft frame err", &print_soft_frame_err, false, "print soft frame err");
  addIOption("soft frame err length", &n_soft_frames, 1, "n soft frames");
  addIOption("n per frame", &n_per_frame, 1, "n per frame");
}

void FrameSegMeasurer::measureExample()
{
	XFile* xfile = file;
	bool empty = true;
	if(n_file_list)
		xfile = new(allocator)DiskXFile(file_list[data->real_current_example_index],"w");

  if (print_desired_timing && frameseg->target->frame_size>1) {
    int begin = 0;
    for (int j=0;j<frameseg->target->n_frames;j++) {
      int end = (int)frameseg->target->frames[j][1]*n_per_frame;
      xfile->printf("%d %d %s\n",begin,end,frameseg->lexicon->vocabulary->words[(int)frameseg->target->frames[j][0]]);
      begin = end;
    }
	if(!n_file_list)
    xfile->printf("--------\n");
  }
  if (print_timing) {
    int start_frame = 0;
    for (int i=0;i<frameseg->obtained->n_frames-1;i++) {
      int current = (int)frameseg->obtained->frames[i][0];
      int next = (int)frameseg->obtained->frames[i+1][0];
      if (current != next && next >=0) {
				empty = false;
        if (print_with_words && next>=0) {
          xfile->printf("%d %d %s-%s-%d\n",start_frame*n_per_frame,(i+1)*n_per_frame,frameseg->lexicon->vocabulary->words[frameseg->states_to_word[next]],frameseg->lexicon->phone_info->phone_names[frameseg->states_to_model[next]],frameseg->states_to_model_states[next]);
        } else if (!print_with_words && current>=0) {
          xfile->printf("%d %d %s-%d\n",start_frame*n_per_frame,(i+1)*n_per_frame,frameseg->lexicon->phone_info->phone_names[frameseg->states_to_model[next]],frameseg->states_to_model_states[next]);
        }
        start_frame = i+1;
      }
    }
	if(!n_file_list)
    xfile->printf("========\n");
  }
  if (print_phoneme_timing) {
    int start_frame = 0;
    for (int i=1;i<frameseg->obtained->n_frames-1;i++) {
      int current = (int)frameseg->obtained->frames[i][0];
      int next = (int)frameseg->obtained->frames[i+1][0];
      int ph_current = current >=0 ? frameseg->states_to_model[current] : -1;
      int ph_next = next >=0 ? frameseg->states_to_model[next] : -1;
      if (ph_current != ph_next) {
				empty = false;
        if (print_with_words && ph_next>=0) {
          xfile->printf("%d %d %s-%s\n",start_frame*n_per_frame,(i+1)*n_per_frame,frameseg->lexicon->vocabulary->words[frameseg->states_to_word[current]],frameseg->lexicon->phone_info->phone_names[ph_current]);
        } else if (!print_with_words && ph_current>=0) {
          xfile->printf("%d %d %s\n",start_frame*n_per_frame,(i+1)*n_per_frame,frameseg->lexicon->phone_info->phone_names[ph_current]);
        }
        start_frame = i+1;
      }
    }
	if(!n_file_list)
    xfile->printf("========\n");
  }
	if(empty && n_file_list)
		warning("File: %s can be empty",file_list[data->real_current_example_index]);
  if (print_frame_err && frameseg->target->frame_size>1) {
    for (int i=0;i<frameseg->obtained->n_frames;i++) {
      int current = (int)frameseg->obtained->frames[i][0];
      int obtained_word = current >=0 ? frameseg->states_to_word[current] : -1;
      // compute desired word
      int desired_word = -1;
      int previous = 0;
      for (int j=0;j<frameseg->target->n_frames;j++) {
        if (i >= previous && i < frameseg->target->frames[j][1]) {
          desired_word = (int)frameseg->target->frames[j][0];
          break;
        }
        previous = (int)frameseg->target->frames[j][1];
      }
      if (obtained_word != desired_word) {
        n_incorrect ++;
      } else {
        n_correct ++;
      }
      n_frames++;
    }
  }
  if (print_soft_frame_err && frameseg->target->frame_size>1) {
    for (int i=0;i<frameseg->obtained->n_frames;i++) {
      int current = (int)frameseg->obtained->frames[i][0];
      int obtained_word = current >= 0 ? frameseg->states_to_word[current] : -1;
      // compute desired word
      int desired_word = -1;
      for (int l=0;l<n_soft_frames;l++) {
        int previous = 0;
        for (int j=0;j<frameseg->target->n_frames;j++) {
          if (i+l >= previous && i+l < frameseg->target->frames[j][1]) {
            desired_word = (int)frameseg->target->frames[j][0];
            break;
          }
          previous = (int)frameseg->target->frames[j][1];
        }
        if (obtained_word == desired_word)
          break;
        previous = 0;
        for (int j=0;j<frameseg->target->n_frames;j++) {
          if (i-l >= previous && i-l < frameseg->target->frames[j][1]) {
            desired_word = (int)frameseg->target->frames[j][0];
            break;
          }
          previous = (int)frameseg->target->frames[j][1];
        }
        if (obtained_word == desired_word)
          break;
      }
      if (obtained_word != desired_word) {
        n_incorrect_soft ++;
      } else {
        n_correct_soft ++;
      }
      n_frames_soft++;
    }
  }
  xfile->flush();
	if(n_file_list)
		allocator->free(xfile);	
}

void FrameSegMeasurer::measureIteration()
{
  if (print_frame_err) {
    file->printf("n_correct %d (%5.2f%%) n_incorrect %d (%5.2f%%) n %d\n",n_correct,100.*n_correct/n_frames,n_incorrect,100.*n_incorrect/n_frames,n_frames);
  }
  if (print_soft_frame_err) {
    file->printf("n_soft_correct %d (%5.2f%%) n_soft_incorrect %d (%5.2f%%) n %d len %d\n",n_correct_soft,100.*n_correct_soft/n_frames_soft,n_incorrect_soft,100.*n_incorrect_soft/n_frames_soft,n_frames_soft,n_soft_frames);
  }
  file->flush();
}

void FrameSegMeasurer::reset()
{
  n_correct = n_incorrect = n_frames = 0;
  n_correct_soft = n_incorrect_soft = n_frames_soft = 0;
}

FrameSegMeasurer::~FrameSegMeasurer()
{
}


}

