// QRSHeartRate.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2013/01/17
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Smart Sensor Integration (SSI) developed at the 
// Lab for Multimedia Concepts and Applications of the University of Augsburg
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

#include "QRSHeartRate.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

QRSHeartRate::QRSHeartRate (const ssi_char_t *file)
	: _last_r (-1),
	_last_hr (0),
	_count (0),
	_file (0) {		

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
}

QRSHeartRate::~QRSHeartRate () {

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}

void QRSHeartRate::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	_last_r = -1;
	_last_hr = _options.defhr;
	_count = 0;
}

void QRSHeartRate::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	/* Matlab Code:

	n_pulse = length (pulse);
	hr_sr = pulse_sr/n_pulse;

	rpos = find (pulse > 0) + hist.count;
	n_rpos = length (rpos);
	hr = hist.lasthr;
	for i=1:n_rpos
		if hist.lastr ~= -1        
			hr = 60 / ((rpos(i) - hist.lastr) / pulse_sr);
			if hist.lasthr > 0
				%hr = (hr + hist.lasthr ) / 2;            
			end
		end
		hist.lastr = rrpos(i);    
		hist.lasthr = hr;
	end

	hist.count = hist.count + n_pulse;

	*/

	ssi_real_t pulse_sr = ssi_cast (ssi_real_t, stream_in.sr);
	ssi_size_t n_pulse = stream_in.num;
	ssi_real_t *pulse = ssi_pcast (ssi_real_t, stream_in.ptr);
	
	ssi_size_t rpos = _count;
	ssi_real_t hr = _last_hr;
	for (ssi_size_t i = 0; i < n_pulse; i++) {
		int tmp = *pulse;
		if (*pulse++ != 0) {
			rpos += i;
			if (_last_r != ssi_size_t (-1)) {
				hr = 60.0f / ((rpos - _last_r) / pulse_sr);
				if (_options.smooth) {
					hr = (hr + _last_hr) / 2.0f;
				}
			}
			_last_r = rpos;
			_last_hr = hr;
		}		
	}
	_count += n_pulse;

	*(ssi_pcast(ssi_real_t, stream_out.ptr)) = hr;
}

void QRSHeartRate::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num ,
	ssi_stream_t xtra_stream_in[]) {
}

}
