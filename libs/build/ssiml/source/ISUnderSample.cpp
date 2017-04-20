// ISUnderSample.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/03/07
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

#include "ISUnderSample.h"
#include "ISSelectClass.h"
#include "KMeans.h"
#include "base/Factory.h"
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

ISUnderSample::ISUnderSample (ISamples *samples)
	: _samples (*samples),
	_n_under (0),
	_under (0) {

	_seed = Random::Seed();

	_n_classes = _samples.getClassSize ();
	_n_samples = _samples.getSize ();
	_n_under_per_class = new ssi_size_t[_n_classes];
	for (ssi_size_t i = 0; i < _n_classes; i++) {
		_n_under_per_class[i] = 0;
	}	
}

ISUnderSample::~ISUnderSample () {

	release ();
	delete[] _n_under_per_class;
}	

void ISUnderSample::release () {

	if (_under) {
		for (ssi_size_t i = 0; i < _n_under; i++) {
			ssi_sample_destroy (_under[i]);
		}
	}
	delete[] _under;
	_under = 0;
	_n_under = 0;
	for (ssi_size_t i = 0; i < _n_classes; i++) {
		_n_under_per_class[i] = 0;
	}	
}

bool ISUnderSample::setUnder (Strategy strategy) {

	ssi_size_t *tmp = new ssi_size_t[_n_classes];	
	ssi_size_t min = _samples.getSize (0);
	for (ssi_size_t i = 0; i < _n_classes; i++) {
		tmp[i] = _samples.getSize (i);
		if (tmp[i] < min) {
			min = tmp[i];
		}
	}
	
	for (ssi_size_t i = 0; i < _n_classes; i++) {
		tmp[i] -= min;
	}

	bool result = setUnder (_n_classes, tmp, strategy);
	delete[] tmp;

	return result;
}

bool ISUnderSample::setUnder (ssi_size_t class_id, ssi_size_t n_reduce, Strategy strategy) {

	ssi_size_t *tmp = new ssi_size_t[_n_classes];
	for (ssi_size_t i = 0; i < _n_classes; i++) {
		tmp[i] = i == class_id ? n_reduce : 0;
	}
	bool result = setUnder (_n_classes, tmp, strategy);
	delete[] tmp;

	return result;
}

bool ISUnderSample::setUnder (ssi_size_t n_classes, ssi_size_t *n_reduce, Strategy strategy) {

	if (n_classes != _n_classes) {
		ssi_wrn ("n_classes (%u) must be equal to #classes in samples (%u)", n_classes, _n_classes);
		return false;
	}

	release ();	

	for (ssi_size_t i = 0; i < _n_classes; i++) {
		ssi_size_t n = _samples.getSize (i);
		if (n_reduce[i] > n) {
			ssi_wrn ("n_reduce (%u) exceeds #samples (%u) for class %u", n_reduce[i], n, i);
			return false;
		}
		_n_under_per_class[i] = n - n_reduce[i];
		_n_under += _n_under_per_class[i];
	}
	_under = new ssi_sample_t[_n_under];

	switch (strategy) {
		case ISUnderSample::RANDOM:
			return doRandom ();					
		case ISUnderSample::KMEANS:
			return doKMeans ();	
		default:
			ssi_err ("unkown strategy");			
			return false;
	}

	return true;
}

void ISUnderSample::setSeed(ssi_size_t seed)
{
	_seed = seed;
}

ssi_sample_t *ISUnderSample::get (ssi_size_t index) {
	
	return &_under[index];	
}

ssi_sample_t *ISUnderSample::next () {
	
	if (_next_under_count < _n_under) {			
		return &_under[_next_under_count++];
	} else {
		return 0;
	}
}

void ISUnderSample::Shuffle(Randomf *random, ssi_size_t n, ssi_size_t *arr, ssi_size_t seed)
{	
	for (ssi_size_t i = 0; i < n; i++) {
		ssi_int_t r = ssi_size_t(random->next() * n);
		ssi_size_t tmp = arr[i];
		arr[i] = arr[r];
		arr[r] = tmp;
	}
}

bool ISUnderSample::doRandom () {

	ssi_size_t count = 0;
	ssi_size_t index = 0;
	ISSelectClass select (&_samples);

	Randomf random(0, 1, _seed);

	for (ssi_size_t i = 0; i < _n_classes; i++) {
		select.setSelection (i);
		ssi_size_t n = select.getSize ();
		ssi_size_t *inds = new ssi_size_t[n];
		for (ssi_size_t j = 0; j < n; j++) {
			inds[j] = j;
		}		
		Shuffle(&random, n, inds, _seed);
		for (ssi_size_t j = 0; j < _n_under_per_class[i]; j++) {
			index = inds[j];
			ssi_sample_clone (*select.get (index), _under[count]);
			_under[count++].class_id = i;
		}		
		delete[] inds;
	}
	
	return true;
}


bool ISUnderSample::doKMeans () {

	ssi_size_t count = 0;	
	ISSelectClass select (&_samples);
	ssi_sample_t *templ = _samples.get (0);
	ssi_size_t _n_features = templ->streams[0]->dim;
	ssi_real_t *const*clusters = 0;
	KMeans *kmeans = ssi_pcast (KMeans, Factory::Create (KMeans::GetCreateName (), 0, false));	
	for (ssi_size_t i = 0; i < _n_classes; i++) {
		select.setSelection (i);
		kmeans->release ();
		kmeans->getOptions ()->k = _n_under_per_class[i];
		kmeans->getOptions ()->pp = true;
		kmeans->getOptions ()->iter = 1;
		kmeans->train (select, 0);
		clusters = kmeans->getClusters ();
		for (ssi_size_t j = 0; j < _n_under_per_class[i]; j++) {
			ssi_sample_clone (*templ, _under[count]);
			memcpy (_under[count].streams[0]->ptr, clusters[j], _n_features * sizeof (ssi_real_t));
			_under[count].class_id = i;
			count++;
		}		
	}
	delete kmeans;

	return true;
}


}
