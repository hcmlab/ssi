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
#include "PhoneModels.h"
#include "DiagonalGMM.h"
#include "log_add.h"
#include "string_stuff.h"
#include "SpeechMLP.h"
#include "SpeechMLPDistr.h"


namespace Torch {



PhoneModels::PhoneModels( SpeechHMM *speech_hmm )
{
    short *emis_prob_indices ;
    
    if ( speech_hmm == NULL )
        error("PhoneModels::PhoneModels(2) - speech_hmm is NULL\n") ;

    phone_info = speech_hmm->lexicon->phone_info ;
    n_models = speech_hmm->n_models ;
    models = NULL ;
    n_features = 0 ;
    n_emission_probs = 0 ;
    input_vecs_are_features = true ;
    curr_input_vec = NULL ;
    curr_emission_probs = NULL ;
    log_phone_priors = NULL ;
    log_phone_del_pen = 0.0 ;
    acoustic_scale_factor = 1.0 ;
    log_emission_prob_floor = 0.0 ;
    apply_pause_del_pen = false ;
    mlp = NULL ;

    models = (DecodingHMM **)Allocator::sysAlloc( n_models * sizeof(DecodingHMM *) ) ;
    log_phone_priors = (real *)Allocator::sysAlloc( n_models * sizeof(real) ) ;
    for ( int i=0 ; i<n_models ; i++ )
    {
        emis_prob_indices = (short *)Allocator::sysAlloc( speech_hmm->models[i]->n_states *
                                                          sizeof(short) ) ;
        emis_prob_indices[0] = -1 ;
        emis_prob_indices[speech_hmm->models[i]->n_states - 1] = -1 ;
        for ( int j=1 ; j<(speech_hmm->models[i]->n_states - 1) ; j++ )
            emis_prob_indices[j] = n_emission_probs++ ;
        models[i] = new DecodingHMM( speech_hmm->models[i] , emis_prob_indices ) ;
        log_phone_priors[i] = 0.0 ;
        free( emis_prob_indices ) ;
    }
    
    // The number of inputs for the Distribution of each state should be the number of features
    if ( (n_features = models[0]->states[1]->distribution->n_inputs) <= 0 )
        error("PhoneModels::PhoneModels(2) - n_features <= 0\n") ;

    // Allocate memory to hold emission probs as they are calculated.
    curr_emission_probs = (real *)Allocator::sysAlloc( n_emission_probs * sizeof(real) ) ;
    for ( int i=0 ; i<n_emission_probs ; i++ )
        curr_emission_probs[i] = LOG_ZERO ;
}


PhoneModels::PhoneModels( PhoneInfo *phone_info_ , char *phone_models_fname ,
                          bool input_vecs_are_features_ , real phone_deletion_penalty , 
                          bool apply_pause_deletion_penalty , char *phone_priors_fname , 
                          char *neural_net_fname , int nn_cw_size , char *norms_fname , 
                          bool online_norm_ , real alpha_m_ , real alpha_v_ )
{
    FILE *models_fd ;
    char line[100] ;

    if ( phone_info_ == NULL )
        error("PhoneModels::PhoneModels - phone_info not defined\n") ;
    if ( (phone_models_fname == NULL) || (strcmp(phone_models_fname,"")==0) )
        error("PhoneModels::PhoneModels - phone_models_fname not defined\n") ;

    phone_info = phone_info_ ;
    n_models = 0 ;
    models = NULL ;
    n_features = 0 ;
    n_emission_probs = 0 ;
    input_vecs_are_features = input_vecs_are_features_ ;
    curr_input_vec = NULL ;
    curr_emission_probs = NULL ;
    log_phone_priors = NULL ;
    mlp = NULL ;

    acoustic_scale_factor = 1.0 ;
    log_emission_prob_floor = 0.0 ;   
   
    // Convert the phone deletion penalties to log domain.
    log_phone_del_pen = (real)log( phone_deletion_penalty ) ;
    apply_pause_del_pen = apply_pause_deletion_penalty ;
    
    // Open the phone models definition file.
    if ( (models_fd = fopen( phone_models_fname , "r" )) == NULL )
        error("PhoneModels::PhoneModels - error opening phone models file\n") ;

    // Read the first line of the file and use it to determine the file type.
    fgets( line , 100 , models_fd ) ;
    if ( strstr( line , "~o" ) )
    {
        // This is a HTK model definition file
        readModelsFromHTK( models_fd ) ;
    }
    else if ( strstr( line , "PHONE" ) ) 
    {
        // This is a NOWAY format phone models file
        readModelsFromNoway( models_fd ) ;
        readPhonePriors( phone_priors_fname ) ;

        if ( input_vecs_are_features == true )
        {
            // We have features as input - therefore we need a neural net to
            //   give us emission probabilities.
            mlp = new SpeechMLP ( neural_net_fname , nn_cw_size , norms_fname , 
                                  online_norm_ , alpha_m_ , alpha_v_ ) ;
            n_features = mlp->n_features ;
            if ( mlp->n_mlp_outputs != n_emission_probs )
                error("PhoneModels::PhoneModels - n_mlp_outputs not match n_emission_probs\n") ;
            
            // Now we need a Distribution that can be associated with each emitting
            //   state of each phone in our phoneset.
            for ( int i=0 ; i<n_models ; i++ )
            {
                for ( int j=1 ; j<(models[i]->n_states)-1 ; j++ )
                {
                     models[i]->states[j]->distribution = 
                        new SpeechMLPDistr( mlp , &curr_emission_probs , 
                              models[i]->states[j]->emission_prob_vec_index , log_phone_priors ) ;
                }
            }
        }
    }
    
    // Check that we were able to determine the input vector size, and/or the
    //   total number of emission probabilities.
    if ( (input_vecs_are_features==true) && (n_features<=0) )
        error("PhoneModels::PhoneModels - cannot have n_features <= 0\n") ;
    if ( n_emission_probs <= 0 )
        error("PhoneModels::PhoneModels - cannot have n_emission_probs <= 0\n") ;

    // Now prepare for the type of input vectors
    if ( input_vecs_are_features == true )
    {
        // Allocate memory to hold emission probs as they are calculated.
        curr_emission_probs = (real *)Allocator::sysAlloc( n_emission_probs * sizeof(real) ) ;
        for ( int i=0 ; i<n_emission_probs ; i++ )
            curr_emission_probs[i] = LOG_ZERO ;
    }
}
        
        
PhoneModels::~PhoneModels()
{
    if ( models != NULL )
    {
        for ( int i=0 ; i<n_models ; i++ )
            delete models[i] ;
        free( models ) ;
    }
    if ( input_vecs_are_features == true )
        free( curr_emission_probs ) ;
    if ( log_phone_priors != NULL )
        free( log_phone_priors ) ;
    if ( mlp != NULL )
        free( mlp ) ;
}


DecodingHMM *PhoneModels::getModel( int index )
{
    if ( (index >= n_models) || (index < 0) )
        error("PhoneModels::getModel - index out of range\n") ;
    return models[index] ;
}


void PhoneModels::setInputVector( real *input_vec )
{
    curr_input_vec = input_vec ;
    
    if ( input_vecs_are_features == true )
    {
        // The current emission prob values are now out of date - reset.
        for ( int j=0 ; j<n_emission_probs ; j++ )
            curr_emission_probs[j] = LOG_ZERO ;
    }
    else
    {
        curr_emission_probs = curr_input_vec ;
        
        if ( log_emission_prob_floor < 0.0 )
        {
            for ( int j=0 ; j<n_emission_probs ; j++ )
            {
                if ( curr_emission_probs[j] < log_emission_prob_floor )
                    curr_emission_probs[j] = LOG_ZERO / 2 ;
            }
        }
        
        // If we have prior probabilities, divide each emission prob by its
        //    prior to give a scaled likelihood (or subtract in log domain).
        if ( log_phone_priors != NULL )
        {
            for ( int j=0 ; j<n_emission_probs ; j++ )
                curr_emission_probs[j] -= log_phone_priors[j] ;
        }
    }
}


real PhoneModels::calcEmissionProb( int prob_vec_index , Distribution *dist )
{
    if ( prob_vec_index < 0 )
        error("PhoneModels::calcEmissionProb - prob_vec_index out of range\n") ;
        
    if ( input_vecs_are_features == true )
    {
#ifdef DEBUG
        if ( dist == NULL )
            error("PhoneModels::calcEmissionProb - distribution cannot be NULL\n") ;
#endif
        if ( curr_emission_probs[prob_vec_index] <= LOG_ZERO )
        {
            curr_emission_probs[prob_vec_index] = 
                   dist->frameLogProbability( 0 , curr_input_vec ) ;
        }
    }
    
    return curr_emission_probs[prob_vec_index] * acoustic_scale_factor ;
}


void PhoneModels::readModelsFromHTK( FILE *models_fd )
{
    // Loads a HTK definition file containing multiple HMM definitions.
    // Only supports limited format - each HMM MUST be exactly same format as 
    //    Fig 7.3 in HTK manual.  Therefore each state distribution can only 
    //    be a DiagonalGMM.
    
    char line[20000] , curr_model_name[100] , *value=NULL ;
    int n_mixtures=0 , temp , phone_index=-1 , read_state=0 , curr_mixture=0 ;
    real curr_mixture_weight=0.0 , **log_trans=NULL ;
    Distribution **states=NULL ;
    short curr_state=0 , curr_emis_prob_index=0 , *emis_prob_indices=NULL , n_states=0 ;
   
    // Allocate memory for the models
    models = (DecodingHMM **)Allocator::sysAlloc( phone_info->n_phones * sizeof(DecodingHMM *) ) ;

    // read the HMM model data from the HTK-format text file
    n_models = 0 ;
    curr_emis_prob_index = 0 ;
    read_state = 0 ;
    n_emission_probs = 0 ;

    while ( fgets( line , 20000 , models_fd ) != NULL )
    {
        if ( strstr(line,"~h") )
        {
            if ( read_state != 1 )
                error("PhoneModels::readModelsFromHTK - ~h out of order\n") ;
            if ( sscanf( line , "%*s \"%[^\"]" , curr_model_name ) != 1 )
                error("PhoneModels::readModelsFromHTK - error extracting phone name\n") ;
            if ( (phone_index = phone_info->getIndex( curr_model_name )) < 0 )
                error("PhoneModels::readModelsFromHTK - %s not in phone_info\n",curr_model_name) ;
            n_models++ ;
            n_states = 0 ;
            read_state = 2 ;
        }
        else
        {
            strtoupper( line ) ;
            if ( strstr( line , "<VECSIZE>" ) )
            {
                if ( read_state != 0 )
                    error("PhoneModels::readModelsFromHTK - <VECSIZE> out of order\n") ;
                if( sscanf( line , "%*s %d" , &n_features ) != 1 )
                    error("PhoneModels::readModelsFromHTK - error reading n_features\n") ;
                read_state = 1 ;
            }
            else if ( strstr( line , "<BEGINHMM>" ) )
            {
                if ( read_state != 2 )
                    error("PhoneModels::readModelsFromHTK - <BEGINHMM> out of order\n") ;
                read_state = 3 ;
            }
            else if ( strstr( line , "<NUMSTATES>" ) )
            {
                if ( read_state != 3 )
                    error("PhoneModels::readModelsFromHTK - <NUMSTATES> out of order\n") ;
                if ( sscanf( line , "%*s %hd" , &n_states ) != 1 )
                    error("PhoneModels::readModelsFromHTK - error extracting n_states\n") ;
                n_emission_probs += (n_states-2) ;
                emis_prob_indices = (short *)Allocator::sysAlloc( n_states * sizeof(short) ) ;
                emis_prob_indices[0] = -1 ;
                emis_prob_indices[n_states-1] = -1 ;
                for ( short j=1 ; j<(n_states-1) ; j++ )
                    emis_prob_indices[j] = curr_emis_prob_index++ ;

                // allocate memory for the array of Diagonal GMM's and the transitions
                states = (Distribution **)Allocator::sysAlloc( n_states * sizeof(Distribution *) ) ;
                log_trans = (real **)Allocator::sysAlloc( n_states * sizeof(real *) ) ;

                for ( short j=0 ; j<n_states ; j++ )
                {
                    states[j] = NULL ;
                    log_trans[j] = (real *)Allocator::sysAlloc( n_states * sizeof(real) ) ;
                }
                read_state = 4 ;
            }
            else if ( strstr(line,"<STATE>") )
            {
                if ( read_state != 4 )
                {
                    printf("%s %d %d\n",curr_model_name,curr_state,curr_mixture);
                    error("PhoneModels::readModelsFromHTK - <STATE> out of order\n%s\n",line) ;
                }

                if ( sscanf( line , "%*s %hd" , &curr_state ) != 1 )
                    error("PhoneModels::readModelsFromHTK - error reading curr_state\n") ;
                curr_state-- ;  // index from 0
                if ( (curr_state<1) || (curr_state>=(n_states-1)) )
                    error("PhoneModels::readModelsFromHTK - invalid curr_state\n") ;

                // There might be a <NUMMIXES> on the same line
                if ( strstr(line,"<NUMMIXES>") )
                {
                    if ( sscanf( line , "%*s %*d %*s %d" , &n_mixtures ) != 1 )
                        error("PhoneModels::readModelsFromHTK - error reading n_mixtures\n") ;
                    curr_mixture = -1 ;
                    read_state = 7 ;
                }
                else
                    read_state = 5 ;
            }
            else if ( strstr(line,"<NUMMIXES>") )
            {
                if ( read_state != 5 )
                    error("PhoneModels::readModelsFromHTK - <NUMMIXES> out of order\n") ;
                if ( sscanf( line , "%*s %d" , &n_mixtures ) != 1 )
                    error("PhoneModels::readModelsFromHTK - error reading n_mixtures (2)\n") ;
                curr_mixture = -1 ;
                read_state = 7 ;
            }
            else if ( strstr(line,"<MIXTURE>") )
            {
                if ( read_state != 7 )
                    error("PhoneModels::readModelsFromHTK - <MIXTURE> out of order\n%s\n",line) ;

                if ( states[curr_state] == NULL )
                    states[curr_state] = new DiagonalGMM( n_features, n_mixtures ) ;

#ifdef USE_DOUBLE
                if ( sscanf( line , "%*s %*d %lf" , &curr_mixture_weight ) != 1 )
#else
                if ( sscanf( line , "%*s %*d %f" , &curr_mixture_weight ) != 1 )
#endif
                    error("PhoneModels::readModelsFromHTK - error reading curr_mix_wt\n") ;

                curr_mixture++ ;
                if ( (curr_mixture<0) || (curr_mixture>=n_mixtures) )
                    error("PhoneModels::readModelsFromHTK - invalid curr_mixture\n") ;
                if ( curr_mixture_weight == 0.0 )
                    ((DiagonalGMM *)states[curr_state])->log_weights[curr_mixture] = LOG_ZERO ;
                else
                {
                    ((DiagonalGMM *)states[curr_state])->log_weights[curr_mixture] = 
                        log(curr_mixture_weight) ;
                }
                read_state = 8 ;
            }
            else if ( strstr(line,"<MEAN>") )
            {
                if ( (read_state != 5) && (read_state != 8) )
                    error("PhoneModels::readModelsFromHTK - <MEAN> out of order\n") ;

                // Check that the number of means matches the number of features
                if ( sscanf( line , "%*s %d" , &temp ) != 1 )
                    error("PhoneModels::readModelsFromHTK - error reading number of means\n") ;
                if ( temp != n_features )
                {
                    error("PhoneModels::readModelsFromHTK - <MEAN> %d not match n_features=%d\n", 
                            temp , n_features ) ;
                }

                // If the distribution has not already been created, then create it.
                if ( states[curr_state] == NULL )
                    states[curr_state] = new DiagonalGMM( n_features, n_mixtures, NULL ) ;

                if ( (fgets(line,20000,models_fd)) == NULL )
                    error("PhoneModels::loadHtkModels - error reading <MEAN> values\n") ;

                value = strtok( line , " " ) ;
                for ( short j=0 ; j<n_features ; j++ )
                {
                    ((DiagonalGMM *)states[curr_state])->means[curr_mixture][j]=(real)atof(value) ;
                    value = strtok( NULL , " " ) ;
                }

                read_state++ ;
            }
            else if ( strstr(line,"<VARIANCE>") )
            {
                if ( (read_state != 6) && (read_state != 9) )
                    error("PhoneModels::readModelsFromHTK - <VARIANCE> out of order\n") ;

                // Check that the number of variances matches the number of features
                if ( sscanf( line , "%*s %d" , &temp ) != 1 )
                    error("PhoneModels::readModelsFromHTK - error reading number of variances\n") ;

                if ( temp != n_features )
                {
                    error("PhoneModels::readModelsFromHTK - <VARIANCE> %d != n_features=%d\n" , 
                          temp , n_features ) ;
                }

                // If the distribution has not already been created, then create it.
                if ( states[curr_state] == NULL )
                    states[curr_state] = new DiagonalGMM( n_features, n_mixtures, NULL ) ;

                if ( (fgets(line,20000,models_fd)) == NULL )
                    error("PhoneModels::loadHtkModels - error reading <VARIANCE> values\n") ;

                value = strtok( line , " " ) ;
                ((DiagonalGMM *)states[curr_state])->sum_log_var_plus_n_obs_log_2_pi[curr_mixture]= 
                    n_features * LOG_2_PI ;
                for ( short j=0 ; j<n_features ; j++ )
                {
                    ((DiagonalGMM *)states[curr_state])->var[curr_mixture][j] = (real)atof(value) ;
                    ((DiagonalGMM *)states[curr_state])->minus_half_over_var[curr_mixture][j] = 
                        -0.5 / (real)atof(value) ;
                    ((DiagonalGMM *)states[curr_state])->sum_log_var_plus_n_obs_log_2_pi[curr_mixture] 
                        += log( (real)atof(value) ) ;
                    value = strtok( NULL , " " ) ;
                }

                ((DiagonalGMM *)states[curr_state])->sum_log_var_plus_n_obs_log_2_pi[curr_mixture] 
                    *= -0.5 ;
                if ( read_state == 6 )
                    read_state = 4 ;
                else if ( read_state == 9 )
                {
                    if ( curr_mixture == (n_mixtures-1) )
                        read_state = 4 ;
                    else
                        read_state = 7 ;
                }
            }
            else if ( strstr(line,"<TRANSP>") )
            {
                if ( read_state != 4 )
                    error("PhoneModels::readModelsFromHTK - <TRANSP> out of order\n") ;

                if ( curr_state != (n_states-2) )
                    error("PhoneModels::readModelsFromHTK - not all states encountered\n") ;

                for ( short j=0 ; j<n_states ; j++ )
                {
                    if ( (fgets(line,20000,models_fd)) == NULL )
                        error("PhoneModels::readModelsFromHTK - error reading <TRANSP> values\n") ;

                    value = strtok( line , " " ) ;
                    for ( short k=0 ; k<n_states ; k++ )
                    {
                        log_trans[j][k] = (real)atof(value) ;
                        if ( log_trans[j][k] == 0.0 )
                            log_trans[j][k] = LOG_ZERO ;
                        else
                        {
                            log_trans[j][k] = log( log_trans[j][k] ) ;

                            // If this is a transition to the final state, apply the
                            //   phone deletion penalty.
                            if ( k==(n_states-1) )
                            {
                                if ( (phone_index != phone_info->pause_index) || 
                                        ( (phone_index == phone_info->pause_index) && 
                                          (apply_pause_del_pen == true) ) )
                                {
                                    log_trans[j][k] += log_phone_del_pen ;
                                }
                            }
                        }
                        value = strtok( NULL , " " ) ;
                    }
                }

                // create the DecodingHMM
                models[phone_index] = new DecodingHMM( n_states , states , log_trans ,
                        emis_prob_indices ) ;
                for ( short j=0 ; j<n_states ; j++ )
                    free( log_trans[j] ) ;
                free( log_trans ) ;
                free( emis_prob_indices ) ;

                read_state = 10 ;
            }
            else if ( strstr(line,"<ENDHMM>") )
            {
                if ( read_state != 10 )
                    error("PhoneModels::readModelsFromHTK - <ENDHMM> out of order\n") ;
                phone_index = -1 ;
                read_state = 1 ;
            }
            else if ( strstr( line , "<GCONST>" ) )
            {
                // Ignore
            }
            else if ( strstr( line , "<STREAMINFO>" ) )
            {
                // Ignore
            }
            else
                error("PhoneModels::readModelsFromHTK - unrecognised line\n%s\n",line) ;
        }
    }

    if ( n_models != phone_info->n_phones )
        error("PhoneModels::readModelsFromHTK - n_models n_phones mismatch\n") ;
    fclose( models_fd ) ;
}


void PhoneModels::readModelsFromNoway( FILE *models_fd )
{
    // The model definitions in the input file must be in the same order
    //   as the phone name file listing.
    char line[20000] , phone_name[1000] ;
    int phone_id=0 , phone_index=-1 ;
    char *num=NULL ;
    short n_states=0 , n_sucs , sucs[100] ,  temp_suc , state_id , prob_index=0 ;
    real trans[100] , temp_tran ;
    
    // Read the number of models, and check against the expected number.
    fgets( line , 20000 , models_fd ) ;
    if ( sscanf( line , "%d" , &n_models ) != 1 )
        error("PhoneModels::readModelsFromNoway - error reading n_models\n") ;
    if ( n_models != phone_info->n_phones )
        error("PhoneModels::readModelsFromNoway - n_models n_phones mismatch\n") ;
        
    // Allocate memory for the phone models and phone names
    models = (DecodingHMM **)Allocator::sysAlloc( n_models * sizeof(DecodingHMM *) ) ;
    for ( int i=0 ; i<n_models ; i++ )
        models[i] = NULL ;
    
    for ( int i=0 ; i<n_models ; i++ )
    {
        // Read and process the line containing: <id> <num states> <label>
        if ( fgets( line , 20000 , models_fd ) == NULL )
            error("PhoneModels::readModelsFromNoway - error reading phone %d label line\n",i) ;

        if ( sscanf( line , "%d %hd %s" , &phone_id , &n_states , phone_name ) != 3 )
            error("PhoneModels::readModelsFromNoway - error interpret phone %d label line\n",i) ;

        if ( (phone_index = phone_info->getIndex( phone_name )) < 0 )
            error("PhoneModels::readModelsFromNoway - %s not in phone_info\n",phone_name) ;
            
        if ( models[phone_index] != NULL )
            error("PhoneModels::readModelsFromNoway - phone %d already present\n",phone_index) ;
            
        models[phone_index] = new DecodingHMM() ;
        models[phone_index]->n_states = n_states ;
        models[phone_index]->states = (DecodingHMMState **)Allocator::sysAlloc( 
                                             n_states * sizeof(DecodingHMMState *) ) ;
        
        // Read and process the line mapping each state to an element in the acoustic
        //   probability vector.
        if ( fgets( line , 20000 , models_fd ) == NULL )
            error("PhoneModels::readModelsFromNoway - error reading phone %d label line\n",i) ;

        if ( (int)atoi( strtok( line , " " ) ) != -1 )
            error("PhoneModels::readModelsFromNoway - expect -1 on prob index mapping line\n",i) ;
        if ( (int)atoi( strtok( NULL , " " ) ) != -2 )
            error("PhoneModels::readModelsFromNoway - expect -2 on prob index mapping line\n",i) ;

        models[phone_index]->states[0] = (DecodingHMMState *)Allocator::sysAlloc( 
                                                                    sizeof(DecodingHMMState) ) ;
        models[phone_index]->initState( models[phone_index]->states[0] , NULL , -1 ) ;
        models[phone_index]->states[n_states-1] = 
                        (DecodingHMMState *)Allocator::sysAlloc( sizeof(DecodingHMMState) ) ;
        models[phone_index]->initState( models[phone_index]->states[n_states-1] , NULL , -1 ) ;
        
        for ( int j=0 ; j<(n_states-2) ; j++ )
        {
            char* cindex = strtok( NULL , " " );
            if (!cindex)
                error("PhoneModels::readModelsFromNoway - prob index line not enough indices\n") ;
            prob_index = (int)atoi(cindex);

            if ( prob_index >= n_emission_probs )
                n_emission_probs = prob_index + 1 ;

            models[phone_index]->states[j+1] = 
                (DecodingHMMState *)Allocator::sysAlloc( sizeof(DecodingHMMState) ) ;
            models[phone_index]->initState( models[phone_index]->states[j+1] , 
                                            NULL , prob_index ) ;
        }

        // Read and process the line for each state that defines the transitions
        //   to other states.
        for ( int j=0 ; j<n_states ; j++ )
        {
            if ( fgets( line , 20000 , models_fd ) == NULL )
                error("PhoneModels::readModelsFromNoway - error reading state transitions line\n") ;

            if ( sscanf( line , "%hd %hd" , &state_id , &n_sucs ) != 2 )
                error("PhoneModels::readModelsFromNoway - error sscanf state transitions line\n") ;

            if ( state_id != j )
                error("PhoneModels::readModelsFromNoway - state id has unexpected value\n") ;
            
            // Get past the state id & the number of successor states.
            strtok( line , " " ) ;
            strtok( NULL , " " ) ;
            
            // Read and process each of the successor states to the current state.
            for ( int k=0 ; k<n_sucs ; k++ )
            {
                if ( (num = strtok( NULL , " " )) == NULL )
                    error("PhoneModels::readModelsFromNoway - error reading successor state\n") ;
                
                sucs[k] = (short)atoi( num ) ;
                if ( sucs[k] == 1 )
                    sucs[k] = n_states-1 ;
                else if ( sucs[k] > 1 )
                    sucs[k] -= 1 ;
                
                if ( (num = strtok( NULL , " " )) == NULL )
                    error("PhoneModels::readModelsFromNoway - error reading transition prob\n") ;
                
                trans[k] = (real)log( atof( num ) ) ;
                
                // If this is a transition to the final state of the model, apply
                //   the phone deletion penalty.
                if ( sucs[k] == (n_states-1) )
                {
                    if ( (phone_index != phone_info->pause_index) || 
                         ( (phone_index == phone_info->pause_index) && 
                           (apply_pause_del_pen == true) ) )
                    {
                        trans[k] += log_phone_del_pen ;
                    }
                }
            }
            
            // Make sure that the sucs list is sorted according to successor state index
            for ( int k=0 ; k<(n_sucs-1) ; k++ )
            {
                for ( int l=(k+1) ; l<n_sucs ; l++ )
                {
                    if ( sucs[l] < sucs[k] )
                    {
                        temp_suc = sucs[l] ;
                        sucs[l] = sucs[k] ;
                        sucs[k] = temp_suc ;
                        temp_tran = trans[l] ;
                        trans[l] = trans[k] ;
                        trans[k] = temp_tran ;
                    }
                }
            }
                    
            // Setup the successor states in the DecodingHMMState instance.
            if ( state_id == 1 )
            {
                // This is actually the final state.  Make sure that it has no
                //   successors and add it at the end of our list of states.
                if ( n_sucs != 0 )
                    error("PhoneModels::readModelsFromNoway - final state does not have 0 sucs\n") ;
                models[phone_index]->setupSuccessorStates( 
                            models[phone_index]->states[n_states-1] , n_sucs , sucs , trans ) ;
            }
            else if ( state_id > 1 )
            {
                models[phone_index]->setupSuccessorStates( 
                            models[phone_index]->states[state_id-1] , n_sucs , sucs , trans ) ;
            }
            else if ( state_id == 0 )
            {
                models[phone_index]->setupSuccessorStates( 
                            models[phone_index]->states[0] , n_sucs , sucs , trans ) ;
            }
        }
    }

    fclose( models_fd ) ;
}


void PhoneModels::readPhonePriors( char *phone_priors_fname )
{
    FILE *priors_fd ;
    char line[1000] ;
    real prob ;
    int prior_cnt=0 ;
    
    if ( phone_priors_fname == NULL )
        return ;

    // Open the input file
    if ( (priors_fd = fopen( phone_priors_fname , "r" )) == NULL )
        error("PhoneModels::readPhonePriors - error opening priors file\n") ;

    // Allocate memory for the prior probabilities
    if ( n_emission_probs <= 0 )
        error("PhoneModels::readPhonePriors - must load phone models before priors\n") ;
    log_phone_priors = (real *)Allocator::sysAlloc( n_emission_probs * sizeof(real) ) ;
    
    while ( fgets( line , 1000 , priors_fd ) != NULL )
    {
#ifdef USE_DOUBLE
        if ( (sscanf(line,"%lf",&prob)!=1) || (line[0] == '#') || (line[0] == '\n') ||
             (line[0] == '\r') || (line[0] == ' ') || (line[0] == '\t') )
#else
        if ( (sscanf(line,"%f",&prob)!=1) || (line[0] == '#') || (line[0] == '\n') ||
             (line[0] == '\r') || (line[0] == ' ') || (line[0] == '\t') )
#endif
        {
            continue ;
        }

        if ( prior_cnt >= n_emission_probs )
            error("PhoneModels::readPhonePriors - too many priors (>n_emission_probs)\n") ;

        log_phone_priors[prior_cnt] = (real)log(prob) ;
        prior_cnt++ ;
    }

    if ( prior_cnt != n_emission_probs )
        error("PhoneModels::readPhonePriors - num priors and num emission probs mismatch\n") ;

    fclose( priors_fd ) ;
}


void PhoneModels::calcEmissionProbsOneFrame( real *features )
{
    // This calculates all emission probabilities for all emitting states
    //   of all phone models.
    // The results are placed in the 'curr_emission_probs' member array.
    
    DecodingHMM *curr_model ;

    if ( input_vecs_are_features == false )
        error("PhoneModels::calcEmissionProbsOneFrame - invalid invocation\n") ;
    
    for ( int i=0 ; i<n_models ; i++ )
    {
        curr_model = models[i] ;
        for ( int j=1 ; j<(curr_model->n_states-1) ; j++ )
        {
            if ( curr_emission_probs[curr_model->states[j]->emission_prob_vec_index] <= LOG_ZERO )
            {
                curr_model->states[j]->distribution->frameLogProbability( 0 , features ) ;
            }
        }
    }
}


void PhoneModels::calcEmissionProbsAllFrames( int n_frames , real **features , 
                                              real ***emission_probs )
{
    // For each feature vector in 'features', calculates the emission probabilities 
    //   of all emitting states of all phones.
    // Returns results in 'emission_probs'.

    int prob_vector_index = 0 ;
    
#ifdef DEBUG
    // Do a rudimentary check : that the number of features input to this function
    //   matches the n_observations member variable of the first emitting state 
    //   of the first phone model.
    if ( n_features != models[0]->states[1]->distribution->n_inputs )
        error("PhoneModels::calcEmissionProbsAllFrames - n_features does not match phone models\n") ;
#endif

    // allocate memory for the emission probabilities for all states for all frames
    *emission_probs = (real **)Allocator::sysAlloc( n_frames * sizeof(real *) ) ;
    for ( int t=0 ; t<n_frames ; t++ )
        (*emission_probs)[t] = (real *)Allocator::sysAlloc( n_emission_probs * sizeof(real) ) ;

    // do the calculations
    for ( int t=0 ; t<n_frames ; t++ )
    {
        prob_vector_index=0 ;
        for ( int phone=0 ; phone<n_models ; phone++ )
        {
            for ( int state=1 ; state<((models[phone]->n_states)-1) ; state++ )
            {
                (*emission_probs)[t][prob_vector_index++] =
                    (models[phone]->states[state]->distribution)->frameLogProbability(
                                                                       0 , features[t] ) ;
            }
        }

        if ( prob_vector_index != n_emission_probs )
            error("PhoneModels::calcEmissionProbsAllFrames - prob_vector_index incorrect\n") ;
    }
}


#ifdef DEBUG
void PhoneModels::outputText()
{
    printf("n_features = %d\n",n_features);
    printf("n_emission_probs = %d\n",n_emission_probs);
    printf("Number of models = %d\n***************************\n",n_models) ;
    for (int i=0;i<n_models;i++)
    {
        printf("%s\n",phone_info->phone_names[i]);
        models[i]->outputText() ;
        printf("\n\n\n*******************************\n\n\n") ;
    }
}
#endif




}
