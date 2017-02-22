// ISDuplStrms.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/03/21
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

#include "ISDuplStrms.h"
#include "signal/SignalTools.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ISDuplStrms::ISDuplStrms (ISamples *samples, ssi_size_t n_dupl)
	: _samples (*samples),
	_n_dupl (n_dupl) {

	if (_samples.getSize () == 0) {
		ssi_err ("empty sample list");
	}

	if (_samples.getStreamSize () > 1) {
		ssi_err ("#streams must not > 1");
	}

	ssi_stream_t stream = _samples.getStream (0);
	ssi_sample_create (_dupl, _n_dupl, 0, 0, 0, 0);
	for (ssi_size_t i = 0; i < _n_dupl; i++) {
		_dupl.streams[i] = new ssi_stream_t;
		ssi_stream_init (*_dupl.streams[i], 0, stream.dim, stream.byte, stream.type, stream.sr);
	}
}

ISDuplStrms::~ISDuplStrms () {

	ssi_sample_destroy (_dupl);
}	

ssi_sample_t *ISDuplStrms::get (ssi_size_t index) {

	ssi_sample_t *tmp = _samples.get (index);

	if (tmp == 0) {
		return 0;
	}

	dupl (*tmp);
	return &_dupl;	
}

ssi_sample_t *ISDuplStrms::next () {

	ssi_sample_t *tmp = _samples.next ();
	if (!tmp) {
		return 0;
	}

	dupl (*tmp);
	return &_dupl;	
}

SSI_INLINE void ISDuplStrms::dupl (ssi_sample_t &sample) {

	_dupl.user_id = sample.user_id;
	_dupl.class_id = sample.class_id;
	_dupl.time = sample.time;
	_dupl.score = sample.score;
	for (ssi_size_t i = 0; i < _n_dupl; i++) {
		_dupl.streams[i]->time = sample.streams[0]->time;
		ssi_stream_adjust (*_dupl.streams[i], sample.num);
		memcpy (_dupl.streams[i]->ptr, sample.streams[0]->ptr, sample.streams[0]->tot);
	}
}


}
