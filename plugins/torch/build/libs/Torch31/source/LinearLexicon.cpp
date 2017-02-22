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
#include "LinearLexicon.h"
#include "log_add.h"
#include "string_stuff.h"


namespace Torch {


LinearLexicon::LinearLexicon( SpeechHMM *speech_hmm , PhoneModels *phone_models_ )
{
    if ( speech_hmm == NULL )
        error("LinearLexicon::LinearLexicon(2) - speech_hmm is NULL\n") ;
   
    createLinearLexicon( speech_hmm->lexicon , phone_models_ ) ;
}


LinearLexicon::LinearLexicon( LexiconInfo *lex_info_ , PhoneModels *phone_models_ )
{
    createLinearLexicon( lex_info_ , phone_models_ ) ;
}


void LinearLexicon::createLinearLexicon( LexiconInfo *lex_info_ , PhoneModels *phone_models_ )
{
    DecodingHMM **temp_models=NULL ;
    int max_n_phones=100 , n_phones ;
    short n_sucs , *sucs ;
    real *sucs_log_trans_probs ;
	
    if ( lex_info_ == NULL )
        error("LinearLexicon::createLinearLexicon - lex_info_ is NULL\n") ;
    if ( phone_models_ == NULL )
        error("LinearLexicon::createLinearLexicon - phone_models_ is NULL\n") ;

    lex_info = lex_info_ ;
    phone_models = phone_models_ ;
    
    if ( (lex_info->sent_start_index >=0) && 
         (lex_info->sent_start_index == lex_info->sent_end_index) )
        error("LinearLexicon::createLinearLexicon - sent start & end cannot be the same word\n") ;
        
    n_models = lex_info->n_entries ;
    models = (DecodingHMM **)Allocator::sysAlloc( n_models * sizeof(DecodingHMM *) ) ; 
    total_states = 0 ;

    // Allocate memory to hold the temporary list of models that gets assembled for each word
    temp_models = (DecodingHMM **)Allocator::sysAlloc( max_n_phones * sizeof(DecodingHMM *) ) ;
    for ( int i=0 ; i<n_models ; i++ )
    {
        n_phones = lex_info->entries[i].n_phones ;
        
        // Check that the temp array that is used to assemble the list of phone models is
        //   big enough.
        if ( n_phones > max_n_phones )
        {
            // Realloc some more memory for our temp array
            max_n_phones = n_phones + 1 ;
            temp_models = (DecodingHMM **)Allocator::sysRealloc( temp_models ,
                                                    max_n_phones * sizeof(DecodingHMM *) ) ;
        }
        
        for ( int j=0 ; j<n_phones ; j++ )
            temp_models[j] = phone_models->models[lex_info->entries[i].phones[j]] ;

        if ( (lex_info->phone_info->pause_index >= 0) &&
             (lex_info->entries[i].phones[n_phones-1] != lex_info->phone_info->pause_index) &&
             (lex_info->entries[i].phones[n_phones-1] != lex_info->phone_info->sil_index) )
        {
            // If there is a pause phone defined, and the word does not already have a pause
            //   or silence phone at the end, then add the pause model to the end of the list.
            temp_models[n_phones] = phone_models->models[phone_models->phone_info->pause_index] ;
            n_phones++ ;
        }
        
        models[i] = new DecodingHMM( n_phones , temp_models ) ;

        // Check that there is not an initial to final state transition
        n_sucs = models[i]->states[0]->n_successors ;
        sucs = models[i]->states[0]->successor_states ;
        sucs_log_trans_probs = models[i]->states[0]->suc_log_trans_probs ;
        if ( sucs[n_sucs-1] == (models[i]->n_states-1) )
            error("LinearLexicon::createLinearLexicon - initial-final transition in word %d\n",i) ;
        
        // Scale the transition probs from the initial state of the new model
        //   using the prior.
        for ( int j=0 ; j<n_sucs ; j++ )
            sucs_log_trans_probs[j] += lex_info->entries[i].log_prior ;
    }
 
    // Calculate the total number of states in all pronunciation models
    for ( int i=0 ; i<n_models ; i++ )
        total_states += models[i]->n_states ;

    if ( temp_models != NULL )
        free( temp_models ) ;
}


LinearLexicon::~LinearLexicon()
{
    if ( models != NULL )
    {
        for ( int i=0 ; i<n_models ; i++ )
            delete models[i] ;
        free( models ) ;
    }
}


real LinearLexicon::calcEmissionProb( int word , int state )
{
#ifdef DEBUG
    if ( (word < 0) || (word >= n_models) )
        error("LinearLexicon::calcEmissionProb - word out of range\n") ;
#endif

    return phone_models->calcEmissionProb( models[word]->states[state]->emission_prob_vec_index ,
                                           models[word]->states[state]->distribution ) ;
}


int LinearLexicon::nStatesInModel( int model )
{
#ifdef DEBUG
    if ( (model < 0) || (model >= n_models) )
        error("LinearLexicon::nStatesInModel - model out of range\n") ;
#endif

    return models[model]->n_states ;
}


void LinearLexicon::getSuccessorInfo( int word , int state , short *n_sucs , 
                                      short **sucs , real **log_trans_probs )
{
#ifdef DEBUG
    if ( (word < 0) || (word >= n_models) )
        error("LinearLexicon::getSuccessorInfo - word out of range\n") ;
    if ( (state < 0) || (state >= models[word]->n_states) )
        error("LinearLexicon::getSuccessorInfo - state out of range\n") ;
#endif    
    *n_sucs = models[word]->states[state]->n_successors ;
    *sucs = models[word]->states[state]->successor_states ;
    *log_trans_probs = models[word]->states[state]->suc_log_trans_probs ;
}


#ifdef DEBUG
void LinearLexicon::outputText()
{
	printf("LinearLexicon: Number of models = %d\n" , n_models) ;
	for ( int i=0 ; i<n_models ; i++ )
	{
		printf( "LinearLexicon: SSI_Model %d, %s has %d states\n" , i , 
                lex_info->vocabulary->getWord( lex_info->entries[i].vocab_index ) , 
                models[i]->n_states ) ;
        models[i]->outputText() ;
	}
}
#endif



}
