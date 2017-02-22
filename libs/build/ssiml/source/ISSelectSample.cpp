// ISSelectSample.cpp
// author: Florian Lingenfelser <florian.lingenfelser@informatik.uni-augsburg.de>
// created: 2010/05/05
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

#include "ISSelectSample.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ISSelectSample::ISSelectSample (ISamples *samples)
	: _samples (*samples),
	_nset(0),
	_sset (0),
	_n_samples (0),
	_n_sampels_per_class (0) {
}

ISSelectSample::~ISSelectSample () {
	release ();
}

void ISSelectSample::release(){
	
	delete[] _sset;
	_sset = 0;	

	delete[] _n_sampels_per_class;
	_n_sampels_per_class = 0;

	_nset = 0;
	_n_samples = 0;
}

bool ISSelectSample::setSelection (ssi_size_t n_samples, 
	const ssi_size_t *sset) {

	release();

	_n_samples = n_samples;

	_sset = new ssi_size_t [_n_samples];

	for(ssi_size_t nselected = 0; nselected < _n_samples; nselected++){
		_sset[nselected] = sset[nselected];
	}

	ssi_size_t n_classes = _samples.getClassSize ();
	_n_sampels_per_class = new ssi_size_t[n_classes];
	for (ssi_size_t nclass = 0; nclass < n_classes; nclass++) {
		_n_sampels_per_class[nclass] = 0;
	}

	for(ssi_size_t nselected = 0; nselected < _n_samples; nselected++) {
		_n_sampels_per_class[_samples.get (sset[nselected])->class_id]++;
	}
	
	return true;
}

ssi_sample_t *ISSelectSample::get (ssi_size_t index) {

	if(index >= _n_samples){ return 0; }

	_sample = _samples.get (_sset[index]);
	if (!_sample) {
		return 0;
	}
	
	return _sample;	
}

ssi_sample_t *ISSelectSample::next () {

	if(_n_samples <= _nset){return 0;}

	_sample =  _samples.get (_sset[_nset]);
	if (!_sample) {
		return 0;
	}
	_nset++;
	
	return _sample;
}

}
