// ISOverSample.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/03/02
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

#include "ISOverSample.h"
#include "ISSelectClass.h"
#include "FindNN.h"
#include "ModelTools.h"
#include "base/Random.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ssi_char_t *ISOverSample::ssi_log_name = "oversample";

ISOverSample::ISOverSample (ISamples *samples)
	: _samples (*samples),
	_n_over (0),
	_over (0),
	_smote_k (5) {

	_seed = Random::Seed();

	_n_classes = _samples.getClassSize ();
	_n_samples = _samples.getSize ();
	_n_over_per_class = new ssi_size_t[_n_classes];
	for (ssi_size_t i = 0; i < _n_classes; i++) {
		_n_over_per_class[i] = 0;
	}	
}

ISOverSample::~ISOverSample () {

	release ();
	delete[] _n_over_per_class;
}	

void ISOverSample::release () {

	for (ssi_size_t i = 0; i < _n_over; i++) {
		ssi_sample_destroy (_over[i]);
	}
	delete[] _over;
	_over = 0;
	_n_over = 0;
	for (ssi_size_t i = 0; i < _n_classes; i++) {
		_n_over_per_class[i] = 0;
	}	
}

bool ISOverSample::setOver (Strategy strategy) {

	ssi_size_t n_max = 0;
	ssi_size_t n_classes = _samples.getClassSize ();
	for (ssi_size_t i = 0; i < n_classes; i++) {
		if (n_max < _samples.getSize (i)) {
			n_max = _samples.getSize (i);
		}
	}	
	ssi_size_t *add_n_samples = new ssi_size_t[n_classes];
	for (ssi_size_t i = 0; i < n_classes; i++) {
		add_n_samples[i] = n_max - _samples.getSize (i);
	}
	bool result = setOver (n_classes, add_n_samples, strategy);
	delete[] add_n_samples;

	return result;
}

bool ISOverSample::setOver (ssi_size_t class_id, ssi_size_t n_add, Strategy strategy) {

	ssi_size_t *tmp = new ssi_size_t[_n_classes];
	for (ssi_size_t i = 0; i < _n_classes; i++) {
		tmp[i] = i == class_id ? n_add : 0;
	}
	bool result = setOver (_n_classes, tmp, strategy);
	delete[] tmp;

	return result;
}

bool ISOverSample::setOver (ssi_size_t n_classes, ssi_size_t *n_adds, Strategy strategy) {

	if (n_classes != _n_classes) {
		ssi_wrn ("n_classes (%u) must be equal to #classes in samples (%u)", n_classes, _n_classes);
		return false;
	}

	release ();	

	for (ssi_size_t i = 0; i < _n_classes; i++) {
		_n_over_per_class[i] = n_adds[i];
		_n_over += n_adds[i];
	}
	_over = new ssi_sample_t[_n_over];

	switch (strategy) {
		case ISOverSample::RANDOM:
			return doDuplicate ();			
		case ISOverSample::SMOTE:
			return doSmote ();
		default:
			ssi_err ("unkown strategy");			
			return false;
	}

	return true;
}

void ISOverSample::setSeed(ssi_size_t seed)
{
	_seed = seed;
}

ssi_sample_t *ISOverSample::get (ssi_size_t index) {

	if (index >= _n_samples + _n_over) {
		return 0;
	}

	if (index < _n_samples) {
		return _samples.get (index);
	} else {
		return &_over[index - _n_samples];
	}
}

ssi_sample_t *ISOverSample::next () {

	ssi_sample_t *tmp = _samples.next ();
	if (!tmp) {
		if (_next_over_count < _n_over) {			
			return &_over[_next_over_count++];
		} else {
			return 0;
		}
	}

	return tmp;
}

bool ISOverSample::doDuplicate () {

	ssi_size_t count = 0;
	ssi_size_t index = 0;
	ISSelectClass select (&_samples);

	Randomf random(0, 1, _seed);

	for (ssi_size_t i = 0; i < _n_classes; i++) {
		select.setSelection (i);
		ssi_size_t max = select.getSize ();				
		for (ssi_size_t j = 0; j < _n_over_per_class[i]; j++) {
			index = ssi_size_t(max * random.next());
			ssi_sample_clone (*select.get (index), _over[count]);
			_over[count++].class_id = i;
		}		
	}
	
	return true;
}

bool ISOverSample::doSmote () {

	ssi_size_t *classes;
	ssi_real_t **matrix;
	ssi_size_t n_features = _samples.getStream (0).dim;
	ModelTools::CreateSampleMatrix (_samples, 0, _n_samples, n_features, &classes, &matrix); 

	ssi_size_t k = _smote_k;
	ssi_size_t *indices = new ssi_size_t[k];
	ssi_real_t *distances = new ssi_real_t[k];

	ssi_size_t count = 0;
	ssi_size_t index = 0;
	ISSelectClass select (&_samples);

	Randomi random_k(0, k - 1, _seed);
	Randomf random(0, 1, _seed);
	Randomf random_max(0, 1, _seed);

	for (ssi_size_t i = 0; i < _n_classes; i++) {
		select.setSelection (i);
		ssi_size_t max = select.getSize ();

		for (ssi_size_t j = 0; j < _n_over_per_class[i]; j++) {

			// clone randomly selected sample 
			index = ssi_size_t(random_max.next()*max);
			ssi_sample_clone (*select.get (index), _over[count]);
			_over[count].class_id = i;

			// randomly choose one its k-nearest neighbors
			ssi_real_t *sample = ssi_pcast (ssi_real_t,_over[count].streams[0]->ptr);			
			FindNN::Find (_n_samples, n_features, sample, matrix, k, indices, distances);
			index = random_k.next();
			ssi_real_t *neighbor = matrix[indices[index]];
			
			// use neighbor to generate synthetic sample					
			for (ssi_size_t nfeat = 0; nfeat < n_features; nfeat++) {
				ssi_real_t gap = ssi_cast (ssi_real_t, random.next ());
				ssi_real_t diff = neighbor[nfeat] - sample[nfeat];
				sample[nfeat] += gap * diff;
			}
			
			count++;
		}		
	}

	ModelTools::ReleaseSampleMatrix (_n_samples, classes, matrix);
	delete[] indices;
	delete[] distances;

	return true;
}


}
