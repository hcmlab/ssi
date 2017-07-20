// PreEmphasis.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2014/10/16
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

#include "PreEmphasis.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

PreEmphasis::PreEmphasis (const ssi_char_t *file)
	: _file (0),
	k (0.97f),
	de (false),
	first_call (true),
	hist (0) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
}

PreEmphasis::~PreEmphasis () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

void PreEmphasis::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	k = _options.k;
	de = _options.de;
	first_call = true;
}

void PreEmphasis::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	ssi_size_t num = stream_in.num;
	ssi_real_t *x = ssi_pcast (ssi_real_t, stream_in.ptr);
	ssi_real_t *y = ssi_pcast (ssi_real_t, stream_out.ptr);

	if (first_call) {
		hist = *x;
		first_call = false;
	}

	if (de) {
		for (ssi_size_t n = 0; n < num; n++) {
			*(y++) = *(x) + k * hist;
			hist = *x++;
		}
	} else {
		for (ssi_size_t n = 0; n < num; n++) {
			*(y++) = *(x) - k * hist;
			hist = *x++;
		}
	}
}

}
