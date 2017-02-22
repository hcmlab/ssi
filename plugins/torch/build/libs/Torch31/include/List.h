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

#ifndef LIST_INC
#define LIST_INC

#include "Object.h"

namespace Torch {

/** List Classes.

A list named NAME with node pointer type TYPE has the following aspect:

\begin{verbatim}
class NAME
{
  /// Pointer which can be used as you want.
  TYPE *nodes;
  
  /// Number of nodes.
  int n_nodes;

  /// Constructor.
  NAME();

  /// Add a list at the end of the current list.
  void add(NAME *list);

  /// Add a node at the end of the current list.
  void addNode(TYPE *node_)
};
\end{verbatim}

To declare a new list, just use the macro (in ".h" files):

\begin{verbatim}
#DEFINE_NEW_LIST(NAME, TYPE);
\end{verbatim}

and to implement this list, use the macro (in ".cc" files):
\begin{verbatim}
#define IMPLEMENT_NEW_LIST(NAME, TYPE)
\end{verbatim}

The name of a list which TYPE nodes should be something like "TYPEList".

    @author Ronan Collobert (collober@idiap.ch)
    @type class
    @name List
    @args
    @memo
*/

#define DEFINE_NEW_LIST(NAME, TYPE) \
class NAME : public Object \
{ \
  public: \
    TYPE **nodes; \
    int n_nodes; \
\
    NAME(); \
    void add(NAME *list); \
    void addNode(TYPE *node); \
}

#define IMPLEMENT_NEW_LIST(NAME, TYPE) \
\
NAME::NAME() \
{ \
  nodes = NULL; \
  n_nodes = 0; \
} \
\
void NAME::add(NAME *list) \
{ \
  if(!list->n_nodes) \
    return; \
\
  nodes = (TYPE **)allocator->realloc(nodes, sizeof(TYPE *)*(n_nodes+list->n_nodes)); \
  for(int i = 0; i < list->n_nodes; i++) \
     nodes[n_nodes++] = list->nodes[i]; \
} \
\
void NAME::addNode(TYPE *node) \
{ \
  nodes = (TYPE **)allocator->realloc(nodes, sizeof(TYPE *)*(n_nodes+1)); \
  nodes[n_nodes++] = node; \
}

}

#endif
