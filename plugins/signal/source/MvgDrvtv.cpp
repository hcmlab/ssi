// MvgDrvtv.cpp
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

#include "MvgDrvtv.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ssi_char_t *MvgDrvtv::ssi_log_name = "mvgdrvtv__";

MvgDrvtv::MvgDrvtv (const ssi_char_t *file) 
	: _impl (0),
	_file (0) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
}

MvgDrvtv::~MvgDrvtv () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}	
}

void MvgDrvtv::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	if (_options.method == MOVING) {
		_impl = new Moving (_options);
	} else {
		_impl = new Sliding (_options);	
	}
	_impl->transform_enter (stream_in, stream_out);
}

void MvgDrvtv::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	_impl->transform (info, stream_in, stream_out);
}

void MvgDrvtv::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	_impl->transform_flush (stream_in, stream_out);

	delete _impl;
	_impl = 0;
}

MvgDrvtv::Moving::Moving (Options &options)
	: _window_size (options.win),
	_window_size_N (0),
	_history_size (0),
	_history (0),
	_history_0 (0),
	_history_N (0),
	_cumsum (0),
	_counter (0),
	_first_call (true) {
}

MvgDrvtv::Moving::~Moving () {
}

void MvgDrvtv::Moving::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out) {

	ssi_size_t sample_dimension = stream_in.dim;
	ssi_time_t sample_rate = stream_in.sr;

	// calculate history size
	_window_size_N = ssi_cast (ssi_size_t, _window_size * sample_rate + 0.5);
	_history_size = _window_size_N + 1;

	// allocate history array
	_history = new ssi_real_t[_history_size * sample_dimension];
	_history_0 = _history + (_history_size - 1);
	_history_N = _history;
	_counter = _history_size;
	_cumsum = new ssi_real_t[sample_dimension];

	// set first call to true
	_first_call = true;
}

void MvgDrvtv::Moving::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out) {

	/* Matlab code:

	[len, dim] = size (signal);
	N = round (w * sr);

	if nargin < 4 | isempty (hist)
	    
		% history
		hist.x = repmat (signal(1,:), N+1, 1); 
		% store history pointer
		hist.head = N;  
		% store sum
		hist.sum = N .* signal (1,:);
	end

	% allocate result matrix
	drv = zeros (len, dim);

	% retrieve history from last call
	head = hist.head;
	x = hist.x;
	sumval = hist.sum;

	for i = 1:len
	    
		% increment history
		head = hinc (head, N);
	    
		% retreive last sample of history
		% the correct position depends on N
		% hpos gives us the correct position for each column
		x_N = x(hpos (head, N, N),:);
		x_0 = signal(i,:);
	    
		% insert new sample in history
		x(head,:) = x_0;   
	  
		% calculate average
		drv(i,:) = (x_N + x_0) ./ 2 - sumval ./ N;

		% update sum
		sumval = sumval - x_N + x_0;
	end
	 
	% update history
	hist.head = head;
	hist.x = x;
	hist.sum = sumval;

	*/

	ssi_size_t sample_dimension = stream_in.dim;
	ssi_size_t sample_number = stream_in.num;

	ssi_real_t *srcptr = ssi_pcast (ssi_real_t, stream_in.ptr);
	ssi_real_t *dstptr = ssi_pcast (ssi_real_t, stream_out.ptr);
	ssi_real_t *histptr;
	ssi_real_t x_0, x_N, sum;

	// initialize history array
	if (_first_call) {
		histptr = _history;
		for (ssi_size_t i = 0; i < _history_size; ++i) {
			for (ssi_size_t j = 0; j < sample_dimension; ++j) {			
				*histptr++ = srcptr[j];
			}
		}
		for (ssi_size_t j = 0; j < sample_dimension; ++j) {			
			_cumsum[j] = _window_size_N * srcptr[j];
		}
		_first_call = false;
	}

	for (ssi_size_t i = 0; i < sample_number; ++i) {

		// increment history;
		++_counter;
		_history_N += sample_dimension;
		_history_0 += sample_dimension;
		if (_counter > _history_size) {
			_counter = 1;
			_history_0 = _history;
		}
		if (_counter == _history_size) {
			_history_N = _history;
		}

		for (ssi_size_t j = 0; j < sample_dimension; ++j) {

			x_0 = *srcptr++;
			x_N = _history_N[j];
			sum = _cumsum[j];

			// insert new sample
			_history_0[j] = x_0;

			// calculate derivative
			*dstptr++ = 0.5f * (x_N + x_0) - sum / _window_size_N;

			// update sum
			_cumsum[j] = sum - x_N + x_0;
		}
	}
}

void MvgDrvtv::Moving::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out) {

	// delete history
	delete[] _cumsum; _cumsum = 0;
	delete[] _history; _history = 0;
}


MvgDrvtv::Sliding::Sliding (Options &options)
	: _alpha (0),
	_1_alpha (0),
	_window_size (options.win),
	_drv_hist (0),
	_first_call (true) {
}

MvgDrvtv::Sliding::~Sliding () {
}

void MvgDrvtv::Sliding::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out) {

	ssi_size_t sample_dimension = stream_in.dim;
	ssi_time_t sample_rate = stream_in.sr;

	// allocate history arrays
	_drv_hist = new ssi_real_t[sample_dimension];
	
	// allocate and initialize alpha array
	_alpha = ssi_cast (ssi_real_t, 1.0 - (2.0 * sqrt (3.0)) / (_window_size * sample_rate));;
	_1_alpha = 1 - _alpha;

	// set first call to true
	_first_call = true;
}

void MvgDrvtv::Sliding::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out) {

	/* Matlab code:

	[len, dim] = size (signal);
	N = round (w * sr);
	alpha = repmat (1 - (2*sqrt(3)) ./ N, 1, dim);

	if nargin < 4 | isempty (hist)
		hist.drv = signal(1,:);
	end

	drv = zeros (len, dim);
	drvval = hist.drv;
	for i = 1:len   
		x = signal(i,:);
		drv(i,:) = x - drvval;    
		drvval = alpha .* drvval + (1 - alpha) .* x;    
	 
	end
	hist.drv = drvval;

	*/

	ssi_size_t sample_dimension = stream_in.dim;
	ssi_size_t sample_number = stream_in.num;

	ssi_real_t *srcptr = ssi_pcast (ssi_real_t, stream_in.ptr);
	ssi_real_t *dstptr = ssi_pcast (ssi_real_t, stream_out.ptr);
	ssi_real_t *drvptr;
	ssi_real_t x, drvval;

	// initialize history array
	if (_first_call) {
		drvptr = _drv_hist;
		for (ssi_size_t i = 0; i < sample_dimension; ++i) {
			*drvptr++ = srcptr[i];
		}
		_first_call = false;
	}

	for (ssi_size_t i = 0; i < sample_number; ++i) {

		drvptr = _drv_hist;

		for (ssi_size_t j = 0; j < sample_dimension; ++j) {

			x = *srcptr++;

			drvval = *drvptr;			
			*dstptr++ = x - drvval;

			drvval = _alpha * drvval + _1_alpha * x;
			*drvptr++ = drvval;
		}
	}
}

void MvgDrvtv::Sliding::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out) {

	// delete history
	delete[] _drv_hist; _drv_hist = 0;
}


}
