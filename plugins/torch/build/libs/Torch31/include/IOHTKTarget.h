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

#ifndef IO_HTK_TARGET_INC
#define IO_HTK_TARGET_INC

#include "IOSequence.h"
#include "LexiconInfo.h"
#include "DiskXFile.h"

namespace Torch {

/** Handles the standard Ascii HTK targets/labels format in Torch.
    There are two format:
    \begin{itemize}
      \item each line is a word/phoneme
      separated by a space).
      \item each line consiste in two integer (begin and end of the sequence) and a string containing the label (word/phoneme)
    \end{itemize}
		@see IOAscii
    @author Johnny Mariethoz (Johnny.Mariethoz@idiap.ch)
*/
class IOHTKTarget : public IOSequence
{
  private:
    DiskXFile *file;
    int current_frame_index;

  public:
    bool one_file_is_one_sequence;
    int n_total_frames;
    char *filename;
    int max_load;
		int n_per_frame;
		LexiconInfo* lexicon;
		bool words;


    IOHTKTarget(const char *filename_, LexiconInfo* lex_, int n_per_frame_, bool words=true);

    /// Saves one #sequence# in #file# using the ascii format.
    /// static added by John Dines
		static void saveSequence(XFile *file, Sequence* sequence, LexiconInfo* lex_, int n_per_frame_, bool words_=true);

    virtual void getSequence(int t, Sequence *sequence);
    virtual int getNumberOfFrames(int t);
    virtual int getTotalNumberOfFrames();

    virtual ~IOHTKTarget();
};

}

#endif
