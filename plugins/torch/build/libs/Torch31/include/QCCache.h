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

#ifndef QC_CACHE_INC
#define QC_CACHE_INC

#include "Object.h"

namespace Torch {

/** "Cache" used by the Quadratic Constrained Trainer (#QCTrainer#).
    This class provides the Q (symetric) matrix of the
    quadratic problem.

    @author Ronan Collobert (collober@idiap.ch)
    @see QCTrainer
*/
class QCCache : public Object
{
  public:

    //-----

    ///
    QCCache();

    /// Returns the adress of the row/column #index# for the Q matrix.
    virtual real *adressCache(int index) = 0;

    /// Allocate the cache.
    virtual void allocate() = 0;

    /// Erase the cache (but don't destroy it).
    virtual void clear() = 0;

    /// Destroy the cache.
    virtual void destroy() = 0;

    /** Set the active variables to be those contained in #active_var#.
        The number of active variable is given by #n_active_var_#.

        (It's usefull for some problems)
        
        @see QCMachine
        @see QCTrainer
    */
    virtual void setActiveVariables(int *active_var_, int n_active_var_) = 0;

    //-----

    virtual ~QCCache();
};

}

#endif
