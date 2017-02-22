// Copyright (C) 2003--2004 Samy Bengio (bengio@idiap.ch)
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

#ifndef SIMPLE_DECODER_SPEECH_HMM_INC
#define SIMPLE_DECODER_SPEECH_HMM_INC

#include "SpeechHMM.h"
#include "Grammar.h"
#include "WordSeg.h"
#include "FrameSeg.h"

namespace Torch {

/** This class implements a special case of Hidden Markov Models that
    can be used to do connected word speech recognition for small
    vocabulary, using embedded training.

    It contains a #SpeechHMM# and a grammar (which states
    the legal sentences of the langage).

    The decoding is done by creating the whole transition matrix
    and hence is not adapted to large vocabulary problems.

    @author Samy Bengio (bengio@idiap.ch)
*/
class SimpleDecoderSpeechHMM : public SpeechHMM
{
  public:

    /// The trained SpeechHMM model to decode
    SpeechHMM* model;

    /// the acceptable grammar
    Grammar* grammar;

    /// the object containing target and obtained word sequences
    WordSeg* wordseg;
    FrameSeg* frameseg;

    /// log word entrance penalty: during viterbi, penalizes large sentences
    real log_word_entrance_penalty;
    /// should we perform forced alignment or real decoding?
    bool forced_alignment;

    /// for each state, how many previous states
    int* n_previous_states;
    /// for each state, the list of previous states
    int** previous_states;


    /** In order to create a SimpleDecoderSpeechHMM, we need to give a vector of #n_models_#
        #HMM#s as well as their corresponding name, a lexicon and a grammar,
        an optional log_word_entrance_penalty and an optional trainer that can be
        used to initialize each model independently.
    */
    SimpleDecoderSpeechHMM(SpeechHMM* model, Grammar* grammar_);

    /// this method sets the test data set to be used
    virtual void setDataSet(DataSet* data_);

    /** this method redefine the normal logViterbi class with 
        constraint on word entrance */
    virtual void logViterbi(Sequence* inputs);

    /// this method returns the sentence associated to the input
    virtual void decode(Sequence* input);

    /// this method computes more efficiently the emission probabilities
    virtual void logProbabilities(Sequence *inputs);

    /// prepare structure previous_states and n_previous_states
    virtual void setPreviousStates();

    /** this method prepare the transition graph associated with a
        given test sentence
    */
    virtual void prepareTestModel(Sequence* input);

    /// this methods returns the number of states in the grammar
    virtual int nStatesInGrammar();

    virtual void setMaxNStates(int max_n_states_);

    virtual ~SimpleDecoderSpeechHMM();
};


}

#endif
