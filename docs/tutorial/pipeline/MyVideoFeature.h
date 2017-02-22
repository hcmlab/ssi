// MyVideoFeature.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2015/05/27
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

#ifndef _MYVIDEOFEATURE_H
#define _MYVIDEOFEATURE_H

#include "base/IFeature.h"

namespace ssi {

class MyVideoFeature : public IFeature {

public:

	static const ssi_char_t *GetCreateName() { return "MyVideoFeature"; };
	static IObject *Create(const ssi_char_t *file) { return new MyVideoFeature(); };
	~MyVideoFeature();

	IOptions *getOptions() { return 0; };
	const ssi_char_t *getName() { return GetCreateName(); };
	const ssi_char_t *getInfo() { return "seeks darkest pixel"; };

	void transform_enter(ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);
	void transform(ITransformer::info info,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);
	void transform_flush(ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);

	ssi_size_t getSampleDimensionOut(ssi_size_t sample_dimension_in) {
		if (sample_dimension_in > 1) {
			ssi_err("#dimension > 1 not supported");
		}
		return 2;
	}

	ssi_size_t getSampleBytesOut(ssi_size_t sample_bytes_in) {
		if (sample_bytes_in != ssi_video_size(_format)) {
			ssi_err("#bytes not compatible");
		}
		return sizeof(ssi_real_t);
	}

	ssi_type_t getSampleTypeOut(ssi_type_t sample_type_in) {
		if (sample_type_in != SSI_IMAGE) {
			ssi_err("unsupported type");
		}
		return SSI_REAL;
	}

	void setMetaData (ssi_size_t size, const void *meta) {
		if (sizeof(_format) != size) {
			ssi_err ("invalid meta size");
		}
		memcpy(&_format, meta, size);
	};

protected:

	MyVideoFeature ();

	ssi_video_params_t _format;

};

}

#endif
