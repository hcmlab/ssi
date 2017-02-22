// CVChange.h
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

#ifndef SSI_IMAGE_CVCHANGE_H
#define SSI_IMAGE_CVCHANGE_H

#include "ICVFilter.h"

namespace ssi {

class CVChange  : public ICVFilter {

public:

	static const ssi_char_t *GetCreateName () { return "CVChange"; };
	static IObject *Create (const ssi_char_t *file) { return new CVChange (); };
	~CVChange ();

	IOptions *getOptions () { return 0; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "calculates difference with previous frame"; };

	void setFormat (ssi_video_params_t format);
	void transform (ssi_time_t frame_rate,
		IplImage *image_in, 
		IplImage *image_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]);

protected:

	CVChange ();
	IplImage *_last;

	static ssi_char_t *ssi_log_name;

};

}

#endif
