// CVResize.h
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

#pragma once

#ifndef SSI_IMAGE_CVRESIZE_H
#define SSI_IMAGE_CVRESIZE_H

#include "ICVFilter.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class CVResize  : public ICVFilter {

public:

	enum METHOD {
		NN = 0,
		LINEAR = 1,
		CUBIC = 2,
		AREA = 3
	};

public:

	class Options : public OptionList {

	public:

		Options () : width (1.0f), height (1.0f), method (LINEAR), scaled(true) {

			addOption ("width", &width, 1, SSI_REAL, "new width in pixels");		
			addOption ("height", &height, 1, SSI_REAL, "new height in pixels");
			addOption ("method", &method, 1, SSI_INT, "interpolation method (0=NN,1=LINEAR,2=CUBIC,3=AREA)");
			addOption ("scaled", &scaled, 1, SSI_BOOL, "width and height are given in [0..1]");
		};


		void setResize (ssi_real_t width, ssi_real_t height, METHOD method = LINEAR) {

			if (width <= 1.0f && height <= 1.0f)
			{
				scaled = true;
			}

			this->width = width;
			this->height = height;
			this->method = method;			
		}

		ssi_real_t width, height;
		METHOD method;
		bool scaled;
		
	};

public:

	static const ssi_char_t *GetCreateName () { return "CVResize"; };
	static IObject *Create (const ssi_char_t *file) { return new CVResize (file); };
	~CVResize ();

	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "resize image"; };

	void setFormat (ssi_video_params_t format);
	void transform (ssi_time_t frame_rate,
		IplImage *image_in, 
		IplImage *image_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]);

protected:

	CVResize (const ssi_char_t *file);
	Options _options;
	ssi_char_t *_file;

	static ssi_char_t *ssi_log_name;
};

}

#endif
