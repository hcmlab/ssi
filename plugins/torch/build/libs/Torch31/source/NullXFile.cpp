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

#include "NullXFile.h"

namespace Torch {

NullXFile::NullXFile()
{
}

int NullXFile::read(void *ptr, int block_size, int n_blocks)
{
  return 0;
}

int NullXFile::write(void *ptr, int block_size, int n_blocks)
{
  return n_blocks;
}

int NullXFile::eof()
{
  return 0;
}

int NullXFile::flush()
{
  return 0;
}

int NullXFile::seek(long offset, int whence)
{
  return 0;
}

long NullXFile::tell()
{
  return 0L;
}

void NullXFile::rewind()
{
}

int NullXFile::printf(const char *format, ...)
{
  return 0;
}

int NullXFile::scanf(const char *format, void *ptr)
{
  return 0;
}

char *NullXFile::gets(char *dest, int size_)
{
  return NULL;
}

NullXFile::~NullXFile()
{
}

}
