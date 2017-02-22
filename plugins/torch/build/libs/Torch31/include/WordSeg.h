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

#ifndef WORD_SEG_INC
#define WORD_SEG_INC

#include "LexiconInfo.h"

namespace Torch {

/** This class keeps track of all information to compute word errors

    @author Samy Bengio (bengio@idiap.ch)
*/
class WordSeg : public Object
{
  public:

    /// the corresponding lexicon
    LexiconInfo* lexicon;
    
    /// the target word sequence
    int* target_word_sequence;
    /// the length of the target word sequence
    int target_word_sequence_size;
    /// the length of the longest target word sequence
    int target_word_sequence_max_size;

    /// the word sequence
    int* word_sequence;
    /// the length of the word sequence
    int word_sequence_size;
    /// the length of the longest word sequence
    int word_sequence_max_size;
    /// the starting time of each word of the word sequence
    int* word_sequence_time;

    ///
    WordSeg(LexiconInfo* lexicon_);

    virtual void resize(int new_size);
    virtual void resizeTargets(int new_size);

    virtual ~WordSeg();
};


}

#endif
