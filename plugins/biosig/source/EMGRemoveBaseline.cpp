// EMGRemoveBaseline.cpp
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

#include "EMGRemoveBaseline.h"
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

ssi_char_t *EMGRemoveBaseline::ssi_log_name = "EMGRemoveBaseline__";

EMGRemoveBaseline::EMGRemoveBaseline (const ssi_char_t *file)
	: _file (0),
	_mvgvar_baseline(0),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
}

EMGRemoveBaseline::~EMGRemoveBaseline () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

void EMGRemoveBaseline::transform_enter(ssi_stream_t &stream_in, ssi_stream_t &stream_out, ssi_size_t xtra_stream_in_num,	ssi_stream_t xtra_stream_in[]) {

	if (stream_in.type != SSI_REAL) ssi_err ("type '%s' not supported", SSI_TYPE_NAMES[stream_in.type]);
	if (stream_in.dim != 1) ssi_err ("dimension > 1 not supported");

	ssi_real_t *ptr_in = ssi_pcast(ssi_real_t, stream_in.ptr);

	MvgAvgVar *mvgvar1 = ssi_pcast(MvgAvgVar, Factory::Create(MvgAvgVar::GetCreateName(), 0, false));
	mvgvar1->getOptions()->format = MvgAvgVar::AVG;
	mvgvar1->getOptions()->win = _options.winsize;
	mvgvar1->getOptions()->method = MvgAvgVar::MOVING;
	_mvgvar_baseline = mvgvar1;
	ssi_stream_init(_stream_var_baseline, 0, _mvgvar_baseline->getSampleDimensionOut(stream_in.dim), _mvgvar_baseline->getSampleBytesOut(stream_in.byte), _mvgvar_baseline->getSampleTypeOut(stream_in.type), stream_in.sr);
	_mvgvar_baseline->transform_enter(stream_in, _stream_var_baseline);
		
}

void EMGRemoveBaseline::transform( ITransformer::info info, ssi_stream_t &stream_in, ssi_stream_t &stream_out, ssi_size_t xtra_stream_in_num, ssi_stream_t xtra_stream_in[]) {

	ssi_size_t n = stream_in.num;
	ssi_time_t sr = stream_in.sr;

	ITransformer::info tinfo;
	tinfo.delta_num = 0;
	tinfo.delta_num = n;
	tinfo.time = info.time;

	ssi_stream_adjust(_stream_var_baseline, n);
	_mvgvar_baseline->transform(tinfo, stream_in, _stream_var_baseline);

	ssi_real_t *ptr_in = ssi_pcast(ssi_real_t, stream_in.ptr);
	ssi_real_t *ptr_var_baseline = ssi_pcast(ssi_real_t, _stream_var_baseline.ptr);
	ssi_real_t *ptr_out = ssi_pcast(ssi_real_t, stream_out.ptr);
	
	for (ssi_size_t nsamp = 0; nsamp < stream_in.num; nsamp++){

		*ptr_out = *ptr_in - *ptr_var_baseline;
		ptr_out++;
		*ptr_out = *ptr_var_baseline;
		ptr_out++;

		ptr_in++;
	}

}


void EMGRemoveBaseline::transform_flush(ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	_mvgvar_baseline->transform_flush(stream_in, _stream_var_baseline);
	ssi_stream_destroy(_stream_var_baseline);
	delete _mvgvar_baseline; _mvgvar_baseline = 0;
}

}
