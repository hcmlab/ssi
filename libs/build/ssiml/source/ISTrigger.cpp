// ISTrigger.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2014/02/25
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

#include "ISTrigger.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ISTrigger::ISTrigger (ISamples *samples)
	: _samples (*samples),
	_stream_index (0) {

	_trigger = new bool *[samples->getStreamSize ()];
	for (ssi_size_t i = 0; i < samples->getStreamSize (); i++) {
		_trigger[i] = new bool[_samples.getSize ()];
		for (ssi_size_t j = 0; j < _samples.getSize (); j++) {
			_trigger[i][j] = 1.0f;
		}
	}

	ssi_sample_create (_sample, _samples.getStreamSize (), 0, 0, 0, 0);
}

ISTrigger::~ISTrigger () {

	for (ssi_size_t i = 0; i < _samples.getStreamSize (); i++) {
		delete[] _trigger[i];
	}
	delete[] _trigger;
	for (ssi_size_t i = 0; i < _sample.num; i++) {
		_sample.streams[i] = 0;
	}
	ssi_sample_destroy (_sample);
}


bool ISTrigger::setTriggerStream (ssi_size_t index, ssi_stream_t &trigger, ssi_real_t thres) {

	if (trigger.type != SSI_REAL) {
		ssi_wrn ("ISTrigger::setTriggerStream () -> stream.type != SSI_REAL"); 
		return false;
	}

	if (trigger.dim != 1) {
		ssi_wrn ("ISTrigger::setTriggerStream () -> stream.dim != 1"); 
		return false;
	}

	_samples.reset ();
	ssi_sample_t *sample;
	ssi_size_t count = 0;
	ssi_real_t *ptr = 0;
	ssi_real_t sum = 0;
	ssi_size_t from, to;
	while (sample = _samples.next ()) {
				
		if (sample->streams[index] == 0) {
			_trigger[index][count] = false;
			++count;
			continue;
		}

		from = ssi_cast (ssi_size_t, sample->time * trigger.sr + 0.5);
		to = from + ssi_cast (ssi_size_t, sample->streams[index]->num / sample->streams[index]->sr * trigger.sr + 0.5) - 1;

		if (to >= trigger.num) {
			ssi_wrn ("sample#%u exceeds trigger stream, setting remaining samples to false", count);
			for (; count < _samples.getSize (); count++) {
				_trigger[index][count] = false;
			}
			break;
		}

		sum = 0; 		
		ptr = ssi_pcast (ssi_real_t, trigger.ptr) + from;
		for (ssi_size_t i = from; i <= to; i++) {
			sum += *ptr++;
		}
		
		_trigger[index][count] = sum / (to - from + 1) > thres;
		++count;
	}
	
	return true;
}

ssi_sample_t *ISTrigger::get (ssi_size_t index) {

	if (index >= _samples.getSize ()) { return 0; }

	ssi_sample_destroy (_sample);
	ssi_sample_t *s = _samples.get (index);
	ssi_sample_clone (*s, _sample);
	
	for (ssi_size_t i = 0; i < _sample.num; i++) {
		_sample.streams[i]->num = _trigger[i][index] ? s->streams[i]->num : 0;			
	}

	return &_sample;	
}

ssi_sample_t *ISTrigger::next () {

	if (_samples.getSize () <= _head) { return 0; }
	return get (_head++);	
}

}
