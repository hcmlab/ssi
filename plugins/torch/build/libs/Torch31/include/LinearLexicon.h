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

#ifndef LINEARLEXICON_INC
#define LINEARLEXICON_INC

#include "general.h"
#include "LexiconInfo.h"
#include "PhoneModels.h"
#include "DecodingHMM.h"
#include "SpeechHMM.h"


namespace Torch {


/**
    This class is essentially an array of DecodingHMM instances,
    representing the HMM's for each pronunciation we can recognise.
    The ordering of this array conforms to the ordering the 
    LexInfo instance that is passed as a parameter to the
    constructor.

    @author Darren Moore (moore@idiap.ch)
*/

class LinearLexicon
{
public:
    LexiconInfo *lex_info ;
    PhoneModels *phone_models ;
	int n_models ;
    DecodingHMM **models ;
    int total_states ;
    
	/* Constructors / destructor */

    /// Uses the phoneme DecodingHMM instances in 'phone_models_',
    ///   and the phonetic transcription info for the pronunciations
    ///   as defined in 'lex_info_' and creates a complete DecodingHMM
    ///   instance for each pronunciation.
    LinearLexicon( LexiconInfo *lex_info_ , PhoneModels *phone_models_ ) ;

    /// Extracts LexiconInfo pointer from 'speech_hmm' then proceeds
    ///   in the same way as the first constructor.
    LinearLexicon( SpeechHMM *speech_hmm , PhoneModels *phone_models_ ) ;
	virtual ~LinearLexicon() ;
	
	/* Methods */
    
    /// Internal function. Both constructors call this to create the
    ///   required data structures.
    void createLinearLexicon( LexiconInfo *lex_info_ , PhoneModels *phone_models_ ) ;
    
    /// Returns the number of states in a particular model including
    ///   non-emitting states.
    int nStatesInModel( int model ) ;

    /// Calculates the emission probability for a state in a particular
    ///   word model using the current input vector. Only checks for 
    ///   out of range input parameters in the debug version.
    real calcEmissionProb( int model , int state ) ;

    /// Returns the number of successor states, the successor states
    ///   themselves and the associated log transition probabilities.
    /// Does not copy data - just returns pointers to the originals.
    void getSuccessorInfo( int model , int state , short *n_sucs , 
                           short **sucs , real **log_trans_probs ) ;

                          

#ifdef DEBUG
	void outputText() ;
#endif
};


}

#endif
