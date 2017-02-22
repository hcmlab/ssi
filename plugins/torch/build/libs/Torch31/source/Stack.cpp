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

#include "Stack.h"

namespace Torch {

Stack::Stack()
{
  stack_size = 0;
  stack = NULL;
  n_stacked_objects = 0;
}

void Stack::push(void *ptr, int size)
{
  if(n_stacked_objects >= stack_size)
  {
    stack_size++;
    stack = (StackNode *)allocator->realloc(stack, sizeof(StackNode)*stack_size);
  }

  stack[n_stacked_objects].address = ptr;
  stack[n_stacked_objects].size = size;
  if(ptr)
  {
    stack[n_stacked_objects].ptr = allocator->alloc(size);
    memcpy(stack[n_stacked_objects].ptr, ptr, size);
  }
  else
    stack[n_stacked_objects].ptr = NULL;

  n_stacked_objects++;
}

void Stack::pop()
{
  n_stacked_objects--;
  if(stack[n_stacked_objects].ptr)
    memcpy(stack[n_stacked_objects].address, stack[n_stacked_objects].ptr, stack[n_stacked_objects].size);
  allocator->free(stack[n_stacked_objects].ptr);
}

Stack::~Stack()
{
}

}
