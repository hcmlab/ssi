// Relief.cpp
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

#include "Relief.h"

#if __gnu_linux__
using std::min;
using std::max;
#endif

namespace ssi {

int Relief::ssi_log_level = SSI_LOG_LEVEL_DEFAULT;
ssi_char_t *Relief::ssi_log_name = "relief____";

Relief::Relief (const ssi_char_t *file) :
	_n_scores (0),
	_scores (0),
	_n_samples (0),
	_classes (0),
	_features (0),
	_n_features (0),
	_file (0) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
}
	
Relief::~Relief () {

	release ();
	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

bool Relief::train (ISamples &samples,
	ssi_size_t stream_index) {

	release ();
	matrix (samples, stream_index);

	ssi_size_t *hits = new ssi_size_t[_n_samples];
	ssi_real_t *hits_val = new ssi_real_t[_n_samples];
	ssi_size_t *miss = new ssi_size_t[_n_samples];
	ssi_real_t *miss_val = new ssi_real_t[_n_samples];
	ssi_real_t *weights = new ssi_real_t[_n_features];
	for (ssi_size_t i = 0; i < _n_features; i++) {
		weights[i] = 0;
	}

	if (_options.mem) {

		for (ssi_size_t i = 0; i < _n_samples; i++) {		
			hits_val[i] = FLT_MAX;		
			miss_val[i] = FLT_MAX;
			for (ssi_size_t j = 0; j < _n_samples; j++) {
				if (i == j) {
					continue;
				}
				ssi_real_t d = dist (_features[i], _features[j], _n_features);
				if (_classes[i] == _classes[j]) {
					if (d < hits_val[i]) {
						hits_val[i] = d;
						hits[i] = j;
					}
				} else if (d < miss_val[i]) {
					miss_val[i] = d;
					miss[i] = j;
				}			
			}		
			//ssi_print ("progress=%.2f%%\n", (100.0f * i) / _n_samples);
		}

	} else {

		ssi_real_t **dists = new ssi_real_t *[_n_samples];
		for (ssi_size_t i = 0; i < _n_samples; i++) {
			dists[i] = new ssi_real_t[_n_samples]; 
		}

		for (ssi_size_t i = 0; i < _n_samples; i++) {
			dists[i][i] = FLT_MAX;
			for (ssi_size_t j = i+1; j < _n_samples; j++) {			
				dists[i][j] = dists[j][i] = dist (_features[i], _features[j], _n_features);			
			}
		}

		for (ssi_size_t i = 0; i < _n_samples; i++) {		
			hits_val[i] = FLT_MAX;		
			miss_val[i] = FLT_MAX;
			for (ssi_size_t j = 0; j < _n_samples; j++) {
				if (_classes[i] == _classes[j]) {
					if (dists[i][j] < hits_val[i]) {
						hits_val[i] = dists[i][j];
						hits[i] = j;
					}
				} else if (dists[i][j] < miss_val[i]) {
					miss_val[i] = dists[i][j];
					miss[i] = j;
				}			
			}		
		}

		for (ssi_size_t i = 0; i < _n_samples; i++) {
			delete[] dists[i]; 
		}
		delete[] dists;
	}

	if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
		SSI_DBG (SSI_LOG_LEVEL_DEBUG, "hits & misses:");
		for (ssi_size_t i = 0; i < _n_samples; i++) {
			SSI_DBG (SSI_LOG_LEVEL_DEBUG, "%u -> %u (%.2f) -> %u (%.2f)", i, hits[i], hits_val[i], miss[i], miss_val[i]);
		}	
	}

	ssi_real_t *ptr = 0;
	ssi_real_t ptr_val = 0;
	for (ssi_size_t i = 0; i < _n_samples; i++) {
		ptr = _features[i];
		for (ssi_size_t j = 0; j < _n_features; j++) {		
			ptr_val = *ptr++;
			weights[j] += SSI_RELIEF_SQR (ptr_val - _features[miss[i]][j]) - SSI_RELIEF_SQR (ptr_val - _features[hits[i]][j]);
		}		
	}

	if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
		SSI_DBG (SSI_LOG_LEVEL_DEBUG, "weights:");
		for (ssi_size_t i = 0; i < _n_features; i++) {
			SSI_DBG (SSI_LOG_LEVEL_DEBUG, "%u -> %.2f", i, weights[i]);
		}	
	}

	if (_options.norm) {	
		ssi_real_t minval, maxval;
		minval = FLT_MAX;
		maxval = FLT_MIN;
		for (ssi_size_t featureIndex = 0; featureIndex < _n_features; ++featureIndex) {
			minval = min (minval, weights[featureIndex]);
			maxval = max (maxval, weights[featureIndex]);
		}
		for (ssi_size_t featureIndex = 0; featureIndex < _n_features; ++featureIndex) {
			if (minval < 0) {
				weights[featureIndex] = (weights[featureIndex] - minval) / (maxval - minval);
			}
			else {
				weights[featureIndex] /= maxval;
			}
		}	
	}

	_n_scores = _n_features;
	_scores = new score[_n_scores];
	score *score_ptr = _scores;
	for (ssi_size_t i = 0; i < _n_features; i++) {		
		score_ptr->index = i;
		score_ptr->value = weights[i];	
		++score_ptr;
	}

	delete[] hits;
	delete[] hits_val;
	delete[] miss;
	delete[] miss_val;
	delete[] weights;

	ssi_msg (SSI_LOG_LEVEL_BASIC, "selected %u dimensions", _n_scores);

	return true;
}

void Relief::print (FILE *file) {
		
	for (ssi_size_t i = 0; i < _n_scores; i++) {
		ssi_fprint (file, "%u: %.2f\n", _scores[i].index, _scores[i].value);
	}
}

void Relief::release () {

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

void Relief::matrix (ISamples &samples, ssi_size_t stream_index) {

	samples.reset ();

	_n_features = samples.getStream (stream_index).dim;
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

SSI_INLINE ssi_real_t Relief::dist (ssi_real_t *x1, ssi_real_t *x2, ssi_size_t n_dim) {

	ssi_real_t retVal = 0.0f;
	ssi_real_t tempDifference = 0.0f;
	for(ssi_size_t i = 0; i < n_dim; ++i)
	{
		tempDifference = *x1++ - *x2++;
		retVal += tempDifference * tempDifference;
	}

	return retVal;
}

}

