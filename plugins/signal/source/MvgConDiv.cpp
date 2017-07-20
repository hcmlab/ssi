// MvgConDiv.cpp
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

#include "MvgConDiv.h"
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

ssi_char_t *MvgConDiv::ssi_log_name = "mvgnorm___";

MvgConDiv::MvgConDiv (const ssi_char_t *file)
: _file (0),
	_mvgs (0),
	_mvgl (0) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
}

MvgConDiv::~MvgConDiv () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}		
}

void MvgConDiv::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {
		
	MvgAvgVar *mvgs = ssi_pcast (MvgAvgVar, MvgAvgVar::Create (0));
	mvgs->getOptions ()->format = MvgAvgVar::AVG;
	mvgs->getOptions ()->method = _options.method == MOVING ? MvgAvgVar::MOVING : MvgAvgVar::SLIDING;
	mvgs->getOptions ()->win = _options.wins;
	_mvgs = mvgs;
	ssi_stream_init (_mvgs_tmp, 0, _mvgs->getSampleDimensionOut (stream_in.dim), stream_in.byte, stream_in.type, stream_in.sr);
	_mvgs->transform_enter (stream_in, _mvgs_tmp);

	MvgAvgVar *mvgl = ssi_pcast (MvgAvgVar, MvgAvgVar::Create (0));
	mvgl->getOptions ()->format = MvgAvgVar::AVG;
	mvgl->getOptions ()->method = _options.method == MOVING ? MvgAvgVar::MOVING : MvgAvgVar::SLIDING;
	mvgl->getOptions ()->win = _options.winl;
	_mvgl = mvgl;
	ssi_stream_init (_mvgl_tmp, 0, _mvgl->getSampleDimensionOut (stream_in.dim), stream_in.byte, stream_in.type, stream_in.sr);
	_mvgl->transform_enter (stream_in, _mvgl_tmp);
}

void MvgConDiv::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	/* Matlab code:

	[avg, vrc, hist] = avgfun (signal, sr, [N_short, N_long], hist);
	macd = avg(:,1:2:dim*2) - avg(:,2:2:dim*2);

	*/

	ssi_size_t sample_dimension = stream_in.dim;
	ssi_size_t sample_number = stream_in.num;

	ssi_stream_adjust (_mvgs_tmp, sample_number);
	ssi_stream_adjust (_mvgl_tmp, sample_number);
	
	_mvgs->transform (info, stream_in, _mvgs_tmp);
	_mvgl->transform (info, stream_in, _mvgl_tmp);	

	ssi_real_t *srcptr = ssi_pcast (ssi_real_t, stream_in.ptr);
	ssi_real_t *dstptr = ssi_pcast (ssi_real_t, stream_out.ptr);
	ssi_real_t *mvgsptr = ssi_pcast (ssi_real_t, _mvgs_tmp.ptr);
	ssi_real_t *mvglptr = ssi_pcast (ssi_real_t, _mvgl_tmp.ptr);

	ssi_real_t short_value, long_value;
	for (ssi_size_t i = 0; i < sample_number; ++i) {
		for (ssi_size_t j = 0; j < sample_dimension; ++j) {
			short_value = *mvgsptr++;
			long_value = *mvglptr++;
			*dstptr++ = short_value - long_value;
		}
	}
}

void MvgConDiv::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num ,
	ssi_stream_t xtra_stream_in[]) {

	_mvgs->transform_flush (stream_in, _mvgs_tmp);
	_mvgl->transform_flush (stream_in, _mvgl_tmp);
	ssi_stream_destroy (_mvgs_tmp);
	ssi_stream_destroy (_mvgl_tmp);
	delete _mvgs; _mvgs = 0;
	delete _mvgl; _mvgl = 0;
}


}
