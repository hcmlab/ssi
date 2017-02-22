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

#ifndef FRAME_SEG_MEASURER_INC
#define FRAME_SEG_MEASURER_INC

#include "Measurer.h"
#include "FrameSeg.h"
#include "EditDistance.h"

namespace Torch {

/** This class can be used to save the frame segmentation of a 
    #SimpleDecoderSpeechHMM#
    in a file. Optionally, it also prints the corresponding target segmentation.

    @author Samy Bengio (bengio@idiap.ch)
*/
class FrameSegMeasurer : public Measurer
{
  public:
    /// the pointer to the corresponding frameseg object
    FrameSeg *frameseg;

    bool print_desired_timing;
    bool print_with_words;
    bool print_timing;
    bool print_phoneme_timing;
    bool print_frame_err;
    bool print_soft_frame_err;
    int n_soft_frames;
    /// when timing is required, what is the number of tics per frames
    int n_per_frame;

    /// some stats
    int n_correct;
    int n_incorrect;
    int n_frames;
    int n_correct_soft;
    int n_incorrect_soft;
    int n_frames_soft;

		char** file_list;
		int n_file_list;

    ///
    FrameSegMeasurer(FrameSeg* frameseg_, DataSet *data_, XFile *file_,char** file_list_=NULL,int n_file_list_ = 0);
    virtual void reset();
    virtual void measureExample();
    virtual void measureIteration();

    virtual ~FrameSegMeasurer();
};


}

#endif
