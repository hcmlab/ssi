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


#include "WordSeg.h"

namespace Torch {

WordSeg::WordSeg(LexiconInfo* lexicon_) 
{
  lexicon = lexicon_;

  target_word_sequence = NULL;
  target_word_sequence_size = 0;
  target_word_sequence_max_size = 0;

  word_sequence = NULL;
  word_sequence_time = NULL;
  word_sequence_size = 0;
  word_sequence_max_size = 0;

}

void WordSeg::resize(int new_size)
{
  if (word_sequence_max_size < new_size) {
    word_sequence_max_size = new_size;
    word_sequence = (int*)allocator->realloc(word_sequence,word_sequence_max_size*sizeof(int));
    word_sequence_time = (int*)allocator->realloc(word_sequence_time,word_sequence_max_size*sizeof(int));
  }
}

void WordSeg::resizeTargets(int new_size) 
{
  if (new_size > target_word_sequence_max_size) {
    target_word_sequence_max_size = new_size;
    target_word_sequence =
      (int*)allocator->realloc(target_word_sequence,new_size*sizeof(int));
  }

}

WordSeg::~WordSeg()
{
}

}

