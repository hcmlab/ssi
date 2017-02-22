// Energy.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/01/18
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

#include "Energy.h"
#include "signal/mathext.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

Energy::Energy (const ssi_char_t *file)
	: _file (0) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
}

Energy::~Energy () {

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}

void Energy::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	ssi_size_t sample_dimension = stream_in.dim;
	ssi_size_t sample_number = stream_in.num;

	ssi_real_t *srcptr = ssi_pcast (ssi_real_t, stream_in.ptr);
	ssi_real_t *dstptr = ssi_pcast (ssi_real_t, stream_out.ptr);

	if (_options.global) {

		ssi_real_t sum = 0;
		ssi_size_t elems = sample_number * sample_dimension;
		ssi_real_t value;
		for (ssi_size_t i = 0; i < elems; i++) {
			value = *srcptr++;
			sum += value * value;
		}
		*dstptr = sqrt (sum / elems);

	} else {

		ssi_real_t *dstptr_tmp = dstptr;
		ssi_real_t value;
		for (ssi_size_t i = 0; i < sample_dimension; i++) {
			value = *srcptr++;
			*dstptr++ = value * value;
		}
		for (ssi_size_t i = 1; i < sample_number; i++) {
			dstptr = dstptr_tmp;	
			for (ssi_size_t j = 0; j < sample_dimension; j++) {
				value = *srcptr++;
				*dstptr++ += value * value;
			}
		}
		dstptr = dstptr_tmp;
		for (ssi_size_t i = 0; i < sample_dimension; i++) {
			value = *dstptr;
			*dstptr++ = sqrt (value / sample_number);
		}
	}
}

}

