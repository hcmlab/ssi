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

#ifndef LMINTERIORLEVELWORDENTRY_INC
#define LMINTERIORLEVELWORDENTRY_INC

#include "general.h"
#include "LMInteriorLevelWordList.h"

#ifdef DEBUG
#include "Vocabulary.h"
#endif


namespace Torch {


typedef struct
{
    int word ;
    real log_prob ;
} LMWordEntry ;


class LMInteriorLevelWordList ;

/** 
    This class is used internally within the language model n-gram
    data structures.  It contains a list of LM probabilities relevant
    to the level of the tree structure where it exists, and a link to 
    the next level of the tree structure.
    
    @author Darren Moore (moore@idiap.ch)
*/

class LMInteriorLevelWordEntry
{
public:
    int word ;
    real log_bo_weight ;
    LMInteriorLevelWordList *next_level ;
    int n_entries ;
    LMWordEntry *entries ;
    
    /* Constructors / destructor */
    LMInteriorLevelWordEntry( int word_ ) ;
    virtual ~LMInteriorLevelWordEntry() ;
    
    /* Methods */
    void addProbEntry( int list_level , int order , int *words , real prob ) ;
    void addBackoffEntry( int list_level , int order , int *words , real bo_wt ) ;
    bool getProbWithBackoff( int order , int *prev_words , real *prob ) ;

    // Internal function.
    real getWordProb( int word ) ;

#ifdef DEBUG
    void outputText( Vocabulary *vocab , int *words , int n_words ) ;
#endif
};


}


#endif
