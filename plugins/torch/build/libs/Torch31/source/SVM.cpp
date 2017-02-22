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

#include "SVM.h"
#include "XFile.h"

namespace Torch {

SVM::SVM(Kernel *kernel_, IOSequenceArray *io_sequence_array_)
{
  data = NULL;
  kernel = kernel_;

  support_vectors = NULL;
  n_support_vectors = 0;
  n_support_vectors_bound = 0;
  sv_alpha = NULL;
  sv_sequences = NULL;
  sv_allocator = new Allocator;

  outputs = new(allocator) Sequence(1, 1);

  if(io_sequence_array_)
    io_sequence_array = io_sequence_array_;
  else
    io_sequence_array = new(allocator) IOSequenceArray;
}

void SVM::forward(Sequence *inputs)
{
  real sum = 0;
  for(int t = 0; t < n_support_vectors; t++)
    sum += sv_alpha[t]*kernel->eval(sv_sequences[t], inputs);

  sum += b;

  outputs->frames[0][0] = sum;
}

bool SVM::bCompute()
{
  real sum = 0;
  int n_ = 0;
  for(int it = 0; it < n_support_vectors; it++)
  {
    int t = support_vectors[it];
    if( (alpha[t] > Cdown[t]+bound_eps) && (alpha[t] < Cup[t]-bound_eps) )
    {
      sum += y[t]*grad[t];
      n_++;
    }
  }
  
  if(n_)
  {
    b = -sum/(real)n_;
    return(true);
  }
  else
    return(false);
}

void SVM::loadXFile(XFile *file)
{
  file->taggedRead(&b, sizeof(real), 1, "b");
  file->taggedRead(&n_support_vectors, sizeof(int), 1, "NSV");
  file->taggedRead(&n_support_vectors_bound, sizeof(int), 1, "NSVB");

  sv_allocator->freeAll();
  if(n_support_vectors > 0)
  {
    sv_alpha = (real *)sv_allocator->alloc(sizeof(real)*n_support_vectors);
    file->taggedRead(sv_alpha, sizeof(real), n_support_vectors, "SVALPHA");
    
    sv_sequences = (Sequence **)sv_allocator->alloc(sizeof(Sequence *)*n_support_vectors);
    io_sequence_array->read(file, sv_sequences, n_support_vectors, sv_allocator);
  }
  else
  {
    sv_alpha = NULL;
    sv_sequences = NULL;
  }
}

void SVM::saveXFile(XFile *file)
{
  file->taggedWrite(&b, sizeof(real), 1, "b");
  file->taggedWrite(&n_support_vectors, sizeof(int), 1, "NSV");
  file->taggedWrite(&n_support_vectors_bound, sizeof(int), 1, "NSVB");
  if(n_support_vectors > 0)
  {
    file->taggedWrite(sv_alpha, sizeof(real), n_support_vectors, "SVALPHA");
    io_sequence_array->write(file, sv_sequences, n_support_vectors);
  }
}

SVM::~SVM()
{
  delete sv_allocator;
}

}
