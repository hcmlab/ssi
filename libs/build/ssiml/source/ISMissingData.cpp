// ISMissingData.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2010/05/21
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

#include "ISMissingData.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ISMissingData::ISMissingData (ISamples *samples)
	: _samples (*samples),
	_stream_index (0),
	_n_samples (0),
	_n_samples_per_class (0) {

	_sample_map = new ssi_size_t[_samples.getSize ()];
	setStream(0);
}

ISMissingData::~ISMissingData () {
	release ();
	delete _sample_map;
}

void ISMissingData::release(){
	
	delete[] _n_samples_per_class;
	_n_samples_per_class = 0;
	_n_samples = 0;
}

bool ISMissingData::setStream (ssi_size_t index) {

	release();

	_stream_index = index;
	_n_samples = 0;

	ssi_size_t n_classes = _samples.getClassSize ();
	_n_samples_per_class = new ssi_size_t[n_classes];
	for (ssi_size_t nclass = 0; nclass < n_classes; nclass++) {
		_n_samples_per_class[nclass] = 0;
	}

	_samples.reset ();
	ssi_sample_t *sample;
	ssi_size_t count = 0;
	while (sample = _samples.next ()) {
		if (sample->streams[_stream_index]->num > 0) {			
			_sample_map[_n_samples] = count;
			++_n_samples;
			++_n_samples_per_class[sample->class_id];
		}
		++count;
	}
	
	return true;
}

ssi_sample_t *ISMissingData::get (ssi_size_t index) {

	if (index >= _n_samples) { return 0; }

	_sample = _samples.get (_sample_map[index]);
	if (!_sample) {
		return 0;
	}
	
	return _sample;	
}

ssi_sample_t *ISMissingData::next () {

	if (_n_samples <= _head) { return 0; }

	_sample =  _samples.get (_sample_map[_head]);
	if (!_sample) {
		return 0;
	}
	_head++;
	
	return _sample;
}

}
