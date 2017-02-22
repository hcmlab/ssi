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
#include "LMInteriorLevelWordEntry.h"
#include "log_add.h"


namespace Torch {


LMInteriorLevelWordEntry::LMInteriorLevelWordEntry( int word_ )
{
#ifdef DEBUG
    if ( word_ < 0 )
        error("LMInteriorLevelWordEntry::LMInteriorLevelWordEntry - word out of range\n") ;
#endif
    word = word_ ;
    log_bo_weight = 0.0 ;
    next_level = NULL ;
    n_entries = 0 ;
    entries = NULL ;
}


LMInteriorLevelWordEntry::~LMInteriorLevelWordEntry()
{
    if ( next_level != NULL )
        delete next_level ;

    if ( entries != NULL )
        free( entries ) ;
}


void LMInteriorLevelWordEntry::addProbEntry( int list_level , int order , int *words , real prob ) 
{
    // There are 'order' entries in 'words'. The ordering is eg. W3,W2,W1,W4 for order=4.
    if ( order == 1 )
    {
        entries = (LMWordEntry *)Allocator::sysRealloc( entries , 
                                                        (n_entries+1)*sizeof(LMWordEntry) ) ;
        
        // Find the place in the list of words where we want to insert the new word
        if ( (n_entries == 0) || ((*words) > entries[n_entries-1].word) )
        {
            entries[n_entries].word = *words ;
            entries[n_entries].log_prob = prob ;
        }
        else
        {
            // The new word does not belong at the end of the list - find
            //   correct position in list and insert there.
            for ( int i=0 ; i<n_entries ; i++ )
            {
                if ( (*words) < entries[i].word ) 
                {
                    // Shuffle down all words from i onwards and place the
                    //   new word in position i.
                    memmove( entries+i+1 , entries+i , (n_entries-i)*sizeof(LMWordEntry) );
                    entries[i].word = *words ;
                    entries[i].log_prob = prob ;
                    break ;
                }
                else if ( *words == entries[i].word )
                    error("LMILWE::addProbEntry - duplicate entry encountered\n") ;
            }
        }

        n_entries++ ;
    }
    else
    {
        // This entry needs to be added to the 'next_level' (interior) list.
        if ( next_level == NULL )
            next_level = new LMInteriorLevelWordList( list_level-1 ) ;

        next_level->addProbEntry( order , words , prob ) ;
    }
}


void LMInteriorLevelWordEntry::addBackoffEntry( int list_level , int order , 
                                                int *words , real bo_wt ) 
{
    // There should be 'order' entries in 'prev_words' and the ordering should
    //   be eg.  W3,W2,W1 if order == 3.
    if ( order == 0 )
        log_bo_weight = bo_wt ;
    else
    {
        // This entry needs to be added to the 'next_level' (interior) list.
        if ( next_level == NULL )
            next_level = new LMInteriorLevelWordList( list_level-1 ) ;

        next_level->addBackoffEntry( order , words , bo_wt ) ;
    }
}


bool LMInteriorLevelWordEntry::getProbWithBackoff( int order , int *prev_words , real *prob )
{
    // There should be 'order' entries in 'prev_words' and the ordering should
    //   be eg.  W3,W2,W1,W4 if order == 4.
    real temp ;
    
#ifdef DEBUG
    if ( order < 1 )
        error("LMInteriorLevelWordEntry::getProbWithBackoff - order out of range\n") ;
#endif

    if ( order == 1 )
    {
        if ( (*prob = getWordProb( *prev_words )) <= LOG_ZERO )
        {
            *prob = log_bo_weight ;
            return false ;
        }
        else
            return true ;
    }
    else
    {
        if ( next_level == NULL )
        {
            if ( (*prob = getWordProb( prev_words[order-1] )) <= LOG_ZERO )
            {
                *prob = log_bo_weight ;
                return false ;
            }
            else
                return true ;
        }
        else
        {
            if ( next_level->getProbWithBackoff( order , prev_words , prob ) == true )
                return true ;
            else
            {
                if ( (temp = getWordProb( prev_words[order-1] )) <= LOG_ZERO )
                {
                    *prob += log_bo_weight ;
                    return false ;
                }
                else
                {
                    *prob += temp ;
                    return true ;
                }
            }
        }
    }
}


real LMInteriorLevelWordEntry::getWordProb( int word_ )
{
    // We assume that the list of words is in ascending order so 
    //   that we can do a binary search.
    int min=0 , max=(n_entries-1) , curr_pos=0 ;
    
    if ( n_entries == 0 )
        return LOG_ZERO ;
    
    if ( n_entries <= 10 )
    {
        // just do a linear search
        for ( int i=0 ; i<n_entries ; i++ )
        {
            if ( word_ < entries[i].word )
                return LOG_ZERO ;
            else if ( word_ == entries[i].word )
                return entries[i].log_prob ;
        }
    }
    else
    {
        // do a binary search
        while (1)
        {
            curr_pos = min+(max-min)/2 ;
            if ( word_ < entries[curr_pos].word )
                max = curr_pos-1 ;
            else if ( word_ > entries[curr_pos].word )
                min = curr_pos+1 ;
            else
                return entries[curr_pos].log_prob ;

            if ( min > max )
                return LOG_ZERO ;
        }
    }
    return LOG_ZERO ;
}


#ifdef DEBUG
void LMInteriorLevelWordEntry::outputText( Vocabulary *vocab , int *words , int n_words )
{
    words[n_words++] = word ;
    for ( int j=0 ; j<n_entries ; j++ )
    {
        printf("%.4f ",entries[j].log_prob) ;
        for ( int i=0 ; i<n_words ; i++ )
            printf("%s " , vocab->getWord( words[n_words-i-1] ) ) ;
        printf("%s\n" , vocab->getWord( entries[j].word )) ;
    }

    printf("BACKOFF ") ;
    for ( int i=0 ; i<n_words ; i++ )
        printf("%s " , vocab->getWord( words[n_words-i-1] ) ) ;
    printf("%f\n" , log_bo_weight ) ;

    if ( next_level != NULL )
        next_level->outputText( vocab , words , n_words ) ;
}
#endif


}
