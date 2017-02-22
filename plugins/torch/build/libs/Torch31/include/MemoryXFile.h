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

#ifndef MEMORY_X_FILE_INC
#define MEMORY_X_FILE_INC

#include "XFile.h"
#include "List.h"

namespace Torch {

struct MemoryXFileNode
{
    void *mem;
    int size;
};

DEFINE_NEW_LIST(MemoryXFileList, MemoryXFileNode);

/** A file in the memory.
    Note that the MemoryXFile is a read-write file!
    When writing, data is buffered (to avoid reallocating blocks of small sizes!).

    Options:
    \begin{tabular}{lcll}
      "buffer size"  &  int  &  buffer size for writing & [65536]
    \end{tabular}

    @author Ronan Collobert (collober@idiap.ch)
*/
class MemoryXFile : public XFile
{
  private:
    static char petit_message_pour_melanie[10000];
    bool is_eof;

  public:

    /// The memory
    MemoryXFileList *memory;

    /// The position in the memory
    int position;

    /// The size of the memory
    int size;

    /// The total size of the memory, including allocated buffer.
    int total_size;

    /// Minimal number of bytes that will be allocated when writing...
    int buffer_size;

    // Internal
    int internal_memory_node_index;
    int internal_position_in_the_node;
    char *buffer_format;
    int buffer_format_size;

    /// Create a read-write file from nothing.
    MemoryXFile(int buffer_format_size_=256);

    /** Give a MemoryXFileList for the memory. A new list is created, but the memory
        in the nodes are not copied.
        If the #size# that you want to be readable is \emph{less} than the
        sum of the #n# in the field of the #MemoryXFileList#, you can provide it in #size_# */
    MemoryXFile(MemoryXFileList *memory_, int size_=-1, int buffer_format_size_=256);

    /// Give a pointer for the memory. (No copy is done!)
    MemoryXFile(void *memory_, int size_, int buffer_format_size_=256);

    /** Concat all the memory in one node. Add a null character at the end.
        This null character is useful for #sscanf# in the #scanf# method. */
    void concat();

    virtual int read(void *ptr, int block_size, int n_blocks);
    virtual int write(void *ptr, int block_size, int n_blocks);
    virtual int eof();
    virtual int flush();
    virtual int seek(long offset, int whence);
    virtual long tell();
    virtual void rewind();
    virtual int printf(const char *format, ...);

    /** Warning: this method call the concat function,
        and therefore could take time if you do intensive read/write. */
    virtual int scanf(const char *format, void *ptr);
    virtual char *gets(char *dest, int size_);

    //-----

    virtual ~MemoryXFile();
};


}

#endif
