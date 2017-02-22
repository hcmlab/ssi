// CVColor.h
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

#pragma once

#ifndef SSI_IMAGE_CVCOLOR_H
#define SSI_IMAGE_CVCOLOR_H

#include "ICVFilter.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class CVColor : public ICVFilter {

public:

	enum CODE {
		BGR2BGRA    = 0,
		BGRA2BGR    = 1,
		BGR2RGBA    = 2,
		RGBA2BGR    = 3,		
		BGR2RGB     = 4,
		BGRA2RGBA   = 5,		
		BGR2GRAY    = 6,
		RGB2GRAY    = 7,
		GRAY2BGR    = 8,		
		GRAY2BGRA   = 9,		
		BGRA2GRAY   = 10,
		RGBA2GRAY   = 11,
		BGR2BGR565  = 12,
		RGB2BGR565  = 13,
		BGR5652BGR  = 14,
		BGR5652RGB  = 15,
		BGRA2BGR565 = 16,
		RGBA2BGR565 = 17,
		BGR5652BGRA = 18,
		BGR5652RGBA = 19,
		GRAY2BGR565 = 20,
		BGR5652GRAY = 21,
		BGR2BGR555  = 22,
		RGB2BGR555  = 23,
		BGR5552BGR  = 24,
		BGR5552RGB  = 25,
		BGRA2BGR555 = 26,
		RGBA2BGR555 = 27,
		BGR5552BGRA = 28,
		BGR5552RGBA = 29,
		GRAY2BGR555 = 30,
		BGR5552GRAY = 31,
		BGR2XYZ     = 32,
		RGB2XYZ     = 33,
		XYZ2BGR     = 34,
		XYZ2RGB     = 35,
		BGR2YCrCb   = 36,
		RGB2YCrCb   = 37,
		YCrCb2BGR   = 38,
		YCrCb2RGB   = 39,
		BGR2HSV     = 40,
		RGB2HSV     = 41,
		BGR2Lab     = 44,
		RGB2Lab     = 45,
		BayerBG2BGR = 46,
		BayerGB2BGR = 47,
		BayerRG2BGR = 48,
		BayerGR2BGR = 49,
		BGR2Luv     = 50,
		RGB2Luv     = 51,
		BGR2HLS     = 52,
		RGB2HLS     = 53,
		HSV2BGR     = 54,
		HSV2RGB     = 55,
		Lab2BGR     = 56,
		Lab2RGB     = 57,
		Luv2BGR     = 58,
		Luv2RGB     = 59,
		HLS2BGR     = 60,
		HLS2RGB     = 61
	};

public:

	class Options : public OptionList {

	public:

		Options () : code (BGR2GRAY) {

			addOption ("code", &code, 1, SSI_INT, "color code (e.g. 2=BGR2RGB, 6=RGB2GRAY, 37=RGB2YCrCb, 41=RGB2HSV, 45=RGB2Lab, 51=RGB2Luv)");		
		};

		CODE code;
	};

public:

	static const ssi_char_t *GetCreateName () { return "CVColor"; };
	static IObject *Create (const ssi_char_t *file) { return new CVColor (file); };
	~CVColor ();

	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "converts video frame in another color space"; };

	void setFormat (ssi_video_params_t format);
	void transform (ssi_time_t frame_rate,
		IplImage *image_in, 
		IplImage *image_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]);

protected:

	CVColor (const ssi_char_t *file);
	Options _options;
	ssi_char_t *_file;

	static ssi_char_t *ssi_log_name;
};

}

#endif
