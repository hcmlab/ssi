// TorchHMM.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/06/11
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute itand/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#include "TorchHMM.h"

#include "TorchTools.h"
#include "Random.h"
#include "NullXFile.h"
#include "KMeans.h"
#include "NLLMeasurer.h"
#include "MeanVarNorm.h"
#include "KFold.h"
#include "HMM.h"
#include "EMTrainer.h"
#include "DiskXFile.h"
#include "MemoryDataSet.h"

namespace ssi {

TorchHMM::TorchHMM (const ssi_char_t *file) 
	: _n_classes (0),
	_n_features (0),
	_n_hmms (0),
	_hmms (0), 
	_file (0),
	_n_state_sequence (0),
	_state_sequence (0) {	

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
}

TorchHMM::~TorchHMM () {

	release ();

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}

bool TorchHMM::train (ISamples &samples,
	ssi_size_t stream_index) {

	if (samples.getSize () == 0) {
		ssi_wrn ("empty sample list");
		return false;
	}

	if (isTrained ()) {
		ssi_wrn ("already trained");
		return false;
	}

	_n_classes = samples.getClassSize ();
	_n_features = samples.getStream (stream_index).dim;
	
	// init random
	Torch::Random::seed ();

	// output file
	Torch::NullXFile logfile;

	// train a single ergodic hmm with one emitting state per class
	if (_options.single) {

		_n_hmms = 1;
		_hmms = new Torch::HMM *[_n_hmms];
		_options.n_states = _n_classes + 2;
		_options.connectivity = ERGODIC;

		// convert samples
		Torch::MemoryDataSet *data_set = TorchTools::CreateMemoryDataSet (samples, 0, samples.supportsShallowCopy ());
		
		// create a KMeans object to initialize the GMM
		Torch::KMeans kmeans (data_set->n_inputs, _options.n_gaussians);
		kmeans.setROption ("prior weights", _options.prior);

		// the kmeans trainer
		Torch::EMTrainer kmeans_trainer (&kmeans);
		kmeans_trainer.setROption ("end accuracy", _options.accuracy);
		kmeans_trainer.setIOption ("max iter", _options.max_iter_kmeans);

		// the kmeans measurer
		Torch::MeasurerList kmeans_measurers;
		Torch::NLLMeasurer nll_kmeans_measurer (kmeans.log_probabilities, data_set, &logfile);
		kmeans_measurers.addNode (&nll_kmeans_measurer);

		// init hmm
		Torch::HMM *hmm = initHMM (_options.n_states, _options.n_gaussians, data_set->n_inputs, _options.connectivity, _options.prior, &kmeans_trainer);

		// init each state with samples of one class
		Torch::Allocator *allocator = new Torch::Allocator ();
		int* selected_frames = (int*)allocator->alloc(sizeof(int)*1);
		for (int i = 1; i < _options.n_states-1; i++) {
			Torch::DiagonalGMM *gmm = (Torch::DiagonalGMM *) hmm->states[i];
			Torch::MemoryDataSet *data_set_class = TorchTools::CreateMemoryDataSet (samples, 0, i-1, samples.supportsShallowCopy ());
			gmm->setDataSet (data_set_class);			
			gmm->initial_kmeans_trainer = 0;
			gmm->initial_kmeans_trainer_measurers = 0;			
			delete data_set_class;
		}

		// now train model once
		retrain (hmm, *data_set, logfile);

		// and retrain
		for (int j = 0; j < _options.retrain; j++) {
			retrain (hmm, *data_set, logfile);
		}		

		// store model
		_hmms[0] = hmm;

		// clean up
		delete data_set;		

	// train one hmm per class
	} else {
		
		_n_hmms = _n_classes;
		_hmms = new Torch::HMM *[_n_hmms];

		for (unsigned int i = 0; i < _n_hmms; i++) {

			_hmms[i] = 0;

			// convert samples
			Torch::MemoryDataSet *data_set = TorchTools::CreateMemoryDataSet (samples, stream_index, i, samples.supportsShallowCopy ());

			// create a KMeans object to initialize the GMM
			Torch::KMeans kmeans (data_set->n_inputs, _options.n_gaussians);
			kmeans.setROption ("prior weights", _options.prior);

			// the kmeans trainer
			Torch::EMTrainer kmeans_trainer (&kmeans);
			kmeans_trainer.setROption ("end accuracy", _options.accuracy);
			kmeans_trainer.setIOption ("max iter", _options.max_iter_kmeans);

			// the kmeans measurer
			Torch::MeasurerList kmeans_measurers;
			Torch::NLLMeasurer nll_kmeans_measurer (kmeans.log_probabilities, data_set, &logfile);
			kmeans_measurers.addNode (&nll_kmeans_measurer);

			// create the HMM
			Torch::HMM *hmm = initHMM (_options.n_states, _options.n_gaussians, data_set->n_inputs, _options.connectivity, _options.prior, &kmeans_trainer);

			// now train model once
			retrain (hmm, *data_set, logfile);

			// next time we dont have to train with kmeans again
			for (int i = 1; i < _options.n_states-1; i++) {
				((Torch::DiagonalGMM *) hmm->states[i])->initial_kmeans_trainer = 0;
				((Torch::DiagonalGMM *) hmm->states[i])->initial_kmeans_trainer_measurers = 0;
			}

			// and retrain
			for (int j = 0; j < _options.retrain; j++) {
				retrain (hmm, *data_set, logfile);
			}		

			// store model
			_hmms[i] = hmm;

			// clean up
			delete data_set;
		}
	}

	return true;
}


void TorchHMM::retrain (Torch::HMM *hmm, Torch::DataSet &data, Torch::XFile &logfile) {

	// temporal allocator
	Torch::Allocator *allocator = new Torch::Allocator ();
	
	// GMM thresholds
	ssi_real_t* thresh = (ssi_real_t*) hmm->allocator->alloc (hmm->n_inputs * sizeof (ssi_real_t));
	TorchTools::InitThreshold (&data, thresh, _options.threshold);	
	for (int i = 1; i < _options.n_states-1; i++) {
		((Torch::DiagonalGMM *) hmm->states[i])->setVarThreshold (thresh);
		((Torch::DiagonalGMM *) hmm->states[i])->setROption ("prior weights", _options.prior);
	}

	// Measurers on the training dataset
	Torch::MeasurerList measurers;
	Torch::NLLMeasurer nll_meas (hmm->log_probabilities, &data, &logfile);
	measurers.addNode (&nll_meas);

	// The Gradient Machine Trainer
	Torch::EMTrainer trainer (hmm);
	trainer.setIOption ("max iter", _options.max_iter_hmm);
	trainer.setROption ("end accuracy", _options.accuracy);
	trainer.setBOption ("viterbi", true);

	//hmm->display ();
	trainer.train(&data, &measurers);

	delete allocator;
}

// help function to set up transition matrix
void TorchHMM::setTransitions (ssi_real_t** transitions, 
	int n_states, 
	HMM_CONNECTIVITY connectivity) {
	
	// set all transitions to 0
	for (int i=0;i<n_states;i++) {
		for (int j=0;j<n_states;j++) {
			transitions[i][j] = 0;
		}
	}

	switch (connectivity) {

		// linear connectivity
		case LINEAR: {

			transitions[1][0] = 1.0f;
			for (int i=1;i<n_states-1;i++) {
				transitions[i][i] = 0.5f;
				transitions[i+1][i] = 0.5f;
			}
			break;
		}

		// left right connectivity
		case LEFTRIGHT: {

			for (int i=1;i<n_states-1;i++) {
				transitions[i][0] = (ssi_real_t) 1./(n_states-2);
				for (int j=i;j<n_states;j++) {
					transitions[j][i] = (ssi_real_t) 1./(n_states-i);
				}
			}
			break;
		}

		// full connectivity
		case ERGODIC: {
			
			for (int i=1;i<n_states-1;i++) {
				transitions[i][0] = (ssi_real_t) 1./(n_states-2);
				for (int j=1;j<n_states;j++) {
					transitions[j][i] = (ssi_real_t) 1./(n_states-1);
				}
			}
			break;
		}

		default:
			SSI_ASSERT (false);
	}
}

bool TorchHMM::forward (ssi_stream_t &stream,
	ssi_size_t n_probs,
	ssi_real_t *probs) {

	if (!_hmms) {
		ssi_wrn ("not trained");
		return false;
	}

	if (n_probs != _n_classes) {
		ssi_wrn ("#classes differs");
		return false;
	}

	if (stream.type != SSI_REAL) {
		ssi_wrn ("type differs");
		return false;
	}

	if (stream.dim != _n_features) {
		ssi_wrn ("feature dimension differs");
	}	
	
	Torch::Sequence *sequence = TorchTools::CreateSequence (stream);

	if (_options.single) {

		for (ssi_size_t i = 0; i < n_probs; i++) {
			probs[i] = 0;
		}

		_hmms[0]->decode (sequence);	
		delete[] _state_sequence;
		_n_state_sequence = _hmms[0]->arg_viterbi->n_frames - 2;
		_state_sequence = new ssi_size_t[_n_state_sequence];
		for (ssi_size_t i =	0; i < _n_state_sequence; i++) {
			_state_sequence[i] = ssi_cast (ssi_size_t, _hmms[0]->arg_viterbi->frames[i+1][1]) - 1;
			probs[_state_sequence[i]]++;
		}

		for (ssi_size_t i = 0; i < n_probs; i++) {
			probs[i] /= _n_state_sequence;
		}

	} else {
	
		ssi_real_t prob, prob_sum = 0;
		for (ssi_size_t i = 0; i < _n_classes; i++) {
			// reset gmm (necessary that frame number will be adapted)
			_hmms[i]->sequenceInitialize (sequence);
			// now calculate how well sequence fits to the model
			probs[i] = _hmms[i]->logProbability (sequence);
		}			
		// find minimum
		ssi_real_t minval = probs[0];
		for (ssi_size_t i = 1; i < _n_classes; i++) {
			minval = probs[i] < minval ? probs[i] : minval;			
		}
		// add abs(minval) if < 0
		if (minval < 0) {
			for (ssi_size_t i = 0; i < _n_classes; i++) {
				probs[i] += abs (minval);			
			}
		}
		// sum
		ssi_real_t sumval = probs[0];
		for (ssi_size_t i = 1; i < _n_classes; i++) {
			sumval += probs[i] ;			
		}
		// normalize
		for (ssi_size_t i = 0; i < _n_classes; i++) {
			probs[i] /= sumval;			
		}
	}

	delete sequence;

	return true;
}

// creates an HMM
Torch::HMM *TorchHMM::initHMM (int n_states, 
	int n_gaussians, 
	int n_inputs, 
	HMM_CONNECTIVITY connectivity, 
	ssi_real_t prior, 
	Torch::EMTrainer *kmeans_trainer) {

	// temporal allocator
	Torch::Allocator *allocator = new Torch::Allocator ();

	// create the GMM pool
	Torch::DiagonalGMM** gmms = (Torch::DiagonalGMM **) allocator->alloc (sizeof (Torch::DiagonalGMM*) * n_states);
	for (int i = 1; i < n_states-1; i++) {
		// create GMM
		Torch::DiagonalGMM* gmm = new (allocator) Torch::DiagonalGMM (n_inputs, n_gaussians, kmeans_trainer);
		gmms[i] = gmm;
	}

	// note that HMMs have two non-emitting states: the initial and final states
	gmms[0] = 0;
	gmms[n_states-1] = 0;

	// we create the transition matrix with initial transition probabilities
	ssi_real_t** transitions = (ssi_real_t**) allocator->alloc (n_states * sizeof(ssi_real_t*));
	for (int i=0; i < n_states; i++) {
		transitions[i] = (ssi_real_t*) allocator->alloc (n_states * sizeof(ssi_real_t));
	}

	// ... the left_right transition matrix
	setTransitions (transitions, n_states, connectivity);
	Torch::HMM *hmm = new Torch::HMM (n_states, (Torch::Distribution**) gmms, transitions);
	hmm->setROption("prior transitions",prior);
	hmm->setBOption ("linear segmentation", !connectivity);

	// now overtake allocation handling and delete allocator
	hmm->allocator->steal (allocator);
	delete allocator;

	return hmm;
}

bool TorchHMM::load (const ssi_char_t *filepath) {

	Torch::DiskXFile xfile (filepath, "rb");

	release ();

	// read parameters
	xfile.taggedRead (&_options.single, sizeof (_options.single), 1, "single");
	xfile.taggedRead (&_options.n_gaussians, sizeof (_options.n_gaussians), 1, "n_gaussians");
	xfile.taggedRead (&_options.n_states, sizeof (_options.n_states), 1, "n_states");
	xfile.taggedRead (&_options.connectivity, sizeof (_options.connectivity), 1, "connectivity");
	xfile.taggedRead (&_options.prior, sizeof (_options.prior), 1, "prior");
	xfile.taggedRead (&_n_classes, sizeof (_n_classes), 1, "n_classes");
	xfile.taggedRead (&_n_features, sizeof (_n_features), 1, "n_features");
	xfile.taggedRead (&_n_hmms, sizeof (_n_hmms), 1, "n_hmms");

	// create HMM model
	_hmms = new Torch::HMM *[_n_hmms];
	for (unsigned int i = 0; i < _n_hmms; i++) {
		_hmms[i] = initHMM (_options.n_states, _options.n_gaussians, _n_features, _options.connectivity, _options.prior, 0);
		_hmms[i]->loadXFile (&xfile);
	}

	return true;
}

bool TorchHMM::save (const ssi_char_t *filepath) {

	if (!_hmms) {
		ssi_wrn ("not trained");
		return false;
	}

	Torch::DiskXFile xfile (filepath, "wb");

	// store parameters
	xfile.taggedWrite (&_options.single, sizeof (_options.single), 1, "single");
	xfile.taggedWrite (&_options.n_gaussians, sizeof (_options.n_gaussians), 1, "n_gaussians");
	xfile.taggedWrite (&_options.n_states, sizeof (_options.n_states), 1, "n_states");
	xfile.taggedWrite (&_options.connectivity, sizeof (HMM_CONNECTIVITY), 1, "connectivity");
	xfile.taggedWrite (&_options.prior, sizeof (_options.prior), 1, "prior");
	xfile.taggedWrite (&_n_classes, sizeof (_n_classes), 1, "n_classes");
	xfile.taggedWrite (&_n_features, sizeof (_n_features), 1, "n_features");
	xfile.taggedWrite (&_n_hmms, sizeof (_n_hmms), 1, "n_hmms");

	// store models
	for (unsigned int i = 0; i < _n_hmms; i++) {
		_hmms[i]->saveXFile (&xfile);
	}

	return true;
}

void TorchHMM::release () {

	for (unsigned int i = 0; i < _n_hmms; i++) {
		delete _hmms[i];
	}
	delete[] _hmms; _hmms = 0;
	_n_hmms = 0;
	_n_classes = 0;
	_n_features = 0;
	_n_state_sequence = 0;
	delete[] _state_sequence; _state_sequence = 0;
}

void TorchHMM::printTransMat () {

	SSI_ASSERT (_hmms);

	for (unsigned int i = 0; i < _n_hmms; i++) {
		_hmms[i]->printTransitions (true);
	}
}


}
