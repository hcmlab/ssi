// MvgMinMax.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/12/09
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

#include "MvgMinMax.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {
	
ssi_char_t *MvgMinMax::ssi_log_name = "mvgminmax_";

MvgMinMax::MvgMinMax (const ssi_char_t *file) 
	: _impl (0),
	_file (0) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
}

MvgMinMax::~MvgMinMax () {

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}	
}

void MvgMinMax::transform_enter (ssi_stream_t &stream_in,
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

void MvgMinMax::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	_impl->transform (info, stream_in, stream_out);
}

void MvgMinMax::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	_impl->transform_flush (stream_in, stream_out);

	delete _impl;
	_impl = 0;
}

MvgMinMax::Moving::Moving (Options &options)
	: _format (options.format),
	_windowSize(options.win),
	_numberOfBlocks(options.nblock),
	_windowSizeInSamples(0),
	_blockLengthInSamples(0),
	_history(0),
	_currentSampleIndex(0),
	_currentBlockIndex(0),
	_minmax(0),
	_store_min ((options.format & MIN) != 0),
	_store_max ((options.format & MAX) != 0) {
}

MvgMinMax::Moving::~Moving () {
}

void MvgMinMax::Moving::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out) {

	ssi_size_t sample_dimension = stream_in.dim;
	ssi_time_t sample_rate = stream_in.sr;
	
	ssi_time_t exactWindowSizeInSamples = _windowSize * sample_rate;
	ssi_time_t exactBlockLengthInSamples = exactWindowSizeInSamples / ssi_cast (ssi_time_t, _numberOfBlocks);
	ssi_size_t newBlockLengthInSamples = ssi_cast (ssi_size_t, exactBlockLengthInSamples + 0.5);
	ssi_size_t newWindowSizeInSamples = newBlockLengthInSamples * _numberOfBlocks;

	_windowSizeInSamples = newWindowSizeInSamples;
	_blockLengthInSamples = newBlockLengthInSamples;

	_history = new ssi_real_t[(_numberOfBlocks << 1) * sample_dimension];
	_minmax = new ssi_real_t[sample_dimension << 1];
	
	ssi_real_t *hist_ptr = _history;
	for(ssi_size_t i = 0; i < _numberOfBlocks; ++i)
	{
		for(ssi_size_t j = 0; j < sample_dimension; ++j)
		{
			*hist_ptr++ = REAL_MAX;
			*hist_ptr++ = -REAL_MAX;
		}
	}

	ssi_real_t *minmax_ptr = _minmax;
	for(ssi_size_t j = 0; j < sample_dimension; ++j)
	{
		*minmax_ptr++ = REAL_MAX;
		*minmax_ptr++ = -REAL_MAX;
	}

	_currentSampleIndex = _blockLengthInSamples-1;
	_currentBlockIndex = _numberOfBlocks-1;
}

void MvgMinMax::Moving::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out) {

	ssi_size_t sample_dimension = stream_in.dim;
	ssi_size_t sample_number = stream_in.num;
	
	ssi_real_t *srcptr = ssi_pcast (ssi_real_t, stream_in.ptr);
	ssi_real_t *dstptr = ssi_pcast (ssi_real_t, stream_out.ptr);
	ssi_real_t *history_ptr, *minmax_ptr;
	
	for(ssi_size_t curRelSample = 0; curRelSample < sample_number; ++curRelSample)
	{
		// check if we have to move to next block
		if(++_currentSampleIndex >= _blockLengthInSamples)
		{
			_currentSampleIndex = 0;
			if(++_currentBlockIndex >= _numberOfBlocks)
			{
				_currentBlockIndex = 0;
			}
			history_ptr = _history + sample_dimension * (_currentBlockIndex << 1);
			for(ssi_size_t forEachDimension = 0; forEachDimension < sample_dimension; ++forEachDimension)
			{
				*history_ptr++ = *(srcptr+forEachDimension);
				*history_ptr++ = *(srcptr+forEachDimension);
			}
		}

		// update min/max in current block
		history_ptr = _history + sample_dimension * (_currentBlockIndex << 1);
		for(ssi_size_t forEachDimension = 0; forEachDimension < sample_dimension; ++forEachDimension)
		{			
			if (*history_ptr > *srcptr)
			{
				*history_ptr = *srcptr;
			}
			++history_ptr;
			if (*history_ptr < *srcptr)
			{
				*history_ptr = *srcptr;
			}
			++history_ptr;
			++srcptr;
		}

		// update min/max in all blocks
		minmax_ptr = _minmax;
		for(ssi_size_t j = 0; j < sample_dimension; ++j)
		{
			*minmax_ptr++ = REAL_MAX;
			*minmax_ptr++ = -REAL_MAX;
		}
		history_ptr = _history;
		for(ssi_size_t forEachBlock = 0; forEachBlock < _numberOfBlocks; ++forEachBlock)
		{
			minmax_ptr = _minmax;
			for(ssi_size_t forEachDimension = 0; forEachDimension < sample_dimension; ++forEachDimension)
			{					
				if(*minmax_ptr > *history_ptr)
				{
					*minmax_ptr = *history_ptr;
				}
				++minmax_ptr;
				++history_ptr;
				if(*minmax_ptr < *history_ptr)
				{
					*minmax_ptr = *history_ptr;
				}
				++minmax_ptr;
				++history_ptr;
			}
		}

		// write back min/max
		minmax_ptr = _minmax;
		for(ssi_size_t forEachDimension = 0; forEachDimension < sample_dimension; ++forEachDimension)
		{
			if (_store_min) {
				*dstptr++ = *minmax_ptr;
			}
			++minmax_ptr;
			if (_store_max) {
				*dstptr++ = *minmax_ptr;
			}
			++minmax_ptr;
		}
	}
}

void MvgMinMax::Moving::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out) {
	
	delete[] _history;
	_history = 0;

	delete[] _minmax;
	_minmax = 0;

}



MvgMinMax::Sliding::Sliding (Options &options)
	: _format (options.format),
	_alpha (0),
	_1_alpha (0),
	_window_size (options.win),
	_min_hist (0),
	_max_hist (0),
	_store_min ((options.format & MIN) != 0),
	_store_max ((options.format & MAX) != 0),
	_first_call (true) {
}

MvgMinMax::Sliding::~Sliding () {
}

void MvgMinMax::Sliding::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out) {

	ssi_size_t sample_dimension = stream_in.dim;
	ssi_time_t sample_rate = stream_in.sr;

	// allocate history arrays
	_min_hist = new ssi_real_t[sample_dimension];
	_max_hist = new ssi_real_t[sample_dimension];
	
	// allocate and initialize alpha array
	_alpha = ssi_cast (ssi_real_t, 1.0 - (2.0 * sqrt (3.0)) / (_window_size * sample_rate));;
	_1_alpha = 1 - _alpha;

	// set first call to true
	_first_call = true;
}

void MvgMinMax::Sliding::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out) {

	/* Matlab code:

	[len, dim] = size (signal);
	N = round (w * sr);
	alpha = repmat (1 - (2*sqrt(3)) ./ N, 1, dim);

	if nargin < 4 | isempty (hist)
		hist.min = signal(1,:);
		hist.max = signal(1,:);
	end

	sldmin = zeros (len, dim);
	sldmax = zeros (len, dim);
	minval = hist.min;
	maxval = hist.max;
	for i = 1:len   
		x = signal(i,:);    
		minval = min (x, alpha .* minval + (1 - alpha) * x);
		maxval = max (x, alpha .* maxval + (1 - alpha) * x);    
		sldmin(i,:) = minval;    
		sldmax(i,:) = maxval;
	end
	hist.min = minval;
	hist.max = maxval;

	*/

	ssi_size_t sample_dimension = stream_in.dim;
	ssi_size_t sample_number = stream_in.num;

	ssi_real_t *srcptr = ssi_pcast (ssi_real_t, stream_in.ptr);
	ssi_real_t *dstptr = ssi_pcast (ssi_real_t, stream_out.ptr);
	ssi_real_t *minptr, *maxptr;
	ssi_real_t x, minval, maxval;

	// initialize history array
	if (_first_call) {
		minptr = _min_hist;
		maxptr = _max_hist;
		for (ssi_size_t i = 0; i < sample_dimension; ++i) {
			*minptr++ = srcptr[i];
			*maxptr++ = srcptr[i];
		}
		_first_call = false;
	}

	for (ssi_size_t i = 0; i < sample_number; ++i) {

		minptr = _min_hist;
		maxptr = _max_hist;

		for (ssi_size_t j = 0; j < sample_dimension; ++j) {

			x = *srcptr++;

			if (_store_min) {
				minval = *minptr;
				minval = min (x, _alpha * minval + _1_alpha * x);		
				*minptr++ = minval;			
				*dstptr++ = minval;
			}

			if (_store_max) {
				maxval = *maxptr;
				maxval = max (x, _alpha * maxval + _1_alpha * x);
				*maxptr++ = maxval;
				*dstptr++ = maxval;
			}
		}
	}
}

void MvgMinMax::Sliding::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out) {

	// delete history
	delete[] _min_hist; _min_hist = 0;
	delete[] _max_hist; _max_hist = 0;
}


}
