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


#include "SpeechHMM.h"
#include "log_add.h"

namespace Torch {

SpeechHMM::SpeechHMM(int n_models_, HMM **models_, LexiconInfo* lex_, EMTrainer** model_trainer_) : HMM(0,(Distribution**)models_,NULL)
{
  n_models = n_models_;
  data = NULL;
  models = models_;
  max_n_states = 1;
  initial_models_trainer_measurers = NULL;
  addROption("prior transitions", &prior_transitions , 1e-3, "minimum weights for each gaussians");
  addBOption("targets are phonemes", &phoneme_targets, false, "for initialization, targets are often phonemes instead of words");
  model_trainer = model_trainer_;
  lexicon = lex_;

  add_to_targets = 0;
  targets = new(allocator) Sequence(1,1);

  for (int i=0;i<n_models;i++) {
    params->add(models[i]->params);
    der_params->add(models[i]->der_params);
  }
  // find the longest sequence in the dataset
  // in terms of frames and number of states of the graph
  log_probabilities_s = new (allocator)Sequence(1,max_n_states);
  log_alpha = new (allocator)Sequence(1,max_n_states);
  log_beta = new (allocator)Sequence(1,max_n_states);
  arg_viterbi = new (allocator)Sequence(1,max_n_states);
  viterbi_sequence = new (allocator)Sequence(1,1);
  states = (Distribution**)allocator->alloc(sizeof(Distribution*)*max_n_states);
  states_to_model_states = (int*)allocator->alloc(sizeof(int)*max_n_states);
  states_to_model = (int*)allocator->alloc(sizeof(int)*max_n_states);
  states_to_word = (int*)allocator->alloc(sizeof(int)*max_n_states);
  log_transitions = (real**)allocator->alloc(sizeof(real*)*max_n_states);
  word_transitions = (bool**)allocator->alloc(sizeof(bool*)*max_n_states);
  for (int i=0;i<max_n_states;i++) {
    log_transitions[i] = (real*)allocator->alloc(sizeof(real)*max_n_states);;
    word_transitions[i] = (bool*)allocator->alloc(sizeof(bool)*max_n_states);;
    for (int j=0;j<max_n_states;j++) {
      log_transitions[i][j] = LOG_ZERO;
      word_transitions[i][j] = false;
    }
  }
}


void SpeechHMM::setMaxNStates(int max_n_states_)
{
  if(max_n_states_ <= max_n_states)
    return;
  for (int i=0;i<max_n_states;i++) {
    allocator->free(log_transitions[i]);
    allocator->free(word_transitions[i]);
  }
  max_n_states = max_n_states_;

  allocator->free(log_probabilities_s);
  allocator->free(log_alpha);
  allocator->free(log_beta);
  allocator->free(arg_viterbi);

  log_probabilities_s = new (allocator)Sequence(1,max_n_states);
  log_alpha = new (allocator)Sequence(1,max_n_states);
  log_beta = new (allocator)Sequence(1,max_n_states);
  arg_viterbi = new (allocator)Sequence(1,max_n_states);

  states = (Distribution**)allocator->realloc(states,sizeof(Distribution*)*max_n_states);
  states_to_model_states = (int*)allocator->realloc(states_to_model_states, sizeof(int)*max_n_states);
  states_to_model = (int*)allocator->realloc(states_to_model, sizeof(int)*max_n_states);
  states_to_word = (int*)allocator->realloc(states_to_word, sizeof(int)*max_n_states);
  log_transitions = (real**)allocator->realloc(log_transitions, sizeof(real*)*max_n_states);
  word_transitions = (bool**)allocator->realloc(word_transitions, sizeof(bool*)*max_n_states);

  for (int i=0;i<max_n_states;i++) {
    log_transitions[i] = (real*)allocator->alloc(sizeof(real)*max_n_states);;
    word_transitions[i] = (bool*)allocator->alloc(sizeof(bool)*max_n_states);;
    for (int j=0;j<max_n_states;j++) {
      log_transitions[i][j] = LOG_ZERO;
      word_transitions[i][j] = false;
    }
  }
}

void SpeechHMM::loadXFile(XFile *file)
{
  for (int i=0;i<n_models;i++)
    models[i]->loadXFile(file);
}

void SpeechHMM::saveXFile(XFile *file)
{
  for (int i=0;i<n_models;i++)
    models[i]->saveXFile(file);
}

void SpeechHMM::setTargets(Sequence* t)
{
  if (!phoneme_targets && add_to_targets > 0) {
    if (targets->frame_size != t->frame_size) {
      allocator->free(targets);
      targets = new(allocator) Sequence(t->n_frames+add_to_targets,t->frame_size);
    } else
      targets->resize(t->n_frames+add_to_targets);
    int j=0;
    if (lexicon->vocabulary->sil_index >= 0) {
      if (t->frame_size > 1)
        targets->frames[j][1] = 1; // silence lasts 1 frame
      targets->frames[j++][0] = lexicon->vocabulary->sil_index;
    }
    for (int i=0;i<t->n_frames;i++,j++) {
      for (int k=0;k<t->frame_size;k++) {
        targets->frames[j][k] = t->frames[i][k];
      }
    }
    if (lexicon->vocabulary->sent_end_index >= 0) {
      if (t->frame_size > 1) {
        targets->frames[j][1] = t->frames[t->n_frames-1][1];
        targets->frames[j-1][1] = t->frames[t->n_frames-1][1]-1;
      }
      targets->frames[j++][0] = lexicon->vocabulary->sent_end_index;
    }
  } else
    targets = t;
}

void SpeechHMM::splitDataSet(DataSet* data_, ExampleFrameSelectorDataSet** split_data_)
{
  // just print some informations
  data_->setExample(0);
  message("SpeechHMM: Targets are given as %s",phoneme_targets ? "phonemes" : "words");
  message("SpeechHMM: Targets are given %s alignments",data_->targets->frame_size > 1 ? "with" : "without");

  for (int i=0; i<data_->n_examples; i++) {
    data_->setExample(i);
    setTargets(data_->targets);
    Sequence* inputs = data_->inputs;
    int n_frames = inputs->n_frames;

    if(targets->frame_size > 1) {
      // we have the alignment
      for (int j=0;j<targets->n_frames;j++) {
        int unit = (int)targets->frames[j][0];
        real begin_unit = j == 0 ? 0. : targets->frames[j-1][1];
        real end_unit = targets->frames[j][1];
        if (phoneme_targets) {
          // the alignment is given in terms of phonemes
          int bu = (int)begin_unit;
          int eu = (int)end_unit;
          if (eu > n_frames) {
            warning("The target alignment has been truncated from %d to %d",eu,n_frames-1);
            eu = n_frames;
          }
          int length = eu - bu;
          split_data_[unit]->addExample(i, bu, length, j,  1);
        } else {
          // the alignment is given in terms of words
          real n_frames_in_word = end_unit - begin_unit;
          // will work for the first pronunciation only!
          int n_states_in_word = 0;
          int lex_ind = lexicon->vocab_to_lex_map[unit].pronuns[0];
          LexiconInfoEntry *lex_ent = &lexicon->entries[lex_ind];
          for (int k=0;k<lex_ent->n_phones;k++) {
            int phoneme = lex_ent->phones[k];
            n_states_in_word += models[phoneme]->n_states-2;    
          }
          real n_frames_per_state = n_frames_in_word/n_states_in_word;
          // finaly bordel a noeud the index!!!
          real begin_phoneme = begin_unit;
          real end_phoneme;
          for (int k=0;k<lex_ent->n_phones;k++) {
            int phoneme = lex_ent->phones[k];
            end_phoneme = begin_phoneme + n_frames_per_state * 
              (models[phoneme]->n_states-2);
            int bu = (int)begin_phoneme;
            int eu = (int)end_phoneme;
            if (eu >= n_frames) {
              warning("SpeechHMM::splitData: The target alignment has been truncated from %d to %d",eu,n_frames-1);
              eu = n_frames - 1;
            }
            int length = eu - bu;
            split_data_[phoneme]->addExample(i, bu, length, j,  1);
            begin_phoneme = end_phoneme;
          }
        }
      }
    } else {
      //linear alignment!
      if (phoneme_targets) {
        // targets are phonemes
        //find the total number of states in the example
        int tot_n_states = 0;
        for( int j=0;j<targets->n_frames;j++) {
          int phoneme = (int)targets->frames[j][0];
          tot_n_states += models[phoneme]->n_states-2;
        }
        //then assign frames for each phoneme model
        real n_frames_per_state = (real)n_frames/tot_n_states;
        real begin_phoneme = 0.;
        for( int j=0;j<targets->n_frames;j++) {
          int phoneme = (int)targets->frames[j][0];
          real end_phoneme = begin_phoneme + n_frames_per_state *
              (models[phoneme]->n_states-2);
          int ib = (int)begin_phoneme;
          int ie = (int)end_phoneme;
          if (ie >= n_frames) {
            ie = n_frames - 1;
          }
          int length = ie - ib;
          if (length < 1) {
            warning("a segmentation produced a sequence of frames (%d) less than then number of states (%d) of the model",n_frames,tot_n_states);
          } else
            split_data_[phoneme]->addExample(i, ib, length, j,  1);
          begin_phoneme = end_phoneme;
        }
      } else {
        // targets are given in terms of words
        //this will work with the first pronounciation of each word only!!!
        //find the total number of states in the example
        int tot_n_states = 0;
        for( int j=0;j<targets->n_frames;j++) {
          int word = (int)targets->frames[j][0];
          int lex_ind = lexicon->vocab_to_lex_map[word].pronuns[0];
          LexiconInfoEntry *lex_ent = &lexicon->entries[lex_ind];
          for (int k=0;k<lex_ent->n_phones;k++) {
            int phoneme = lex_ent->phones[k];
            tot_n_states += models[phoneme]->n_states-2;
          }
        }
        //then assign frames for each phoneme model
        real n_frames_per_state = (real)n_frames/tot_n_states;
        real begin_phoneme = 0.;
        for(int j=0;j<targets->n_frames;j++) {
          int word = (int)targets->frames[j][0];
          int lex_ind = lexicon->vocab_to_lex_map[word].pronuns[0];
          LexiconInfoEntry *lex_ent = &lexicon->entries[lex_ind];
          for (int k=0;k<lex_ent->n_phones;k++) {
            int phoneme = lex_ent->phones[k];
            real end_phoneme = begin_phoneme + n_frames_per_state *
              (models[phoneme]->n_states-2);
            int ib = (int)begin_phoneme;
            int ie = (int)end_phoneme;
            if (ie >= n_frames) {
              ie = n_frames - 1;
            }
            int length = ie - ib;
            if (length < 1) {
              warning("a segmentation produced a sequence of frames (%d) less than then number of states (%d) of the model",n_frames,tot_n_states);
            } else
              split_data_[phoneme]->addExample(i, ib, length, j,  1);
            begin_phoneme = end_phoneme;
          }
        }
      }
    }
  }
}

void SpeechHMM::setDataSet(DataSet* data_)
{
  // if alignment information is given in the dataset, use it.
  // otherwise, do a linear alignment along the states

  data = data_;

  add_to_targets = 0;
  if (!phoneme_targets) {
    if (lexicon->vocabulary->sil_index >= 0)
      add_to_targets++;
    if (lexicon->vocabulary->sent_end_index >= 0)
      add_to_targets++;
  }

  if (initialize) {
    // initialize model
    ExampleFrameSelectorDataSet** sub_dataset = (ExampleFrameSelectorDataSet**)allocator->alloc(sizeof(ExampleFrameSelectorDataSet*)*n_models);
    for (int m=0;m<n_models;m++) {
      sub_dataset[m] = new(allocator) ExampleFrameSelectorDataSet(data_);
    }
    splitDataSet(data_, sub_dataset);

    for (int m=0;m<n_models;m++) {
      // the transitions and emission parameters will be set in each model
      if (sub_dataset[m]->n_examples==0) {
          message("KMeans initialization of model %d with all data",m);
          models[m]->setDataSet(data_);
      } else if (model_trainer) {
          message("HMM initialization of model %d with own aligned data",m);
          model_trainer[m]->train(sub_dataset[m], initial_models_trainer_measurers);
      } else {
          message("KMeans initialization of model %d with own aligned data",m);
          models[m]->setDataSet(sub_dataset[m]);
      }
    }
  } else {
    // we still need to set the dataset of each model
    for (int m=0;m<n_models;m++) {
      models[m]->setDataSet(data_);
    }
  }
}

void SpeechHMM::addConnectionsBetweenWordsToModel(int word,int next_word, int current_state,int next_current_state, real log_n_next)
{
  int n_states_word_pronun = 0;
  int n_pronuns = lexicon->vocab_to_lex_map[word].n_pronuns;
  for (int l=0;l<n_pronuns;l++) {
    int lex_ind = lexicon->vocab_to_lex_map[word].pronuns[l];
    LexiconInfoEntry *lex_ent = &lexicon->entries[lex_ind];
    int current_model = lex_ent->phones[lex_ent->n_phones-1];
    int n_states_model = models[current_model]->n_states;
    int n_states_next_word_pronun = 0;
    int n_pronuns_next = lexicon->vocab_to_lex_map[next_word].n_pronuns;
    int n_states_current_word_pronun = nStatesInWordPronunciation(word,l);
    n_states_word_pronun += n_states_current_word_pronun;
    for (int m=0;m<n_pronuns_next;m++) {
      int next_lex_ind = lexicon->vocab_to_lex_map[next_word].pronuns[m];
      LexiconInfoEntry *next_lex_ent = &lexicon->entries[next_lex_ind];
      int next_model = next_lex_ent->phones[0];
      int n_states_next_model = models[next_model]->n_states;
      for (int j=1;j<n_states_model;j++) {
        if (models[current_model]->log_transitions[n_states_model-1][j] != LOG_ZERO) {
          for (int k=1;k<n_states_next_model-1;k++) {
            if (models[next_model]->log_transitions[k][0] != LOG_ZERO) {
              log_transitions[next_current_state+n_states_next_word_pronun+k-1][current_state+n_states_word_pronun-n_states_model+1+j] = 
                models[current_model]->log_transitions[n_states_model-1][j] +
                models[next_model]->log_transitions[k][0] - log_n_next;
              word_transitions[next_current_state+n_states_next_word_pronun+k-1][current_state+n_states_word_pronun-n_states_model+1+j] = true;
            }
          }
        }
      }
      n_states_next_word_pronun += nStatesInWordPronunciation(next_word,m);
    }
  }
}

int SpeechHMM::addWordToModel(int word, int current_state)
{
  int n_pronuns = lexicon->vocab_to_lex_map[word].n_pronuns;
  for (int i=0;i<n_pronuns;i++) {
    int lex_ind = lexicon->vocab_to_lex_map[word].pronuns[i];
    LexiconInfoEntry *lex_ent = &lexicon->entries[lex_ind];
    // for each phoneme
    for (int l=0;l<lex_ent->n_phones;l++) {
      int current_model = lex_ent->phones[l];
      // for each emitting state of the current model
      int n_states_model = models[current_model]->n_states;
      for (int j=1;j<n_states_model-1;j++,current_state++) {
//printf("In word %d Loading model %d state %d into state %d\n",word, current_model, j, current_state);
        states[current_state] = models[current_model]->states[j];
        states_to_model_states[current_state] = j;
        states_to_model[current_state] = current_model;
        states_to_word[current_state] = word;
        // for each transition from current_state
        for (int k=1;k<n_states_model-1;k++) {
          log_transitions[current_state+k-j][current_state] = 
            models[current_model]->log_transitions[k][j];
        }
      }
      // add transitions between phonemes
      if (l<lex_ent->n_phones-1) {
        int next_model = lex_ent->phones[l+1];
        int n_states_next_model = models[next_model]->n_states;
        for (int j=1;j<n_states_model;j++) {
           if (models[current_model]->log_transitions[n_states_model-1][j] != LOG_ZERO) {
           for (int k=1;k<n_states_next_model-1;k++) {
              if (models[next_model]->log_transitions[k][0] != LOG_ZERO)
                log_transitions[current_state+k-1][current_state-n_states_model+1+j] = 
                  models[current_model]->log_transitions[n_states_model-1][j] +
                  models[next_model]->log_transitions[k][0];
            }
          }
        }
      }
    }
  }
  return current_state;
}

void SpeechHMM::prepareTrainModel(Sequence* inputs)
{
  // create the new transition matrix, based on the models and the target sentence
  setTargets(data->targets);
  n_states = 2;
  for (int j=0;j<targets->n_frames;j++) {
    int word = (int)targets->frames[j][0];
    int n_pronuns = lexicon->vocab_to_lex_map[word].n_pronuns;
    for (int l=0;l<n_pronuns;l++) {
      int lex_ind = lexicon->vocab_to_lex_map[word].pronuns[l];
      LexiconInfoEntry *lex_ent = &lexicon->entries[lex_ind];
      // for each phoneme
      for (int k=0;k<lex_ent->n_phones;k++) {
        int current_model = lex_ent->phones[k];
        n_states += models[current_model]->n_states-2;
      }
    }
  }
  // first realloc if necessary and test for minimum sequence length
  if (inputs->n_frames < n_states-2)
    warning("your sentence has %d frames where your HMM model has %d emitting states", inputs->n_frames,n_states-2);
  setMaxNStates(n_states);

  // then put all transitions to 0
  for (int i=0;i<n_states;i++) {
    for (int j=0;j<n_states;j++) {
      log_transitions[i][j] = LOG_ZERO;
      word_transitions[i][j] = false;
    }
  }

  // the transitions from the initial state
  int word = (int)targets->frames[0][0];
  int lex_ind = lexicon->vocab_to_lex_map[word].pronuns[0];
  LexiconInfoEntry *lex_ent = &lexicon->entries[lex_ind];
  int current_model = lex_ent->phones[0];
  states_to_model_states[0] = 0;
  states_to_model[0] = current_model;
  states_to_word[0] = word;
  states[0] = NULL;
  states[n_states-1] = NULL;
  int n_states_in_word = 0;
  int n_pronuns = lexicon->vocab_to_lex_map[word].n_pronuns;
  real norm = log((real)n_pronuns);
  for (int i=0;i<n_pronuns;i++) {
    lex_ind = lexicon->vocab_to_lex_map[word].pronuns[i];
    lex_ent = &lexicon->entries[lex_ind];
    int current_model = lex_ent->phones[0];
    for (int j=1;j<models[current_model]->n_states;j++)
      log_transitions[n_states_in_word+j][0] = models[current_model]->log_transitions[j][0] - norm;
    n_states_in_word += nStatesInWordPronunciation(word,i);
  }
  int current_state = 1;
  for (int i=0;i<targets->n_frames;i++) {
    word = (int)targets->frames[i][0];
    n_pronuns = lexicon->vocab_to_lex_map[word].n_pronuns;
    int next_current_state = addWordToModel(word, current_state);
    if (i<targets->n_frames-1) {
      int next_word = (int)targets->frames[i+1][0];
      // add transitions between words
      int n_pronuns_next = lexicon->vocab_to_lex_map[next_word].n_pronuns;
      norm = log((real)n_pronuns*n_pronuns_next);
      addConnectionsBetweenWordsToModel(word,next_word,current_state,
        next_current_state,norm);
    } else {
      // add last transitions
      n_states_in_word = 0;
      for (int k=0;k<n_pronuns;k++) {
        n_states_in_word += nStatesInWordPronunciation(word,k);
        int lex_ind = lexicon->vocab_to_lex_map[word].pronuns[k];
        LexiconInfoEntry *lex_ent = &lexicon->entries[lex_ind];
        int current_model = lex_ent->phones[lex_ent->n_phones-1];
        int n_states_in_model = models[current_model]->n_states;
        for (int j=1;j<n_states_in_model-1;j++)
          log_transitions[next_current_state][current_state+n_states_in_word-n_states_in_model+1+j] = models[current_model]->log_transitions[n_states_in_model-1][j];
      }
    }
    current_state = next_current_state;
  }
}

int SpeechHMM::nStatesInWordPronunciation(int word, int pronun)
{
  int word_n_states=0;
  int lex_ind = lexicon->vocab_to_lex_map[word].pronuns[pronun];
  LexiconInfoEntry *lex_ent = &lexicon->entries[lex_ind];
  for (int j=0;j<lex_ent->n_phones;j++) {
    word_n_states += models[lex_ent->phones[j]]->n_states - 2;
  }
  return word_n_states;
}

int SpeechHMM::nStatesInWord(int word)
{
  int word_n_states=0;
  int n_pronuns = lexicon->vocab_to_lex_map[word].n_pronuns;
  for (int i=0;i<n_pronuns;i++) {
    word_n_states += nStatesInWordPronunciation(word,i);
  }
  return word_n_states;
}


void SpeechHMM::eMSequenceInitialize(Sequence* inputs)
{
  prepareTrainModel(inputs);
  log_probabilities_s->resize(inputs->n_frames);
  log_alpha->resize(inputs->n_frames);
  log_beta->resize(inputs->n_frames);
  arg_viterbi->resize(inputs->n_frames);
  viterbi_sequence->resize(inputs->n_frames);

  // propagate to each model
  for (int i=0;i<n_models;i++)
    models[i]->eMSequenceInitialize(inputs);

}

void SpeechHMM::sequenceInitialize(Sequence* inputs)
{
  prepareTrainModel(inputs);
  log_probabilities_s->resize(inputs->n_frames);
  log_alpha->resize(inputs->n_frames);
  log_beta->resize(inputs->n_frames);
  arg_viterbi->resize(inputs->n_frames);
  viterbi_sequence->resize(inputs->n_frames);

  // propagate to each model
  for (int i=0;i<n_models;i++)
    models[i]->sequenceInitialize(inputs);

}

void SpeechHMM::eMIterInitialize()
{
  for (int i=0;i<n_models;i++)
    models[i]->eMIterInitialize();
}

void SpeechHMM::iterInitialize()
{
  for (int i=0;i<n_models;i++)
    models[i]->iterInitialize();
}

void SpeechHMM::eMAccPosteriors(Sequence *inputs, real log_posterior)
{
  // compute the beta by backward recursion
  logBeta(inputs);

  // accumulate the emission and transition posteriors
  for (int f=0;f<inputs->n_frames;f++) {
    for (int i=1;i<n_states-1;i++) {
      if (log_alpha->frames[f][i] != LOG_ZERO && 
          log_beta->frames[f][i] != LOG_ZERO) {
        real log_posterior_i_f = log_posterior + log_alpha->frames[f][i] +
          log_beta->frames[f][i] - log_probability;
        states[i]->frameEMAccPosteriors(f,inputs->frames[f],log_posterior_i_f);
      }
    }
  }
  for (int f=1;f<inputs->n_frames;f++) {
    for (int i=1;i<n_states-1;i++) {
      real log_emit_i = states[i]->log_probabilities->frames[f][0];
      int model_to = states_to_model[i];
      int state_to = states_to_model_states[i];
      for (int j=1;j<n_states;j++) {
        if (log_transitions[i][j] == LOG_ZERO || log_alpha->frames[f-1][j] == LOG_ZERO || log_beta->frames[f][i] == LOG_ZERO || log_emit_i == LOG_ZERO)
          continue;
        // find the real transition
        int model_from = states_to_model[j];
        int state_from = states_to_model_states[j];
        if (model_from == model_to) {
          models[model_from]->transitions_acc[state_to][state_from] += 
            exp(log_posterior + log_alpha->frames[f-1][j] + 
            log_transitions[i][j] + 
            log_emit_i + log_beta->frames[f][i] - log_probability);
        } else {
          int last_state_from = models[model_from]->n_states-1;
          models[model_from]->transitions_acc[last_state_from][state_from] +=
            exp(log_posterior + log_alpha->frames[f-1][j] + 
            models[model_from]->log_transitions[last_state_from][state_from] +
            log_emit_i + log_beta->frames[f][i] - log_probability);
          models[model_to]->transitions_acc[state_to][0] += 
            exp(log_posterior + log_alpha->frames[f-1][j] + 
            models[model_to]->log_transitions[state_to][0] +
            log_emit_i + log_beta->frames[f][i] - log_probability);
        }
      }
    }
  }
  // particular case of transitions from initial state
  for (int j=1;j<n_states-1;j++) {
    if (log_transitions[j][0] == LOG_ZERO || log_beta->frames[0][j] == LOG_ZERO)
      continue;
    int model_to = states_to_model[j];
    int state_to = states_to_model_states[j];
    real log_emit_j = states[j]->log_probabilities->frames[0][0];
    models[model_to]->transitions_acc[state_to][0] +=
      exp(log_posterior + log_beta->frames[0][j] + log_emit_j +
      models[model_to]->log_transitions[state_to][0] -
      log_probability);
  }
  // particular case of transitions to last state
  int f = inputs->n_frames-1;
  int i = n_states-1;
  for (int j=1;j<n_states-1;j++) {
    if (log_transitions[i][j] == LOG_ZERO || log_alpha->frames[f][j] == LOG_ZERO)
      continue;
    int model_from = states_to_model[j];
    int state_from = states_to_model_states[j];
    int last_state_from = models[model_from]->n_states-1;
    models[model_from]->transitions_acc[last_state_from][state_from] +=
      exp(log_posterior + log_alpha->frames[f][j] + 
      models[model_from]->log_transitions[last_state_from][state_from] -
      log_probability);
  }
}

void SpeechHMM::viterbiAccPosteriors(Sequence *inputs, real log_posterior)
{
  // accumulate the emission and transition posteriors
  real p = exp(log_posterior);
  for (int f=0;f<inputs->n_frames;f++) {
    int i = (int)viterbi_sequence->frames[f][0];
    if (i>=0) {
      int model_to = states_to_model[i];
      int state_to = states_to_model_states[i];
      states[i]->frameEMAccPosteriors(f, inputs->frames[f], log_posterior);
      int j = (int)arg_viterbi->frames[f][i];
      // find the real transition
      if (j>=0) {
        int model_from = states_to_model[j];
        int state_from = states_to_model_states[j];
        if (model_from == model_to) {
          models[model_from]->transitions_acc[state_to][state_from] += p;
        } else {
          int last_state_from = models[model_from]->n_states-1;
          models[model_from]->transitions_acc[last_state_from][state_from] += p;
          models[model_to]->transitions_acc[state_to][0] += p;
        }
      }
    }
  }
}

void SpeechHMM::eMUpdate()
{
  // for each model
  for (int i=0;i<n_models;i++) {
    models[i]->eMUpdate();
  }
}

void SpeechHMM::backward(Sequence *inputs, Sequence *alpha)
{
  // compute the beta by backward recursion
  logBeta(inputs);

  // accumulate the emission and transition posteriors
  for (int f=0;f<inputs->n_frames;f++) {
    for (int i=1;i<n_states-1;i++) {
      if (log_alpha->frames[f][i] != LOG_ZERO && 
        log_beta->frames[f][i] != LOG_ZERO) {
        real posterior_i_f = *alpha->frames[0] * exp(log_alpha->frames[f][i] + 
          log_beta->frames[f][i] - log_probability);
        states[i]->frameBackward(f,inputs->frames[f],NULL,NULL,&posterior_i_f);
      }
    }
  }
  for (int f=1;f<inputs->n_frames;f++) {
    for (int i=1;i<n_states-1;i++) {
      real log_emit_i = states[i]->log_probabilities->frames[f][0];
      int model_to = states_to_model[i];
      int state_to = states_to_model_states[i];
      for (int j=1;j<n_states;j++) {
        if (log_transitions[i][j] == LOG_ZERO || 
          log_alpha->frames[f-1][j] == LOG_ZERO ||
          log_beta->frames[f][i] == LOG_ZERO)
          continue;
        int model_from = states_to_model[j];
        int state_from = states_to_model_states[j];
        if (model_from == model_to) {
          real posterior_i_j_f = *alpha->frames[0] * exp(log_alpha->frames[f-1][j] +
            log_transitions[i][j] + log_emit_i + log_beta->frames[f][i] - 
            log_probability);
          models[model_from]->dlog_transitions[state_to][state_from] +=
            posterior_i_j_f;
          for (int k=1;k<n_states;k++) {
            if (log_transitions[k][j] == LOG_ZERO)
              continue;
            models[model_from]->dlog_transitions[state_to][state_from] -=
              posterior_i_j_f * exp(log_transitions[k][j]);
          }
        } else {
          int last_state_from = models[model_from]->n_states-1;
          real posterior_i_j_f_from = *alpha->frames[0] * exp(log_alpha->frames[f-1][j] +
            models[model_from]->log_transitions[last_state_from][state_from] + 
            log_emit_i + log_beta->frames[f][i] - log_probability);
          real posterior_i_j_f_to = *alpha->frames[0] * exp(log_alpha->frames[f-1][j] +
            models[model_to]->log_transitions[state_to][0] + 
            log_emit_i + log_beta->frames[f][i] - log_probability);
          models[model_from]->dlog_transitions[last_state_from][state_from] +=
            posterior_i_j_f_from;
          for (int k=1;k<n_states;k++) {
            if (log_transitions[k][j] == LOG_ZERO)
              continue;
            models[model_from]->dlog_transitions[last_state_from][state_from] -=
              posterior_i_j_f_from * exp(models[model_from]->log_transitions[last_state_from][state_from]);
            models[model_to]->dlog_transitions[state_to][0] -=
              posterior_i_j_f_to * exp(models[model_to]->log_transitions[state_to][0]);
          }
        }
      }
    }
  }
  // particular case of transitions from initial state
  for (int j=1;j<n_states-1;j++) {
    if (log_transitions[j][0] == LOG_ZERO)
      continue;
    int model_to = states_to_model[j];
    int state_to = states_to_model_states[j];
    real log_emit_j = models[model_to]->states[state_to]->log_probabilities->frames[0][0];
    real posterior_i_j_f = *alpha->frames[0] * exp(log_beta->frames[0][j] +
      log_emit_j + models[model_to]->log_transitions[state_to][0] - 
      log_probability);
    models[model_to]->dlog_transitions[state_to][0] += posterior_i_j_f;
    for (int k=1;k<n_states-1;k++) {
      if (log_transitions[k][0] == LOG_ZERO)
        continue;
      int k_model_to = states_to_model[k];
      int k_state_to = states_to_model_states[k];
      models[k_model_to]->dlog_transitions[k_state_to][0] -=
        posterior_i_j_f * exp(models[k_model_to]->log_transitions[k_state_to][0]);
    }
  }
  // particular case of transitions to last state
  int f = inputs->n_frames-1;
  int i = n_states-1;
  for (int j=1;j<n_states-1;j++) {
    if (log_transitions[i][j] == LOG_ZERO ||
      log_alpha->frames[f][j] == LOG_ZERO)
      continue;
    int model_from = states_to_model[j];
    int state_from = states_to_model_states[j];
    int last_state_from = models[model_from]->n_states-1;
    real posterior_i_j_f = *alpha->frames[0] * exp(log_alpha->frames[f][j] +
      models[model_from]->log_transitions[last_state_from][state_from] -
      log_probability);
    models[model_from]->dlog_transitions[last_state_from][state_from] +=
      posterior_i_j_f;
    for (int k=1;k<n_states-1;k++) {
      if (log_transitions[k][j] == LOG_ZERO)
        continue;
      int k_model_from = states_to_model[k];
      int k_state_from = states_to_model_states[k];
      int k_last_state_from = models[k_model_from]->n_states-1;
      models[k_model_from]->dlog_transitions[k_last_state_from][k_state_from] -=
        posterior_i_j_f * exp(models[k_model_from]->log_transitions[k_last_state_from][k_state_from]);
    }
  }
}

SpeechHMM::~SpeechHMM()
{
}

}

