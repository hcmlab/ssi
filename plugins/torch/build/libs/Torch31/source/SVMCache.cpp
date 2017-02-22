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

#include "SVMCache.h"

namespace Torch {

SVMCache::SVMCache(int n_alpha_, Kernel *kernel_, real cache_size_in_megs_)
{
  n_alpha = n_alpha_;
  n_cache_entries = 0;

  kernel = kernel_;
  cache_size_in_megs = cache_size_in_megs_;

  memory_cache = NULL;
  cached = NULL;
  list_index = NULL;

  n_active_var = -1;
  active_var = NULL;
}

void SVMCache::allocate()
{
  n_active_var = -1;
  active_var = NULL;

  // Allocs...
  temp_allocator = new Allocator;
  n_cache_entries = (int)(cache_size_in_megs*1048576./((real)sizeof(real)*n_alpha));
  list_index = (SVMCacheList **)temp_allocator->alloc(sizeof(SVMCacheList *)*n_alpha);
  cached = (SVMCacheList *)temp_allocator->alloc(sizeof(SVMCacheList)*n_cache_entries);

  message("SVMCache: max columns in cache: %d", n_cache_entries);
  if(n_cache_entries < 2)
    error("SVMCache: please change the cache size : it's too small");

  // Init
  SVMCacheList *ptr = cached;
  for(int i = 0; i < n_alpha; i++)
    list_index[i] = NULL;

  memory_cache = (real *)temp_allocator->alloc(sizeof(real)*n_cache_entries*n_alpha);
  for(int i = 0; i < n_cache_entries; i++)
  {
    ptr->adr = memory_cache+i*n_alpha;
    ptr->index = -1;
    if(i != 0)
      ptr->prev = (ptr-1);
    else
      ptr->prev = &cached[n_cache_entries-1];
    if(i != n_cache_entries-1)
      ptr->next = (ptr+1);
    else
      ptr->next = cached;

    ptr++;
  }
}

void SVMCache::destroy()
{
  delete temp_allocator;

  list_index = NULL;
  memory_cache = NULL;
}

void SVMCache::clear()
{
  SVMCacheList *ptr = cached;
  for(int i = 0; i < n_cache_entries; i++)
  {
    ptr->index = -1;
    ptr = ptr->next;
  }

  for(int i = 0; i < n_cache_entries; i++)
    list_index[i] = NULL;
}

real *SVMCache::adressCache(int index)
{
  SVMCacheList *ptr;

  // Rq: en regression faudrait faire gaffe a pas recalculer deux trucs...
  // Mais pb: -1 +1 a inverser dans la matrice...
  // Donc faich.

  ptr = list_index[index];
  if( (ptr != NULL) && (ptr != cached) )
  {
    ptr->prev->next = ptr->next;
    ptr->next->prev = ptr->prev;

    ptr->next = cached;
    ptr->prev = cached->prev;
    cached->prev->next = ptr;
    cached->prev = ptr;
    cached = ptr;
  }
  else
  {
    cached = cached->prev;
    if(cached->index != -1)
      list_index[cached->index] = NULL;
    cached->index = index;
    list_index[index] = cached;
    getColumn(index, cached->adr);
  }

  return(cached->adr);
}

void SVMCache::setActiveVariables(int *active_var_, int n_active_var_)
{
  n_active_var = n_active_var_;
  active_var = active_var_;
}

SVMCache::~SVMCache()
{
}

SVMCacheClassification::SVMCacheClassification(DataSet *data_, Kernel *kernel_, real cache_size_in_megs_)
  : SVMCache(data_->n_examples, kernel_, cache_size_in_megs_)
{
  data = data_;
  y = (real *)allocator->alloc(sizeof(real)*data->n_examples);
  for(int i = 0; i < data->n_examples; i++)
  {
    data->setExample(i);
    y[i] = data->targets->frames[0][0];
  }
}

void SVMCacheClassification::getColumn(int index, real *adr)
{
  data->setExample(index);
  Sequence *inputs = data->inputs;
  data->pushExample();

  if(active_var)
  {
    if(y[index] > 0)
    {
      for(int it = 0; it < n_active_var; it++)
      {
        int t = active_var[it];
        data->setExample(t);
        adr[t] =  y[t]*kernel->eval(inputs, data->inputs);
      }
    }
    else
    {
      for(int it = 0; it < n_active_var; it++)
      {
        int t = active_var[it];
        data->setExample(t);
        adr[t] = -y[t]*kernel->eval(inputs, data->inputs);
      }
    }
  }
  else
  {
    if(y[index] > 0)
    {
      for(int i = 0; i < n_alpha; i++)
      {
        data->setExample(i);
        adr[i] =  y[i]*kernel->eval(inputs, data->inputs);
      }
    }
    else
    {
      for(int i = 0; i < n_alpha; i++)
      {
        data->setExample(i);
        adr[i] = -y[i]*kernel->eval(inputs, data->inputs);
      }
    }
  }

  data->popExample();
}

SVMCacheRegression::SVMCacheRegression(DataSet *data_, Kernel *kernel_, real cache_size_in_megs_)
  : SVMCache(2*data_->n_examples, kernel_, cache_size_in_megs_)
{
  data = data_;
  n_examples = data->n_examples;
}

void SVMCacheRegression::getColumn(int index, real *adr)
{
  data->setExample(index%n_examples);
  Sequence *inputs = data->inputs;
  data->pushExample();
  
  if(active_var)
  {
    for(int i = 0; i < n_active_var; i++)
    {
      int k = active_var[i]%n_examples;
      data->setExample(k);
      adr[k] = kernel->eval(inputs, data->inputs);
    }
  }
  else
  {
    for(int i = 0; i < n_examples; i++)
    {
      data->setExample(i);
      adr[i] = kernel->eval(inputs, data->inputs);
    }
  }

  for(int i = 0; i < n_examples; i++)
    adr[i+n_examples] = adr[i];

  data->popExample();
}

}
