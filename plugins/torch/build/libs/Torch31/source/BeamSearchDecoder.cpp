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
#include "BeamSearchDecoder.h"


namespace Torch {


BeamSearchDecoder::BeamSearchDecoder( LinearLexicon *lexicon_ , LanguageModel *lang_model_ ,
                                      real log_word_entrance_penalty_ , real word_int_beam_ ,
                                      real word_end_beam_ , bool delayed_lm_ , bool verbose_mode_ )
{
    if ( lexicon_ == NULL )
        error("BeamSearchDecoder::BeamSearchDecoder - no lexicon defined\n") ;
    
    lexicon = lexicon_ ;
    vocabulary = lexicon->lex_info->vocabulary ;
    lang_model = lang_model_ ;
    phone_models = lexicon->phone_models ;
    
    n_frames = 0 ;
    log_word_entrance_penalty = log_word_entrance_penalty_ ;
    verbose_mode = verbose_mode_ ;
    delayed_lm = delayed_lm_ ;

    word_state_hyps_1 = (DecodingHypothesis ***)Allocator::sysAlloc( lexicon->n_models * 
                                                   sizeof(DecodingHypothesis **) ) ;
    word_state_hyps_2 = (DecodingHypothesis ***)Allocator::sysAlloc( lexicon->n_models * 
                                                   sizeof(DecodingHypothesis **) ) ;
    word_end_hyps_1 = (DecodingHypothesis **)Allocator::sysAlloc( lexicon->n_models * 
                                                   sizeof(DecodingHypothesis *) ) ;
    word_end_hyps_2 = (DecodingHypothesis **)Allocator::sysAlloc( lexicon->n_models * 
                                                   sizeof(DecodingHypothesis *) ) ;
    word_entry_hyps_1 = (DecodingHypothesis **)Allocator::sysAlloc( lexicon->n_models * 
                                                   sizeof(DecodingHypothesis *) ) ;
    word_entry_hyps_2 = (DecodingHypothesis **)Allocator::sysAlloc( lexicon->n_models * 
                                                   sizeof(DecodingHypothesis *) ) ;
    curr_word_hyps = word_state_hyps_1 ;
    prev_word_hyps = word_state_hyps_2 ;
    curr_word_end_hyps = word_end_hyps_1 ;
    prev_word_end_hyps = word_end_hyps_2 ;
    curr_word_entry_hyps = word_entry_hyps_1 ;
    prev_word_entry_hyps = word_entry_hyps_2 ;
    
    for ( int w=0 ; w<lexicon->n_models ; w++ )
    {
        word_state_hyps_1[w] = (DecodingHypothesis **)Allocator::sysAlloc( 
                                    lexicon->nStatesInModel(w) * sizeof(DecodingHypothesis *) ) ;
        word_state_hyps_2[w] = (DecodingHypothesis **)Allocator::sysAlloc( 
                                    lexicon->nStatesInModel(w) * sizeof(DecodingHypothesis *) ) ;
        for ( int s=0 ; s<lexicon->nStatesInModel(w) ; s++ )
        {
            word_state_hyps_1[w][s] = new DecodingHypothesis() ;
            word_state_hyps_1[w][s]->initHyp( w , s ) ;
            word_state_hyps_2[w][s] = new DecodingHypothesis() ;
            word_state_hyps_2[w][s]->initHyp( w , s ) ;
        }
        
        word_end_hyps_1[w] = word_state_hyps_1[w][lexicon->nStatesInModel(w)-1] ;
        word_end_hyps_2[w] = word_state_hyps_2[w][lexicon->nStatesInModel(w)-1] ;
        word_entry_hyps_1[w] = word_state_hyps_1[w][0] ; 
        word_entry_hyps_2[w] = word_state_hyps_2[w][0] ; 
    }

    sent_start_index = lexicon->lex_info->sent_start_index ;
    sent_end_index = lexicon->lex_info->sent_end_index ;
    
    max_interior_score = LOG_ZERO ;
    best_word_end_hyp = NULL ;
    
    if ( word_int_beam_ > 0.0 )
        word_int_beam = word_int_beam_ ;
    else
        word_int_beam = -LOG_ZERO ;
    
    if ( word_end_beam_ > 0.0 )
        word_end_beam = word_end_beam_ ;
    else
        word_end_beam = -LOG_ZERO ;
}


BeamSearchDecoder::~BeamSearchDecoder()
{
    resetHypotheses() ;

    if ( word_state_hyps_1 != NULL )
    {
        for ( int w=0 ; w<lexicon->n_models ; w++ )
        {
            for ( int s=0 ; s<lexicon->nStatesInModel(w) ; s++ )
                delete word_state_hyps_1[w][s] ;
            free( word_state_hyps_1[w] ) ;
        }
        free( word_state_hyps_1 ) ;
    }
    if ( word_state_hyps_2 != NULL )
    {
        for ( int w=0 ; w<lexicon->n_models ; w++ )
        {
            for ( int s=0 ; s<lexicon->nStatesInModel(w) ; s++ )
                delete word_state_hyps_2[w][s] ;
            free( word_state_hyps_2[w] ) ;
        }
        free( word_state_hyps_2 ) ;
    }

    if ( word_end_hyps_1 != NULL )
        free( word_end_hyps_1 ) ;
    if ( word_end_hyps_2 != NULL )
        free( word_end_hyps_2 ) ;
    if ( word_entry_hyps_1 != NULL )
        free( word_entry_hyps_1 ) ;
    if ( word_entry_hyps_2 != NULL )
        free( word_entry_hyps_2 ) ;
}      


void BeamSearchDecoder::decode( real **input_data , int n_frames_ , int *num_result_words , 
                                int **result_words , int **result_words_times )
{
    int temp_words[5000] , temp_times[5000] ;
    DecodingHypothesis *curr_hyp ;
    real score ;

#ifdef DEBUG
    if ( (num_result_words==NULL) || (result_words==NULL) || (result_words_times==NULL) )
        error("BeamSearchDecoder::decode - Result variables are NULL\n") ;
#endif

    n_frames = n_frames_ ;

    // Initialise the hypothesis buffers and queues.
    init() ;
    
    // process the inputs
    for ( int t=0 ; t<n_frames ; t++ )
    {
        if ( verbose_mode == true )
        {
            fprintf( stderr , "\r                                                                        \r") ;
            fprintf( stderr , "Frame %d of %d",t+1,n_frames) ; fflush(stderr) ;
        }

        // Swap hypothesis buffers
        swapHypBuffers() ;
        
        // Pass the new input vector to the phone set - it knows whether the inputs
        //   are emission probabilities or features and how to handle each.
        phone_models->setInputVector( input_data[t] ) ;

        // Process the transitions between states inside words.
        processWordInteriorStates() ;

        if ( t == (n_frames-1) )
        {
            // We've reached the end of the input data - no need to evaluate word transitions.
            if ( verbose_mode == true )
                fprintf( stderr , "\r                                                                    \r") ;
            break ;
        }

        // If there is a language model, then tune the word-end hypotheses (that remain
        //   after pruning) using the language model.
        // After that, evaluate word transitions.
        if ( lang_model != NULL )
        {
            if ( delayed_lm == false )
                processWordTransitionsLM( t ) ;
            else
            {
                applyLMProbs() ;
                processWordTransitionsNoLM( t ) ;
            }
        }
        else
            processWordTransitionsNoLM( t ) ;
        
        // We now have hypotheses for the initial states of all possible next words.
        // These hypotheses cannot remain in the (non-emitting) initial states.
        // We have to consider transitions from each initial state to all possible
        //   (emitting) successor states and see if the word entry hypothesis score is better
        //   than the current hypothesis in the successor state.
        // If so, we update the hypothesis in the successor state using the word entry
        //   hypothesis.
        processWordEntryHypotheses() ;
    }
   
    if ( sent_end_index >= 0 )
    {
        // We look at the hypothesis that is in the final state of the sentence end pronunciation.
        curr_hyp = curr_word_end_hyps[sent_end_index] ;
    }
    else
    {
        if ( (sent_start_index >= 0) && (best_word_end_hyp==curr_word_end_hyps[sent_start_index]) )
        {
            // Cannot have start word as end of sentence word.
            // Find the next best.
            score = LOG_ZERO ;
            curr_hyp = NULL ;
            for ( int i=0 ; i<lexicon->n_models ; i++ )
            {
                if ( i == sent_start_index )
                    continue ;

                if ( curr_word_end_hyps[i]->score > score )
                {
                    score = curr_word_end_hyps[i]->score ;
                    curr_hyp = curr_word_end_hyps[i] ;
                }
            }
        }
        else
            curr_hyp = best_word_end_hyp ;
    }
            
    if ( (curr_hyp == NULL) || (curr_hyp->score <= LOG_ZERO) )
    {
        // There is no hypothesis that is in the final state of the sentence end model.
        *num_result_words = 0 ;
        *result_words = NULL ;
        *result_words_times = NULL ;
        return ;
    }
    
    // Allocate memory for the result array (ie. array of indices corresponding to words
    //   in the lexicon).
    WordChainElem *temp_elem = curr_hyp->word_level_info ;
    *num_result_words = 0 ;
    while ( temp_elem != NULL )
    {
        temp_words[*num_result_words] = temp_elem->word ;
        temp_times[*num_result_words] = temp_elem->word_start_frame ;
        temp_elem = temp_elem->prev_elem ;
        (*num_result_words)++ ;
    }
    *result_words = (int *)Allocator::sysAlloc( (*num_result_words) * sizeof(int) ) ;
    *result_words_times = (int *)Allocator::sysAlloc( (*num_result_words) * sizeof(int) ) ;
    for ( int w=0 ; w<(*num_result_words) ; w++ )
    {
        (*result_words)[w] = temp_words[(*num_result_words)-w-1] ;
        (*result_words_times)[w] = temp_times[(*num_result_words)-w-1] ;
    }
}


void BeamSearchDecoder::resetHypotheses()
{
    // Reset the scores of the new state hypotheses buffers
    // If the decoder has already been used, only the curr_word_hyps will
    //   contain active hypotheses that need to be deactivated.
    for ( int w=0 ; w<lexicon->n_models ; w++ )
    {
        for ( int s=0 ; s<(lexicon->nStatesInModel(w)) ; s++ )
        {
            curr_word_hyps[w][s]->deactivate() ;
            // word_state_hyps_1[w][s]->deactivate() ;
            // word_state_hyps_2[w][s]->deactivate() ;
        }
    }
}


void BeamSearchDecoder::swapHypBuffers()
{
    // Swap buffers
    if ( curr_word_hyps == word_state_hyps_1 )
    {
        curr_word_hyps = word_state_hyps_2 ;
        prev_word_hyps = word_state_hyps_1 ;
        curr_word_end_hyps = word_end_hyps_2 ;
        prev_word_end_hyps = word_end_hyps_1 ;
        curr_word_entry_hyps = word_entry_hyps_2 ;
        prev_word_entry_hyps = word_entry_hyps_1 ;
    }    
    else
    {
        curr_word_hyps = word_state_hyps_1 ;
        prev_word_hyps = word_state_hyps_2 ;
        curr_word_end_hyps = word_end_hyps_1 ;
        prev_word_end_hyps = word_end_hyps_2 ;
        curr_word_entry_hyps = word_entry_hyps_1 ;
        prev_word_entry_hyps = word_entry_hyps_2 ;
    }
    
    if ( verbose_mode == true )
    {
        fprintf( stderr , ": " ) ; fflush(stderr) ;
    }
}

void BeamSearchDecoder::processWordInteriorStates()
{
    DecodingHypothesis *prev_hyp ;
    real emission_prob , new_score , *suc_log_trans_probs , int_prune_thresh ;
    real temp_int_prune_thresh , temp_end_prune_thresh , max_end_score ;
    int n_processed=0 , n_states_minus_one ;
    short n_sucs , *sucs ; 

    // Process the interior state hypotheses for the "normal" lexicon words.
    int_prune_thresh = max_interior_score - word_int_beam ;

    max_interior_score = LOG_ZERO ;
    max_end_score = LOG_ZERO ;
    temp_int_prune_thresh = LOG_ZERO ;
    temp_end_prune_thresh = LOG_ZERO ;
    best_word_end_hyp = NULL ;
    
    for ( int w=0 ; w<lexicon->n_models ; w++ )
    {
        n_states_minus_one = lexicon->nStatesInModel(w) - 1 ;
        for ( int s=1 ; s<n_states_minus_one ; s++ )      // for all emitting states
        {
            prev_hyp = prev_word_hyps[w][s] ;
            if ( prev_hyp->score <= LOG_ZERO )
                continue ;

#ifdef DEBUG
            // We assume from this point on that the word/state field in the hypothesis
            //   correspond to the indices in the nested loops (s & w). Check that this is so.
            if ( (prev_hyp->word != w) || (prev_hyp->state != s) )
                error("BeamSearchDecoder::processWordIntStates - word-state index mismatch\n") ;
#endif
            if ( w == sent_end_index )
            {
                // We don't want to prune any of the sentence end hypotheses.
                emission_prob = lexicon->calcEmissionProb( w , s ) ;
                lexicon->getSuccessorInfo( w , s , &n_sucs , &sucs , &suc_log_trans_probs ) ;
                for ( int suc=0 ; suc<n_sucs ; suc++ )
                {
                    new_score = prev_hyp->score + emission_prob + suc_log_trans_probs[suc] ;
                    if ( new_score > curr_word_hyps[w][sucs[suc]]->score )
                        curr_word_hyps[w][sucs[suc]]->extendState( prev_hyp , new_score ) ;
                }
            }
            else if ( prev_hyp->score >= int_prune_thresh )
            {
                n_processed++ ;
                
                // Retrieve/calculate the emission probability for the current state.
                emission_prob = lexicon->calcEmissionProb( w , s ) ;
                
                // The hypothesis we've just retrieved is for a particular word, w,
                //   and state, sprev.
                // See if a path through (w,sprev) improves the current hypothesis for
                //   every (next) state, s, of word w.
                lexicon->getSuccessorInfo( w , s , &n_sucs , &sucs , &suc_log_trans_probs ) ;
                for ( int suc=0 ; suc<n_sucs ; suc++ )
                {
                    new_score = prev_hyp->score + emission_prob + suc_log_trans_probs[suc] ;

                    if ( sucs[suc] == n_states_minus_one )
                    {
                        // The final state is a special case. If we have a language model,
                        //   then we want to prune word end hyps before we apply LM probs.
                        // If we don't have a language model, then we only need to keep 
                        //   track of the most likely word end.
                        if ( lang_model != NULL )
                        {
                            if ( (new_score >= temp_end_prune_thresh) &&
                                 (new_score > curr_word_hyps[w][n_states_minus_one]->score ) )
                            { 
                                if ( new_score > max_end_score )
                                {
                                    best_word_end_hyp = curr_word_hyps[w][n_states_minus_one] ;
                                    max_end_score = new_score ;
                                    temp_end_prune_thresh = new_score - word_end_beam ;
                                }
                                curr_word_hyps[w][n_states_minus_one]->extendState( prev_hyp , 
                                                                                    new_score ) ;
                            }
                        }
                        else
                        {
                            if ( new_score > max_end_score )
                            {
                                if ( best_word_end_hyp != NULL )
                                    best_word_end_hyp->deactivate() ;
                                best_word_end_hyp = curr_word_hyps[w][n_states_minus_one] ;
                                max_end_score = new_score ;
                                curr_word_hyps[w][n_states_minus_one]->extendState( prev_hyp , 
                                                                                    new_score ) ;
                            }
                        }
                    }
                    else
                    {
                        if ( new_score > curr_word_hyps[w][sucs[suc]]->score )
                        {
                            if ( new_score >= temp_int_prune_thresh )
                            {
                                if ( new_score > max_interior_score )
                                {
                                    max_interior_score = new_score ;
                                    temp_int_prune_thresh = new_score - word_int_beam ;
                                }
                                curr_word_hyps[w][sucs[suc]]->extendState( prev_hyp , new_score );
                            }
                        }
                    }                    
                }
            }
            
            // We've finished with this hypothesis, so deactivate it.
            prev_hyp->deactivate() ;
        }
    }

    if ( verbose_mode == true )
    {
        fprintf( stderr , "%d," , n_processed ) ; fflush(stderr) ;
    }
}


void BeamSearchDecoder::applyLMProbs()
{
    real temp_end_prune_thresh , best_score , score ;
   
    if ( best_word_end_hyp != NULL )
        temp_end_prune_thresh = best_word_end_hyp->score - word_end_beam ;
    else
        temp_end_prune_thresh = LOG_ZERO ;
        
    best_word_end_hyp = NULL ;
    best_score = LOG_ZERO ;
    for ( int i=0 ; i<lexicon->n_models ; i++ )
    { 
        if ( i == sent_end_index )
        {
            curr_word_end_hyps[i]->deactivate() ;
            continue ;
        }
            
        score = curr_word_end_hyps[i]->score ;
        if ( score > LOG_ZERO )
        {
            if ( score < temp_end_prune_thresh )
                curr_word_end_hyps[i]->deactivate() ;
            else
            {
                score += lang_model->calcLMProb( curr_word_end_hyps[i] ) ;
                if ( score > best_score )
                {
                    curr_word_end_hyps[i]->score = score ;
                    best_score = score ;
                    if ( best_word_end_hyp != NULL )
                        best_word_end_hyp->deactivate() ;
                    best_word_end_hyp = curr_word_end_hyps[i] ;
                }
                else
                    curr_word_end_hyps[i]->deactivate() ;
            }
        }
    }
}


void BeamSearchDecoder::processWordTransitionsLM( int curr_frame )
{   
    real prob ;
    int *pronuns , n_pronuns , n_processed ;
    WordChainElem *next_word_chain_elem ;
    real temp_end_prune_thresh ;
    
    if ( best_word_end_hyp != NULL )
        temp_end_prune_thresh = best_word_end_hyp->score - word_end_beam ;
    else
        temp_end_prune_thresh = LOG_ZERO ;
        
    n_processed=0 ;
    for ( int i=0 ; i<lexicon->n_models ; i++ )
    {
        if ( i == sent_end_index )
        {
            curr_word_end_hyps[i]->deactivate() ;
            continue ;
        }
        if ( curr_word_end_hyps[i]->score <= LOG_ZERO )
            continue ;
        if ( curr_word_end_hyps[i]->score < temp_end_prune_thresh )
        {
            curr_word_end_hyps[i]->deactivate() ;
            continue ;
        }

        n_processed++ ;

        for ( int w=0 ; w<vocabulary->n_words ; w++ )
        {
            if ( (w == vocabulary->sent_end_index) && (i == sent_start_index) )
                continue ;

            prob = log_word_entrance_penalty + curr_word_end_hyps[i]->score +
                   lang_model->calcLMProb( curr_word_end_hyps[i] , w ) ;
            
            pronuns = lexicon->lex_info->vocab_to_lex_map[w].pronuns ;
            n_pronuns = lexicon->lex_info->vocab_to_lex_map[w].n_pronuns ;
        
            next_word_chain_elem = DecodingHypothesis::word_chain_elem_pool.getElem( w , 
                                        curr_word_end_hyps[i]->word_level_info , curr_frame ) ;
            for ( int p=0 ; p<n_pronuns ; p++ )
            {
                if ( pronuns[p] == sent_start_index )
                    continue ;
                    
                if ( prob > curr_word_entry_hyps[pronuns[p]]->score )
                    curr_word_entry_hyps[pronuns[p]]->extendWord( prob , next_word_chain_elem ) ;
            }
            if ( next_word_chain_elem->n_connected <= 0 )
                DecodingHypothesis::word_chain_elem_pool.returnElem( next_word_chain_elem ) ;
        }
        curr_word_end_hyps[i]->deactivate() ;
    }

    if ( verbose_mode == true )
    {
        fprintf( stderr , "%d " , n_processed ) ; fflush(stderr) ;
    }
}


void BeamSearchDecoder::processWordTransitionsNoLM( int curr_frame )
{
    // The best_word_end_hyp member points to the best word end hypothesis.
    int *pronuns , n_pronuns ;
    WordChainElem *next_word_chain_elem ;
    real score ;

    if ( verbose_mode == true )
    {
        fprintf( stderr , ":" ) ; fflush(stderr) ;
    }

    // Now extend the best word end hypothesis to the initial states of all
    //   words and the initial state of the sentence end word.
    // If the best word end hyp was the final state of the sentence end hypothesis
    //   then we don't extend it to any other words.
    if ( best_word_end_hyp != NULL )
    {
        score = best_word_end_hyp->score + log_word_entrance_penalty ;
        for ( int w=0 ; w<vocabulary->n_words ; w++ )
        {
            if ( (w == vocabulary->sent_end_index) && (sent_start_index >= 0) &&
                 (best_word_end_hyp == curr_word_end_hyps[sent_start_index]) )
                continue ; // A start-to-end transition is invalid
            pronuns = lexicon->lex_info->vocab_to_lex_map[w].pronuns ;
            n_pronuns = lexicon->lex_info->vocab_to_lex_map[w].n_pronuns ;
            
#ifdef DEBUG
            if ( n_pronuns == 0 )
                error("BeamSearchDecoder::processWordTransNoLM - voc word %d has no pronuns\n",w);
#endif
            next_word_chain_elem = DecodingHypothesis::word_chain_elem_pool.getElem( w , 
                                            best_word_end_hyp->word_level_info , curr_frame) ;
            for ( int p=0 ; p<n_pronuns ; p++ )
            {
                if ( pronuns[p] == sent_start_index )
                    continue ; // Cannot make a transition to the sentence start word.
                
#ifdef DEBUG
                // If the score of each word entry hyp at this point is not LOG_ZERO then we
                //   have a problem.
                if ( curr_word_entry_hyps[pronuns[p]]->score > LOG_ZERO )
                    error("BeamSearchDecoder::processWordTransNoLM - word entry hyp not reset\n") ;
#endif
                curr_word_entry_hyps[pronuns[p]]->extendWord( score , next_word_chain_elem ) ;
            }
            if ( next_word_chain_elem->n_connected <= 0 )
                DecodingHypothesis::word_chain_elem_pool.returnElem( next_word_chain_elem ) ;
        }
    }
    
    // Deactivate the best word-end hypotheses and the sentence end word-end hypothesis.
    if ( best_word_end_hyp != NULL )
        best_word_end_hyp->deactivate() ;
    best_word_end_hyp = NULL ;
    if ( sent_end_index >= 0 )
        curr_word_end_hyps[sent_end_index]->deactivate() ;
}
    

void BeamSearchDecoder::processWordEntryHypotheses()
{
    DecodingHypothesis *curr_hyp ;
    short n_sucs , *sucs ;
    real *suc_log_trans_probs , new_score , temp_prune_thresh ;

    temp_prune_thresh = max_interior_score - word_int_beam ;
    for ( int w=0 ; w<lexicon->n_models ; w++ )
    {
        curr_hyp = curr_word_entry_hyps[w] ;
        if ( curr_hyp->score <= LOG_ZERO )
        {
            curr_hyp->deactivate() ;
            continue ;
        }
        
        // For each successor state, s, for the initial state of word, w, is our hyposthesis 
        //   improved if we consider the best word boundary hypothesis ? 
        //   (ie. Is there a better path ending in state s that comes in through a word boundary?)
        lexicon->getSuccessorInfo( w , 0 , &n_sucs , &sucs , &suc_log_trans_probs ) ;
        for ( int s=0 ; s<n_sucs ; s++ )
        {
            new_score = curr_hyp->score + suc_log_trans_probs[s] ;
            if ( new_score > curr_word_hyps[w][sucs[s]]->score )
            {
                if ( w == sent_end_index )
                {
                    // We don't want to prune sentence end hypotheses.
                    curr_word_hyps[w][sucs[s]]->extendState( curr_hyp , new_score ) ;
                }
                else if ( new_score >= temp_prune_thresh )
                {
                    if ( new_score > max_interior_score )
                    {
                        max_interior_score = new_score ;
                        temp_prune_thresh = max_interior_score - word_int_beam ;
                    }
                    curr_word_hyps[w][sucs[s]]->extendState( curr_hyp , new_score ) ;
                }
            }
        }

        curr_hyp->deactivate() ;
    }
}


void BeamSearchDecoder::init()
{
    short n_sucs , *sucs ;
    int n_pronuns , *pronuns ;
    real *suc_log_trans_probs ;
    real new_score ;
    WordChainElem *next_word_chain_elem ;

    // Reset all hypotheses.
    resetHypotheses() ;

    max_interior_score = LOG_ZERO ;
    best_word_end_hyp = NULL ;
   
    // If there is a sentence start word defined, initialise just the initial state 
    //   of the sentence start pronun.
    if ( sent_start_index >= 0 )
    {
        next_word_chain_elem = DecodingHypothesis::word_chain_elem_pool.getElem( 
                                                    vocabulary->sent_start_index , NULL , 0) ;
        curr_word_hyps[sent_start_index][0]->extendWord( 0.0 , next_word_chain_elem ) ;
    
        // Extend to the successor states of the initial state of the sentence start pronun.
        lexicon->getSuccessorInfo( sent_start_index , 0 , &n_sucs , &sucs , &suc_log_trans_probs ) ;
        for ( int s=0 ; s<n_sucs ; s++ )
        {
            new_score = curr_word_hyps[sent_start_index][0]->score + suc_log_trans_probs[s] ;
            curr_word_hyps[sent_start_index][sucs[s]]->extendState( 
                                        curr_word_hyps[sent_start_index][0] , new_score ) ;
        }

        curr_word_hyps[sent_start_index][0]->deactivate() ;
    }
    else
    {
        // There is no sentence start pronunciation defined.
        // Initialise hypotheses for the initial states of all models 
        //   in the lexicon (except the sent end word if defined).
        for ( int w=0 ; w<vocabulary->n_words ; w++ )
        {
            next_word_chain_elem = DecodingHypothesis::word_chain_elem_pool.getElem( w, NULL, 0 ) ;
            
            n_pronuns = lexicon->lex_info->vocab_to_lex_map[w].n_pronuns ;
            pronuns = lexicon->lex_info->vocab_to_lex_map[w].pronuns ;
            for ( int p=0 ; p<n_pronuns ; p++ )
            {
                if ( pronuns[p] == sent_end_index )
                    continue ;
                    
                curr_word_hyps[pronuns[p]][0]->extendWord( 0.0 , next_word_chain_elem ) ;
            }
            
            if ( next_word_chain_elem->n_connected <= 0 )
                DecodingHypothesis::word_chain_elem_pool.returnElem( next_word_chain_elem ) ;
        }

        // Now go through all models and extend the intial state hypotheses.
        for ( int m=0 ; m<lexicon->n_models ; m++ )
        {
            if ( m == sent_end_index )
                continue ;

            lexicon->getSuccessorInfo( m , 0 , &n_sucs , &sucs , &suc_log_trans_probs ) ;
            for ( int s=0 ; s<n_sucs ; s++ )
            {
                new_score = curr_word_hyps[m][0]->score + suc_log_trans_probs[s] ;
                curr_word_hyps[m][sucs[s]]->extendState( curr_word_hyps[m][0] , new_score ) ;
            }

            curr_word_hyps[m][0]->deactivate() ;
        }
    }
}


}
