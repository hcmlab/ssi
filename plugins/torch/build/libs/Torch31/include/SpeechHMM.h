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

#ifndef SPEECH_HMM_INC
#define SPEECH_HMM_INC

#include "HMM.h"
#include "LexiconInfo.h"
#include "EditDistance.h"
#include "log_add.h"
#include "EMTrainer.h"
#include "ExampleFrameSelectorDataSet.h"

namespace Torch {

/** This class implements a special case of Hidden Markov Models that
    can be used to do connected word speech recognition for small
    vocabulary, using embedded training.

    It contains a set of phoneme models (represented by HMMs), a lexicon
    of words (which are sequences of phonemes) 

    @author Samy Bengio (bengio@idiap.ch)
*/
class SpeechHMM : public HMM
{
  public:
    /// the number of basic phoneme models 
    int n_models;
    /// the basic phoneme models
    HMM** models;

    /// a dataset for initialization
		DataSet* data;
		
    /** if an initial alignment is given and an emtrainer for each model
        then it is used to train the models after kmeans during reset
    */
    EMTrainer** model_trainer;

		/// as well as a measurer of this trainer
    MeasurerList* initial_models_trainer_measurers;

    /// the acceptable lexicon
    LexiconInfo* lexicon;

    /// the current target sequence, with start and end words/phonemes
    Sequence* targets;
    /// number of words to add
    int add_to_targets;

    /// true if the given transition is a transition between words
    bool **word_transitions;

    /// the maximum number of states in the graph (used for allocation)
    int max_n_states;
    
    /// the relation between model states and SpeechHMM states
    int* states_to_model_states;

    /// the relation between models and SpeechHMM states
    int* states_to_model;

    /// the relation between words and SpeechHMM states
    int* states_to_word;
  
    /// are targets expressed in words or phonemes?
    bool phoneme_targets;

    /** In order to create a SpeechHMM, we need to give a vector of #n_models_#
        #HMM#s as well as their corresponding name, a lexicon,
        an optional log_word_entrance_penalty and an optional trainer that can be
        used to initialize each model independently.
    */
    SpeechHMM(int n_models_, HMM **models_, LexiconInfo* lex_, EMTrainer** model_trainer_ = NULL);

    virtual void setDataSet(DataSet* data_);
		void splitDataSet(DataSet* data_, ExampleFrameSelectorDataSet** split_data_);
    virtual void loadXFile(XFile *file);
    virtual void saveXFile(XFile *file);

    virtual void iterInitialize();
    virtual void eMIterInitialize();
    virtual void eMSequenceInitialize(Sequence* inputs);
    virtual void sequenceInitialize(Sequence* inputs);
    virtual void eMAccPosteriors(Sequence *inputs, real log_posterior);
    virtual void viterbiAccPosteriors(Sequence *inputs, real log_posterior);
    virtual void eMUpdate();


    /** this method prepare the transition graph associated with a
        given training sentence
    */
    virtual void prepareTrainModel(Sequence* input);

    /** this method is used by #prepareTrainModel# 
        to prepare the model. It adds a given word to the current graph.
    */
    virtual int addWordToModel(int word, int current_state);

    virtual	void setMaxNStates(int max_n_states_);

    /** this method is used by #prepareTrainModel#
        to prepare the model. It adds the connections between words.
    */
    virtual void addConnectionsBetweenWordsToModel(int word,int next_word, int current_state,int next_current_state, real log_n_next);

    /// this methods returns the number of states in a given word
    virtual int nStatesInWord(int word);

    /// this methods returns the number of states in a given word pronunciation
    virtual int nStatesInWordPronunciation(int word, int pronun);

    virtual void backward(Sequence *inputs, Sequence *alpha);

    virtual void setTargets(Sequence* t);

    virtual ~SpeechHMM();
};


}

#endif
