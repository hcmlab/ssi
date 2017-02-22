// ISSplitStream.cpp
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

#include "ISSelectDim.h"
#include "signal/SignalTools.h"
#include "ISSplitStream.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

int ISSplitStream::compare (const void * a, const void * b)
{
  return ( *(ssi_size_t*)a - *(ssi_size_t*)b );
}

ISSplitStream::ISSplitStream (ISamples *samples)
	: _samples (*samples),
	_n_streams (0),
	_streams (0),
	_offsets (0) {

	if (samples->getStreamSize () != 1) {
		ssi_err ("#streams of input samples must be equal to 1");
	}
}

ISSplitStream::~ISSplitStream () {

	delete[] _streams;
	for (ssi_size_t i = 0; i < _n_streams; i++) {
		if (_offsets[i]) {
			ssi_stream_destroy (*_sample_out.streams[i]);
			delete _sample_out.streams[i];
		}
		delete _offsets[i];
	}
	delete[] _sample_out.streams;
	delete[] _offsets;
}	

void ISSplitStream::setSplitting ( ssi_size_t n_splits, ssi_size_t *n_dims, ssi_size_t **dims) {

	_n_streams = n_splits;

	_streams = new ssi_stream_t[_n_streams];
	ssi_sample_create (_sample_out, _n_streams, 0, 0, 0, 0);
	for (ssi_size_t i = 0; i < _n_streams; i++) {
		_sample_out.streams[i] = 0;
	}

	_offsets = new ssi_size_t *[_n_streams];
	for (ssi_size_t nsplit = 0; nsplit < n_splits; nsplit++) {

		ssi_size_t ndim = n_dims[nsplit];		

		// sortiert die gewählten Dimensionen mit quicksort
		ssi_size_t *dims_sort = new ssi_size_t[ndim];
		memcpy (dims_sort, dims[nsplit], sizeof (ssi_size_t) * ndim);
		qsort (dims_sort, ndim, sizeof(ssi_size_t), compare);

		// bestimmt den Offset für die ausgewählten dims (dim_index * byte)
		_offsets[nsplit] = new ssi_size_t[ndim+1];
		ssi_stream_t stream = _samples.getStream (0);
		ssi_size_t byte = stream.byte;
		_offsets[nsplit][0] = dims_sort[0] * byte;
		for (ssi_size_t i = 1; i < ndim; i++) {
			_offsets[nsplit][i] = (dims_sort[i] - dims_sort[i-1]) * byte;
		}
		_offsets[nsplit][ndim] = (_samples.getStream (0).dim - dims_sort[ndim-1]) * byte;

		_sample_out.streams[nsplit] = new ssi_stream_t;
		ssi_stream_init (*_sample_out.streams[nsplit], 0, ndim, stream.byte, stream.type, stream.sr); 
		ssi_stream_init (_streams[nsplit], 0, ndim, stream.byte, stream.type, stream.sr); 

		delete[] dims_sort;

	}
}

SSI_INLINE void ISSplitStream::select () {

	// Angabe: SampleListe enthält nur einen Stream
	int _stream_index = 0;

	for (ssi_size_t i = 0; i < _n_streams; i++) {
		if (_offsets[i]) {
			ssi_stream_t &sout = *_sample_out.streams[i];

			// Pointer auf den ursprünglichen stream
			ssi_stream_t &sin = *_sample_in.streams[_stream_index];
			ssi_size_t byte = sin.byte;
			ssi_size_t num = sin.num;
			ssi_stream_adjust (sout, num);
			ssi_byte_t *ptrin = sin.ptr;
			ssi_byte_t *ptrout = sout.ptr;
			ssi_size_t ndims = _streams[i].dim;
			ssi_size_t *offsets = _offsets[i];
			for (ssi_size_t nnum = 0; nnum < num; nnum++) {
				for (ssi_size_t ndim = 0; ndim < ndims; ndim++) {
					ptrin += offsets[ndim];
					memcpy (ptrout, ptrin, byte);
					ptrout += byte;
				}
				ptrin += offsets[ndims];
			}
		} else {
			_sample_out.streams[i] = _sample_in.streams[_stream_index];
		}
		_sample_out.class_id = _sample_in.class_id;
		_sample_out.score = _sample_in.score;
		_sample_out.time = _sample_in.time;
		_sample_out.user_id = _sample_in.user_id;
	}
	
}

ssi_sample_t *ISSplitStream::get (ssi_size_t index) {

	_sample_in = *(_samples.get (index));
	select ();
	return &_sample_out;	
}

ssi_sample_t *ISSplitStream::next () {

	ssi_sample_t *tmp = _samples.next ();
	if (!tmp) {
		return 0;
	}

	_sample_in = *tmp;
	select ();
	return &_sample_out;
}

}
