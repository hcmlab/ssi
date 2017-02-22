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

#include "Allocator.h"
#include "LMCacheEntry.h"
#include "log_add.h"

namespace Torch {


LMCacheEntry::LMCacheEntry( int max_n_prev_words_ , int n_vocab_words )
{
    max_n_prev_words = max_n_prev_words_ ;
    prev_words = (int *)Allocator::sysAlloc( max_n_prev_words * sizeof(int) ) ;
    n_probs = n_vocab_words ;
    probs = (real *)Allocator::sysAlloc( n_probs * sizeof(real) ) ;

    for ( int i=0 ; i<n_probs ; i++ )
        probs[i] = -LOG_ZERO ;

    age = 0 ;
    n_prev_words = 0 ;
}


LMCacheEntry::~LMCacheEntry()
{
    if ( prev_words != NULL )
        free( prev_words ) ;
    if ( probs != NULL )
        free( probs ) ;
}


void LMCacheEntry::resetProbs()
{
    // We use -LOG_ZERO to indicate that no entry has been added to the cache.
    // LOG_ZERO would be for a word that doesn't exist in the n-gram
    for ( int i=0 ; i<n_probs ; i++ )
        probs[i] = -LOG_ZERO ;
}


void LMCacheEntry::addNewPrevWords( int n_prev_words_ , int *prev_words_ )
{
#ifdef DEBUG
    if ( (n_prev_words_ > max_n_prev_words) || (n_prev_words_ <= 0) )
        error("LMCacheEntry::addNewPrevWords - n_prev_words_ out of range\n") ;
#endif
    memcpy( prev_words , prev_words_ , n_prev_words_*sizeof(int) ) ;
    n_prev_words = n_prev_words_ ;
    resetProbs() ;
    age = 0 ;
}


void LMCacheEntry::addProb( int vocab_word , real prob )
{
#ifdef DEBUG
    if ( (vocab_word<0) || (vocab_word>=n_probs) )
        error("LMCacheEntry::addProb - vocab word index out of range\n") ;
    if ( probs[vocab_word] < -LOG_ZERO )
        error("LMCacheEntry::addProb - entry already set\n") ;
#endif

    probs[vocab_word] = prob ;
    age = 0 ;
}


real LMCacheEntry::getProb( int vocab_word )
{
#ifdef DEBUG
    if ( (vocab_word<0) || (vocab_word>=n_probs) )
        error("LMCacheEntry::getProb - vocab word index out of range\n") ;
#endif

    age = 0 ;
    return probs[vocab_word] ;
}


}
