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

#ifndef LANGUAGEMODEL_INC
#define LANGUAGEMODEL_INC

#include "general.h"
#include "DecodingHypothesis.h"
#include "Vocabulary.h"
#include "LMNGram.h"

namespace Torch {


/** 
    This object implements an n-gram language model. The n-gram data structures
    are encapsulated in the ngram member variable (see LMNGram class). Methods
    are provided to read a LM file in ARPA format or in Noway binary format.
    A method is provided to calculate a LM prob (with backoff) for a given 
    sequence of words.
      
    @author Darren Moore (moore@idiap.ch)
*/

class LanguageModel
{
public:
    int order ;
    int n_words ;
    Vocabulary *vocabulary ;
    LMNGram *ngram ;
    real lm_scaling_factor ;

    bool lm_has_start_word ;
    bool lm_has_end_word ;

    /* constructors / destructor */

    /// Creates the language model. 
    /// 'order_' is the order of the LM (eg. 3 for trigram).
    LanguageModel( int order_ , Vocabulary *vocabulary_ , char *lm_fname , 
                   real lm_scaling_factor_=1.0 ) ; 
    virtual ~LanguageModel() ;

    /* methods */
    /// Calculates the language model probability (with backoff) of the word
    ///   sequence stored in the hypothesis pointed to by 'word_end_hyp'.
    real calcLMProb( DecodingHypothesis *word_end_hyp ) ;
    
    /// Calculates the language model probability (with backoff) of 'next_word'
    ///   given the previous word sequence stored in the hypothesis pointed to 
    ///   by 'prev_word_end_hyp'.
    real calcLMProb( DecodingHypothesis *prev_word_end_hyp , int next_word ) ;

    /// Creates a language model from an ARPA format file. Internal function.
    void readARPA( FILE *arpa_fd ) ;
    
    /// Creates a language model from an Noway binary LM format file. Internal function.
    /// nb. Only the TR2 and NG3 types are supported (ie. trigrams). 
    void readNowayBin( FILE *nw_fd ) ;

#ifdef DEBUG
    void outputText() ;
#endif
};



}


#endif
