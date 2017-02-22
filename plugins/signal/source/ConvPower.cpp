// ConvPower.cpp
// author: Johannes Wagner <johannes.wagner@informatik.uni-augsburg.de>
// created: 2008/01/18
// Copyright (C) 2007-11 University of Augsburg, Johannes Wagner
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute itand/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#include "ConvPower.h"
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

ConvPower::ConvPower (const ssi_char_t *file)
	: _file (0) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
}

ConvPower::~ConvPower () {

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}

void ConvPower::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	ssi_size_t dim = stream_in.dim;
	ssi_size_t num = stream_in.num;

	ssi_real_t *srcptr = ssi_pcast (ssi_real_t, stream_in.ptr);
	ssi_real_t *dstptr = ssi_pcast (ssi_real_t, stream_out.ptr);
	
	int lenz = 2 * num - 1;
	ssi_real_t *z = new ssi_real_t[dim*lenz];
	conv (num, num, dim, srcptr, srcptr, z);

	for (ssi_size_t j = 0; j < dim; j++) {
		dstptr[j] = 0;
	}
	ssi_real_t *zptr = z;
	for (int i = 0; i < lenz; i++) {
		for (ssi_size_t j = 0; j < dim; j++) {
			dstptr[j] += *zptr * *zptr;
			zptr++;
		}
	}
	for (ssi_size_t j = 0; j < dim; j++) {
		dstptr[j] = sqrt (dstptr[j]);
	}
}

}

