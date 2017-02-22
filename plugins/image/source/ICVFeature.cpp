// ICVFeature.cpp
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

#include "ICVFeature.h"
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

ICVFeature::ICVFeature () 
	: _stride_in (0),
	_image_in (0) {
}

ICVFeature::~ICVFeature () {

	if (_image_in) {
		cvReleaseImageHeader (&_image_in);
	}
}

void ICVFeature::setFormatIn (ssi_video_params_t format) {

	_format_in = format;
	_stride_in = ssi_video_stride (format);
	_image_in = cvCreateImageHeader (cvSize (_format_in.widthInPixels, _format_in.heightInPixels), _format_in.depthInBitsPerChannel, _format_in.numOfChannels);
}

void ICVFeature::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	transform_enter(stream_in.sr, _image_in, stream_out, xtra_stream_in_num, xtra_stream_in);
}

void ICVFeature::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	cvSetData (_image_in, stream_in.ptr, _stride_in);

	transform (stream_in.sr, _image_in, stream_out, xtra_stream_in_num, xtra_stream_in);
}

void ICVFeature::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	transform_flush (stream_in.sr, _image_in, stream_out, xtra_stream_in_num, xtra_stream_in);
}

}
