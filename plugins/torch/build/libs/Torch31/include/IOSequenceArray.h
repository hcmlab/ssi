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

#ifndef IO_SEQUENCE_ARRAY_INC
#define IO_SEQUENCE_ARRAY_INC

#include "Object.h"
#include "Sequence.h"

namespace Torch {

/** Load and save in an efficiently manner an array of sequences.
    (in an arbitrary binary format).

    This could be useful for some (rare) classes which needs to save
    sequences which could be in another format that the standard
    sequence format (if you plan to use them with strange subclasses of
    #Sequence#).
   
    @author Ronan Collobert (collober@idiap.ch)
*/
class IOSequenceArray : public Object
{
  public:

    ///
    IOSequenceArray(); 

    /** Read an array of sequences in #file#. #n_sequences# will be read.
        If #allocator_# is non-NULL, it will be used to allocate the memory
        of the sequences. Else, the memory of the sequences will be destroyed
        when destroying the class. #sequences_array# must have the size #n_sequences#.
    */
    virtual void read(XFile *file, Sequence **sequences_array, int n_sequences, Allocator *allocator_=NULL);
    
    /** Write an array of sequences in #file#. #n_sequences# will be written.
        #sequences_array# must have the size #n_sequences#.
    */
    virtual void write(XFile *file, Sequence **sequences_array, int n_sequences);
    
    virtual ~IOSequenceArray();
};

}

#endif
