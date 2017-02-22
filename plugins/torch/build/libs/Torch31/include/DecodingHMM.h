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

#ifndef DECODINGHMM_INC
#define DECODINGHMM_INC

#include "general.h"
#include "Distribution.h"
#include "HMM.h"

namespace Torch {


/* This structure is used to store all HMM state information 
    required for decoding. Each state contains a list of successor 
    states and associated transition probabilities.

    @author Darren Moore (moore@idiap.ch)
*/
typedef struct
{
    Distribution *distribution ;
    short emission_prob_vec_index ;
    short n_successors ;
    short *successor_states ;
    real *suc_log_trans_probs ;
} DecodingHMMState ;


/** This class contains all HMM information required for decoding.  Most information
    is embedded in the states themselves (DecodingHMMState structures).  The DecodingHMM
    can be created a number of ways to facilitate compatibility with the Torch HMM class
    and to allow easy concatenation of models (eg. when constructing word models from
    phoneme models).
    
    @author Darren Moore (moore@idiap.ch)
*/
class DecodingHMM
{
public:
    short n_states ;
    DecodingHMMState **states ;

    /* Constructors / destructor */
    DecodingHMM() ;

    /// Converts a Torch HMM instance to a DecodingHMM representation.  The Torch HMM
    ///   class contains a lot of member variables used in training that are not
    ///   required when decoding, as well as a full transition matrix.
    DecodingHMM( HMM *orig_model , short *emis_prob_vec_indices ) ;

    /// Concatenates all elements in the array of DecodingHMM instances into a single
    ///   DecodingHMM.  Component models that have initial-final state transitions are ok.
    DecodingHMM( int n_models , DecodingHMM **models ) ;

    /// Creates a DecodingHMM using the distributions in 'states_' and the log
    ///   transition probabilties in 'log_trans_probs_'.
    DecodingHMM( short n_states_ , Distribution **states_ , real **log_trans_probs_ ,
                 short *emis_prob_vec_indices ) ; 
    virtual ~DecodingHMM() ;

    /* Methods */

    /// Internal function to merge component models into a big HMM
    void mergeModels( int n_models , DecodingHMM **models ) ;

    /// Configures successor state information for the state denoted by 'state'.
    void setupSuccessorStates( DecodingHMMState *state , short n_successors_ , 
                               short *sucessor_states_ , real *log_trans_probs_ ) ;

    /// Initialises the state denoted by 'state' with a Distribution and
    ///   optionally an index into the vector of emission probabilities.
    void initState( DecodingHMMState *state , Distribution *distribution_ , 
                    short emission_prob_vec_index_=-1 ) ;
                               
#ifdef DEBUG
    void outputText() ;
#endif
} ;
    

}


#endif
