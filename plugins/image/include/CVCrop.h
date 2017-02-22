// CVCrop.h
// author: Johannes Wagner <johannes.wagner@informatik.uni-augsburg.de>
// created: 2008/07/29
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

#pragma once

#ifndef SSI_IMAGE_CVCROP_H
#define SSI_IMAGE_CVCROP_H

#include "ICVFilter.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class CVCrop  : public ICVFilter {

public:

	struct ORIGIN {
		enum List {
			LEFTTOP = 0,
			CENTER
		};
	};

	struct METHOD {
		enum List {
			NN = 0,
			LINEAR = 1,
			CUBIC = 2,
			AREA = 3
		};
	};
public:

	class Options : public OptionList {

	public:

		Options () : width (0), height (0), method (METHOD::LINEAR), origin (ORIGIN::LEFTTOP), scaled (true), flip (false), keep (false) {

			region[0] = 0.0f;
			region[1] = 0.0f;
			region[2] = 1.0f;
			region[3] = 1.0f;

			addOption ("flip", &flip, 1, SSI_BOOL, "flip coordinate system to left upper corner");
			addOption ("keep", &keep, 1, SSI_BOOL, "keep last cropped image if region out of range");			
			addOption ("origin", &origin, 1, SSI_INT, "origin (0=left top, 1=center)");
			addOption ("region", region, 4, SSI_REAL, "crop region if not provided in an extra stream (x, y, width, height)");
			addOption ("scaled", &scaled, 1, SSI_BOOL, "coordinates are scaled to [0..1]");
			addOption ("width", &width, 1, SSI_SIZE, "rescale to width in pixels (ignored if 0)");
			addOption ("height", &height, 1, SSI_SIZE, "rescale to height in pixels (ignored if 0)");
			addOption ("method", &method, 1, SSI_INT, "interpolation method (0=NN,1=LINEAR,2=CUBIC,3=AREA)");
		};

		void setResize (ssi_size_t width, ssi_size_t height, METHOD::List method = METHOD::LINEAR) {
			this->width = width;
			this->height = height;
			this->method = method;
		}

		ssi_size_t width, height;
		METHOD::List method;
		ORIGIN::List origin;
		ssi_real_t region[4];
		bool scaled;
		bool flip;
		bool keep;
	};

public:

	static const ssi_char_t *GetCreateName () { return "CVCrop"; };
	static IObject *Create (const ssi_char_t *file) { return new CVCrop (file); };
	~CVCrop ();

	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "crop image region"; };

	void setFormat (ssi_video_params_t format);
	void transform (ssi_time_t frame_rate,
		IplImage *image_in, 
		IplImage *image_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]);

protected:

	CVCrop (const ssi_char_t *file);
	Options _options;
	ssi_char_t *_file;
	
	static ssi_char_t *ssi_log_name;

	void cropAndResize (IplImage *image_in, IplImage *image_out);
	IplImage *_tmpImage;
	CvRect *_region;
	CvSize *_size;
};

}

#endif
