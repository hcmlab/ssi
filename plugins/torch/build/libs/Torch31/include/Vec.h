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

#ifndef VEC_INC
#define VEC_INC

#include "Object.h"
#include "Vec.h"

namespace Torch {

/** Vector object.
    
    @author Ronan Collobert (collober@idiap.ch)
*/
class Vec : public Object
{
  public:
    /// Size of the vector
    int n;

    /// Data of the vector
    real *ptr;

    /** Create a vector from values in #ptr_#.
        (No memory copy).
    */
    Vec(real * ptr_, int n_dim);

    /// Create a new vector
    Vec(int n_dim);

    /// Copy the vector #vec# starting from index #start_i#
    void copy(Vec * vec, int start_i = 0);

    /// Zero the matrix
    void zero();

    /// Compute the norm1
    real norm1(Vec * weights = NULL);

    /// Compute the norm2
    real norm2(Vec * weights = NULL);

    /// Compute the norm inf
    real normInf();

    /// Inner product of two vectors from #start_i# downwards
    real iP(Vec * vec, int start_i = 0);

    /** Return a sub-vector.
        Note that the memory is shared with the original
        vector, so *be carefull*.
    */
    Vec *subVec(int dim1, int dim2);

    ~Vec();
};


}

#endif
