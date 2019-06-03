// TorchKMeans.cpp
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

#include "TorchKMeans.h"

#include "TorchTools.h"
#include "Random.h"
#include "NullXFile.h"
#include "DiskXFile.h"
#include "KMeans.h"
#include "EMTrainer.h"
#include "NLLMeasurer.h"
#include "MeanVarNorm.h"
#include "KMeans.h"
#include "MemoryDataSet.h"

namespace ssi {

TorchKMeans::TorchKMeans (const ssi_char_t *file) 
	: _kmeans (0),
	_centers (0),
	_n_features (0),
	_file (0) {	

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
}

TorchKMeans::~TorchKMeans () {

	release ();

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

bool TorchKMeans::train (ISamples &samples, ssi_size_t stream_index) {

	if (samples.getSize () == 0) {
		ssi_wrn ("empty sample list");
		return false;
	}

	if (isTrained ()) {
		ssi_wrn ("already trained");
		return false;
	}

	_n_features = samples.getStream (stream_index).dim;
		
	Torch::Random::seed ();
	Torch::NullXFile logfile;
	
	Torch::MemoryDataSet *data_set = TorchTools::CreateMemoryDataSet (samples, stream_index, samples.supportsShallowCopy ());

	// create kmeans to initialize the GMM
	_kmeans = new Torch::KMeans (data_set->n_inputs, _options.n_cluster);
	_kmeans->setROption ("prior weights", _options.prior);

	// the kmeans trainer
	Torch::EMTrainer trainer (_kmeans);
	trainer.setROption ("end accuracy", _options.accuracy);
	trainer.setIOption ("max iter", _options.max_iter_kmeans);

	// the kmeans measurer
	Torch::MeasurerList measurers;
	Torch::NLLMeasurer nll_measurer (_kmeans->log_probabilities, data_set, &logfile);
	measurers.addNode (&nll_measurer);

	// The Gradient Machine Trainer
	trainer.train (data_set, &measurers);
	_centers = _kmeans->means;

	delete data_set;

	return true;
}

// help function


bool TorchKMeans::forward(ssi_stream_t &stream,
	ssi_size_t n_probs,
	ssi_real_t *probs,
	ssi_real_t &confidence, ssi_video_params_t &params)
{
	return forward(stream, n_probs, probs, confidence);
}


bool TorchKMeans::forward (ssi_stream_t &stream,
	ssi_size_t n_probs,
	ssi_real_t *probs,
	ssi_real_t &confidence) {

	if (!_kmeans) {
		ssi_wrn ("not trained");
		return false;
	}

	if (n_probs != _options.n_cluster) {
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

	ssi_real_t *probsptr = probs;
	for (int i = 0; i < _options.n_cluster; i++) {
		*probsptr++ = 1.0f / _kmeans->frameLogProbabilityOneGaussian (i, ssi_pcast (ssi_real_t, stream.ptr));
	}

	ssi_max(n_probs, 1, probs, &confidence);

	return true;
}

bool TorchKMeans::load (const ssi_char_t *filepath) {

	Torch::DiskXFile xfile (filepath, "rb");

	release ();

	// read parameters
	xfile.taggedRead (&_options.n_cluster, sizeof (_options.n_cluster), 1, "n_cluster");
	xfile.taggedRead (&_n_features, sizeof (_n_features), 1, "n_features");

	// create GMM model
	_kmeans = new Torch::KMeans (_n_features, _options.n_cluster);
	_centers = _kmeans->means;
	
	// load model
	_kmeans->loadXFile (&xfile);

	return true;
}


bool TorchKMeans::save (const ssi_char_t *filepath) {

	if (!_kmeans) {
		ssi_wrn ("not trained");
		return false;
	}

	Torch::DiskXFile xfile (filepath, "wb");

	// store parameters
	xfile.taggedWrite (&_options.n_cluster, sizeof (_options.n_cluster), 1, "n_cluster");
	xfile.taggedWrite (&_n_features, sizeof (_n_features), 1, "n_features");

	// store model 
	_kmeans->saveXFile (&xfile);

	return true;
}

void TorchKMeans::release () {

	delete _kmeans;
	_kmeans = 0;
	_centers = 0;
	_n_features = 0;
}

}
