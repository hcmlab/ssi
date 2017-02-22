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
#include "LMInteriorLevelWordList.h"
#include "log_add.h"


namespace Torch {


LMInteriorLevelWordList::LMInteriorLevelWordList( int level_ )
{
    if ( level_ < 1 )
        error("LMInteriorLevelWordList::LMInteriorLevelWordList - cannot have level_ < 1\n") ;
    level = level_ ;
    n_entries = 0 ;
    entries = NULL ;
}


LMInteriorLevelWordList::~LMInteriorLevelWordList()
{
    if ( entries != NULL )
    {
        for ( int i=0 ; i<n_entries ; i++ )
            delete entries[i] ;
        free( entries ) ;
    }
}


void LMInteriorLevelWordList::addProbEntry( int order , int *words , real prob )
{
    // 'words' is an array of word indices.  There should be 'order' elements
    //    in this array and the ordering should be, eg, [w3 w2 w1 w4] for 4-gram.

    LMInteriorLevelWordEntry *new_word=NULL ;
    
#ifdef DEBUG
    if ( (order-1) > level )
        error("LMInteriorLevelWordList - addEntry - order of new entry must be <= level\n") ;
#endif
    
    // Do we have an entry for the word at this level ?
    if ( (new_word = findWord( *words )) == NULL )
    { 
        // No we don't so we need to create an entry and add it to our 'entries' array.
        entries = (LMInteriorLevelWordEntry **)Allocator::sysRealloc( entries , 
                             (n_entries+1)*sizeof(LMInteriorLevelWordEntry *) ) ;
        new_word = new LMInteriorLevelWordEntry( *words ) ;

        // If the list is empty, just insert it straight off.  Also
        //   do an initial check to see if the word belongs at the end of the array
        if ( (n_entries == 0) || (new_word->word > entries[n_entries-1]->word) )
        {
            entries[n_entries] = new_word ;
        }
        else
        {
            // Find the place in the list of words where we want to insert the new word
            for ( int i=0 ; i<n_entries ; i++ )
            {
                if ( (*words) < entries[i]->word ) 
                {
                    // Shuffle down all words from i onwards and place the
                    //   new word in position i.
                    memmove( entries+i+1 , entries+i , 
                             (n_entries-i)*sizeof(LMInteriorLevelWordEntry *) ) ;
                    entries[i] = new_word ;
                    break ;
                }
            }
        }

        n_entries++ ;
    }

    // 'new_word' now points to the entry for the word corresponding to this level.
    // Continue adding our entry.
    new_word->addProbEntry( level , order-1 , words+1 , prob ) ;
}


void LMInteriorLevelWordList::addBackoffEntry( int order , int *words , real bo_wt )
{
    LMInteriorLevelWordEntry *new_word ;
    
    // There should be 'order' entries in 'words' and the order should be
    //   most-recent word first. eg. [w4 w3 w2 w1] for 4-gram
#ifdef DEBUG
    if ( (order > level) || (order < 1) )
        error("LMInteriorLevelWordList::addBackoffEntry - order out of range\n") ;
#endif

    if ( (new_word = findWord( *words )) == NULL )
    {
        // No we don't so we need to create an entry and add it to our 'entries' array.
        entries = (LMInteriorLevelWordEntry **)Allocator::sysRealloc( entries , 
                                         (n_entries+1)*sizeof(LMInteriorLevelWordEntry *) ) ;
        new_word = new LMInteriorLevelWordEntry( *words ) ;

        // If the list is empty, just insert it straight off.  Also
        //   do an initial check to see if the word belongs at the end of the array
        if ( (n_entries == 0) || (new_word->word > entries[n_entries-1]->word) )
        {
            entries[n_entries] = new_word ;
        }
        else
        {
            // Find the place in the list of words where we want to insert the new word
            for ( int i=0 ; i<n_entries ; i++ )
            {
                if ( entries[i]->word > new_word->word )
                {
                    // Shuffle down all words from i onwards and place the
                    //   new word in position i.
                    memmove( entries+i+1 , entries+i , 
                             (n_entries-i)*sizeof(LMInteriorLevelWordEntry *) ) ;
                    entries[i] = new_word ;
                    break ;
                }
            }
        }
        n_entries++ ;
    }

    new_word->addBackoffEntry( level , order-1 , words+1 , bo_wt ) ;
}


bool LMInteriorLevelWordList::getProbWithBackoff( int order , int *words , real *prob )
{
    LMInteriorLevelWordEntry *word_entry ;
    
    // There should be 'order' entries in 'words'.  The ordering should be eg.
    //   W3,W2,W1,W4 when order=4
    if ( (word_entry = findWord( *words )) != NULL )
        return word_entry->getProbWithBackoff( order-1 , words+1 , prob ) ;
    else
    {
        *prob = 0.0 ;
        return false ;
    }
}


LMInteriorLevelWordEntry *LMInteriorLevelWordList::findWord( int word_ )
{
    // We assume that the list of words is in ascending order so 
    //   that we can do a binary search.
    int min=0 , max=(n_entries-1) , curr_pos=0 ;
    
    if ( n_entries == 0 )
        return NULL ;

    if ( n_entries <= 10 )
    {
        // just do a linear search
        for ( int i=0 ; i<n_entries ; i++ )
        {
            if ( word_ < entries[i]->word )
                return NULL ;
            else if ( word_ == entries[i]->word )
                return entries[i] ;
        }
    }
    else
    {
        // do a binary search
        while (1)
        {
            curr_pos = min+(max-min)/2 ;
            if ( word_ < entries[curr_pos]->word )
                max = curr_pos-1 ;
            else if ( word_ > entries[curr_pos]->word )
                min = curr_pos+1 ;
            else
                return entries[curr_pos] ;

            if ( min > max )
                return NULL ;
        }
    }
    return NULL ;
}


#ifdef DEBUG
void LMInteriorLevelWordList::outputText( Vocabulary *vocab , int *words , int n_words )
{
    for ( int i=0 ; i<n_entries ; i++ )
        entries[i]->outputText( vocab , words , n_words ) ;
}
#endif


}


