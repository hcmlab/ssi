// CVColor.cpp
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

#include "CVColor.h"
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

ssi_char_t *CVColor::ssi_log_name = "ocvcolor__";

CVColor::CVColor (const ssi_char_t *file) 
	: _file (0) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
}

CVColor::~CVColor () {

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}

void CVColor::transform (ssi_time_t frame_rate,
	IplImage *image_in, 
	IplImage *image_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	cvCvtColor (image_in, image_out, _options.code);
};

void CVColor::setFormat (ssi_video_params_t format) {

	ICVFilter::setFormatIn (format);
	switch (_options.code) {

		case BGR2BGRA: 
		case BGR2RGBA: 
		case BGRA2RGBA:
		case GRAY2BGRA:
		case BGR5652BGRA:
		case BGR5652RGBA:
		case BGR5552BGRA:
		case BGR5552RGBA:

			format.numOfChannels = 4;
			ICVFilter::setFormatOut (format);
			break;
	
		case BGRA2BGR:
		case RGBA2BGR:
		case BGR2RGB:
		case BGR2BGR565:
		case RGB2BGR565:
		case BGR5652BGR:
		case BGR5652RGB:
		case BGRA2BGR565:
		case RGBA2BGR565:
		case BGR2BGR555:
		case GRAY2BGR555:
		case BGR2XYZ:
		case RGB2XYZ:
		case XYZ2BGR:
		case XYZ2RGB:
		case BGR2YCrCb:
		case RGB2YCrCb:
		case YCrCb2BGR:
		case YCrCb2RGB:
		case BGR2HSV:
		case RGB2HSV:
		case BGR2Lab:
		case RGB2Lab:
		case BayerBG2BGR:
		case BayerGB2BGR:
		case BayerRG2BGR:
		case BayerGR2BGR:
		case BGR2Luv:
		case RGB2Luv:
		case BGR2HLS:
		case RGB2HLS:
		case HSV2BGR:
		case HSV2RGB:
		case Lab2BGR:
		case Lab2RGB:
		case Luv2BGR:
		case Luv2RGB:
		case HLS2BGR:
		case HLS2RGB:
		case RGB2BGR555:
		case BGR5552BGR:
		case BGR5552RGB:
		case BGRA2BGR555:
		case RGBA2BGR555:

			format.numOfChannels = 3;
			ICVFilter::setFormatOut (format);
			break;

		case BGR2GRAY:
		case RGB2GRAY:
		case BGRA2GRAY:
		case RGBA2GRAY:
		case BGR5652GRAY:
		case BGR5552GRAY:

			format.numOfChannels = 1;
			ICVFilter::setFormatOut (format);
			break;

		case GRAY2BGR:		   
		case GRAY2BGR565:
		
			format.numOfChannels = 3;
			ICVFilter::setFormatOut (format);
			break;

		default:
			ssi_err ("unkown color conversion code");
	}
}

}
