// Multiply.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/01/03
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

#include "Multiply.h"
#include "signal/MatrixOps.h"
#include "base/Factory.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif
#if __gnu_linux__
using std::min;
using std::max;
#endif
namespace ssi {
Multiply::Multiply (const ssi_char_t *file) 
	: _factors (0),
	_n_factors (0),
	_factor (0),
	_file (0) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
}

Multiply::~Multiply () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

void Multiply::readOptions() {

	_options.lock();

	if (_options.single) {
		for (ssi_size_t i = 0; i < _n_factors; i++) {
			_factors[i] = _factor;
		}
	}
	else {
		ssi_size_t n_found = ssi_string2array_count(_options.factors, ',');
		if (n_found < _n_factors) {
			ssi_wrn("#factors (%u) < #dimensions (%u)", n_found, _n_factors);
		}
		ssi_real_t *tmp = new ssi_real_t[n_found];
		ssi_string2array(n_found, tmp, _options.factors, ',');
		for (ssi_size_t i = 0; i < min(n_found, _n_factors); i++) {
			_factors[i] = tmp[i];
		}
		delete[] tmp;
	}
	_join = _options.join;

	_options.unlock();

}

void Multiply::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {
	
	_factor = _options.factor;
	_n_factors = stream_in.dim;
	_factors = new ssi_real_t[_n_factors];
	for (ssi_size_t i = 0; i < _n_factors; i++) {
		_factors[i] = 1.0f;
	}

	readOptions();
}

void Multiply::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {	

	ssi_real_t *srcptr = ssi_pcast (ssi_real_t, stream_in.ptr);
	ssi_real_t *dstptr = ssi_pcast (ssi_real_t, stream_out.ptr);

	ssi_real_t result;
	switch (_join) {
		case JOIN::OFF:		
			for (ssi_size_t i = 0; i < stream_in.num; i++) {
				for (ssi_size_t j = 0; j < stream_in.dim; j++) {
					*dstptr++ = _factors[j] * *srcptr++;
				}	
			}
			break;
		case JOIN::MULT:			
			for (ssi_size_t i = 0; i < stream_in.num; i++) {	
				result = 1.0f;
				for (ssi_size_t j = 0; j < stream_in.dim; j++) {	
					result *= _factors[j] * *srcptr++;
				}
				*dstptr++ = result;
			}			
			break;
		case JOIN::SUM:
			for (ssi_size_t i = 0; i < stream_in.num; i++) {	
				result = 0.0f;
				for (ssi_size_t j = 0; j < stream_in.dim; j++) {	
					result += _factors[j] * *srcptr++;
				}
				*dstptr++ = result;
			}	
			break;
		case JOIN::SUMSQUARE:
			result = 0.0f;
			ssi_real_t tmp = 0.0f;
			for (ssi_size_t i = 0; i < stream_in.num; i++) {	
				result = 0.0f;
				for (ssi_size_t j = 0; j < stream_in.dim; j++) {		
					tmp = _factors[j] * *srcptr++;
					result += tmp * tmp;
				}
				*dstptr++ = result;
			}
			break;
	}
}

void Multiply::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num ,
	ssi_stream_t xtra_stream_in[]) {

	delete[] _factors; _factors = 0;
}

bool Multiply::notify(COMMAND::List command, const ssi_char_t *message) {

	if (command == INotify::COMMAND::OPTIONS_CHANGE) {
		if (_factors) {
			readOptions();
			return true;
		}
	}

	return false;
}

}
