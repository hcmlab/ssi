// MyVideoFeature.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2015/05/27
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

#include "MyVideoFeature.h"
#include "ssiocv.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

MyVideoFeature::MyVideoFeature () {
}

MyVideoFeature::~MyVideoFeature () {
}

void MyVideoFeature::transform_enter(ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {
}

void MyVideoFeature::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	ssi_uchar_t *inptr = ssi_pcast(ssi_uchar_t, stream_in.ptr);
	ssi_real_t *outptr = ssi_pcast(ssi_real_t, stream_out.ptr);

	ssi_uchar_t darkest = 255;
	for (int y = 0; y < _format.heightInPixels; y++) {
		for (int x = 0; x < _format.widthInPixels; x++) {
			if (inptr[x] <= darkest) {
				outptr[0] = ssi_real_t (x);
				outptr[1] = ssi_real_t (y);
				darkest = inptr[x];
			}
		}
		inptr += ssi_video_stride(_format);
	}

	outptr[0] /= _format.widthInPixels;
	outptr[1] /= _format.heightInPixels;
}

void MyVideoFeature::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {
}

}
