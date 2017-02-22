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


#include "SimpleDecoderSpeechHMM.h"
#include "log_add.h"

namespace Torch {

SimpleDecoderSpeechHMM::SimpleDecoderSpeechHMM(SpeechHMM* model_,Grammar* grammar_) : SpeechHMM(model_->n_models,model_->models,model_->lexicon,NULL)
{
  model = model_;
  grammar = grammar_;

  addROption("log word entrance penalty", &log_word_entrance_penalty, LOG_ONE, "log word entrance penalty");
  addBOption("forced alignment", &forced_alignment, false, "forced alignment");

  wordseg = new(allocator)WordSeg(lexicon);
  frameseg = new(allocator)FrameSeg(lexicon);

  n_previous_states = NULL;
  previous_states = NULL;
  n_states = nStatesInGrammar();
  setMaxNStates(n_states);
}

void SimpleDecoderSpeechHMM::setMaxNStates(int max_n_states_)
{
  if (max_n_states_ <= max_n_states)
    return;
  // we have to resize the previous_states matrix
  if (previous_states) {
    for (int i=0;i<max_n_states;i++) {
      allocator->free(previous_states[i]);
    }
  }
  n_previous_states = (int*)allocator->realloc(n_previous_states,sizeof(int)*max_n_states_);
  previous_states = (int**)allocator->realloc(previous_states,sizeof(int*)*max_n_states_);
  for (int i=0;i<max_n_states_;i++) {
    previous_states[i] = (int*)allocator->alloc(sizeof(int)*max_n_states_);
  }
  SpeechHMM::setMaxNStates(max_n_states_);
}


int SimpleDecoderSpeechHMM::nStatesInGrammar()
{
  int grammar_n_states=2;
  for (int i=1;i<grammar->n_words-1;i++) {
    int word = grammar->words[i];
    grammar_n_states += nStatesInWord(word);
  }
  return grammar_n_states;
}

void SimpleDecoderSpeechHMM::prepareTestModel(Sequence* ex)
{
  // create the new transition matrix, based on the models and the target sentence
  // first realloc if necessary
  int n_frames = ex->n_frames;
  log_probabilities_s->resize(n_frames);
  log_alpha->resize(n_frames);
  log_beta->resize(n_frames);
  arg_viterbi->resize(n_frames);
  viterbi_sequence->resize(n_frames);

  // then put all transitions to 0
  for (int i=0;i<n_states;i++) {
    for (int j=0;j<n_states;j++) {
      log_transitions[i][j] = LOG_ZERO;
      word_transitions[i][j] = false;
    }
  }

  // then create the new transition matrix
  states_to_word[0] = -1;
  states_to_model_states[0] = -1;
  states_to_model[0] = -1;
  states[0] = NULL;
  states[n_states-1] = NULL;
  // the transitions from the initial state
  // first count the total probability, and normalize it
  real total_prob = 0;
  for (int i=1;i<grammar->n_words-1;i++) {
    if (grammar->transitions[i][0]) {
      int m = grammar->words[i];
      int lex_ind = lexicon->vocab_to_lex_map[m].pronuns[0];
      LexiconInfoEntry *lex_ent = &lexicon->entries[lex_ind];
      int p = lex_ent->phones[0];
      for (int k=1;k<models[p]->n_states;k++)
        total_prob += exp(models[p]->log_transitions[k][0]);
    }
  }
  // then update the probabilities
  int j=1;
  real log_total_prob = log(total_prob);
  for (int i=1;i<grammar->n_words-1;i++) {
    if (grammar->transitions[i][0]) {
      int m = grammar->words[i];
      int lex_ind = lexicon->vocab_to_lex_map[m].pronuns[0];
      LexiconInfoEntry *lex_ent = &lexicon->entries[lex_ind];
      int p = lex_ent->phones[0];
      for (int k=1;k<models[p]->n_states;k++)
        log_transitions[j+k-1][0] = models[p]->log_transitions[k][0] - 
          log_total_prob;
      j += nStatesInWord(m);
    }
  }

  //then, for each word in the grammar, add it
  int current_state = 1;
  for (int i=1;i<grammar->n_words-1;i++) {
    grammar->start[i] = current_state;
    current_state = addWordToModel(grammar->words[i],current_state);
  }
  // then add the transitions between words
  for (int i=1;i<grammar->n_words-1;i++) {
    int word = grammar->words[i];
    int lex_ind = lexicon->vocab_to_lex_map[word].pronuns[0];
    LexiconInfoEntry *lex_ent = &lexicon->entries[lex_ind];
    // count the transitions starting from word
    real log_n_transitions = 0;
    for (int j=1;j<grammar->n_words;j++) {
      log_n_transitions += (grammar->transitions[j][i]);
    }
    log_n_transitions = log_n_transitions>0 ? log(log_n_transitions) : LOG_ONE;
    for (int j=1;j<grammar->n_words;j++) {
      if (grammar->transitions[j][i]) {
        int next_word = grammar->words[j];
        if (next_word != -1) {
          // add transitions between words
          addConnectionsBetweenWordsToModel(word,next_word,grammar->start[i],
            grammar->start[j],log_n_transitions);
        } else {
          // add last transitions
          int current_model = lex_ent->phones[lex_ent->n_phones-1];
          int n_states_in_model = models[current_model]->n_states;
          int n_states_in_word = nStatesInWord(word);
          int last_state = n_states-1;
          for (int k=1;k<n_states_in_model-1;k++)
            log_transitions[last_state][grammar->start[i]+n_states_in_word-n_states_in_model+2+k-1] = models[current_model]->log_transitions[n_states_in_model-1][k];
        }
      }
    }
  }
/*
  printTransitions(false,true);
  for (int i=0;i<n_states;i++) {
    printf("state %d corresponds to state %d in model %d in word %d\n",i,states_to_model_states[i],states_to_model[i],states_to_word[i]);
  }
*/
}

void SimpleDecoderSpeechHMM::setDataSet(DataSet* data_)
{
  data = data_;
}

void SimpleDecoderSpeechHMM::logProbabilities(Sequence *inputs)
{
  for (int f=0;f<inputs->n_frames;f++) {
    for (int i=0;i<n_models;i++) {
      if (models[i]->n_shared_states == 0) {
        for (int j=1;j<models[i]->n_states-1;j++) {
          models[i]->states[j]->frameLogProbability(f, inputs->frames[f]);
        }
      } else {
        for (int j=0;j<models[i]->n_shared_states;j++) {
          models[i]->shared_states[j]->frameLogProbability(f, inputs->frames[f]);
        }
      }
    }
    for (int i=1;i<n_states-1;i++) {
      log_probabilities_s->frames[f][i] = states[i]->log_probabilities->frames[f][0];
    }
  }
}

void SimpleDecoderSpeechHMM::setPreviousStates()
{
  for (int i=0;i<n_states;i++) {
    n_previous_states[i] = 0;
    for (int j=0;j<n_states;j++) {
      if (log_transitions[i][j] != LOG_ZERO)
        n_previous_states[i]++;
    }
    int k=0;
    for (int j=0;j<n_states;j++) {
      if (log_transitions[i][j] != LOG_ZERO)
        previous_states[i][k++] = j;
    }
  }
}

void SimpleDecoderSpeechHMM::logViterbi(Sequence* inputs)
{
  // first, initialize everything to LOG_ZERO
  for (int f=0;f<inputs->n_frames;f++) {
    for (int i=0;i<n_states;i++) {
      log_alpha->frames[f][i] = LOG_ZERO;
    }
  }
  // case for first frame
  for (int i=1;i<n_states-1;i++) {
    if (log_transitions[i][0] == LOG_ZERO)
      continue;
    log_alpha->frames[0][i] = log_probabilities_s->frames[0][i] +
      log_transitions[i][0];
    arg_viterbi->frames[0][i] = 0.0;
  }
  // other cases
  for (int f=1;f<inputs->n_frames;f++) {
    for (int i=1;i<n_states-1;i++) {
      for (int k=0;k<n_previous_states[i];k++) {
        int j = previous_states[i][k];
        real v = log_transitions[i][j] + log_probabilities_s->frames[f][i] +
          log_alpha->frames[f-1][j];
        if (word_transitions[i][j]) {
          v += log_word_entrance_penalty;
        }
        if (log_alpha->frames[f][i] < v) {
          log_alpha->frames[f][i] = v;
          arg_viterbi->frames[f][i] = (real)j;
        }
      }
    }
  }
  // last case
  log_probability = LOG_ZERO;
  int f = inputs->n_frames-1;
  int i = n_states-1;
  for (int j=1;j<n_states-1;j++) {
    if (log_transitions[i][j] == LOG_ZERO)
      continue;
    real v = log_alpha->frames[f][j]+log_transitions[i][j];
    if (log_probability < v) {
      log_probability = v;
      last_arg_viterbi = j;
    }
  }
  if (log_probability > LOG_ZERO) {
    // now recall the state sequence
    viterbi_sequence->frames[inputs->n_frames-1][0] = (real)last_arg_viterbi;
    for (int f=inputs->n_frames-2;f>=0;f--) {
      viterbi_sequence->frames[f][0] = (real)(arg_viterbi->frames[f+1][(int)(viterbi_sequence->frames[f+1][0])]);
    }
  } else {
    warning("sequence impossible to decode: probably too short for grammar");
    for (int f=0;f<inputs->n_frames;f++)
      viterbi_sequence->frames[f][0] = -1;
    log_probability = 0;
  }
}

void SimpleDecoderSpeechHMM::decode(Sequence* ex)
{
  for (int i=0;i<n_models;i++)
    models[i]->eMSequenceInitialize(ex);
  if (forced_alignment) {
    prepareTrainModel(ex);
    log_probabilities_s->resize(ex->n_frames);
    log_alpha->resize(ex->n_frames);
    log_beta->resize(ex->n_frames);
    arg_viterbi->resize(ex->n_frames);
    viterbi_sequence->resize(ex->n_frames);
  } else
    prepareTestModel(ex);
  setPreviousStates();
  logProbabilities(ex);
  logViterbi(ex);

	// keep in memory the word sequence
  wordseg->resize(ex->n_frames);

  // convert the state sequence to a word sequence
  wordseg->word_sequence_size = 0;
  int previous_state = -1;
  for (int i=0;i<ex->n_frames;i++) {
    int state = (int)viterbi_sequence->frames[i][0];
    int word = state>=0 ? states_to_word[state] : lexicon->vocabulary->sil_index;
    // do not keep silences and register each time we exit a model
    if (word != lexicon->vocabulary->sent_start_index &&
        word != lexicon->vocabulary->sent_end_index &&
        word != lexicon->vocabulary->sil_index) {
      if ((previous_state == -1) ||
          (previous_state>=0 && word_transitions[state][previous_state])) {
        //wordseg->word_sequence_time[wordseg->word_sequence_size] = i>0 ? i-1 : i;
        wordseg->word_sequence_time[wordseg->word_sequence_size] = i;
        wordseg->word_sequence[wordseg->word_sequence_size++] = word;
      }
    }
    previous_state = state;
  }

  // keep in memory the target word sequence
  if (data->targets) {
    setTargets(data->targets);
    int t_f = targets->n_frames;
    wordseg->resizeTargets(t_f);

    int j=0;
    for (int i=0;i<t_f;i++) {
      int word = (int)targets->frames[i][0];
      if (word != lexicon->vocabulary->sent_start_index &&
          word != lexicon->vocabulary->sent_end_index &&
          word != lexicon->vocabulary->sil_index) {
        wordseg->target_word_sequence[j++] = word;
      }
    }
    wordseg->target_word_sequence_size = j;
    
    frameseg->states_to_model = states_to_model;
    frameseg->states_to_model_states = states_to_model_states;
    frameseg->states_to_word = states_to_word;
    frameseg->target = targets;
    frameseg->obtained = viterbi_sequence;
  }

  //outputs->frames[0][0] = edit_distance->accuracy;
}

SimpleDecoderSpeechHMM::~SimpleDecoderSpeechHMM()
{
}

}

