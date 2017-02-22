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

#ifndef SPEECHMLP_INC
#define SPEECHMLP_INC

#include "general.h"
#include "Sequence.h"
#include "ConnectedMachine.h"
#include "Linear.h"
#include "GradientMachine.h"


namespace Torch {

typedef enum
{
    MLP_TANH=0 ,
    MLP_SIGMOID ,
    MLP_SOFTMAX ,
    MLP_LOGSOFTMAX ,
    MLP_NONE
} MLPNonLinTransformType ;

/**
   @author Darren Moore (moore@idiap.ch)
*/
class SpeechMLP : public ConnectedMachine
{
  public:
    /// the first #Linear# layer
    Linear *hidden_layer_lin ;
    /// the first #Tanh# or #Sigmoid# layer
    GradientMachine *hidden_layer_nonlin ;
    /// the second #Linear# layer
    Linear *output_layer_lin ;
    /// the optional second #Softmax#, #Sigmoid# or #Tanh# layer
    GradientMachine *output_layer_nonlin ;
    
    MLPNonLinTransformType hidden_nl_transf ;
    MLPNonLinTransformType output_nl_transf ;
    
    /// the number of hidden units
    int n_mlp_hidden ;
    /// the number of inputs
    int n_mlp_inputs ;
    /// the number of outputs
    int n_mlp_outputs ;

    int n_features ;
    int n_cw_vecs ;
    real *context_window ;
    Sequence *mlp_input_seq ;

    real *ftr_norms_means ;
    real *orig_ftr_norms_means ;
    real *ftr_norms_inv_stddevs ;
    real *orig_ftr_norms_inv_stddevs ;
    real *ftr_norms_vars ;
    real *orig_ftr_norms_vars ;
    bool online_norm ;
    real alpha_m ;
    real alpha_v ;
    bool lna8_outputs ;
    
    /// Constructor for the SpeechMLP class - reads a Quicknet MLPW format file
    SpeechMLP( char *quicknet_mlpw_filename , int n_cw_vecs_=9 , 
               char *quicknet_norms_filename=NULL , bool online_norm=false ,
               real alpha_m_=0.005 , real alpha_v_=0.005 , bool lna8_outputs_=false ) ; 
    virtual ~SpeechMLP();

    void createMLP( int n_inputs_ , int n_hidden_ , int n_outputs_ ,
                    MLPNonLinTransformType hidden_nl_transf_ ,
                    MLPNonLinTransformType output_nl_transf_ ) ;
    
    void convertWeightsToReal( int n_weights , int bytes_per_weight , bool weights_are_float , 
                               int exponent , void *inputs , real *outputs ) ;

    void feedForwardOneFrame( real *features , real *mlp_outputs ) ;
    void feedForward( int n_frames_ , real **features , int *n_out_frames , real ***mlp_outputs ) ;
    int initContextWindow( real **frames ) ;
    void loadFeatureNorms( char *norms_filename ) ;
    void normaliseFeatures( real *features ) ;

#ifdef DEBUG
    void outputText() ;
#endif
} ;


}

#endif
