// Merge.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2014/02/27
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
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

#include "Merge.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

Merge::Merge (const ssi_char_t *file)
	: _file (0) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
}

Merge::~Merge () {

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}


void Merge::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	ssi_size_t sum = 0;
	for (ssi_size_t i = 0; i < xtra_stream_in_num; i++) {
		sum += xtra_stream_in[i].dim;
	}

	if (sum != _options.dims) {
		ssi_err ("#dimension (%u) of additional streams does not fit #dimension (%u) set in options", sum, _options.dims);
	}
}

void Merge::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	if (_options.warning) {
		for (ssi_size_t i = 0; i < xtra_stream_in_num; i++) {
			if (xtra_stream_in[i].num != stream_in.num) {
				ssi_wrn ("#samples (%u) in additional stream#%u' != #samples (%u) in main stream", xtra_stream_in[i].num, i, stream_in.num);
			}
		}
	}

	ssi_real_t **srcptr = new ssi_real_t *[1 + xtra_stream_in_num];
	ssi_size_t *srcnum = new ssi_size_t[1 + xtra_stream_in_num];
	ssi_size_t *srcdim = new ssi_size_t[1 + xtra_stream_in_num];
	srcptr[0] = ssi_pcast (ssi_real_t, stream_in.ptr);
	srcnum[0] = stream_in.num;
	srcdim[0] = stream_in.dim;
	for (ssi_size_t i = 0; i < xtra_stream_in_num; i++) {
		srcptr[i+1] = ssi_pcast (ssi_real_t, xtra_stream_in[i].ptr);
		srcnum[i+1] = xtra_stream_in[i].num;
		srcdim[i+1] = xtra_stream_in[i].dim;
	}
	ssi_real_t *dstptr = ssi_pcast (ssi_real_t, stream_out.ptr);
	
	for (ssi_size_t i = 0; i < stream_out.num; i++) {
		for (ssi_size_t j = 0; j < 1 + xtra_stream_in_num; j++) {
			for (ssi_size_t k = 0; k < srcdim[j]; k++) {
				*dstptr++ = i < srcnum[j] ? *(srcptr[j])++ : 0;
			}
		}
	}

	delete[] srcptr;
	delete[] srcnum;
	delete[] srcdim;
}

void Merge::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {
}

}

