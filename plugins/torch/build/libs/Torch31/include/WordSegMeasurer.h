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

#ifndef WORD_SEG_MEASURER_INC
#define WORD_SEG_MEASURER_INC

#include "Measurer.h"
#include "WordSeg.h"
#include "EditDistance.h"

namespace Torch {

/** This class can be used to save the word segmentation of a 
    #SimpleDecoderSpeechHMM#
    in a file. Optionally, it also prints the corresponding target segmentation.

    @author Samy Bengio (bengio@idiap.ch)
*/
class WordSegMeasurer : public Measurer
{
  public:
    /// the pointer to the corresponding wordseg object
    WordSeg *wordseg;

    /// a boolean stating whether we print the obtained word sequence
    bool print_obtained;
    /// a boolean stating whether we also print the corresponding targets
    bool print_targets;
    /// a boolean stating whether we also print the alignment timings
    bool print_timing;
    /// a boolean stating whether we also print the desired alignment timings
    bool print_desired_timing;
    /// when timing is required, what is the number of tics per frames
    int n_per_frame;
    /// eventually compute the edit_distance
    EditDistance* edit_distance;
    /// ... and keep its sum
    EditDistance* sum_edit_distance;

		char** file_list;
		int n_file_list;

    ///
    WordSegMeasurer(WordSeg* wordseg_, DataSet *data_, XFile *file_,EditDistance* edit_distance_=NULL,char** file_list_=NULL,int n_file_list_ = 0);
    virtual void reset();
    virtual void measureExample();
    virtual void measureIteration();

    virtual ~WordSegMeasurer();
};


}

#endif
