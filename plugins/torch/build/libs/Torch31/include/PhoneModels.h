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

#ifndef PHONEMODELS_INC
#define PHONEMODELS_INC

#include "general.h"
#include "PhoneInfo.h"
#include "DecodingHMM.h"
#include "SpeechMLP.h"
#include "SpeechHMM.h"

namespace Torch {

/**
   @author Darren Moore (moore@idiap.ch)
*/
class PhoneModels
{
public:
    PhoneInfo *phone_info ;
    int n_models ;
    DecodingHMM **models ;
    int n_features ;
    int n_emission_probs ;
    bool input_vecs_are_features ;
    real *curr_input_vec ;
    real *curr_emission_probs ;
    real *log_phone_priors ;
    real log_phone_del_pen ;
    real acoustic_scale_factor ;
    real log_emission_prob_floor ;
    bool apply_pause_del_pen ;
    SpeechMLP *mlp ;
    
    /* Constructors / destructor ***/
    PhoneModels( SpeechHMM *speech_hmm ) ;
    PhoneModels( PhoneInfo *phone_info_ , char *phone_models_fname ,
                 bool input_vecs_are_features_ , real phone_deletion_penalty=1.0 , 
                 bool apply_pause_deletion_penalty=false , char *phone_priors_fname=NULL , 
                 char *neural_net_fname=NULL , int nn_cw_size=9 , char *norms_fname=NULL , 
                 bool online_norm_=false , real alpha_m_=0.005 , real alpha_v_=0.005 ) ;
    ~PhoneModels() ;

    /* Methods */
    DecodingHMM *getModel( int index ) ;
    void setInputVector( real *input_vec ) ;
    real calcEmissionProb( int prob_vec_index , Distribution *dist ) ;
    void readModelsFromHTK( FILE *models_fd ) ;
    void readModelsFromNoway( FILE *models_fd ) ;
    void readPhonePriors( char *phone_priors_filename ) ;
    void calcEmissionProbsOneFrame( real *features ) ;
    void calcEmissionProbsAllFrames( int n_frames , real **features , real ***emission_probs ) ;

#ifdef DEBUG
    void outputText() ;
#endif
};



}


#endif
    
    
