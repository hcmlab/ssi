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
#include "DecodingHMM.h"
#include "log_add.h"


namespace Torch {


DecodingHMM::DecodingHMM()
{
    n_states = 0 ;
    states = NULL ;
}


DecodingHMM::DecodingHMM( HMM *orig_model , short *emis_prob_vec_indices )
{
    // We just want to extract only the bits of the 'HMM' instance
    //   that are required for decoding.
    // We also want to associate a list of possible predecessor states
    //   with each state, as well as transition probabilities for each
    //   predecessor.
    // This will replace the transition 2D-array used in HMM, which is
    //   general but not optimal for the varieties of HMM's used for speech
    //   recognition.
    
    real *log_trans ;
    short *neighbour_states ;
    short n_neighbours=0 ;
    
    if ( orig_model == NULL )
        error("DecodingHMM::DecodingHMM - original HMM NULL\n") ;

    log_trans = (real *)Allocator::sysAlloc( 1000 * sizeof(real) ) ;
    neighbour_states = (short *)Allocator::sysAlloc( 1000 * sizeof(short) ) ;
    n_states = (short)orig_model->n_states ;
    
    // Allocate memory to hold the states
    states = (DecodingHMMState **)Allocator::sysAlloc( n_states * sizeof(DecodingHMMState *) ) ;

    // Create each state in turn.
    for ( short i=0 ; i<n_states ; i++ )
    {
        states[i] = (DecodingHMMState *)Allocator::sysAlloc( sizeof(DecodingHMMState) ) ;
        initState( states[i] , orig_model->states[i] , emis_prob_vec_indices[i] ) ;
    }

    // Now go through the log_transitions array in the HMM instance
    //   and extract only the non-zero transitions FROM this state.
    for ( short from=0 ; from<n_states ; from++ )
    {
        n_neighbours = 0 ;
        for ( short to=0 ; to<n_states ; to++ )
        {
            if ( orig_model->log_transitions[to][from] > LOG_ZERO )
            {
                log_trans[n_neighbours] = orig_model->log_transitions[to][from] ;
                neighbour_states[n_neighbours++] = to ;
            }

        }
        setupSuccessorStates( states[from] , n_neighbours , neighbour_states , log_trans ) ;
    }

    free( log_trans ) ;
    free( neighbour_states ) ;
}


DecodingHMM::DecodingHMM( int n_models , DecodingHMM **models )
{
    // This will take a list of smaller models and concatenate them
    //   into a single model.
    // Typically used to form a word-level model from a collection
    //   of phoneme models.
    mergeModels( n_models , models ) ;
}


DecodingHMM::DecodingHMM( short n_states_ , Distribution **states_ , real **log_trans_probs_ ,
                          short *emis_prob_vec_indices )
{
    real *log_trans ;
    short *neighbour_states , n_neighbours=0 ;
    
    n_states = n_states_ ;
    
    log_trans = (real *)Allocator::sysAlloc( 1000 * sizeof(real) ) ;
    neighbour_states = (short *)Allocator::sysAlloc( 1000 * sizeof(short) ) ;
    
    // Allocate memory to hold the states
    states = (DecodingHMMState **)Allocator::sysAlloc( n_states * sizeof(DecodingHMMState *) ) ;

    // Create each state in turn
    for ( short i=0 ; i<n_states ; i++ )
    {
        states[i] = (DecodingHMMState *)Allocator::sysAlloc( sizeof(DecodingHMMState) ) ;
        initState( states[i] , states_[i] , emis_prob_vec_indices[i] ) ;
    }

    // Now go through the log_transitions array in the HMM instance
    //   and extract only the non-zero transitions FROM this state.
    for ( short from=0 ; from<n_states ; from++ )
    {
        n_neighbours = 0 ;
        for ( short to=0 ; to<n_states ; to++ )
        {
            if ( log_trans_probs_[from][to] > LOG_ZERO )
            {
                log_trans[n_neighbours] = log_trans_probs_[from][to] ;
                neighbour_states[n_neighbours++] = to ;
            }
        }
        setupSuccessorStates( states[from] , n_neighbours , neighbour_states , log_trans ) ;
    }

    free( log_trans ) ;
    free( neighbour_states ) ;
}


DecodingHMM::~DecodingHMM()
{
    if ( states != NULL )
    {
        for ( short i=0 ; i<n_states ; i++ )
        {
            free( states[i]->successor_states ) ;
            free( states[i]->suc_log_trans_probs ) ;
            free( states[i] ) ;
        }
        free( states ) ;
    }
}


void DecodingHMM::mergeModels( int n_models , DecodingHMM **models )
{
    short index , prev_n_states , old_n_sucs ;
    real old_prob ;
    DecodingHMMState **new_states=NULL ;
    
    if ( n_models > 1 )
    {
        mergeModels( n_models-1 , models+1 ) ;

        // We now need to merge models[0] with the current contents of this instance
        //   into a model that has the initial state of model[0], emitting states of
        //   model[0], emitting states of this instance, final state of this instance.
        prev_n_states = n_states ;
        n_states += (models[0]->n_states - 2) ;
        new_states = (DecodingHMMState **)Allocator::sysAlloc( n_states * 
                                                               sizeof(DecodingHMMState *) ) ;

        // Create new state instances corresponding to each state in the model we have
        //   to merge with  (except the final state) and insert these at the start
        //   of the new array of states.
        index = 0 ;
        for ( short i=0 ; i<(models[0]->n_states-1) ; i++ )
        {
            new_states[index] = 
                        (DecodingHMMState *)Allocator::sysAlloc( sizeof(DecodingHMMState) ) ;
            initState( new_states[index] , models[0]->states[i]->distribution ,
                       models[0]->states[i]->emission_prob_vec_index ) ;
            index++ ;
        }

        // Copy all existing states except the initial state into the correct positions
        //   at the end of the array of states and update their successor indices to
        //   reflect the new positions.
        for ( short i=1 ; i<prev_n_states ; i++ )
        {
            new_states[index] = states[i] ;
            
            // Update the state indices to match the combined model
            for ( short j=0 ; j<new_states[index]->n_successors ; j++ )
                new_states[index]->successor_states[j] += (models[0]->n_states - 2) ;

            index++ ;
        }

        // Update the successor indices of the existing initial state of this instance
        //   to reflect the new state positions.
        for ( short j=0 ; j<states[0]->n_successors ; j++ )
            states[0]->successor_states[j] += (models[0]->n_states - 2) ;

        // Now update the successor information for the states from the first model.
        for ( short i=0 ; i<(models[0]->n_states-1) ; i++ )
        {
            // Copy the successor information
            setupSuccessorStates( new_states[i] , models[0]->states[i]->n_successors ,
                                  models[0]->states[i]->successor_states ,
                                  models[0]->states[i]->suc_log_trans_probs ) ;
            
            // Look at the last successor entry for each state.  If it is the final
            //   state of the first model, remove the entry and replace it with the successors
            //   of the initial state of the second model.
            old_prob = new_states[i]->suc_log_trans_probs[new_states[i]->n_successors-1] ;
            old_n_sucs = new_states[i]->n_successors ;
            if ( new_states[i]->successor_states[new_states[i]->n_successors-1] == 
                                                                    (models[0]->n_states-1) )
            {
                new_states[i]->n_successors += (states[0]->n_successors - 1) ;
                new_states[i]->successor_states = (short *)Allocator::sysRealloc( 
                                                new_states[i]->successor_states ,
                                                new_states[i]->n_successors * sizeof(short) ) ;
                                                     
                new_states[i]->suc_log_trans_probs = (real *)Allocator::sysRealloc( 
                                                     new_states[i]->suc_log_trans_probs ,
                                                     new_states[i]->n_successors * sizeof(real) ) ;

                for ( short j=0 ; j<(states[0]->n_successors) ; j++ )
                {
                    new_states[i]->successor_states[old_n_sucs+j-1] = 
                                                                states[0]->successor_states[j] ;
                    new_states[i]->suc_log_trans_probs[old_n_sucs+j-1] = old_prob + 
                                                                states[0]->suc_log_trans_probs[j] ;
                }
            }
        }
        
        if ( states[0]->successor_states != NULL )
            free( states[0]->successor_states ) ;
        if ( states[0]->suc_log_trans_probs != NULL )
            free( states[0]->suc_log_trans_probs ) ;
        free( states[0] ) ;
        free( states ) ;
        states = new_states ;
    }
    else if ( n_models == 1 )
    {
        // If we only have 1 model in the input array, just copy its contents
        n_states = models[0]->n_states ;
        states = (DecodingHMMState **)Allocator::sysAlloc( n_states * sizeof(DecodingHMMState *) ) ;
        
        for ( short i=0 ; i<n_states ; i++ )
        {
            states[i] = (DecodingHMMState *)Allocator::sysAlloc( sizeof(DecodingHMMState) ) ;
            initState( states[i] , models[0]->states[i]->distribution ,
                       models[0]->states[i]->emission_prob_vec_index ) ;
            setupSuccessorStates( states[i] , models[0]->states[i]->n_successors ,
                                  models[0]->states[i]->successor_states ,
                                  models[0]->states[i]->suc_log_trans_probs ) ;
        }
    }
}


void DecodingHMM::initState( DecodingHMMState *state , Distribution *distribution_ ,
                             short emission_prob_vec_index_ )
{
    state->distribution = distribution_ ;
    state->emission_prob_vec_index = emission_prob_vec_index_ ;
    state->n_successors = 0 ;
    state->successor_states = NULL ;
    state->suc_log_trans_probs = NULL ;
}


void DecodingHMM::setupSuccessorStates( DecodingHMMState *state , short n_successors_ , 
                                        short *successor_states_ , real *log_trans_probs_ )
{
    state->n_successors = n_successors_ ;

    if ( n_successors_ > 0 )
    {
        state->successor_states = (short *)Allocator::sysAlloc( n_successors_ * sizeof(short) ) ;
        state->suc_log_trans_probs = (real *)Allocator::sysAlloc( n_successors_ * sizeof(real) ) ;
        for ( int i=0 ; i<n_successors_ ; i++ )
        {
            state->successor_states[i] = successor_states_[i] ;
            state->suc_log_trans_probs[i] = log_trans_probs_[i] ;
        }
    }
}


#ifdef DEBUG
void DecodingHMM::outputText()
{
    printf("DecodingHMM with %d states\n*************************\n" , n_states) ;
    for ( int i=0 ; i<n_states ; i++ )
    {
        printf("State %d :\n" , i ) ;
        printf("DecodingHMMState: n_sucs=%d    ",states[i]->n_successors) ;
        for ( int j=0 ; j<states[i]->n_successors ; j++ )
            printf("%d ",states[i]->successor_states[j]) ;
        printf("   ") ;
        for ( int j=0 ; j<states[i]->n_successors ; j++ )
            printf("%.20f ",states[i]->suc_log_trans_probs[j]) ;
        printf("\n") ;
    }
    printf("\n") ;
}
#endif


}
