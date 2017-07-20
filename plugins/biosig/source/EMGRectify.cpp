// EMGRectify.cpp
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

#include "EMGRectify.h"
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

ssi_char_t *EMGRectify::ssi_log_name = "EMGRectify__";

EMGRectify::EMGRectify (const ssi_char_t *file)
	: _file (0),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
}

EMGRectify::~EMGRectify () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

void EMGRectify::transform_enter(ssi_stream_t &stream_in, ssi_stream_t &stream_out, ssi_size_t xtra_stream_in_num,	ssi_stream_t xtra_stream_in[]) {

	if (stream_in.type != SSI_REAL) ssi_err ("type '%s' not supported", SSI_TYPE_NAMES[stream_in.type]);
	if (stream_in.dim > 2) ssi_err ("dimension > 2 not supported");
		
}

void EMGRectify::transform( ITransformer::info info, ssi_stream_t &stream_in, ssi_stream_t &stream_out, ssi_size_t xtra_stream_in_num, ssi_stream_t xtra_stream_in[]) {

	ssi_real_t *ptr_in = ssi_pcast(ssi_real_t, stream_in.ptr);
	ssi_real_t *ptr_out = ssi_pcast(ssi_real_t, stream_out.ptr);


	for (ssi_size_t nsamp = 0; nsamp < stream_in.num; nsamp++){

		*ptr_out = (*ptr_in > 0 ? *ptr_in : -(*ptr_in) );

		ptr_in += stream_in.dim;
		ptr_out++;
	}

}


void EMGRectify::transform_flush(ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

}

}
