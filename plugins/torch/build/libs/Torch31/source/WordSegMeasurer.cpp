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

#include "WordSegMeasurer.h"
#include "DiskXFile.h"

namespace Torch {

WordSegMeasurer::WordSegMeasurer(WordSeg* wordseg_, DataSet *data_, XFile *file_,EditDistance* edit_distance_,char** file_list_,int n_file_list_) : Measurer(data_, file_)
{
  wordseg = wordseg_;
  edit_distance = edit_distance_;
  if (edit_distance) {
    sum_edit_distance = new(allocator)EditDistance(edit_distance->is_confusion);
    sum_edit_distance->reset();
  } else {
    sum_edit_distance = NULL;
  }
	file_list = file_list_;
	n_file_list = n_file_list_;
  addBOption("print targets", &print_targets, true, "print targets");
  addBOption("print obtained", &print_obtained, true, "print obtained");
  addBOption("print timing", &print_timing, false, "print timing");
  addBOption("print desired timing", &print_desired_timing, false, "print desired timing");
  addIOption("n per frame", &n_per_frame, 1, "n per frame");
}

void WordSegMeasurer::measureExample()
{
	XFile* xfile = file;
	if(n_file_list)
		xfile = new(allocator)DiskXFile(file_list[data->real_current_example_index],"w");

  if (print_obtained) {
    xfile->printf("obtained: ");
    for (int j=0;j<wordseg->word_sequence_size;j++) {
      xfile->printf("%s ",wordseg->lexicon->vocabulary->words[wordseg->word_sequence[j]]);
    }
    xfile->printf("\n");
  }
  if (print_timing && wordseg->word_sequence_size>0) {
    for (int j=0;j<wordseg->word_sequence_size-1;j++) {
      xfile->printf("%d %d %s\n",wordseg->word_sequence_time[j]*n_per_frame,wordseg->word_sequence_time[j+1]*n_per_frame,wordseg->lexicon->vocabulary->words[wordseg->word_sequence[j]]);
    }
    xfile->printf("%d %d %s\n",wordseg->word_sequence_time[wordseg->word_sequence_size-1]*n_per_frame,data->inputs->n_frames*n_per_frame,wordseg->lexicon->vocabulary->words[wordseg->word_sequence[wordseg->word_sequence_size-1]]);
  }

  if (print_targets) {
    xfile->printf("desired: ");
    for (int j=0;j<wordseg->target_word_sequence_size;j++)
      xfile->printf("%s ",wordseg->lexicon->vocabulary->words[wordseg->target_word_sequence[j]]);
    xfile->printf("\n");
  }
  if (print_desired_timing) {
    Sequence* targets = data->targets;
    if (targets->frame_size > 1) {
      int begin = 0;
      for (int j=0;j<targets->n_frames;j++) {
        int end = (int)targets->frames[j][1]*n_per_frame;
        xfile->printf("%d %d %s\n",begin,end,wordseg->lexicon->vocabulary->words[(int)targets->frames[j][0]]);
        begin = end;
      }
    }
  }
  if (edit_distance) {
    edit_distance->reset();
    edit_distance->distance(wordseg->word_sequence,wordseg->word_sequence_size,
      wordseg->target_word_sequence,wordseg->target_word_sequence_size);
    sum_edit_distance->add(edit_distance);
  }
  xfile->flush();
	
	if(n_file_list)
		allocator->free(xfile);	
		
}

void WordSegMeasurer::measureIteration()
{
  if (sum_edit_distance) {
    if (sum_edit_distance->is_confusion) {
      for (int i=0;i<wordseg->lexicon->vocabulary->n_words;i++)
        file->printf("%s ",wordseg->lexicon->vocabulary->words[i]);
      file->printf("\n");
    }
    sum_edit_distance->print(file);
    sum_edit_distance->printRatio(file);
  }
  file->flush();
}

void WordSegMeasurer::reset()
{
  if (sum_edit_distance) {
    sum_edit_distance->reset();
  }
}

WordSegMeasurer::~WordSegMeasurer()
{
}


}

