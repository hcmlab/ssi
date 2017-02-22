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

#include "SVMRegression.h"
#include "SVMCache.h"

namespace Torch {

SVMRegression::SVMRegression(Kernel *kernel_, real *C_, IOSequenceArray *io_sequence_array_) : SVM(kernel_, io_sequence_array_)
{
  Cuser = C_;
  addROption("eps regression", &eps_regression, 0.7, "size of the error tube");
  addROption("C", &C_cst, 100, "trade off margin/classification error");
  addROption("cache size", &cache_size_in_megs, 50., "cache size (in Mo)");

  sequences_buffer = NULL;
  frames_buffer = NULL;
}

void SVMRegression::setDataSet(DataSet *dataset_)
{
  data = dataset_;
  QCMachine::reInit(2*data->n_examples);

  for(int i = 0; i < n_alpha; i++)
    alpha[i] = 0;

  b = 0;

  int n_examples_ = data->n_examples;
  if(Cuser)
  {
    for(int i = 0; i < n_examples_; i++)
    {
      Cup[i] = 0;
      Cdown[i] = -Cuser[data->selected_examples[i]];
    }

    for(int i = n_examples_; i < n_alpha; i++)
    {
      Cup[i] = Cuser[data->selected_examples[i-n_examples_]+n_examples_];
      Cdown[i] = 0;
    }
  }
  else
  {
    for(int i = 0; i < n_examples_; i++)
    {
      Cup[i] = 0;
      Cdown[i] = -C_cst;
    }

    for(int i = n_examples_; i < n_alpha; i++)
    {
      Cup[i] = C_cst;
      Cdown[i] = 0;
    }
  }

  data->pushExample();
  for(int i = 0; i < n_examples_; i++)
  {
    data->setExample(i);
    real z = data->targets->frames[0][0];
    
    grad[i] =  -z - eps_regression;
    y[i] =  1;
    
    grad[i+n_examples_] = -z + eps_regression;
    y[i+n_examples_] = 1;
  }
  data->popExample();

  if(cache)
    allocator->free(cache);
  cache = new(allocator) SVMCacheRegression(data, kernel, cache_size_in_megs);

  n_support_vectors = 0; 
  n_support_vectors_bound = 0; 
}

void SVMRegression::checkSupportVectors()
{
  // Get the number of support vectors, and the size need to duplicate them...
  n_support_vectors = 0;
  n_support_vectors_bound = 0;
  int frame_buffer_size = 0;
  for(int i = 0; i < data->n_examples; i++)
  {
    if(alpha[i] < -bound_eps)
    {
      if(alpha[i] < Cdown[i] + bound_eps)
        n_support_vectors_bound++;
      
      n_support_vectors++;

      data->setExample(i);
      frame_buffer_size += data->inputs->getFramesSpace();
    }
  }
  for(int i = data->n_examples; i < 2*data->n_examples; i++)
  {
    if(alpha[i] > bound_eps)
    {
      if(alpha[i] > Cup[i] - bound_eps)
        n_support_vectors_bound++;
      
      n_support_vectors++;

      data->setExample(i-data->n_examples);
      frame_buffer_size += data->inputs->getFramesSpace();
    }
  }

  // Allocate all the stuff
  sv_allocator->freeAll();
  support_vectors = (int *)sv_allocator->alloc(sizeof(int)*n_support_vectors);
  sv_alpha = (real *)sv_allocator->alloc(sizeof(real)*n_support_vectors);
  sv_sequences = (Sequence **)sv_allocator->alloc(sizeof(Sequence *)*n_support_vectors);

  data->setExample(0);
  int sequence_size = data->inputs->getSequenceSpace();
  sequences_buffer = (char *)sv_allocator->alloc(sequence_size*n_support_vectors);
  frames_buffer = (char *)sv_allocator->alloc(frame_buffer_size);

  // Duplicate support vectors, and save their index...
  n_support_vectors = 0;
  char *frames_buffer_ = frames_buffer;
  char *sequences_buffer_ = sequences_buffer;
  for(int i = 0; i < data->n_examples; i++)
  {
    if(alpha[i] < -bound_eps)
    {
      support_vectors[n_support_vectors  ] = i;
      sv_alpha[n_support_vectors] = y[i]*alpha[i];
      data->setExample(i);
      sv_sequences[n_support_vectors] = data->inputs->clone(sv_allocator, sequences_buffer_, frames_buffer_);
      frames_buffer_ += data->inputs->getFramesSpace();
      sequences_buffer_ += sequence_size;
      n_support_vectors++;
    }
  }
  for(int i = data->n_examples; i < 2*data->n_examples; i++)
  {
    if(alpha[i] > bound_eps)
    {
      support_vectors[n_support_vectors  ] = i;
      sv_alpha[n_support_vectors] = y[i]*alpha[i];
      data->setExample(i-data->n_examples);
      sv_sequences[n_support_vectors] = data->inputs->clone(sv_allocator, sequences_buffer_, frames_buffer_);
      frames_buffer_ += data->inputs->getFramesSpace();
      sequences_buffer_ += sequence_size;
      n_support_vectors++;
    }
  }

  // Compute the "b" variable...
  if(!bCompute())
  {
    warning("SVMRegression: b is not unique. It's probably wrong");
    warning("SVMRegression: I think you are using silly parameters");
  }
}

SVMRegression::~SVMRegression()
{
}

}
