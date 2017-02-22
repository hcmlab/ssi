// ICVFilter.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/05/27
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

#include "ICVFilter.h"
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

ICVFilter::ICVFilter () 
	: _stride_in (0),
	_stride_out (0),
	_image_out (0),
	_image_in (0) {
}

ICVFilter::~ICVFilter () {

	if (_image_in) {
		cvReleaseImageHeader (&_image_in);
	}
	if (_image_out) {
		cvReleaseImageHeader (&_image_out);
	}
}

void ICVFilter::setFormatIn (ssi_video_params_t format) {

	_format_in = format;
	_stride_in = ssi_video_stride (format);
	_image_in = cvCreateImageHeader (cvSize (_format_in.widthInPixels, _format_in.heightInPixels), _format_in.depthInBitsPerChannel, _format_in.numOfChannels);
}

void ICVFilter::setFormatOut (ssi_video_params_t format) {

	_format_out = format;
	_stride_out = ssi_video_stride (format);
	_image_out = cvCreateImageHeader (cvSize (_format_out.widthInPixels, _format_out.heightInPixels), _format_out.depthInBitsPerChannel, _format_out.numOfChannels);
}

void ICVFilter::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	transform_enter (stream_in.sr, _image_in, _image_out, xtra_stream_in_num, xtra_stream_in);
}

void ICVFilter::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	cvSetData (_image_in, stream_in.ptr, _stride_in);
	cvSetData (_image_out, stream_out.ptr, _stride_out);

	transform (stream_in.sr, _image_in, _image_out, xtra_stream_in_num, xtra_stream_in);
}

void ICVFilter::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	transform_flush (stream_in.sr, _image_in, _image_out, xtra_stream_in_num, xtra_stream_in);
}

}
