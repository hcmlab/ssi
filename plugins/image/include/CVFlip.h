// CVFlip.h
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

#ifndef SSI_IMAGE_CVFLIP_H
#define SSI_IMAGE_CVFLIP_H

#include "ICVFilter.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class CVFlip  : public ICVFilter {

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

		Options () : flip (true), mirror (false) {

			addOption ("flip", &flip, 1, SSI_BOOL, "flip image");		
			addOption ("mirror", &mirror, 1, SSI_BOOL, "mirror image");		
		};

		bool flip, mirror;
		
	};

public:

	static const ssi_char_t *GetCreateName () { return "CVFlip"; };
	static IObject *Create (const ssi_char_t *file) { return new CVFlip (file); };
	~CVFlip ();

	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "flip / mirror image"; };

	void setFormat (ssi_video_params_t format);
	void transform_enter (ssi_time_t frame_rate,
		IplImage *image_in, 
		IplImage *image_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]);
	void transform (ssi_time_t frame_rate,
		IplImage *image_in, 
		IplImage *image_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]);

protected:

	CVFlip (const ssi_char_t *file);
	Options _options;
	ssi_char_t *_file;

	int _mode;
	bool _copy;

	static ssi_char_t *ssi_log_name;
};

}

#endif
