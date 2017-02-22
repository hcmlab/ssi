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

#include "SpeechMLPDistr.h"


namespace Torch {


SpeechMLPDistr::SpeechMLPDistr( SpeechMLP *mlp_ , real **outputs_ , int prob_vec_index_ , 
                                real *log_priors_ ) : Distribution(0,0)
{
#ifdef DEBUG
    if ( mlp_ == NULL )
        error("SpeechMLPDistr::SpeechMLPDistr - mlp_ is NULL\n") ;
    if ( outputs_ == NULL )
        error("SpeechMLPDistr::SpeechMLPDistr - outputs_ is NULL\n") ;
    if ( (prob_vec_index_ < 0) || (prob_vec_index_ >= mlp_->n_mlp_outputs) )
        error("SpeechMLPDistr::SpeechMLPDistr - prob_vec_index out of range\n") ;
#endif
    mlp = mlp_ ;
    mlp_outputs = outputs_ ;
    prob_vec_index = prob_vec_index_ ;
    log_priors = log_priors_ ;
}


SpeechMLPDistr::~SpeechMLPDistr()
{
}


real SpeechMLPDistr::frameLogProbability( int t , real *f_inputs )
{
    mlp->feedForwardOneFrame( f_inputs , *mlp_outputs ) ;
//for ( int i=0 ; i<mlp->n_mlp_outputs ; i++ )
//    printf("%6.3f ",(*mlp_outputs)[i]);
//printf("\n\n");

    if ( log_priors != NULL )
    {
        for ( int i=0 ; i<mlp->n_mlp_outputs ; i++ )
            (*mlp_outputs)[i] -= log_priors[i] ;
    }

    return (*mlp_outputs)[prob_vec_index] ;
}
    

}
