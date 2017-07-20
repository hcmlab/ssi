// Pulse.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/01/16
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

#include "Pulse.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

Pulse::Pulse (const ssi_char_t *file)
	: _delta (0),
	_minval (0),
	_maxval (0),
	_minpos (0),
	_maxpos (0),
	_lookformax (0),
	_offset (0),
	_min_dist (0), 
	_max_dist (0),
	_last (0),
	_new_rate (0),
	_old_rate (0),
	_min_rate (0),
	_max_rate (0),
	_counter (0),
	_file (0) {		

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
}

Pulse::~Pulse () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

void Pulse::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	ssi_size_t sample_dimension = stream_in.dim;
	ssi_time_t sample_rate = stream_in.sr;

	_min_rate = _options.min_rate;
	_max_rate = _options.max_rate;
	_delta = _options.delta;

	_lookformax = new bool[sample_dimension];
	_minval = new ssi_real_t[sample_dimension];
	_maxval = new ssi_real_t[sample_dimension];
	_minpos = new ssi_size_t[sample_dimension];
	_maxpos = new ssi_size_t[sample_dimension];
	_offset = 0;	
	if (_min_rate > 0) {
		_max_dist = ssi_cast (ssi_size_t, (60.0 / _min_rate) * sample_rate + 0.5);
	} else {
		_max_dist = 0xFFFFFFFF;
	}
	if (_max_rate > 0) {
		_min_dist = ssi_cast (ssi_size_t, (60.0 / _max_rate) * sample_rate + 0.5); 
	} else {
		_min_dist = 0;
	}
	_last = new ssi_size_t[sample_dimension];
	_new_rate = new ssi_real_t[sample_dimension];
	_old_rate = new ssi_real_t[sample_dimension];
	_counter = new ssi_size_t[sample_dimension];
	_first_call = true;
}

void Pulse::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	/* Matlab Code:

	dists = [];
	for i=1:num
	    
		this = frame(i);
		if this > maxval, maxval = this; maxpos = offset+i; end
		if this < minval, minval = this; minpos = offset+i; end

		if lookformax
			if this < maxval - delta
				minval = this; 
				minpos = offset+i;
				% decide if we keep it
				candidate = maxpos - last;
				if candidate >= min_dist %& candidate > 0.6 * last_dist
					% decide if add extra
					if candidate <= max_dist %& (last_dist == 0 | candidate < 1.4 * last_dist)
						% add candidate
						dists = [dists; candidate];
						last_dist = candidate;
					end
					last = maxpos;                
				end
				lookformax = 0;
			end
		else
			if this > minval + delta
				maxval = this; 
				maxpos = offset+i;            
				lookformax = 1;
			end
		end
	end

	if not (isempty (dists))
		rate = (rate + 60 / (mean (dists ./ sr))) / 2;
	end

	*/

	ssi_size_t sample_dimension = stream_in.dim;
	ssi_size_t sample_number = stream_in.num;
	ssi_time_t sample_rate = stream_in.sr;

	ssi_real_t *srcptr = ssi_pcast (ssi_real_t, stream_in.ptr);
	ssi_real_t *dstptr = ssi_pcast (ssi_real_t, stream_out.ptr);

	if (_first_call) {
		for (ssi_size_t i = 0; i < sample_dimension; i++) {
			_lookformax[i] = false;
			_minval[i] = srcptr[i];
			_maxval[i] = srcptr[i];
			_last[i] = 0;
			_new_rate[i] = (_max_rate + _min_rate) / 2.0f;
		}
		_first_call = false;
	}

	for (ssi_size_t i = 0; i < sample_dimension; i++) {
		_old_rate[i] = _new_rate[i];
		_new_rate[i] = 0;
		_counter[i] = 0;
	}

	ssi_real_t value;
	ssi_size_t candidate;
	for (ssi_size_t j = 0; j < sample_number; j++) {
		for (ssi_size_t i = 0; i < sample_dimension; i++) {

			value = *srcptr++;

			if (value < _minval[i]) {
				_minval[i] = value;
				_minpos[i] = _offset + j;
			} else if (value > _maxval[i]) {
				_maxval[i] = value;
				_maxpos[i] = _offset + j;
			}

			if (_lookformax[i]) {
				if (value < _maxval[i] - _delta) {
					_minval[i] = value;
					_minpos[i] = _offset + j;
					_lookformax[i] = false;
					candidate = _maxpos[i] - _last[i];
					if (candidate >= _min_dist) {
						if (candidate <= _max_dist) {
							++_counter[i];
							_new_rate[i] += candidate;
						}
						_last[i] = _maxpos[i];
					}
				}
			} else {
				if (value > _minval[i] + _delta) {
					_maxval[i] = value;
					_maxpos[i] = _offset + j;
					_lookformax[i] = true;
				}
			}			
		}
	}

	for (ssi_size_t i = 0; i < sample_dimension; i++) {
		if (_counter[0] > 0) {
			_new_rate[i] = (_old_rate[i] + 60.0f / ((_new_rate[i] / _counter[i]) / ssi_cast (ssi_real_t, sample_rate))) / 2;
		} else {
			_new_rate[i] = _old_rate[i];
		}
		*dstptr++ = _new_rate[i];
	}

	_offset += sample_number;
}

void Pulse::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num ,
	ssi_stream_t xtra_stream_in[]) {

	delete[] _minval; _minval = 0;
	delete[] _maxval; _maxval = 0;
	delete[] _minpos; _minpos = 0;
	delete[] _maxpos; _maxpos = 0;
	delete[] _lookformax; _lookformax = 0;
	delete[] _last; _last = 0;
	delete[] _new_rate; _new_rate = 0;
	delete[] _old_rate; _old_rate = 0;
	delete[] _counter; _counter = 0;
}

}
