// GSRRemoveBaseline.cpp
// author: Florian Lingenfelser <lingenfelser@hcm-lab.de>
// created: 2013/02/15 
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

#include "GSRRemoveBaseline.h"
#include "../../signal/include/MvgAvgVar.h"
#include "../../signal/include/MvgNorm.h"
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

GSRRemoveBaseline::GSRRemoveBaseline (const ssi_char_t *file) 
	:	_file (0),
		_mvgvar (0){

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
}

GSRRemoveBaseline::~GSRRemoveBaseline () {

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}

void GSRRemoveBaseline::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

		MvgAvgVar *mvgvar = ssi_pcast (MvgAvgVar, Factory::Create (MvgAvgVar::GetCreateName (), 0, false));
		mvgvar->getOptions ()->format = MvgAvgVar::ALL;
		mvgvar->getOptions ()->win = _options.winsize;
		mvgvar->getOptions ()->method = MvgAvgVar::SLIDING;
		_mvgvar = mvgvar;
		ssi_stream_init (_var_stream, 0, _mvgvar->getSampleDimensionOut (stream_in.dim), _mvgvar->getSampleBytesOut (stream_in.byte), _mvgvar->getSampleTypeOut (stream_in.type), stream_in.sr);
		_mvgvar->transform_enter (stream_in, _var_stream);

		
}

void GSRRemoveBaseline::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

		ssi_size_t n = stream_in.num;
		ssi_time_t sr = stream_in.sr;

		ITransformer::info tinfo;
		tinfo.delta_num = 0;
		tinfo.delta_num = n;
		tinfo.time = info.time;

		ssi_stream_adjust (_var_stream, n);
		_mvgvar->transform (tinfo, stream_in, _var_stream);
		
		ssi_real_t *var = ssi_pcast (ssi_real_t, _var_stream.ptr);
		ssi_real_t *ptr_in = ssi_pcast (ssi_real_t, stream_in.ptr);
		ssi_real_t *ptr_out = ssi_pcast(ssi_real_t, stream_out.ptr);
		
		for(ssi_size_t nsamp = 0; nsamp < stream_in.num; nsamp++){
			
			*ptr_out = *ptr_in - *var;
			
			ptr_in++;
			ptr_out++;
			var += 2;
		}
}

void GSRRemoveBaseline::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num ,
	ssi_stream_t xtra_stream_in[] ) {

		_mvgvar->transform_flush (stream_in, _var_stream);
		ssi_stream_reset (_var_stream);
		delete _mvgvar; _mvgvar = 0;
}

}


