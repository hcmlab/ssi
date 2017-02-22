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

#ifndef IO_SEQUENCE_INC
#define IO_SEQUENCE_INC

#include "Sequence.h"

namespace Torch {

/** Class which provides an ensemble of sequences, which have the same
    frame size, but could have different number of frames.
    
    @author Ronan Collobert (collober@idiap.ch)
*/
class IOSequence : public Object
{
  public:
    /// Number of sequences in the interface.
    int n_sequences;

    /// Frame size of each sequence.
    int frame_size;

    ///
    IOSequence(); 

    /// Returns the number of frames of the sequence indexed by #t#.
    virtual int getNumberOfFrames(int t) = 0;

    /** Write the sequence #t# in #sequence#.
        Sequence must have the size returned by #getNumberOfFrames()#.
    */
    virtual void getSequence(int t, Sequence *sequence) = 0;

    /// Returns the total number of frames in the IO.
    virtual int getTotalNumberOfFrames() = 0;

    virtual ~IOSequence();
};

}

#endif
