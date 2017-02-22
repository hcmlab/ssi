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

#ifndef LEXICONINFO_INC
#define LEXICONINFO_INC

#include "general.h"
#include "Object.h"
#include "Vocabulary.h"
#include "PhoneInfo.h"


namespace Torch {


typedef struct
{
    int n_phones ;
    int *phones ;
    real log_prior ;
    int vocab_index ;
} LexiconInfoEntry ;


typedef struct
{
    int n_pronuns ;
    int *pronuns ;
} VocabToLexMapEntry ;


/** 
    This class stores information about how phonemes are assembled
    into pronunciations. For each pronunciation, a list of indices
    into a PhoneInfo instance is stored, along with a prior and a 
    index into a Vocabulary instance. Information is also stored to
    map Vocabulary entries to one or more pronunciations.
    
    @author Darren Moore (moore@idiap.ch)
*/

class LexiconInfo : public Object
{
public:
    int n_entries ;
    LexiconInfoEntry *entries ;
    int sent_start_index ;
    int sent_end_index ;
    int sil_index ;
    PhoneInfo *phone_info ;
    Vocabulary *vocabulary ;
    VocabToLexMapEntry *vocab_to_lex_map ;

    /// Creates a LexiconInfo instance. 'phones_fname' is used to create a
    ///   PhoneInfo instance (see PhoneInfo header). 'lex_fname' is used to
    ///   create a Vocabulary instance and then to create pronunciation
    ///   entries and the mapping between Vocabulary entries and pronunciation
    ///   entries.
    LexiconInfo( const char *phones_fname , const char *sil_phone , const char *pause_phone ,
                 const char *lex_fname , const char *sent_start_word=NULL , const char *sent_end_word=NULL ,
                 const char *sil_word=NULL ) ;
    virtual ~LexiconInfo() ;

    void initLexInfoEntry( LexiconInfoEntry *entry ) ;

#ifdef DEBUG
    void outputText() ;
#endif
};


}

#endif
