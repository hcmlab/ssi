// KNearestNeighbors.cpp
// author: Frank Jung <frank.jung@informatik.uni-augsburg.de>
// created: 2009/03/23
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

#include "KNearestNeighbors.h"
#include "FindNN.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

KNearestNeighbors::KNearestNeighbors (const ssi_char_t *file) 
	: _n_samples (0),	
	_n_features (0),
	_data (0),
	_classes (0),
	_file (0) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
} 

KNearestNeighbors::~KNearestNeighbors () { 

	release ();
	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}

void KNearestNeighbors::release () 
{
	if (_data && _classes) {
		ModelTools::ReleaseSampleMatrix (_n_samples, _classes, _data);
	}
	_data = 0;
	_classes = 0;
	_n_features = 0;
	_n_samples = 0;
	_n_classes = 0;
}

bool KNearestNeighbors::train (ISamples &samples,
	ssi_size_t stream_index) {

	if (samples.getSize () == 0) {
		ssi_wrn ("empty sample list");
		return false;
	}

	if (samples.getSize () < _options.k) {
		ssi_wrn ("sample list has less than '%u' entries", _options.k);
		return false;
	}

	if (isTrained ()) {
		ssi_wrn ("already trained");
		return false;
	}

	_n_classes = samples.getClassSize ();
	ModelTools::CreateSampleMatrix (samples, stream_index, _n_samples, _n_features, &_classes, &_data);

	return true;
}

bool KNearestNeighbors::forward (ssi_stream_t &stream,
	ssi_size_t n_probs,
	ssi_real_t *probs) {

	if (!_data) {
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

	ssi_size_t k = _options.k;
	ssi_size_t *indices = new ssi_size_t[k];
	ssi_real_t *distances = new ssi_real_t[k];
	ssi_real_t *sample = ssi_pcast (ssi_real_t, stream.ptr);

	// find k nearest neighbors

	FindNN::Find (_n_samples, _n_features, sample, _data, k, indices, distances);
	
	// map indices to classes
	ssi_size_t *nearest = new ssi_size_t[k];
	for (ssi_size_t i = 0; i < k; i++) {
		nearest[i] = _classes[indices[i]];
	}

	// count nn
	ssi_size_t *counter = new ssi_size_t[_n_classes];
	for (ssi_size_t i = 0; i < n_probs; i++) {		
		counter[i] = 0;
	}
	for (ssi_size_t i = 0; i < k; i++) {
		counter[nearest[i]]++;
	}

	if (_options.distsum) {

		// instead of just counting the neighbors per class we calculate their average distance
		
		bool all_in_one = false;
		for (ssi_size_t i = 0; i < n_probs; i++) {
			probs[i] = 0;
			if (counter[i] == k) {
				probs[i] = 1.0f;				
				all_in_one = true;
			}
		}

		if (!all_in_one) {

			ssi_real_t *avgdist = new ssi_real_t[_n_classes];
			ssi_real_t distsum = 0;
			for (ssi_size_t i = 0; i < _n_classes; i++) {		
				avgdist[i] = 0;
			}
			for (ssi_size_t i = 0; i < k; i++) {			
				avgdist[nearest[i]] += distances[i] / counter[nearest[i]];			
			}
			for (ssi_size_t i = 0; i < _n_classes; i++) {			
				distsum += avgdist[i];
			}
			ssi_real_t probsum = 0;
			for (ssi_size_t i = 0; i < _n_classes; i++) {
				if (counter[i] == 0) {
					probs[i] = 0.0f;
				} else {
					probs[i] = 1.0f - avgdist[i] / distsum;				
				}
				probsum += probs[i];
			}
			for (ssi_size_t i = 0; i < _n_classes; i++) {
				probs[i] /= probsum;
			}

			delete[] avgdist;
		}

	} else {
	
		for (ssi_size_t i = 0; i < n_probs; i++) {
			probs[i] = ssi_cast (ssi_real_t, counter[i]) / ssi_cast (ssi_real_t, k);
		}
	}

	delete[] nearest;
	delete[] counter;
	delete[] indices;
	delete[] distances;

	return true;
}

bool KNearestNeighbors::load (const ssi_char_t *filepath) {
	
	File *file = File::CreateAndOpen (File::BINARY, File::READ, filepath);

	release ();

	file->read (&_options.k, sizeof (_options.k), 1);
	file->read (&_options.distsum, sizeof (_options.distsum), 1);
	file->read (&_n_classes, sizeof (_n_classes), 1);
	file->read (&_n_samples, sizeof (_n_samples), 1);
	file->read (&_n_features, sizeof (_n_features), 1);	
	_data = new ssi_real_t *[_n_samples];
	ssi_real_t *data_mat = new ssi_real_t[_n_samples * _n_features];
	for (ssi_size_t i = 0; i < _n_samples; i++) {
		_data[i] = data_mat + i * _n_features;
		file->read (_data[i], sizeof (ssi_real_t), _n_features);
	}
	_classes = new ssi_size_t[_n_samples];
	file->read (_classes, sizeof (ssi_size_t), _n_samples);

	delete file;

	return true;
}

bool KNearestNeighbors::save (const ssi_char_t *filepath) {

	if (!_data) {
		ssi_wrn ("not trained");
		return false;
	}

	File *file = File::CreateAndOpen (File::BINARY, File::WRITE, filepath);

	file->write (&_options.k, sizeof (_options.k), 1);
	file->write (&_options.distsum, sizeof (_options.distsum), 1);
	file->write (&_n_classes, sizeof (_n_classes), 1);
	file->write (&_n_samples, sizeof (_n_samples), 1);
	file->write (&_n_features, sizeof (_n_features), 1);
	for (ssi_size_t i = 0; i < _n_samples; i++) {
		file->write (_data[i], sizeof (ssi_real_t), _n_features);
	}
	file->write (_classes, sizeof (ssi_size_t), _n_samples);

	delete file;

	return true;
}

}
