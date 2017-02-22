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
#include "DecodingHypothesis.h"
#include "WordChainElemPool.h"
#include "log_add.h"


namespace Torch {


WordChainElemPool DecodingHypothesis::word_chain_elem_pool(1000) ;


DecodingHypothesis::DecodingHypothesis()
{
    word = -1 ;
    state = -1 ;
    score = LOG_ZERO ;
    word_level_info = NULL ;
}


DecodingHypothesis::DecodingHypothesis( int word_ , int state_ )
{
    word = word_ ;
    state = state_ ;
    score = LOG_ZERO ;
    word_level_info = NULL ;
}


DecodingHypothesis::~DecodingHypothesis()
{
    if ( word_level_info != NULL )
    {
        if ( --(word_level_info->n_connected) <= 0 )
            DecodingHypothesis::word_chain_elem_pool.returnElem( word_level_info ) ;
    }
}


void DecodingHypothesis::initHyp( int word_ , int state_ )
{
    word = word_ ;
    state = state_ ;
    score = LOG_ZERO ;
    word_level_info = NULL ;
}


void DecodingHypothesis::deactivate()
{
    score = LOG_ZERO ;
    if ( word_level_info != NULL )
    {
        if ( --(word_level_info->n_connected) <= 0 )
        {
            // Only this hypothesis is accessing this word-level information.
            // Return the word_level_info instance to the word_chain_elem_pool.
#ifdef DEBUG
            if ( word_level_info->n_connected < 0 )
                error("DecodingHypothesis::deactivate - n_connected < 0\n") ;
#endif
            DecodingHypothesis::word_chain_elem_pool.returnElem( word_level_info ) ;
        }
        word_level_info = NULL ;
    }
}


void DecodingHypothesis::extendWord( real new_score , WordChainElem *new_word_chain_elem )
{
#ifdef DEBUG
    if ( new_word_chain_elem == NULL )
        error("DecodingHypothesis:extendWord - new_word_chain_elem is NULL\n");
#endif
    deactivate() ;
    score = new_score ;
    word_level_info = new_word_chain_elem ;
    new_word_chain_elem->n_connected++ ;
}
            

void DecodingHypothesis::extendState( DecodingHypothesis *prev_hyp , real new_score )
{
#ifdef DEBUG
    if ( prev_hyp->word_level_info == NULL )
        error("DecodingHypothesis:extendState - prev_hyp->word_level_info is NULL\n") ;
#endif
    deactivate() ;
    score = new_score ;
    word_level_info = prev_hyp->word_level_info ;
    if ( word_level_info != NULL )
        word_level_info->n_connected++ ;
}


#ifdef DEBUG
void DecodingHypothesis::outputText()
{
    printf("word=%d, state=%d, score=%.3f\n",word,state,score);
}
#endif


}
