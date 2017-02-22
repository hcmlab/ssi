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
#include "LMNGram.h"

namespace Torch {


LMNGram::LMNGram( int n_ , Vocabulary *vocab_ )
{
#ifdef DEBUG
    if ( n_ < 1 )
        error("LMNGram::LMNGram - n cannot be < 1\n") ;
#endif
    n = n_ ;
    vocab = vocab_ ;

    //  Create a unigram entry for every word in our vocabulary.
    unigrams = (real *)Allocator::sysAlloc( vocab->n_words * sizeof(real) ) ;

    for ( int i=0 ; i<vocab->n_words ; i++ )
        unigrams[i] = LOG_ZERO ;

    if ( n > 1 )
        next_level = new LMInteriorLevelWordList( n-1 ) ;
    else
        next_level = NULL ;

    // Configure the cache - TODO make the cache size configurable (?)
    cache = new LMCache( 10 , n , vocab->n_words ) ;
}


LMNGram::~LMNGram()
{
    if ( unigrams != NULL )
        free(unigrams) ;

    if ( next_level != NULL )
        delete next_level ;

    if ( cache != NULL )
        delete cache ;
}


void LMNGram::addEntry( int order , int *words , real prob , real bo_wt )
{
    // We assume that the 'words' array is in oldest-word-first order.
    //   ie.  [w1 w2 w3] for trigram entry.
    int r_words_1[30] ;   // rearranged words [w4,w3,w2,w1]
    int r_words_2[30] ;   // rearranged words [w3,w2,w1,w4]
    
#ifdef DEBUG
    if ( (order < 1) || (order > n) )
        error("LMNGram::addEntry - order out of range\n") ;
#endif

    if ( order == 1 )
    {
        if ( unigrams[*words] > LOG_ZERO )
            error("LMNGram::addEntry - duplicate unigram entry encountered\n") ;
        unigrams[*words] = prob ;

        if ( n > 1 )
        {
            // We are adding a unigram entry into a language model
            //   of higher order, so the backoff weight is important.
            // Add the backoff weight.
            next_level->addBackoffEntry( order , words , bo_wt ) ;
        }
    }
    else
    {
        // The first thing we want to do is to rearrange the words in the entry
        //   so that the order is straight forward and matches the architecture
        //   of the language model, for adding both probs and backoffs.
        for ( int i=0 ; i<(order-1) ; i++ )
        {
            r_words_1[i] = words[order-1-i] ;   // backoff ordering
            r_words_2[i] = words[order-2-i] ;   // prob ordering
        }
        r_words_1[order-1] = words[0] ;
        r_words_2[order-1] = words[order-1] ;
        
        if ( prob > LOG_ZERO )
            next_level->addProbEntry( order , r_words_2 , prob ) ;
            
        if ( order < n )
        {
            // Add the backoff - we don't have/need, eg, trigram backoffs in our
            //   trigram language model.
            next_level->addBackoffEntry( order , r_words_1 , bo_wt ) ;
        }
    }
}


real LMNGram::getLogProbBackoff( int order , int *words )
{
    // There are 'order' entries in 'words'.
    // The ordering in words is W3,W2,W1,W4 for a 4-gram query.
    //   ie. for the query : what is P(W4|W1,W2,W3) ?
    real temp , prob ;
    
#ifdef DEBUG
    if ( order < 1 )
        error("LMNGram::getNextWordList - order out of range\n") ;

    bool output_debug=false ;
    if ( output_debug == true )
    {
        printf( "P( %s | " , vocab->words[words[order-1]] ) ;
        for ( int i=0 ; i<(order-1) ; i++ )
            printf( "%s " , vocab->words[words[i]] ) ;
        printf(") = ") ;
    }
#endif

    if ( order > n )
        order = n ;

    if ( order == 1 )
    {
        // Just return the unigrams prob - no need to backoff or to use the cache.
#ifdef DEBUG
        if ( output_debug == true )
            printf("%f\n",unigrams[*words]);
#endif
        return unigrams[*words] ;
    }
    else
    {
        // look in the cache ...
        prob = cache->getProb( order , words ) ;
        if ( prob >= (-LOG_ZERO) )
        {
            // The n-gram entry is not in the cache, so we need to search for it
            //   and then add it to the cache.
            if ( next_level->getProbWithBackoff( order , words , &prob ) == false )
            {
                // No bigram, trigram, etc entries so backoff to unigram
                temp = unigrams[words[order-1]] ;
                if ( temp <= LOG_ZERO )
                    prob = LOG_ZERO ;
                else if ( prob <= LOG_ZERO )
                    prob = temp ;
                else
                    prob += temp ;
            }
            
            cache->addEntry( order , words , prob ) ;
#ifdef DEBUG
            if ( output_debug == true )
                printf("%f\n",prob) ;
#endif
            return prob ;
        }
        else
        {
            // The entry was in the cache. Just return it
#ifdef DEBUG
            if ( output_debug == true )
                printf("(c)%f\n",prob);
#endif
            return prob ;
        }
    }
}


#ifdef DEBUG
void LMNGram::outputText()
{
    int words[30] ;
    
    // Print the unigrams
    printf("\\1-gram\\\n") ;
    for ( int i=0 ; i<vocab->n_words ; i++ )
        printf("%s %f\n",vocab->words[i],unigrams[i]) ;
   
    // Print the rest
    if ( next_level != NULL )
        next_level->outputText( vocab , words , 0 ) ;
}
#endif


}
