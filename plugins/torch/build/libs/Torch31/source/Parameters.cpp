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

#include "Parameters.h"
#include "XFile.h"

namespace Torch {

Parameters::Parameters()
{
  // General
  data = NULL;
  n_data = 0;
  size = NULL;
  n_params = 0;
}

Parameters::Parameters(int n_params_)
{
  if(n_params_ <= 0)
  {
    data = NULL;
    n_data = 0;
    size = NULL;
    n_params = 0;
    return;
  }

  // General
  data = (real **)allocator->alloc(sizeof(real *));
  data[0] = (real *)allocator->alloc(sizeof(real)*n_params_);
  n_data = 1;
  size = (int *)allocator->alloc(sizeof(int));
  size[0] = n_params_;
  n_params = n_params_;
}

void Parameters::addParameters(real *params_, int n_params_, bool do_copy)
{
  if(!params_)
    return;

  data = (real **)allocator->realloc(data, sizeof(real *)*(n_data+1));
  size = (int *)allocator->realloc(size, sizeof(int)*(n_data+1));
  size[n_data] = n_params_;
  if(do_copy)
  {
    data[n_data] = (real *)allocator->alloc(sizeof(real)*n_params_);
    real *dest_ = data[n_data];
    for(int i = 0; i < n_params_; i++)
      dest_[i] = params_[i];
  }
  else
    data[n_data] = params_;

  n_data++;
  n_params += n_params_;
}

void Parameters::add(Parameters *params_, bool do_copy)
{
  if(!params_)
    return;

  for(int i = 0; i < params_->n_data; i++)
    this->addParameters(params_->data[i], params_->size[i], do_copy);
}

void Parameters::copy(Parameters *from)
{
  if(n_params == 0)
    return;

  int src_number = 0;
  int src_index = 0;

  int src_size = from->size[0];
  real *src_ = from->data[0];
  for(int i = 0; i < n_data; i++)
  {
    real *dest_ = data[i];
    for(int j = 0; j < size[i]; j++)
    {
      if(src_index == src_size)
      {
        src_index = 0;
        src_size = from->size[++src_number];
        src_ = from->data[src_number];
      }
      dest_[j] = src_[src_index++];
    }
  }
}

void Parameters::copyFrom(real *vec)
{
  for(int i = 0; i < n_data; i++)
  {
    real *dest_ = data[i];
    for(int j = 0; j < size[i]; j++)
      dest_[j] = vec[j];
    vec += size[i];
  }
}

void Parameters::copyTo(real *vec)
{
  for(int i = 0; i < n_data; i++)
  {
    real *src_ = data[i];
    for(int j = 0; j < size[i]; j++)
      vec[j] = src_[j];
    vec += size[i];
  }
}

void Parameters::loadXFile(XFile *file)
{
  for(int i = 0; i < n_data; i++)
    file->taggedRead(data[i], sizeof(real), size[i], "PARAMS");
}

void Parameters::saveXFile(XFile *file)
{
  for(int i = 0; i < n_data; i++)
    file->taggedWrite(data[i], sizeof(real), size[i], "PARAMS");
}

Parameters::~Parameters()
{
}

}
