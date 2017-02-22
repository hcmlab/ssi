// GSRArousalCombination.cpp
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
#include "../include/GSRArousalCombination.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

GSRArousalCombination::GSRArousalCombination (const ssi_char_t *file) 
	:	_file (0) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
}

GSRArousalCombination::~GSRArousalCombination () {

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}

void GSRArousalCombination::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

		
}

void GSRArousalCombination::transform (ITransformer::info info,
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

		ssi_real_t *ptr_in_long = ssi_pcast (ssi_real_t, stream_in.ptr);
		ssi_real_t *ptr_in_short = ssi_pcast (ssi_real_t, xtra_stream_in[0].ptr);
		ssi_real_t *ptr_out = ssi_pcast(ssi_real_t, stream_out.ptr);
		
		for(ssi_size_t nsamp = 0; nsamp < stream_in.num; nsamp++){
			if(*ptr_in_long >= 0.0f && *ptr_in_short >= 0.0f){
				*ptr_out = (sqrt(*ptr_in_short)  * sqrt(*ptr_in_long)) * ssi_real_t(1.5);
				if(*ptr_out >= 1.0f) { *ptr_out = 1.0f; }
				if(*ptr_out <= 0.0f) { *ptr_out = 0.0f; }
			}else{
				*ptr_out = 0.0f;
			}
			ptr_in_short++;
			ptr_in_long++;
			ptr_out++;
		}
}

void GSRArousalCombination::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num ,
	ssi_stream_t xtra_stream_in[] ) {

		
}

}
