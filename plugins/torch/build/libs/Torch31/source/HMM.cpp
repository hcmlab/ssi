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


#include "HMM.h"
#include "FrameSelectorDataSet.h"
#include "Random.h"
#include "log_add.h"
#include "XFile.h"

namespace Torch {

	HMM::HMM(int n_states_, Distribution **states_, real** transitions_, int n_shared_states_,Distribution **shared_states_) : Distribution(states_[1]->n_inputs,n_states_*n_states_)
	{
		n_states = n_states_;
		states = states_;
		transitions = transitions_;

		// if given, these represent the "real" states, while "states" are
		// only pointers to shared_states. note that shared_states should all
		// exist (not like states which is null for states[0] and states[n_states-1])
		shared_states = shared_states_;
		n_shared_states = n_shared_states_;

		addBOption("initialize", &initialize , true, "initialize the model before training");
		addBOption("linear segmentation", &linear_segmentation, false, "linear segmentation to initialize the states");
		addROption("prior transitions", &prior_transitions , 1e-3, "minimum weights for each gaussians");

		if (n_states > 0) {

			log_transitions = (real**)allocator->alloc(sizeof(real*)*n_states);
			dlog_transitions = (real**)allocator->alloc(sizeof(real*)*n_states);
			transitions_acc = (real**)allocator->alloc(sizeof(real*)*n_states);
			for (int i=1;i<n_states-1;i++) {
				params->add(states[i]->params);
				der_params->add(states[i]->der_params);
			}
			for (int i=0;i<n_states;i++) {
				transitions_acc[i] = (real*)allocator->alloc(sizeof(real)*n_states);;
				log_transitions[i] = ((real*)params->data[0]) + i*n_states;
				dlog_transitions[i] = ((real*)der_params->data[0]) + i*n_states;
			}
			log_probabilities_s = new (allocator)Sequence(1,n_states);
			log_alpha = new (allocator)Sequence(1,n_states);
			log_beta = new (allocator)Sequence(1,n_states);
			arg_viterbi = new (allocator)Sequence(1,n_states);
			viterbi_sequence = new (allocator)Sequence(1,1);
		}
	}

	void HMM::loadXFile(XFile *file)
	{
		// first the transitions
		file->taggedRead(params->data[0], sizeof(real), n_states*n_states,"HMM");
		for (int i=0;i<n_states;i++)
			log_transitions[i] = params->data[0] + i*n_states;
		// then the emissions
		for (int i=1;i<n_states-1;i++) {
			states[i]->loadXFile(file);
		}
	}

	void HMM::saveXFile(XFile *file)
	{
		// first the transitions
		file->taggedWrite(params->data[0], sizeof(real), n_states*n_states,"HMM");
		// then the emissions
		for (int i=1;i<n_states-1;i++) {
			states[i]->saveXFile(file);
		}
	}

	void HMM::copy(HMM *other)
	{
		// first the transitions
		memcpy(params->data[0], other->params->data[0], sizeof(real)*n_states*n_states);
		for (int i=0;i<n_states;i++)
			log_transitions[i] = params->data[0] + i*n_states;

		// then the emissions
		for (int i=1; i < other->n_states-1; i++) 
		{
			if(!other->states[i]->params)
				continue;

			for(int j = 0; j < states[i]->params->n_data; j++)
				memcpy(states[i]->params->data[i], other->states[i]->params->data[i], sizeof(real) * states[i]->params->size[i]);
			
			states[i]->eMIterInitialize();
		}
	}

	void HMM::setDataSet(DataSet* data_)
	{
		if (initialize) {
			// the emission distributions
			int* selected_frames = (int*)allocator->alloc(sizeof(int)*1);
			for (int i=1;i<n_states-1;i++){
				FrameSelectorDataSet frame_sel(data_);
				int n = data_->n_examples;
				if (linear_segmentation) {
					for (int j=0;j<n;j++) {
						data_->setExample(j);
						int n_frames = data_->inputs->n_frames;
						real n_frames_per_state = (real)n_frames/(n_states-2);
						int from = (int)((i-1)*n_frames_per_state);
						int to = (i == n_states-2 ? n_frames : (int)(i*n_frames_per_state));
						int n_selected_frames = to - from;
						selected_frames = (int*)allocator->realloc(selected_frames,sizeof(int)*n_selected_frames);
						int k = 0;
						for (int l=from;l<to;l++,k++)
							selected_frames[k] = l;
						frame_sel.selectInputFrames(j,selected_frames,n_selected_frames);
					}
				} else {
					// select a bootstrap
					for (int j=0;j<n;j++) {
						data_->setExample(j);
						int n_frames = data_->inputs->n_frames;
						int n_selected_frames = n_frames;
						selected_frames = (int*)allocator->realloc(selected_frames,sizeof(int)*n_selected_frames);
						for (int l=0;l<n_frames;l++)
							selected_frames[l] = (int)floor(Random::boundedUniform(0,n_frames));
						frame_sel.selectInputFrames(j,selected_frames,n_selected_frames);
					}
				}
				states[i]->setDataSet(&frame_sel);
			}

			// for the transitions, re-initialize to initial values given in constructor
			for (int i=0;i<n_states;i++) {
				real *p = transitions[i];
				real *lp = log_transitions[i];
				for (int j=0;j<n_states;j++,lp++,p++) {
					if (*p > 0)
						*lp = log(*p);
					else
						*lp = LOG_ZERO;
				}
			}
		} else {
			// we still need to set the dataset of the emission distributions
			for (int i=1;i<n_states-1;i++) 
				states[i]->setDataSet(data_);
		}
	}

	void HMM::printTransitions(bool real_values, bool transitions_only)
	{
		print("transitions: %d x %d\n",n_states,n_states);
		for (int i=0;i<n_states;i++) {
			for (int j=0;j<n_states;j++) {
				if (transitions_only) {
					if (log_transitions[j][i] != LOG_ZERO) {
						print("%d -> %d = %f\n",i,j,exp(log_transitions[j][i]));
					}
				} else if (real_values) {
					print("%f ",exp(log_transitions[j][i]));
				} else {
					print("%d ",(log_transitions[j][i] != LOG_ZERO));
				}
			}
			print("\n");
		}
	}

	void HMM::logAlpha(Sequence* inputs)
	{
		// first, initialize everything to LOG_ZERO
		for (int f=0;f<inputs->n_frames;f++) {
			for (int i=1;i<n_states-1;i++) {
				log_alpha->frames[f][i] = LOG_ZERO;
			}
		}
		// case for first frame
		for (int i=1;i<n_states-1;i++) {
			if (log_transitions[i][0] != LOG_ZERO)
				log_alpha->frames[0][i] = log_probabilities_s->frames[0][i] + 
				log_transitions[i][0];
		}
		// other cases
		for (int f=1;f<inputs->n_frames;f++) {
			for (int i=1;i<n_states-1;i++) {
				for (int j=1;j<n_states-1;j++) {
					log_alpha->frames[f][i] = logAdd(log_alpha->frames[f][i],
						log_transitions[i][j] + 
						log_probabilities_s->frames[f][i] +
						log_alpha->frames[f-1][j]);
				}
			}
		}
		// last case
		log_probability = LOG_ZERO;
		int f = inputs->n_frames-1;
		int i = n_states-1;
		for (int j=1;j<n_states-1;j++) {
			log_probability = logAdd(log_probability,
				log_alpha->frames[f][j]+log_transitions[i][j]);
		}
	}

	void HMM::logBeta(Sequence* inputs)
	{
		// first, initialize everything to LOG_ZERO
		for (int f=0;f<inputs->n_frames;f++) {
			for (int i=1;i<n_states-1;i++) {
				log_beta->frames[f][i] = LOG_ZERO;
			}
		}
		// case for last frame
		int f_final = inputs->n_frames-1;
		for (int i=1;i<n_states-1;i++) {
			log_beta->frames[f_final][i] = log_transitions[n_states-1][i];
		}
		// other cases
		for (int f=inputs->n_frames-2;f>=0;f--) {
			for (int i=1;i<n_states-1;i++) {
				for (int j=1;j<n_states-1;j++) {
					log_beta->frames[f][j] = logAdd(log_beta->frames[f][j],
						log_transitions[i][j] +
						log_probabilities_s->frames[f+1][i] +
						log_beta->frames[f+1][i]);
				}
			}
		}
	}

	void HMM::logViterbi(Sequence* inputs)
	{
		// first, initialize everything to LOG_ZERO
		for (int f=0;f<inputs->n_frames;f++) {
			for (int i=1;i<n_states-1;i++) {
				log_alpha->frames[f][i] = LOG_ZERO;
			}
		}
		// case for first frame
		for (int i=1;i<n_states-1;i++) {
			real v = log_probabilities_s->frames[0][i] + log_transitions[i][0];
			if (v > log_alpha->frames[0][i]) {
				log_alpha->frames[0][i] = v;
				arg_viterbi->frames[0][i] = 0.0;
			}
		}
		// other cases
		for (int f=1;f<inputs->n_frames;f++) {
			for (int i=1;i<n_states-1;i++) {
				for (int j=1;j<n_states-1;j++) {
					real v = log_transitions[i][j] + log_probabilities_s->frames[f][i] +
						log_alpha->frames[f-1][j];
					if (v > log_alpha->frames[f][i]) {
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
			real v = log_alpha->frames[f][j]+log_transitions[i][j];
			if (v > log_probability) {
				log_probability = v;
				last_arg_viterbi = j;
			}
		}
		// now recall the state sequence
		if (log_probability > LOG_ZERO) {
			viterbi_sequence->frames[inputs->n_frames-1][0] = last_arg_viterbi;
			for (int f=inputs->n_frames-2;f>=0;f--) {
				viterbi_sequence->frames[f][0] = (real)(arg_viterbi->frames[f+1][(int)(viterbi_sequence->frames[f+1][0])]);
			}
		} else {
			warning("sequence impossible to train: probably too short for target");
			for (int f=0;f<inputs->n_frames;f++)
				viterbi_sequence->frames[f][0] = -1;
			log_probability = 0;
		}
	}

	void HMM::logProbabilities(Sequence *inputs)
	{
		if (n_shared_states == 0) {
			for (int f=0;f<inputs->n_frames;f++) {
				for (int i=1;i<n_states-1;i++) {
					log_probabilities_s->frames[f][i] = states[i]->frameLogProbability(f, inputs->frames[f]);
				}
			}
		} else {
			for (int f=0;f<inputs->n_frames;f++) {
				for (int i=0;i<n_shared_states;i++) {
					shared_states[i]->frameLogProbability(f, inputs->frames[f]);
				}
				for (int i=1;i<n_states-1;i++) {
					log_probabilities_s->frames[f][i] = states[i]->log_probabilities->frames[f][0];
				}
			}
		}
	}

	real HMM::logProbability(Sequence *inputs)
	{
		logProbabilities(inputs);
		logAlpha(inputs);
		log_probabilities->frames[0][0] = log_probability;
		return log_probability;
	}

	real HMM::viterbiLogProbability(Sequence *inputs)
	{
		logProbabilities(inputs);
		logViterbi(inputs);
		log_probabilities->frames[0][0] = log_probability;
		return log_probability;
	}


	void HMM::eMSequenceInitialize(Sequence* inputs)
	{
		log_probabilities_s->resize(inputs->n_frames);
		log_alpha->resize(inputs->n_frames);
		log_beta->resize(inputs->n_frames);
		arg_viterbi->resize(inputs->n_frames);
		viterbi_sequence->resize(inputs->n_frames);

		for (int i=1;i<n_states-1;i++)
			states[i]->eMSequenceInitialize(inputs);
	}

	void HMM::sequenceInitialize(Sequence* inputs)
	{
		log_probabilities_s->resize(inputs->n_frames);
		log_alpha->resize(inputs->n_frames);
		log_beta->resize(inputs->n_frames);
		arg_viterbi->resize(inputs->n_frames);
		viterbi_sequence->resize(inputs->n_frames);

		for (int i=1;i<n_states-1;i++)
			states[i]->sequenceInitialize(inputs);
	}

	void HMM::eMIterInitialize()
	{
		for (int i=1;i<n_states-1;i++)
			states[i]->eMIterInitialize();
		for (int i=0;i<n_states;i++)
			for (int j=0;j<n_states;j++)
				transitions_acc[i][j] = prior_transitions;
	}

	void HMM::iterInitialize()
	{
		for (int i=1;i<n_states-1;i++)
			states[i]->iterInitialize();
		for (int i=0;i<n_states;i++)
			for (int j=0;j<n_states;j++)
				transitions_acc[i][j] = prior_transitions;
	}

	void HMM::eMAccPosteriors(Sequence *inputs, real log_posterior)
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
						states[i]->frameEMAccPosteriors(f, inputs->frames[f],log_posterior_i_f);
				}
			}
		}
		for (int f=1;f<inputs->n_frames;f++) {
			for (int i=1;i<n_states-1;i++) {
				real log_emit_i = log_probabilities_s->frames[f][i];
				for (int j=1;j<n_states-1;j++) {
					if (log_transitions[i][j] != LOG_ZERO && log_alpha->frames[f-1][j] != LOG_ZERO && log_beta->frames[f][i] != LOG_ZERO && log_emit_i != LOG_ZERO)
						transitions_acc[i][j] += exp(log_posterior + 
						log_alpha->frames[f-1][j] + 
						log_transitions[i][j] + log_emit_i + log_beta->frames[f][i] - 
						log_probability);
				}
			}
		}
		// particular case of transitions from initial state
		for (int j=1;j<n_states-1;j++) {
			if (log_transitions[j][0] != LOG_ZERO && log_beta->frames[0][j] != LOG_ZERO && log_probabilities_s->frames[0][j] != LOG_ZERO)
				transitions_acc[j][0] += exp(log_posterior + log_beta->frames[0][j] + 
				log_probabilities_s->frames[0][j] +
				log_transitions[j][0] - log_probability);
		}
		// particular case of transitions to last state
		int f = inputs->n_frames-1;
		int i = n_states-1;
		for (int j=1;j<n_states-1;j++) {
			if (log_transitions[i][j] != LOG_ZERO && log_alpha->frames[f][j] != LOG_ZERO)
				transitions_acc[i][j] += exp(log_posterior + log_alpha->frames[f][j] + 
				log_transitions[i][j] - log_probability);
		}
	}

	void HMM::viterbiAccPosteriors(Sequence *inputs, real log_posterior)
	{
		// accumulate the emission and transition posteriors
		real p = exp(log_posterior);
		for (int f=0;f<inputs->n_frames;f++) {
			int i = (int)(viterbi_sequence->frames[f][0]);
			if (i>=0) {
				states[i]->frameEMAccPosteriors(f, inputs->frames[f],log_posterior);
				int j = (int)arg_viterbi->frames[f][i];
				if (j>0) {
					transitions_acc[i][j] += p;
				}
			}
		}
		// attention, il me manque le premier et le dernier arg_viterbi...
	}

	void HMM::eMUpdate()
	{
		// first the states
		for (int i=1;i<n_states-1;i++) {
			states[i]->eMUpdate();
		}
		// then the transitions;
		for (int i=0;i<n_states-1;i++) {
			real log_sum = 0;
			for (int j=1;j<n_states;j++) {
				if (log_transitions[j][i] != LOG_ZERO)
					log_sum += transitions_acc[j][i];
			}
			log_sum = log(log_sum);
			for (int j=0;j<n_states;j++) {
				if (log_transitions[j][i] != LOG_ZERO) {
					log_transitions[j][i] = log(transitions_acc[j][i]) - log_sum;
				}
			}
		}
	}

	void HMM::update()
	{
		// first the states
		for (int i=1;i<n_states-1;i++) {
			states[i]->update();
		}
		// then the transitions;
		for (int i=0;i<n_states-1;i++) {
			real log_sum = LOG_ZERO;
			for (int j=1;j<n_states;j++) {
				if (log_transitions[j][i] != LOG_ZERO)
					log_sum += logAdd(log_sum,log_transitions[j][i]);
			}
			for (int j=0;j<n_states;j++) {
				if (log_transitions[j][i] != LOG_ZERO)
					log_transitions[j][i] -= log_sum;
			}
		}
	}


	void HMM::backward(Sequence *inputs, Sequence *alpha)
	{
		// compute the beta by backward recursion
		logBeta(inputs);

		// accumulate the emission posteriors
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
		// accumulate the transition posteriors
		for (int f=1;f<inputs->n_frames;f++) {
			for (int i=1;i<n_states-1;i++) {
				real log_emit_i = log_probabilities_s->frames[f][i];
				for (int j=1;j<n_states;j++) {
					if (log_transitions[i][j] == LOG_ZERO || log_alpha->frames[f-1][j] == LOG_ZERO || log_emit_i == LOG_ZERO || log_beta->frames[f][i] == LOG_ZERO)
						continue;
					real posterior_i_j_f = *alpha->frames[0] * exp(log_alpha->frames[f-1][j] + 
						log_transitions[i][j] + log_emit_i + 
						log_beta->frames[f][i] - log_probability);
					dlog_transitions[i][j] += posterior_i_j_f;
					for (int k=1;k<n_states;k++) {
						if (log_transitions[k][j] != LOG_ZERO)
							dlog_transitions[k][j] -= posterior_i_j_f * exp(log_transitions[k][j]);
					}
				}
			}
		}
		// particular case of transitions from initial state
		for (int j=1;j<n_states-1;j++) {
			if (log_transitions[j][0] == LOG_ZERO || log_beta->frames[0][j] == LOG_ZERO || log_probabilities_s->frames[0][j] == LOG_ZERO)
				continue;
			real posterior_i_j_f = *alpha->frames[0] * exp(log_beta->frames[0][j] +
				log_probabilities_s->frames[0][j] + log_transitions[j][0] - 
				log_probability);
			dlog_transitions[j][0] += posterior_i_j_f;
			for (int k=1;k<n_states;k++) {
				if (log_transitions[k][0] != LOG_ZERO)
					dlog_transitions[k][0] -= posterior_i_j_f * exp(log_transitions[k][0]);
			}
		}
		// particular case of transitions to last state
		int f = inputs->n_frames-1;
		int i = n_states-1;
		for (int j=1;j<n_states-1;j++) {
			if (log_transitions[i][j] == LOG_ZERO || log_alpha->frames[f][j] == LOG_ZERO)
				continue;
			real posterior_i_j_f = *alpha->frames[0] * exp(log_alpha->frames[f][j] +
				log_transitions[i][j] - log_probability);
			dlog_transitions[i][j] += posterior_i_j_f;
			for (int k=1;k<n_states;k++) {
				if (log_transitions[k][j] != LOG_ZERO)
					dlog_transitions[k][j] -= posterior_i_j_f * exp(log_transitions[k][j]);
			}
		}
	}

	void HMM::viterbiBackward(Sequence *inputs, Sequence *alpha)
	{
		// accumulate the emission and transition posteriors
		for (int f=0;f<=inputs->n_frames;f++) {
			int i = (int)viterbi_sequence->frames[f][0];
			if (f<inputs->n_frames) {
				states[i]->frameBackward(f, inputs->frames[f], NULL, NULL, alpha->frames[0]);
			}
			int j = (int)arg_viterbi->frames[f][i];
			dlog_transitions[i][j] -= *alpha->frames[0];
			for (int k=1;k<n_states;k++) {
				if (log_transitions[k][j] != LOG_ZERO)
					dlog_transitions[k][j] += *alpha->frames[0] *exp(log_transitions[k][j]);
			}
		}
	}

	void HMM::decode(Sequence* inputs)
	{
		eMSequenceInitialize(inputs);
		logProbabilities(inputs);
		logViterbi(inputs);
	}


	HMM::~HMM()
	{
	}

}

