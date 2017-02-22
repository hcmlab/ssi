// ISSelectClass.cpp
// author: Florian Lingenfelser <florian.lingenfelser@informatik.uni-augsburg.de>
// created: 2010/05/06
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

#include "ISSelectClass.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ISSelectClass::ISSelectClass (ISamples *samples)
	: _samples (*samples),
	_cset (0),
	_n_samples (0),
	_n_classes (0),
	_class_inds (0),
	_class_ids (0),
	_n_sampels_per_class (0) {
}

ISSelectClass::~ISSelectClass () { release (); }

void ISSelectClass::release(){
	
	delete[] _cset;
	_cset = 0;	

	delete[] _n_sampels_per_class;
	_n_sampels_per_class = 0;

	delete[] _class_inds;
	_class_inds = 0;

	delete[] _class_ids;
	_class_ids = 0;

	_n_classes = 0;
	_n_samples = 0;
}

bool ISSelectClass::setSelection (ssi_size_t class_id) {

	return setSelection (1, &class_id);
}

bool ISSelectClass::setSelection (ssi_size_t n_classes, 
	ssi_size_t* cset) {

	release();

	_n_classes = n_classes;

	_cset = new ssi_size_t [_n_classes];

	for(ssi_size_t nselected = 0; nselected < _n_classes; nselected++){
		_cset[nselected] = cset[nselected];
	}
	
	_n_sampels_per_class = new ssi_size_t[_n_classes];
	for (ssi_size_t nclass = 0; nclass < _n_classes; nclass++) {
		_n_sampels_per_class[nclass] = 0;
	}
	
	ssi_sample_t *tmp = 0;
	_samples.reset();
	while( tmp = _samples.next() ){
		for(ssi_size_t nselected = 0; nselected < _n_classes; nselected++){
			if (tmp->class_id == _cset[nselected]){
				_n_sampels_per_class[nselected]++;
				_n_samples++;
			}
		}
	}
	_samples.reset();

	ssi_size_t counter_org = 0;
	ssi_size_t counter_sel = 0;
	_class_inds = new ssi_size_t[_n_samples];
	_class_ids = new ssi_size_t[_n_samples];
	while( tmp = _samples.next() ){
		for(ssi_size_t nselected = 0; nselected < _n_classes; nselected++){
			if (tmp->class_id == _cset[nselected]){
				_class_inds[counter_sel] = counter_org;
				_class_ids[counter_sel] = nselected;
				counter_sel++;
			}
		}
		counter_org++;
	}
	_samples.reset();
	
	return true;
}

ssi_sample_t *ISSelectClass::get (ssi_size_t index) {

	ssi_sample_t *tmp = 0;
	tmp = _samples.get (_class_inds[index]);

	if (tmp == 0) {
		return 0;
	}
	
	_sample = *tmp;
	_sample.class_id = _class_ids[index];

	return &_sample;	
}

ssi_sample_t *ISSelectClass::next () {

	ssi_sample_t *tmp = 0;
	while( tmp = _samples.next() ){
		for(ssi_size_t nselected = 0; nselected < _n_classes; nselected++){
			if (tmp->class_id == _cset[nselected]){
				_sample = *tmp;
				_sample.class_id = nselected;
				return &_sample;
			}
		}
	}
	return 0;
}

}
