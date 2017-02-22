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

#ifndef VOCABULARY_INC
#define VOCABULARY_INC

#include "general.h"
#include "Object.h"

namespace Torch {


/** 
	This object contains the list of words we want our recogniser to recognise
    plus a few "special" words (eg. sentence markers, silence word). There are
    no duplicates in the list, and the list is sorted alphabetically.
      
    @author Darren Moore (moore@idiap.ch)
*/

class Vocabulary : public Object
{
public:
	int n_words ;
    char **words ;
    int sent_start_index ;
    int sent_end_index ;
    int sil_index ;
	
	/* Constructors / Destructor */

    /// Creates the vocabulary.
    /// 'lex_fname' is the name of the lexicon file containing the pronunciations to be
    ///   recognised.  The format is the standard "word(prior) ph ph ph" format
    ///   where the (prior) is optional.
    /// 'sent_start_word' and 'sent_end_word' are the words that will start and
    ///   end every recognised utterance. 
	Vocabulary( const char *lex_fname , const char *sent_start_word , const char *sent_end_word , 
                const char *sil_word=NULL ) ;
	virtual ~Vocabulary() ;

	/* Methods */
    
    /// Adds a word to the vocabulary. Maintains alphabetic order. Does not add
    ///   duplicate entries.
	void addWord( char *word ) ;

    /// Returns the word given the index into the vocabulary
	char *getWord( int index ) ;
    
    /// Returns the index of a given word.  If 'guess' is defined, then the
    ///   words at indices of 'guess' and 'guess+1' are checked for a match
    ///   before the rest of the vocab is searched.
    int getIndex( char *word , int guess=-1 ) ;

#ifdef DEBUG
	void outputText() ;
#endif
};


}
#endif
