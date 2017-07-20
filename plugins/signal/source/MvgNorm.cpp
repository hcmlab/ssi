// MvgNorm.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/12/11
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

#include "MvgNorm.h"
#include "MvgMinMax.h"
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

ssi_char_t *MvgNorm::ssi_log_name = "mvgnorm___";

MvgNorm::MvgNorm (const ssi_char_t *file)
: _file (0),
	_mvg (0) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
}

MvgNorm::~MvgNorm () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}		
}

void MvgNorm::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	_range_a = _options.rangea;
	_range_d = _options.rangeb - _options.rangea;
	_norm = _options.norm;

	switch (_norm) {
		case AVGVAR: {				
			MvgAvgVar *mvgavgvar = ssi_pcast (MvgAvgVar, MvgAvgVar::Create (0));
			mvgavgvar->getOptions ()->format = MvgAvgVar::ALL;
			mvgavgvar->getOptions ()->method = _options.method == MOVING ? MvgAvgVar::MOVING : MvgAvgVar::SLIDING;
			mvgavgvar->getOptions ()->win = _options.win;
			_mvg = mvgavgvar;
			break; 
		}
		case MINMAX: {
			MvgMinMax *mvgminmax = ssi_pcast (MvgMinMax, MvgMinMax::Create (0));
			mvgminmax->getOptions ()->format = MvgMinMax::ALL;	
			mvgminmax->getOptions ()->method = _options.method == MOVING ? MvgMinMax::MOVING : MvgMinMax::SLIDING;
			mvgminmax->getOptions ()->win = _options.win;
			mvgminmax->getOptions ()->nblock = _options.nblock;
			_mvg = mvgminmax;
			break;
		}
		case SUBAVG: {
			MvgAvgVar *mvgavgvar = ssi_pcast (MvgAvgVar, MvgAvgVar::Create (0));
			mvgavgvar->getOptions ()->format = MvgAvgVar::AVG;
			mvgavgvar->getOptions ()->method = _options.method == MOVING ? MvgAvgVar::MOVING : MvgAvgVar::SLIDING;
			mvgavgvar->getOptions ()->win = _options.win;
			_mvg = mvgavgvar;
			break; 
		}
		case SUBMIN: {
			MvgMinMax *mvgminmax = ssi_pcast (MvgMinMax, MvgMinMax::Create (0));
			mvgminmax->getOptions ()->format = MvgMinMax::MIN;	
			mvgminmax->getOptions ()->method = _options.method == MOVING ? MvgMinMax::MOVING : MvgMinMax::SLIDING;
			mvgminmax->getOptions ()->win = _options.win;
			mvgminmax->getOptions ()->nblock = _options.nblock;
			_mvg = mvgminmax;
			break;
		}
	}

	ssi_stream_init (_data_tmp, 0, _mvg->getSampleDimensionOut (stream_in.dim), stream_in.byte, stream_in.type, stream_in.sr);
	_mvg->transform_enter (stream_in, _data_tmp);
}

void MvgNorm::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	/* Matlab code:

	[mins, maxs, hist] = sldminmax (x, sr, w, hist);
	s = warning('off','MATLAB:divideByZero');
	y = (x - mins) ./ (maxs - mins);
	s = warning('on','MATLAB:divideByZero');
	y(isnan (y)) = 0;
	y = range(1) + y * (range(2) - range(1));

	*/

	ssi_size_t sample_dimension = stream_in.dim;
	ssi_size_t sample_number = stream_in.num;

	ssi_stream_adjust (_data_tmp, sample_number);
	_mvg->transform (info, stream_in, _data_tmp);

	ssi_real_t *srcptr = ssi_pcast (ssi_real_t, stream_in.ptr);
	ssi_real_t *dstptr = ssi_pcast (ssi_real_t, stream_out.ptr);
	ssi_real_t *tmpptr = ssi_pcast (ssi_real_t, _data_tmp.ptr);

	switch (_norm) {

		case AVGVAR: {

			ssi_real_t x, y, avgval, varval;

			for (ssi_size_t i = 0; i < sample_number; ++i) {
				for (ssi_size_t j = 0; j < sample_dimension; ++j) {

					x = *srcptr++;
					avgval = *tmpptr++;
					varval = *tmpptr++;					
					y = (x - avgval) / sqrt (varval);
						
					*dstptr++ = y;
				}
			}
			break;
		}

		case MINMAX: {

			ssi_real_t x, y, minval, maxval, difval;

			for (ssi_size_t i = 0; i < sample_number; ++i) {
				for (ssi_size_t j = 0; j < sample_dimension; ++j) {

					x = *srcptr++;
					minval = *tmpptr++;
					maxval = *tmpptr++;
					difval = maxval - minval;

					if (difval != 0) {
						y = (x - minval) / difval;
					} else {
						y = 0;
					}
					y = _range_a + y * _range_d;
						
					*dstptr++ = y;
				}
			}
			break;
		}

		case SUBAVG: {

			ssi_real_t x, y, avgval;

			for (ssi_size_t i = 0; i < sample_number; ++i) {
				for (ssi_size_t j = 0; j < sample_dimension; ++j) {

					x = *srcptr++;
					avgval = *tmpptr++;								
					y = x - avgval;
						
					*dstptr++ = y;
				}
			}
			break;
		}

		case SUBMIN: {

			ssi_real_t x, y, minval;

			for (ssi_size_t i = 0; i < sample_number; ++i) {
				for (ssi_size_t j = 0; j < sample_dimension; ++j) {

					x = *srcptr++;
					minval = *tmpptr++;
					y = x - minval;
						
					*dstptr++ = y;
				}
			}
			break;
		}
	}
}

void MvgNorm::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num ,
	ssi_stream_t xtra_stream_in[]) {

	_mvg->transform_flush (stream_in, _data_tmp);
	ssi_stream_destroy (_data_tmp);
	delete _mvg; _mvg = 0;
}


}
