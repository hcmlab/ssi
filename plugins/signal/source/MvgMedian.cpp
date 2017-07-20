// MvgMedian.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/12/12
// Copyright (C) 2007-10 University of Augsburg, Johannes Wagner
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

#include "MvgMedian.h"
#include "MvgMedianHelper.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ssi_char_t *MvgMedian::ssi_log_name = "mvgmedian_";

MvgMedian::MvgMedian (const ssi_char_t *file) 
	: _median (0),
	_ndim (0),
	_nwin (0),
	_first_call (true),
	_file (0) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
}

MvgMedian::~MvgMedian () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}	
}

void MvgMedian::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	_first_call = true;
	
	_ndim = stream_in.dim;
	if (_options.winInSamples > 0) {
		_nwin = _options.winInSamples;
	} else {
		_nwin = ssi_cast (ssi_size_t, _options.win * stream_in.sr + 0.5);	
	}

	_median = new MvgMedianHelper *[_ndim];
	for (ssi_size_t i = 0; i < _ndim; i++) {
		_median[i] = new MvgMedianHelper (_nwin);
	}
}

void MvgMedian::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {
	
	ssi_real_t *src = ssi_pcast (ssi_real_t, stream_in.ptr);
	ssi_real_t *dst = ssi_pcast (ssi_real_t, stream_out.ptr);

	if (_first_call) {
		for (ssi_size_t i = 0; i < _ndim; i++) {
			_median[i]->init (src[i]);
		}
		_first_call = false;
	}
		
	for (ssi_size_t i = 0; i < stream_in.num; i++) {
		for (ssi_size_t j = 0; j < stream_in.dim; j++) {					
			*dst++ = _median[j]->move (*src++);
		}	
	}
}

void MvgMedian::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	for (ssi_size_t i = 0; i <_ndim; i++) {
		delete[] _median[i];		
	}	
	delete[] _median; _median = 0;	
	
}




}
