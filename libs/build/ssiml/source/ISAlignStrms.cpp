// ISAlignStrms.cpp
// author: Florian Lingenfelser <florian.lingenfelser@informatik.uni-augsburg.de>
// created: 2010/05/19
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

#include "ISAlignStrms.h"
#include "signal/SignalTools.h"
#include "model/ModelTools.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ISAlignStrms::ISAlignStrms (ISamples *samples)
	: _samples (*samples),
	_dim (0),
	_n_streams (samples->getStreamSize ()) {

	for(ssi_size_t nstrms = 0; nstrms < _n_streams; nstrms++){
		_dim += _samples.get(0)->streams[nstrms]->dim;
	}

	ssi_sample_create (_sample_out, 1, 0, 0, 0, 0);
	_sample_out.streams[0] = &_stream;

	// determine aligned bytes
	ssi_size_t byte = _samples.get(0)->streams[0]->byte;
	for (ssi_size_t i = 1; i < _n_streams; i++) {
		if (byte != _samples.get(0)->streams[i]->byte) {
			ssi_err ("number of bytes must be equal in all streams");
		}
	}

	//create aligned streams
	ssi_stream_init (_stream, 0, _dim, byte, _samples.getStream (0).type, _samples.getStream (0).sr);
	ssi_stream_init (_stream_ref, 0, _dim, byte, _samples.getStream (0).type, _samples.getStream (0).sr);

}

ISAlignStrms::~ISAlignStrms () {

	ssi_stream_destroy (_stream);
	delete[] _sample_out.streams;

}	

SSI_INLINE void ISAlignStrms::align () {

	static ssi_size_t count = 0;

	_sample_out.class_id = _sample_in.class_id;
	_sample_out.user_id = _sample_in.user_id;
	_sample_out.score = _sample_in.score;
	_sample_out.time = _sample_in.time;

	if (_samples.hasMissingData ()) {
		for(ssi_size_t i = 0; i < _n_streams; i++) {
			if (_sample_in.streams[i]->num == 0) {				
				_stream.num = 0;
				return;
			}
		}
	}

	ssi_size_t num = _sample_in.streams[0]->num;
	for(ssi_size_t i = 1; i < _n_streams; i++) {
		if (_sample_in.streams[i]->num != num) {
			ssi_err ("streams differ in #frames differ (%u != %u)", num, _sample_in.streams[i]->num);
		}
	}
	ssi_stream_adjust (_stream, num);

	ssi_byte_t *ptr = _stream.ptr;
	for (ssi_size_t j = 0; j < num; j++) {
		for(ssi_size_t i = 0; i < _n_streams; i++){
			ssi_size_t tot = _sample_in.streams[i]->byte * _sample_in.streams[i]->dim;
			memcpy(ptr, _sample_in.streams[i]->ptr + j*tot, tot);
			ptr += tot;		
		}
	}

}

ssi_sample_t *ISAlignStrms::get (ssi_size_t index) {

	ssi_sample_t *tmp = _samples.get (index);

	if (tmp == 0) {
		return 0;
	}

	_sample_in = *tmp;
	align ();
	return &_sample_out;	
}

ssi_sample_t *ISAlignStrms::next () {

	ssi_sample_t *tmp = _samples.next ();
	if (!tmp) {
		return 0;
	}

	_sample_in = *tmp;
	align ();
	return &_sample_out;
}

}
