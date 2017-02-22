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

#ifndef DISK_HTK_DATA_SET_INC
#define DISK_HTK_DATA_SET_INC

#include "DiskDataSet.h"
#include "MeanVarNorm.h"
#include "IOHTK.h"
#include "LexiconInfo.h"

namespace Torch {

/** Provides an interface to manipulate HTK data which are
    kept on disk, and not fully loaded in memory.
    It uses #IOSequence#.
    Usefull for large databases.

    @see DiskMatDataSet
    @see IOSequence
		@see IOHTK
		@see IOHTKTarget
		@author Johnny Mariethoz (Johnny.Mariethoz@idiap.ch)
 */
class DiskHTKDataSet : public DiskDataSet
{
  private:
    void init_(IOSequence *io_file, int n_inputs_, int n_targets_);

  public:
    int n_per_frame;

    DiskHTKDataSet(const char *inputs_filenames, bool one_file_is_one_sequence, int max_load, const char * targets_filename = NULL, LexiconInfo* lex_=NULL, bool words=true);
    DiskHTKDataSet(char **inputs_filenames, char ** targets_filename, int n_files_, bool one_file_is_one_sequence, int max_load, LexiconInfo* lex_=NULL, bool words=true);

    virtual ~DiskHTKDataSet();
};

}

#endif
