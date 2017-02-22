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

#ifndef BEAMSEARCHDECODER_INC
#define BEAMSEARCHDECODER_INC

#include "general.h"
#include "DecodingHypothesis.h"
#include "PhoneModels.h"
#include "LinearLexicon.h"
#include "LanguageModel.h"
#include "Vocabulary.h"
#include "log_add.h"


namespace Torch {


/** This class implements a Viterbi decoder with beam search
    capabilities.  A Lexicon and LanguageModel are required at
    creation time (the LanguageModel is optional).  By default, 
    no pruning occurs.  Two levels of pruning can be configured - 
    word interior hypothesis pruning and word end hypothesis pruning.
    The application of language model probabilities can be
    delayed or performed normally.

    @author Darren Moore (moore@idiap.ch)
*/
class BeamSearchDecoder
{
public:
    PhoneModels *phone_models ;
    LinearLexicon *lexicon ;
    LanguageModel *lang_model ;
    Vocabulary *vocabulary ;
    int n_frames ;
    real log_word_entrance_penalty ;
    real word_int_beam ;
    real word_end_beam ;

    DecodingHypothesis ***word_state_hyps_1 ;
    DecodingHypothesis ***word_state_hyps_2 ;
    DecodingHypothesis **word_end_hyps_1 ;   
    DecodingHypothesis **word_end_hyps_2 ;
    DecodingHypothesis **word_entry_hyps_1 ;
    DecodingHypothesis **word_entry_hyps_2 ;
    DecodingHypothesis ***curr_word_hyps ;
    DecodingHypothesis ***prev_word_hyps ;
    DecodingHypothesis **curr_word_entry_hyps ;
    DecodingHypothesis **prev_word_entry_hyps ;
    DecodingHypothesis **curr_word_end_hyps ;
    DecodingHypothesis **prev_word_end_hyps ;

    int sent_start_index ;
    int sent_end_index ;

    bool verbose_mode ;
    real max_interior_score ;
    DecodingHypothesis *best_word_end_hyp ;
    bool delayed_lm ;
    
    /* Constructors/destructor */
    BeamSearchDecoder( LinearLexicon *lexicon_ , LanguageModel *lang_model_ ,
                       real log_word_entrance_penalty_=0.0 , real word_int_beam_=LOG_ZERO ,
                       real word_end_beam_=LOG_ZERO , bool delayed_lm_=true , 
                       bool verbose_mode_=false ) ;
    virtual ~BeamSearchDecoder() ;

    /* Methods */

    /// Decodes using the input data vectors in 'input_data'.
    /// 'n_frames_' is the number of vectors of input data.
    /// 'vec_size' is the number of elements in each vector.  The input data can be either
    ///   features or emission probabilities and 'vec_size' must reflect this.
    /// After this function returns, 'num_result_words' contains the number of words 
    ///   recognised and 'result_words' contains the vocabulary indices of the recognised
    ///   words.  The 'result_words' array is allocated inside this function.
    void decode( real **input_data , int n_frames_ , int *num_result_words , int **result_words ,
                 int **result_words_times ) ;

    void resetHypotheses() ;
    void swapHypBuffers() ;
    void processWordInteriorStates() ;
    void applyLMProbs() ;
    void processWordTransitionsLM( int curr_frame ) ;
    void processWordTransitionsNoLM( int curr_frame ) ;
    void processWordEntryHypotheses() ;
    void init() ;
} ;


}



#endif
