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
#include "LMCache.h"
#include "log_add.h"

namespace Torch {


LMCache::LMCache( int max_entries_ , int lm_order , int n_vocab_words )
{
    if ( max_entries_ <= 0 )
        error("LMCache::LMCache - max_entries_ cannot be <= 0\n") ;
        
    max_entries = max_entries_ ;
    entries = (LMCacheEntry **)Allocator::sysAlloc( max_entries * sizeof(LMCacheEntry *) ) ;
    for ( int i=0 ; i<max_entries ; i++ )
        entries[i] = new LMCacheEntry( lm_order-1 , n_vocab_words ) ;
    oldest = -1 ;
    n_entries = 0 ;
}


LMCache::~LMCache()
{
    if ( entries != NULL )
    {
        for ( int i=0 ; i<max_entries ; i++ )
            delete entries[i] ;
        free( entries ) ;
    }
}


void LMCache::addEntry( int order , int *words , real prob )
{
    // The ordering of the words must be W3 W2 W1 W4 (for a 4-gram LM)
    //   where W4 is "next" word and remainder are prev words.
    
    int i , j , max_age=0 , n_prev_words=(order-1) ;
    
    // Do we already have an entry for this n-gram ?  Just do a linear search.
    for ( i=0 ; i<n_entries ; i++ )
    {
        if ( n_prev_words == entries[i]->n_prev_words )
        {
            if ( memcmp( words , entries[i]->prev_words , n_prev_words*sizeof(int) ) == 0 )
            {
                // Yes we do - add the probability to the existing entry
                entries[i]->addProb( words[n_prev_words] , prob ) ;

                // Increase the age of all cache entries and keep track of the oldest
                for ( j=0 ; j<n_entries ; j++ )
                {
                    if ( ++(entries[j]->age) > max_age )
                    {
                        max_age = entries[j]->age ;
                        oldest = j ;
                    }
                }

                return ;
            }
        }
    }

    // No we don't, we need to add this n-gram to the cache
    if ( n_entries < max_entries )
    {
        // Our cache isn't full - we can simply add the new entry at the end
        entries[n_entries]->addNewPrevWords( n_prev_words , words ) ;
        entries[n_entries]->addProb( words[n_prev_words] , prob ) ;
        n_entries++ ;
    }
    else
    {
        // The cache is full - we need to replace the oldest entry with the new one
        entries[oldest]->addNewPrevWords( n_prev_words , words ) ;
        entries[oldest]->addProb( words[n_prev_words] , prob ) ;
    }
    
    // Increase the age of all cache entries and keep track of oldest
    for ( i=0 ; i<n_entries ; i++ )
    {
        if ( ++(entries[i]->age) > max_age )
        {
            max_age = entries[i]->age ;
            oldest = i ;
        }
    }
}
        

real LMCache::getProb( int n_words , int *words )
{
    // The ordering of the words must be W3 W2 W1 W4 (for a 4-gram LM)
    //   where W4 is "next" word and remainder are prev words.
    
    int max_age=0 , i , n_prev_words=(n_words-1) ;
    real prob = -LOG_ZERO ;
    
    for ( i=0 ; i<n_entries ; i++ )
    {
        if ( entries[i]->n_prev_words == n_prev_words )
        {
            if ( memcmp( words , entries[i]->prev_words , n_prev_words*sizeof(int) ) == 0 )
            {
                prob = entries[i]->getProb( words[n_prev_words] ) ;
                break ;
            }
        }
    }
    
    // Increase the age of all cache entries and keep track of oldest
    for ( i=0 ; i<n_entries ; i++ )
    {
        if ( ++(entries[i]->age) > max_age )
        {
            max_age = entries[i]->age ;
            oldest = i ;
        }
    }

    return prob ;
}        


}

