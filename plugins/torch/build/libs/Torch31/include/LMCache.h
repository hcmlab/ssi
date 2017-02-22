// Copyright (C) 2003--2004 Darren Moore (moore@idiap.ch)
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

#ifndef LMCACHE_INC
#define LMCACHE_INC

#include "general.h"
#include "LMCacheEntry.h"

namespace Torch {


/**
    This class implements a rudimentary caching scheme for language
    model lookup. It basically consists of a (small) list of the most 
    recently accessed language model entries. A cache lookup entails 
    a linear search of the list of entries. The oldest entry is 
    overwritten when the cache is full and a new entry is added.
      
    @author Darren Moore (moore@idiap.ch)
*/

class LMCache
{
public:
    int max_entries ;
    int n_entries ;
    LMCacheEntry **entries ;
    int oldest ;

    /// Creates an empty cache.
    /// 'max_entries_' is the maximum number of entries in the cache.
    /// 'lm_order' is the order of the language model n-gram (ie 3
    ///   for a trigram LM)
    /// 'n_vocab_words' is the number of words in the vocabulary.
    LMCache( int max_entries_ , int lm_order , int n_vocab_words ) ;
    virtual ~LMCache() ;

    /// Adds an entry to the cache. If the cache is full and the new
    ///   entry is not already in the cache, the oldest entry is
    ///   overwritten.
    /// 'order' is the order of the entry, which can be <= the lm_order
    ///   used during cache creation.
    /// 'words' are the words in the n-gram. The order is W3 W2 W1 W4
    ///   for a 4-gram entry.
    /// 'prob' is the log probability of the n-gram as calculated by the
    ///   language model.
    void addEntry( int order , int *words , real prob ) ;

    /// Looks for the n-gram in 'words' within the cache and returns
    ///   its probability if found, otherwise returns -LOG_ZERO.
    real getProb( int order , int *words ) ;
};
 

}

#endif
