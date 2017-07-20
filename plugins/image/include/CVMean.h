// CVMean.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2017/07/19
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

#ifndef SSI_IMAGE_CVMEAN_H
#define SSI_IMAGE_CVMEAN_H

#include "ICVFeature.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class CVMean  : public ICVFeature {

public:

	class Options : public OptionList {

	public:

		Options() : scale(false) {

			addOption("scale", &scale, 1, SSI_BOOL, "scale mean values to [0..1]");
		};

		bool scale;
	};

public:

	static const ssi_char_t *GetCreateName () { return "CVMean"; };
	static IObject *Create (const ssi_char_t *file) { return new CVMean (); };
	~CVMean ();

	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "calculates mean of channels"; };

	void setFormat (ssi_video_params_t format);
	void transform(ssi_time_t frame_rate,
		IplImage *image_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]);

	ssi_size_t getSampleDimensionOut(ssi_size_t sample_dimension_in);
	ssi_size_t getSampleBytesOut(ssi_size_t sample_bytes_in);
	ssi_type_t getSampleTypeOut(ssi_type_t sample_type_in);

protected:

	CVMean ();

	static ssi_char_t *ssi_log_name;

	Options _options;
};

}

#endif
