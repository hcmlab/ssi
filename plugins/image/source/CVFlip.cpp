// CVFlip.cpp
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

#include "CVFlip.h"
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

ssi_char_t *CVFlip::ssi_log_name = "ocvflip___";

CVFlip::CVFlip (const ssi_char_t *file) 
	: _file (0),
	_mode (0),
	_copy (false) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
}

CVFlip::~CVFlip () {

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}

void CVFlip::transform_enter (ssi_time_t frame_rate,
	IplImage *image_in, 
	IplImage *image_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {
}

void CVFlip::transform (ssi_time_t frame_rate,
	IplImage *image_in, 
	IplImage *image_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	if (_copy) {
		cvCopy (image_in, image_out);
	} else {
		cvFlip (image_in, image_out, _mode); 
	}
}

void CVFlip::setFormat (ssi_video_params_t format) {

	ICVFilter::setFormatIn (format);	

	_copy = false;
	if (_options.flip && !_options.mirror) {
		format.flipImage = !format.flipImage;
		_mode = 0;
	} else if (!_options.flip && _options.mirror) {
		_mode = 1;		
	} else if (_options.flip && _options.mirror) {
		format.flipImage = !format.flipImage;
		_mode = -1;
	} else {
		_copy = true;
	}
	
	ICVFilter::setFormatOut (format);
}

}
