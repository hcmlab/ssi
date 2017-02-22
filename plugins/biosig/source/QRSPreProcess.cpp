// QRSPreProcess.cpp
// author: Florian Lingenfelser <florian.lingenfelser@informatik.uni-augsburg.de>
// created: 2012/12/14
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
#include "../include/QRSPreProcess.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

QRSPreProcess::QRSPreProcess (const ssi_char_t *file) 
	: _history (0),
	_sum (0),
	_file (0) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
}

QRSPreProcess::~QRSPreProcess () {

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}

void QRSPreProcess::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

		_history = new ssi_real_t[_options.N];
		_sum = 0;
		_first_call = true;
		_head = 0;
}

void QRSPreProcess::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	
		ssi_size_t sample_dimension = stream_in.dim;
		ssi_size_t sample_number = stream_in.num;

		SSI_ASSERT(sample_dimension == 1);

		ssi_real_t *ptr_in = ssi_pcast (ssi_real_t, stream_in.ptr);
		ssi_real_t *ptr_out = ssi_pcast (ssi_real_t, stream_out.ptr);

		// init _history with processed values of first sample
		if (_first_call) {
			for (ssi_size_t k = 0; k < _options.N; k++) {
				_history[k] = sqrt((*ptr_in)*(*ptr_in));
			}
			_sum = _options.N * sqrt((*ptr_in)*(*ptr_in));
			_first_call = false;
		}

		// now filter input
		ssi_real_t value;

		for (ssi_size_t i = 0; i < sample_number; i++) {
			// get value, square it and draw sqrt
			value = sqrt((*ptr_in)*(*ptr_in));
			// update sum
			_sum += value - _history[_head];
			// normalize sum
			*ptr_out = _sum / (_options.N+1);
			// update _history			
			_history[_head] = value;			
			// move head
			_head = (_head + 1) % _options.N;

			ptr_in++;
			ptr_out++;
		}

}

void QRSPreProcess::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num ,
	ssi_stream_t xtra_stream_in[] ) {

		if(_history){
			delete[] _history;
			_history = 0;
		}
}

}
