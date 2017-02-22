// FScore.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2010/05/03
// Copyright (C) University of Augsburg
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

#include "FScore.h"

namespace ssi {

int FScore::ssi_log_level = SSI_LOG_LEVEL_DEFAULT;
ssi_char_t *FScore::ssi_log_name = "relief____";

FScore::FScore (const ssi_char_t *file) :
	_n_scores (0),
	_scores (0),
	_n_samples (0),
	_classes (0),
	_features (0),
	_n_features (0),
	_file (0) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
}
	
FScore::~FScore () {

	release ();
	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}

bool FScore::train (ISamples &samples,
	ssi_size_t stream_index) {

	release ();
	matrix (samples, stream_index);

	

	ssi_msg (SSI_LOG_LEVEL_BASIC, "selected %u dimensions", _n_scores);

	return true;
}

void FScore::print (FILE *file) {
		
	for (ssi_size_t i = 0; i < _n_scores; i++) {
		ssi_fprint (file, "%u: %.2f\n", _scores[i].index, _scores[i].value);
	}
}

void FScore::release () {

	delete[] _scores; _scores = 0;	
	delete[] _classes; _classes = 0;
	if (_features) {
		for (ssi_size_t i = 0; i < _n_samples; i++) {
			delete[] _features[i];
		}
		delete[] _features; _features = 0;
	}
	_n_features = 0;
	_n_scores = 0;
	_n_samples = 0;
}

void FScore::matrix (ISamples &samples, ssi_size_t stream_index) {

	samples.reset ();
	#if _WIN32||_WIN64
	_n_features = samples.getStreamDim (stream_index);
	#else
	_n_features = (samples.getStream(stream_index)).dim;
	#endif
	_n_samples = samples.getSize ();
	_classes = new ssi_size_t[_n_samples];
	_features = new ssi_real_t *[_n_samples];
	
	ssi_sample_t *sample = 0;
	ssi_size_t bytes = _n_features * sizeof (ssi_real_t);
	for (ssi_size_t i = 0; i < _n_samples; i++) {
		sample = samples.next ();	
		_classes[i] = sample->class_id;
		_features[i] = new ssi_real_t[_n_features];	
		memcpy (_features[i], sample->streams[stream_index]->ptr, bytes);
	}
}

}

