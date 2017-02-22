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

#ifndef MAT_DATA_SET_INC
#define MAT_DATA_SET_INC

#include "MemoryDataSet.h"

namespace Torch {

/** Matrix DataSet...
    The standard dataset, with data fully loaded in memory.
    Usefull for large databases.

    @see IOAscii
    @see IOBin
    @author Ronan Collobert (collober@idiap.ch)
 */
class MatDataSet : public MemoryDataSet
{
  private:
    void init_(IOSequence *io_file, int n_inputs_, int n_targets_);
    Allocator *io_allocator;

  public:
    /** Create a new dataset from the file #filename#. If the file contains only one sequence, set #one_file_is_one_sequence#
        to true. If there is several sequences, and you want only to load the first #n# ones, set #max_load# to #n# (else #max_load#
        should be a negative number). If #binary_mode# is true, the IOBin format will be used, else it will be the IOAscii format.
        
        Input and target sequence will have the same number of frames. For \emph{each} frame given by the dataset, the first #n_inputs_#
        real are for the inputs and then the next #n_targets_# real are for the targets. (#n_inputs_# is the input frame size and
        #n_targets_# is the target frame size).
    */       
    MatDataSet(const char *filename, int n_inputs_, int n_targets_,
               bool one_file_is_one_sequence=false, int max_load=-1, bool binary_mode=false);

    /** Same as the previous constructor, but for several files. If #one_file_is_one_sequence# is true, each files will be considered as they
        had only one sequence.
    */
    MatDataSet(char **filenames, int n_files_, int n_inputs_, int n_targets_,
               bool one_file_is_one_sequence=false, int max_load=-1, bool binary_mode=false);

    /** Here the inputs and the targets are in separated files.
        Input and target frame sizes are therefore auto-detected.
        One file must correspond to one sequence.
    */
    MatDataSet(char **input_filenames, char **target_filenames, int n_files_,
               int max_load=-1, bool binary_mode=false);

    virtual ~MatDataSet();
};

}

#endif
