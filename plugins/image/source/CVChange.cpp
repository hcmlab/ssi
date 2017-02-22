// CVChange.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/07/29
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

#include "CVChange.h"
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

ssi_char_t *CVChange::ssi_log_name = "ocvchange_";	

CVChange::CVChange () 
	: _last(0) {
}

CVChange::~CVChange () {

	if (_last) {
		cvReleaseImage (&_last);
	}
}

void CVChange::transform (ssi_time_t frame_rate,
	IplImage *image_in, 
	IplImage *image_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	if (_last) {
		//cvAddWeighted(_last, 0.5, image_in, -0.5, 128, image_out);
		cvSub (_last, image_in, image_out);
		cvReleaseImage(&_last);
	} else {
		cvAddWeighted(image_in, 0.5, image_in, -0.5, 128, image_out);
		cvSub (image_in, image_in, image_out);
	}
	_last = cvCloneImage (image_in);
}

void CVChange::setFormat (ssi_video_params_t format) {
	ICVFilter::setFormatIn (format);
	ICVFilter::setFormatOut (format);
}

}
