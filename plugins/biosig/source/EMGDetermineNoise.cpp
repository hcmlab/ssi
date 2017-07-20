// EMGDetermineNoise.cpp
// author: Daniel Schork
// created: 2016
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

#include "EMGDetermineNoise.h"
#include "../../signal/include/MvgAvgVar.h"
#include "base/Factory.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ssi_char_t *EMGDetermineNoise::ssi_log_name = "EMGDetermineNoise__";

ssi_size_t n_current = 0;
ssi_size_t n_window = 0;
ssi_real_t sum = 0;

EMGDetermineNoise::EMGDetermineNoise (const ssi_char_t *file)
	: _file (0),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
}

EMGDetermineNoise::~EMGDetermineNoise () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}



void EMGDetermineNoise::transform_enter(ssi_stream_t &stream_in, ssi_stream_t &stream_out, ssi_size_t xtra_stream_in_num,	ssi_stream_t xtra_stream_in[]) {

	if (stream_in.type != SSI_REAL) ssi_err ("type '%s' not supported", SSI_TYPE_NAMES[stream_in.type]);
	if (stream_in.dim != 1) ssi_err ("dimension > 1 not supported");

	ssi_real_t *ptr_in = ssi_pcast(ssi_real_t, stream_in.ptr);

	MvgAvgVar *mvgavg = ssi_pcast(MvgAvgVar, Factory::Create(MvgAvgVar::GetCreateName(), 0, false));
	mvgavg->getOptions()->format = MvgAvgVar::AVG;
	mvgavg->getOptions()->win = _options.winsize;
	mvgavg->getOptions()->method = MvgAvgVar::MOVING;
	_mvgavg = mvgavg;
	ssi_stream_init(_stream_avg, 0, _mvgavg->getSampleDimensionOut(stream_in.dim), _mvgavg->getSampleBytesOut(stream_in.byte), _mvgavg->getSampleTypeOut(stream_in.type), stream_in.sr);
	_mvgavg->transform_enter(stream_in, _stream_avg);

	n_window = _options.winsize * stream_in.sr;
	n_current = 0;
	sum = 0;
}

void EMGDetermineNoise::transform( ITransformer::info info, ssi_stream_t &stream_in, ssi_stream_t &stream_out, ssi_size_t xtra_stream_in_num, ssi_stream_t xtra_stream_in[]) {

	ssi_size_t n = stream_in.num;
	ssi_time_t sr = stream_in.sr;

	ITransformer::info tinfo;
	tinfo.delta_num = 0;
	tinfo.delta_num = n;
	tinfo.time = info.time;

	ssi_stream_adjust(_stream_avg, n);
	_mvgavg->transform(tinfo, stream_in, _stream_avg);

	ssi_real_t *ptr_in = ssi_pcast(ssi_real_t, stream_in.ptr);
	ssi_real_t *ptr_avg = ssi_pcast(ssi_real_t, _stream_avg.ptr);
	ssi_real_t *ptr_out = ssi_pcast(ssi_real_t, stream_out.ptr);
	
	for (ssi_size_t nsamp = 0; nsamp < stream_in.num; nsamp++){

		if (n_current > n_window){
			*ptr_out = *ptr_avg;
		}
		else{
			sum += *ptr_in;
			*ptr_out = sum / (n_current + 1);
			n_current++;
		}

		ptr_in++;
		ptr_avg++;
		ptr_out++;

	}

}


void EMGDetermineNoise::transform_flush(ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	_mvgavg->transform_flush(stream_in, _stream_avg);
	ssi_stream_destroy(_stream_avg);
	delete _mvgavg; _mvgavg = 0;
}

}
