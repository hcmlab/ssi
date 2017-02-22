// ISReClass.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/01/10
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

#include "ISReClass.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ISReClass::ISReClass (ISamples *samples)
	: _samples (*samples),
	_n_classes (0),
	_class_names (0),
	_n_ids (0),
	_n_ids_per_class (0),
	_ids (0),
	_counter (0) {
}

ISReClass::~ISReClass () {

	release ();
}	

void ISReClass::release () {
	
	_n_ids = 0;
	delete[] _ids;
	_ids = 0;
	delete[] _n_ids_per_class;
	_n_ids_per_class = 0;
	for (ssi_size_t i = 0; i < _n_classes; i++) {
		delete[] _class_names[i];
	}
	_n_classes = 0;
	delete[] _class_names;
	_class_names = 0;
}

bool ISReClass::setReClass (ssi_size_t n_classes,
	const ssi_char_t **class_names,
	ssi_size_t n_class_ids, 
	ssi_size_t *class_ids) { 

	if (_ids) {
		release ();
	}

	if (n_class_ids != _samples.getSize ()) {
		ssi_wrn ("#ids differs from #samples");
		return false;
	} 

	_n_classes = n_classes;
	_class_names = new ssi_char_t *[_n_classes];
	for (ssi_size_t i = 0; i < n_classes; i++) {
		_class_names[i] = ssi_strcpy (class_names[i]);
	}
	_n_ids = n_class_ids;
	_ids = new ssi_size_t[_n_ids];
	_n_ids_per_class = new ssi_size_t[_n_classes];

	for (ssi_size_t i = 0; i < _n_classes; i++) {
		_n_ids_per_class[i] = 0;
	}
	for (ssi_size_t i = 0; i < _n_ids; i++) {
		_ids[i] = class_ids[i];
		_n_ids_per_class[class_ids[i]]++;
	}

	return true;
}

ssi_sample_t *ISReClass::get (ssi_size_t index) {

	if (_ids == 0) {
		ssi_wrn ("#ids not set");
		return 0;
	}

	ssi_sample_t *tmp = _samples.get (index);

	if (tmp == 0) {
		return 0;
	}

	_sample = *tmp;
	_sample.class_id = _ids[index];

	return &_sample;	
}

ssi_sample_t *ISReClass::next () {

	if (_ids == 0) {
		ssi_wrn ("#ids not set");
		return 0;
	}

	ssi_sample_t *tmp = _samples.next ();
	if (!tmp) {
		return 0;
	}

	_sample = *tmp;
	_sample.class_id = _ids[_counter++];
	return &_sample;
}

}
