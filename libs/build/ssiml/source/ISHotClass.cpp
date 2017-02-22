// ISHotClass.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2010/05/01
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

#include "ISHotClass.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ISHotClass::ISHotClass (ISamples *samples)
	: _samples (*samples),
	_hotties (0),
	_n_hotties (0),
	_hot_name (0),
	_rest_name(0),
	_n_samples (0) {
}

ISHotClass::~ISHotClass () {

	release ();
}	

void ISHotClass::release () {

	delete[] _hotties;
	_hotties = 0;
	_n_hotties = 0;
	delete[] _rest_name;
	_rest_name = 0;
	delete[] _hot_name;
	_hot_name = 0;
	delete[] _n_samples;
	_n_samples = 0;
}

bool ISHotClass::setHotClass(ssi_size_t hot_id, const ssi_char_t *rest_name) {

	if (_hotties) {
		//ssi_wrn ("overwrite hot class");
		release ();
	}

	if (hot_id >= _samples.getClassSize ()) {
		ssi_wrn ("index exceeds class size");
		return false;
	} 

	_n_hotties = 1;
	_hotties = new ssi_size_t[1];
	_hotties[0] = hot_id;
	_hot_name = ssi_strcpy (_samples.getClassName (hot_id));
	_rest_name = ssi_strcpy(rest_name);
	_n_samples = new ssi_size_t[2];
	_n_samples[0] = _samples.getSize (hot_id);
	_n_samples[1] = _samples.getSize () - _n_samples[0];

	return true;
}

bool ISHotClass::setHotClass(ssi_size_t n_hotties, ssi_size_t *hotties, const ssi_char_t *hot_name, const ssi_char_t *rest_name) {

	if (_hotties) {
		//ssi_wrn ("overwrite hot class");
		release ();
	}

	for (ssi_size_t i = 0; i < n_hotties; i++) {
		if (hotties[i] >= _samples.getClassSize ()) {
			ssi_wrn ("index %u exceeds class size", i);
			return false;
		}
	}

	_n_hotties = n_hotties;
	_hotties = new ssi_size_t[_n_hotties];
	_n_samples = new ssi_size_t[2];
	_n_samples[0] = 0;
	for (ssi_size_t i = 0; i < _n_hotties; i++) {
		_hotties[i] = hotties[i];
		_n_samples[0] += _samples.getSize (_hotties[i]);
	}
	_n_samples[1] = _samples.getSize () - _n_samples[0];
	_hot_name = ssi_strcpy (hot_name);
	_rest_name = ssi_strcpy(rest_name);

	return true;
}

ssi_sample_t *ISHotClass::get (ssi_size_t index) {

	ssi_sample_t *tmp = _samples.get (index);

	if (tmp == 0) {
		return 0;
	}

	_sample = *tmp;	
	bool found = false;
	for (ssi_size_t i = 0; i < _n_hotties; i++) {
		if (_sample.class_id == _hotties[i]) {
			_sample.class_id = 0;
			found = true;
			break;
		};	
	}
	if (!found) {
		_sample.class_id = 1;
	}

	return &_sample;	
}

ssi_sample_t *ISHotClass::next () {

	ssi_sample_t *tmp = _samples.next ();
	if (!tmp) {
		return 0;
	}

	_sample = *tmp;	
	bool found = false;
	for (ssi_size_t i = 0; i < _n_hotties; i++) {
		if (_sample.class_id == _hotties[i]) {
			_sample.class_id = 0;
			found = true;
			break;
		};	
	}
	if (!found) {
		_sample.class_id = 1;
	}

	return &_sample;
}

}
