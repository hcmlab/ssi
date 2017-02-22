// ISSelectDim.cpp
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

#include "ISSelectDim.h"
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

int ISSelectDim::Compare (const void * a, const void * b)
{
  return ( *(ssi_size_t*)a - *(ssi_size_t*)b );
}

ISSelectDim::ISSelectDim (ISamples *samples)
	: _samples (samples),
	_n_streams (0),
	_streams (0),
	_offsets (0),
	_dims (0) {

	_n_streams = _samples->getStreamSize ();

	_streams = new ssi_stream_t[_n_streams];
	_offsets = new int *[_n_streams];
	_dims = new ssi_size_t *[_n_streams];
	ssi_sample_create (_sample_out, _n_streams, 0, 0, 0, 0);
	for (ssi_size_t i = 0; i < _n_streams; i++) {
		_offsets[i] = 0;
		_sample_out.streams[i] = 0;
		_dims[i] = 0;
		ssi_stream_init (_streams[i], 0, 0, samples->getStream (i).byte, samples->getStream (i).type, samples->getStream (i).sr, 0);
	}
}

ISSelectDim::~ISSelectDim () {

	release ();
}	

void ISSelectDim::release () {

	delete[] _streams;
	for (ssi_size_t i = 0; i < _n_streams; i++) {
		if (_offsets[i]) {
			ssi_stream_destroy (*_sample_out.streams[i]);
			delete _sample_out.streams[i];
		}
		delete _offsets[i];
		delete _dims[i];
	}
	delete[] _sample_out.streams;
	_sample_out.streams = 0;
	delete[] _offsets;
	delete[] _dims;
}

bool ISSelectDim::setSelection (ssi_size_t index, 
	ssi_size_t n_dims, 
	const ssi_size_t dims[],
	bool sort_dims) {

	if (!_samples) {
		ssi_wrn ("samples not set");
		return false;
	}

	if (index >= _n_streams) {
		ssi_wrn ("index exceeds stream size");
		return false;
	} 

	delete[] _dims[index];
	_dims[index] = new ssi_size_t[n_dims];
	memcpy (_dims[index], dims, sizeof (ssi_size_t) * n_dims);
	if (sort_dims) {
		qsort (_dims[index], n_dims, sizeof(ssi_size_t), Compare);
	}

	_streams[index].dim = n_dims;
	delete[] _offsets[index];
	_offsets[index] = new int[n_dims+1];
	ssi_stream_t &stream = *_samples->get (0)->streams[index];
	ssi_size_t byte = stream.byte;
	_offsets[index][0] = _dims[index][0] * byte;
	for (ssi_size_t i = 1; i < n_dims; i++) {
		_offsets[index][i] = (ssi_cast (int, _dims[index][i]) - ssi_cast (int, _dims[index][i-1])) * byte;
	}
	_offsets[index][n_dims] = (_samples->getStream (index).dim - _dims[index][n_dims-1]) * byte;
	if (_sample_out.streams[index]) {
		ssi_stream_destroy (*_sample_out.streams[index]);
	}
	delete _sample_out.streams[index];
	_sample_out.streams[index] = new ssi_stream_t;
	ssi_stream_init (*_sample_out.streams[index], 0, n_dims, stream.byte, stream.type, stream.sr);   

	return true;
}

SSI_INLINE void ISSelectDim::select () {

	for (ssi_size_t i = 0; i < _n_streams; i++) {
		if (_offsets[i]) {
			ssi_stream_t &sout = *_sample_out.streams[i];
			ssi_stream_t &sin = *_sample_in.streams[i];
			ssi_size_t byte = sin.byte;
			ssi_size_t num = sin.num;
			ssi_stream_adjust (sout, num);
			ssi_byte_t *ptrin = sin.ptr;
			ssi_byte_t *ptrout = sout.ptr;
			ssi_size_t ndims = _streams[i].dim;
			int *offsets = _offsets[i];
			for (ssi_size_t nnum = 0; nnum < num; nnum++) {
				for (ssi_size_t ndim = 0; ndim < ndims; ndim++) {
					ptrin += offsets[ndim];
					memcpy (ptrout, ptrin, byte);
					ptrout += byte;
				}
				ptrin += offsets[ndims];
			}
		} else {
			_sample_out.streams[i] = _sample_in.streams[i];
		}
		_sample_out.class_id = _sample_in.class_id;
		_sample_out.score = _sample_in.score;
		_sample_out.time = _sample_in.time;
		_sample_out.user_id = _sample_in.user_id;
	}
	
}

ssi_sample_t *ISSelectDim::get (ssi_size_t index) {

	ssi_sample_t *tmp = _samples->get (index);

	if (tmp == 0) {
		return 0;
	}

	_sample_in = *tmp;
	select ();
	return &_sample_out;	
}

ssi_sample_t *ISSelectDim::next () {

	ssi_sample_t *tmp = _samples->next ();
	if (!tmp) {
		return 0;
	}

	_sample_in = *tmp;
	select ();
	return &_sample_out;
}

void ISSelectDim::print (FILE *file) {

	if (!_samples) {
		ssi_wrn ("samples not set");
		return;
	}

	for (ssi_size_t i = 0; i < _n_streams; i++) {
		ssi_fprint (file, "stream#%u:", i);
		if (_dims[i]) {
			for (ssi_size_t j = 0; j < _streams[i].dim; j++) {
				ssi_fprint (file, " %u", _dims[i][j]);
			}
		} else {
			ssi_fprint (file, " NOT SET");
		}
		ssi_fprint (file, "\n");
	}	

}


}
