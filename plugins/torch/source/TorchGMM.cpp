// TorchGMM.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/02/27
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

#include "TorchGMM.h"

#include "TorchTools.h"
#include "Random.h"
#include "NullXFile.h"
#include "DiskXFile.h"
#include "KMeans.h"
#include "EMTrainer.h"
#include "NLLMeasurer.h"
#include "MeanVarNorm.h"
#include "DiagonalGMM.h"
#include "MemoryDataSet.h"

namespace ssi {

TorchGMM::TorchGMM (const ssi_char_t *file) 
	: _n_classes (0),
	_n_features (0),
	_gmms (0),
	_file (0) {	

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
}

TorchGMM::~TorchGMM () {

	release ();

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

bool TorchGMM::train (ISamples &samples, ssi_size_t stream_index) {

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
	_gmms = new Torch::DiagonalGMM *[_n_classes];

	Torch::Random::seed ();

	// output file
	Torch::NullXFile logfile;

	for (ssi_size_t i = 0; i < _n_classes; i++) {

		_gmms[i] = 0;

		// create data set
		if (samples.getSize (i) == 0) {
			ssi_wrn ("no samples found for class %u", i);
			return false;
		}

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

		// create the GMM
		Torch::DiagonalGMM *gmm = new Torch::DiagonalGMM (data_set->n_inputs, _options.n_gaussians, &kmeans_trainer);

		// set the training _options
		gmm->setOOption("initial kmeans trainer measurers", &kmeans_measurers);

		// next time we dont have to train with kmeans again
		gmm->initial_kmeans_trainer = NULL;
		gmm->initial_kmeans_trainer_measurers = NULL;

		// set training _options
		ssi_real_t* thresh = (ssi_real_t*) data_set->allocator->alloc (data_set->n_inputs * sizeof (ssi_real_t));
		TorchTools::InitThreshold (data_set, thresh, _options.threshold);	
		gmm->setVarThreshold (thresh);
		gmm->setROption("prior weights", _options.prior);

		// Measurers on the training dataset
		Torch::MeasurerList measurers;
		Torch::NLLMeasurer nll_meas (gmm->log_probabilities, data_set, &logfile);
		measurers.addNode (&nll_meas);

		// The Gradient Machine Trainer
		Torch::EMTrainer trainer (gmm);
		trainer.setIOption ("max iter", _options.max_iter_gmm);
		trainer.setROption ("end accuracy", _options.accuracy);

		for (int j = 0; j < _options.retrain; j++) {
			trainer.train (data_set, &measurers);
		}

		delete data_set;

		_gmms[i] = gmm;
	}

	return true;
}

bool TorchGMM::forward(ssi_stream_t &stream,
	ssi_size_t n_probs,
	ssi_real_t *probs,
	ssi_real_t &confidence, ssi_video_params_t &params)
{
	return forward(stream, n_probs, probs, confidence);
}



bool TorchGMM::forward (ssi_stream_t &stream,
	ssi_size_t n_probs,
	ssi_real_t *probs,
	ssi_real_t &confidence) {

	if (!_gmms) {
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
		return false;
	}

	ssi_size_t sample_dimension = stream.dim;
	ssi_size_t sample_number = stream.num;
	ssi_real_t *sample_data = ssi_pcast (ssi_real_t, stream.ptr);

	Torch::Sequence *sequence = TorchTools::CreateSequence (sample_dimension, sample_number, sample_data);
	
	ssi_real_t *probptr = probs;
	ssi_real_t prob, prob_sum = 0;
	for (ssi_size_t i = 0; i < _n_classes; i++) {
		// reset gmm (necessary that frame number will be adapted)
		_gmms[i]->sequenceInitialize (sequence);
		// now calculate how well sequence fits to the model
		prob = _gmms[i]->logProbability (sequence);
		prob_sum += prob;
		*probptr++ = prob;
	}
	// normalize
	/*probptr = probs;
	for (ssi_size_t i = 0; i < _n_classes; i++) {
		*probptr++ /= prob_sum;
	}*/

	ssi_max(n_probs, 1, probs, &confidence);

	delete sequence;

	return true;
}

bool TorchGMM::load (const ssi_char_t *filepath) {

	Torch::DiskXFile xfile (filepath, "rb");

	release ();

	// read parameters
	xfile.taggedRead (&_options.n_gaussians, sizeof (_options.n_gaussians), 1, "n_gaussians");
	xfile.taggedRead (&_n_classes, sizeof (_n_classes), 1, "n_classes");
	xfile.taggedRead (&_n_features, sizeof (_n_features), 1, "n_inputs");

	// create GMM model
	_gmms = new Torch::DiagonalGMM *[_n_classes];
	for (unsigned int i = 0; i < _n_classes; i++) {
		_gmms[i] = new Torch::DiagonalGMM (_n_features, _options.n_gaussians);
		_gmms[i]->loadXFile (&xfile);
	}

	return true;
}

bool TorchGMM::save (const ssi_char_t *filepath) {

	if (!_gmms) {
		ssi_wrn ("not trained");
		return false;
	}

	Torch::DiskXFile xfile (filepath, "wb");

	// store parameters
	xfile.taggedWrite (&_options.n_gaussians, sizeof (ssi_size_t), 1, "n_gaussians");
	xfile.taggedWrite (&_n_classes, sizeof (ssi_size_t), 1, "n_classes");
	xfile.taggedWrite (&_gmms[0]->n_inputs, sizeof (ssi_size_t), 1, "n_inputs");

	// store models
	for (unsigned int i = 0; i < _n_classes; i++) {
		_gmms[i]->saveXFile (&xfile);
	}

	return true;
}

void TorchGMM::release () {
	
	for (ssi_size_t i = 0; i < _n_classes; i++) {
		delete _gmms[i];
	}
	delete[] _gmms;
	_gmms = 0;
	_n_classes = 0;
	_n_features = 0;
}

void TorchGMM::print (FILE *file) {

	for (ssi_size_t i = 0; i < _n_classes; i++) {
		for (int j = 0; j < _options.n_gaussians; j++) {
			fprintf (file, "gaussian #%d\n", j);
			fprintf (file, "mean\n");
			for (int k = 0; k < _gmms[i]->n_inputs; k++) {
				fprintf (file, "%.2f ", _gmms[i]->means[j][k]);
			}
			fprintf (file, "var\n");
			for (int k = 0; k < _gmms[i]->n_inputs; k++) {
				fprintf (file, "%.2f ", _gmms[i]->means[j][k]);
			}
			fprintf (file, "\n");
		}
		fprintf (file, "\n");
	}
}

}
