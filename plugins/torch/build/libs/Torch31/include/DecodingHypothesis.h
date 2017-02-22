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

#ifndef DECODINGHYPOTHESIS_INC
#define DECODINGHYPOTHESIS_INC

#include "general.h"
#include "WordChainElemPool.h"

namespace Torch {


/** This class contains all hypothesis data that needs to be updated
    and propagated as hypotheses are extended through word models and 
    across word boundaries.  Methods are provided to transfer data
    between DecodingHypothesis objects in varying ways.

    @author Darren Moore (moore@idiap.ch)
*/

class DecodingHypothesis
{
public:
    int word ;      // the index of the pronunciation in the lexicon
    int state ;
    real score ;
    WordChainElem *word_level_info ;
    static WordChainElemPool word_chain_elem_pool ;

    /* Constructors / destructor */
    
    DecodingHypothesis() ;
    DecodingHypothesis( int word_ , int state_ ) ;
    virtual ~DecodingHypothesis() ;

    /* Methods */

    void initHyp( int word_ , int state_ ) ;

    /// Unlinks this instance from its word_level_info member variable.
    /// If this instance was the only entity connected to the
    ///   word_level_info object, then the word_level_info object is
    ///   returned to the global word_chain_elem_pool.
    void deactivate() ;

    /// Updates the hypothesis information when a word boundary
    ///   has been crossed.
    void extendWord( real new_score , WordChainElem *new_word_chain_elem ) ;

    /// Updates the hypothesis information when a word-interior state
    ///   transition has been made.
    void extendState( DecodingHypothesis *prev_hyp , real new_score ) ;

#ifdef DEBUG
    void outputText() ;
#endif
} ;


}


#endif
