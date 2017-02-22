// MvgAvgVar.cpp
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

#include "MvgAvgVar.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ssi_char_t *MvgAvgVar::ssi_log_name = "mvgavgvar_";

MvgAvgVar::MvgAvgVar (const ssi_char_t *file) 
	: _impl (0),
	_file (0) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
}

MvgAvgVar::~MvgAvgVar () {

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}	
}

void MvgAvgVar::transform_enter (ssi_stream_t &stream_in,
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

void MvgAvgVar::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	_impl->transform (info, stream_in, stream_out);
}

void MvgAvgVar::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	_impl->transform_flush (stream_in, stream_out);

	delete _impl;
	_impl = 0;
}

MvgAvgVar::Moving::Moving (Options &options)
	: _window_size (options.win),
	_window_size_N (0),
	_history_size (0),
	_history (0),
	_history_0 (0),
	_history_N (0),
	_cumsum (0),
	_cumsum_2 (0),
	_counter (0),
	_first_call (true),
	_format (options.format) {
}

MvgAvgVar::Moving::~Moving () {
}

void MvgAvgVar::Moving::transform_enter (ssi_stream_t &stream_in,
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
	_cumsum_2 = new ssi_real_t[sample_dimension];

	// set first call to true
	_first_call = true;
}

void MvgAvgVar::Moving::transform (ITransformer::info info,
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
		hist.sum2 = N .* (signal (1,:) .^ 2);
	end

	% allocate result matrix
	avg = zeros (len, dim);
	vrc = zeros (len, dim);

	% retrieve history from last call
	head = hist.head;
	x = hist.x;
	sumval = hist.sum;
	sumval2 = hist.sum2;

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
	  
		sumval = sumval - x_N + x_0;
		sumval2 = sumval2 - x_N.*x_N + x_0.*x_0;    

		% calculate average/variance and store result
		avg(i,:) = sumval ./ N;
		vrc(i,:) = (N .* sumval2 - sumval.^2) ./ (N .* (N - 1));

	end

	% update history
	hist.head = head;
	hist.x = x;
	hist.sum = sumval;
	hist.sum2 = sumval2;

	*/

	ssi_size_t sample_dimension = stream_in.dim;
	ssi_size_t sample_number = stream_in.num;

	ssi_real_t *srcptr = ssi_pcast (ssi_real_t, stream_in.ptr);
	ssi_real_t *dstptr = ssi_pcast (ssi_real_t, stream_out.ptr);
	ssi_real_t *histptr;
	ssi_real_t x_0, x_N, sum, sum_2;
	ssi_real_t var;

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
			_cumsum_2[j] = _window_size_N * srcptr[j] * srcptr[j];
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
			sum_2 = _cumsum_2[j];

			// insert new sample
			_history_0[j] = x_0;

			// update sum
			sum = sum - x_N + x_0;
			sum_2 = sum_2 - x_N * x_N + x_0 * x_0;

			// calculate avg and var
			if (_format & MvgAvgVar::AVG) {
				*dstptr++ = sum / _window_size_N;
			}
			if (_format & MvgAvgVar::VAR) {
				var = (_window_size_N * sum_2 - sum * sum) / (_window_size_N * (_window_size_N - 1));
				*dstptr++ = var > 0 ? var : FLT_EPSILON;
			}

			_cumsum[j] = sum;
			_cumsum_2[j] = sum_2;
		}
	}
}

void MvgAvgVar::Moving::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out) {

	// delete history
	delete[] _cumsum; _cumsum = 0;
	delete[] _cumsum_2; _cumsum_2 = 0;
	delete[] _history; _history = 0;
}


MvgAvgVar::Sliding::Sliding (Options &options)
	: _format (options.format),	
	_alpha (0),
	_1_alpha (0),
	_window_size (options.win),
	_avg_hist (0),
	_var_hist (0),
	_first_call (true) {
}

MvgAvgVar::Sliding::~Sliding () {
}

void MvgAvgVar::Sliding::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out) {

	ssi_size_t sample_dimension = stream_in.dim;
	ssi_time_t sample_rate = stream_in.sr;

	// allocate history arrays
	_avg_hist = new ssi_real_t[sample_dimension];
	_var_hist = new ssi_real_t[sample_dimension];
	
	// allocate and initialize alpha array
	_alpha = ssi_cast (ssi_real_t, 1.0 - (2.0 * sqrt (3.0)) / (_window_size * sample_rate));
	_1_alpha = 1 - _alpha;

	// set first call to true
	_first_call = true;
}

void MvgAvgVar::Sliding::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out) {

	/* Matlab code:

	[len, dim] = size (data);
	number = length (Ns);
	alpha = repmat (1 - (2*sqrt(3)) ./ Ns(:)', 1, dim);

	inds = repmat (1:dim, number, 1);
	data = data(:,inds(:));

	if nargin == 2
		hist.avg = data(1,:);
		hist.vrc = zeros (1,number*dim);
	end

	avg = zeros (len, dim*number);
	vrc = zeros (len, dim*number);
	avg_ = hist.avg;
	vrc_ = hist.vrc;
	for i = 1:len
	    
		x = data(i,:);
		avg_ = alpha .* avg_ + (1 - alpha) .* x;    
		vrc_ = alpha .* vrc_ + (1 - alpha) .* (x - avg_).^2;
		avg(i,:) = avg_;
		vrc(i,:) = vrc_;
	 
	end
	hist.avg = avg_;
	hist.vrc = vrc_;

	*/

	ssi_size_t sample_dimension = stream_in.dim;
	ssi_size_t sample_number = stream_in.num;

	ssi_real_t *srcptr = ssi_pcast (ssi_real_t, stream_in.ptr);
	ssi_real_t *dstptr = ssi_pcast (ssi_real_t, stream_out.ptr);
	ssi_real_t *avgptr, *varptr;
	ssi_real_t x, x_avg, avg, var;

	// initialize history array
	if (_first_call) {
		avgptr = _avg_hist;
		varptr = _var_hist;
		for (ssi_size_t i = 0; i < sample_dimension; ++i) {			
			*avgptr++ = srcptr[i];
			*varptr++ = 0;			
		}
		_first_call = false;
	}

	// do transformation
	switch (_format) {

		case MvgAvgVar::AVG: {

			for (ssi_size_t i = 0; i < sample_number; ++i) {

				avgptr = _avg_hist;

				for (ssi_size_t j = 0; j < sample_dimension; ++j) {

					x = *srcptr++;

					avg = *avgptr;
					avg = _alpha * avg + _1_alpha * x;

					*avgptr++ = avg;
					*dstptr++ = avg;
				}
			}

			break;
		}

		case MvgAvgVar::VAR:
		case MvgAvgVar::ALL: {

			bool store_all = _format == MvgAvgVar::ALL;

			for (ssi_size_t i = 0; i < sample_number; ++i) {

				avgptr = _avg_hist;
				varptr = _var_hist;

				for (ssi_size_t j = 0; j < sample_dimension; ++j) {

					x = *srcptr++;

					avg = *avgptr;
					var = *varptr;

					avg = _alpha * avg + _1_alpha * x;
					x_avg = x - avg;
					var = _alpha * var + _1_alpha * x_avg * x_avg;
					var = var > 0 ? var : FLT_EPSILON;

					*avgptr++ = avg;
					*varptr++ = var;
					if (store_all)
						*dstptr++ = avg;
					*dstptr++ = var;

				}
			}

			break;
		}
	}
}

void MvgAvgVar::Sliding::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out) {

	delete[] _avg_hist;
	_avg_hist = 0;
	delete[] _var_hist;
	_var_hist = 0;	
	_alpha = 0;	
	_1_alpha = 0;
}


}
