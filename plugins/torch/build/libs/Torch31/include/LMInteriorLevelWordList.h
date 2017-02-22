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

#ifndef LMINTERIORLEVELWORDLIST_INC
#define LMINTERIORLEVELWORDLIST_INC

#include "general.h"
#include "LMInteriorLevelWordEntry.h"

#ifdef DEBUG
#include "Vocabulary.h"
#endif

namespace Torch {


/** 
    This class is used internally within the language model n-gram
    data structures.  It is basically a list of LMInteriorLevelWordEntry
    instances that are sorted by vocab word id.
    
    @author Darren Moore (moore@idiap.ch)
*/

class LMInteriorLevelWordEntry ;

class LMInteriorLevelWordList
{
public:
    int level ;
    int n_entries ;
    LMInteriorLevelWordEntry **entries ;

    /* Constructors / destructor */

    /// Creates the list. Instances of this object are used at different
    ///   levels in the tree-like n-gram data structures.  
    /// 'level_' denotes the level in the tree of this instance.
    LMInteriorLevelWordList( int level_ ) ;
    virtual ~LMInteriorLevelWordList() ;

    /* Methods */

    /// Adds a language model probability. 'order' denotes the order
    ///   of the entry that is being added, and 'words' contains the
    ///   vocabulary indices of the words in the n-gram. The ordering
    ///   of 'words' should be eg. W3,W2,W1,W4 when 'order' is 4 and
    ///   where W4 is the "most-recent" word.
    void addProbEntry( int order , int *words , real prob ) ;
    
    /// Adds a language model backoff weight. 'order' denotes the order
    ///   of the entry that is being added, and 'words' contains the
    ///   vocabulary indices of the words in the n-gram. The ordering
    ///   of 'words' should be eg. W4,W3,W2,W1 when 'order' is 4 and
    ///   where W4 is the "most-recent" word.
    void addBackoffEntry( int order , int *words , real bo_wt ) ;

    /// Calculates a language model probability for a particular word
    ///   sequence with backoff. 'order' denotes the number of words
    ///   in the word sequence, and 'words' contains the vocabulary 
    ///   indices of the words. The ordering of 'words' should be 
    ///   eg. W3,W2,W1,W4 when 'order' is 4 and where W4 is the 
    ///   "most-recent" word. Returns false if no entry was found,
    ///   indicating that the value of prob contains only an
    ///   accumulated backoff weight. Returns true if a valid
    ///   LM prob has been determined.
    bool getProbWithBackoff( int order , int *words , real *prob ) ;

    /// Internal function.
    LMInteriorLevelWordEntry *findWord( int word ) ;

#ifdef DEBUG
    void outputText( Vocabulary *vocab , int *words , int n_words ) ;
#endif
};


}

#endif
