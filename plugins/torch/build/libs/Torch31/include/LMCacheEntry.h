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

#ifndef LMCACHEENTRY_INC
#define LMCACHEENTRY_INC

#include "general.h"

namespace Torch {


/**
    This class implements the internal entries within the LMCache
    class. Each entry consists of an array of previous words (eg.
    2 previous words in a trigram entry), and a list of probabilities
    of next words given the previous words. There is also an age
    field that is used to keep track of how recently the entry was
    accessed.
    
    @author Darren Moore (moore@idiap.ch)
*/

class LMCacheEntry
{
public:
    int age ;
    int max_n_prev_words ;
    int n_prev_words ;
    int *prev_words ;
    int n_probs ;
    real *probs ;

    /// Creates the cache entry.
    /// 'max_n_prev_words_' is the maximum number of prev words that will
    ///   ever be used with the cache entry (eg. 2 for trigram LM)
    /// 'n_vocab_words' is the number of words in the vocabulary.
    LMCacheEntry( int max_n_prev_words_ , int n_vocab_words ) ;
    virtual ~LMCacheEntry() ;

    /// Replaces the current prev words with new ones. The new entry can
    ///   a number of prev words that is <= max_n_prev_words (ie. can 
    ///   cache unigram entries when using a trigram LM).
    void addNewPrevWords( int n_prev_words_ , int *prev_words_ ) ;

    /// Resets all of the next-word log probs to -LOG_ZERO (to indicate that
    ///   we have no cached probs for all next words).
    void resetProbs() ;

    /// Adds a log probability for the next-word denoted by 'vocab_word'.
    void addProb( int vocab_word , real prob ) ;

    /// Returns the cached log probability for the next-word denoted by
    ///   'vocab_word'.
    real getProb( int word ) ;
};


}

#endif
