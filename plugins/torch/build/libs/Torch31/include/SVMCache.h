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

#ifndef SVM_CACHE_INC
#define SVM_CACHE_INC

#include "QCCache.h"
#include "Kernel.h"
#include "SVMClassification.h"
#include "SVMRegression.h"

namespace Torch {

struct SVMCacheList
{
    real *adr;
    int index;
    SVMCacheList *prev;
    SVMCacheList *next;
};

/** #QCCache# implementation for SVMs.
    Compute the Q matrix.

    @see SVM
    @see QCMachine
    @see QCTrainer

    @author Ronan Collobert (collober@idiap.ch)
*/
class SVMCache : public QCCache
{
  public:
    int n_alpha;
    int n_cache_entries;
    real cache_size_in_megs;
    real *memory_cache;
    
    SVMCacheList *cached;
    SVMCacheList **list_index;

    Kernel *kernel;

    int n_active_var;
    int *active_var;

    Allocator *temp_allocator;

    //-----

    ///
    SVMCache(int n_alpha_, Kernel *kernel_, real cache_size_in_megs_);

    //-----

    void allocate();
    virtual void clear();
    virtual void destroy();
    virtual void setActiveVariables(int *active_var_, int n_active_var_);

    //-----

    virtual real *adressCache(int index);  
    virtual void getColumn(int index, real *adr) = 0;    
    virtual ~SVMCache();
};

/** Cache for SVM classification.

    @author Ronan Collobert (collober@idiap.ch)
 */
class SVMCacheClassification : public SVMCache
{
  public:
    DataSet *data;
    real *y;

    //-----

    ///
    SVMCacheClassification(DataSet *data_, Kernel *kernel_, real cache_size_in_megs_);

    //-----

    virtual void getColumn(int index, real *adr);
};

/** Cache for SVM regression.

    @author Ronan Collobert (collober@idiap.ch)
 */
class SVMCacheRegression : public SVMCache
{
  public:
    DataSet *data;
    int n_examples;

    //-----

    ///
    SVMCacheRegression(DataSet *data_, Kernel *kernel_, real cache_size_in_megs_);

    //-----

    virtual void getColumn(int index, real *adr);
};

}

#endif
