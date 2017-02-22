// MyVideoFilter.h
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

#ifndef _MYVIDEOFILTER_H
#define _MYVIDEOFILTER_H

#include "base/IFilter.h"
typedef struct _IplImage IplImage;
typedef struct CvRect CvRect;
typedef struct CvMat CvMat;
typedef struct CvSize CvSize;

namespace ssi {

class MyVideoFilter : public IFilter {

public:

	static const ssi_char_t *GetCreateName() { return "MyVideoFilter"; };
	static IObject *Create(const ssi_char_t *file) { return new MyVideoFilter(); };
	~MyVideoFilter();

	IOptions *getOptions() { return 0; };
	const ssi_char_t *getName() { return GetCreateName(); };
	const ssi_char_t *getInfo() { return "flips image"; };

	void transform_enter (ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]);
	void transform (ITransformer::info info,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]);
	void transform_flush (ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]);

	ssi_size_t getSampleDimensionOut (ssi_size_t sample_dimension_in) {
		if (sample_dimension_in > 1) {
			ssi_err ("#dimension > 1 not supported");
		}		
		return 1;
	}

	ssi_size_t getSampleBytesOut (ssi_size_t sample_bytes_in) {
		if (sample_bytes_in != ssi_video_size (_format_in)) {
			ssi_err ("#bytes not compatible");
		}
		return ssi_video_size (_format_out);
	}

	ssi_type_t getSampleTypeOut (ssi_type_t sample_type_in) {
		if (sample_type_in != SSI_IMAGE) {
			ssi_err ("unsupported type");
		}
		return SSI_IMAGE;
	}

	const void *getMetaData (ssi_size_t &size) { 
		size = sizeof (_format_out); 
		return &_format_out; 
	};

	void setMetaData (ssi_size_t size, const void *meta) {
		if (sizeof (_format_in) != size) {
			ssi_err ("invalid meta size");
		}
		memcpy (&_format_in, meta, size);
		memcpy (&_format_out, meta, size);
	};

protected:

	MyVideoFilter ();

	ssi_video_params_t _format_in, _format_out;
	ssi_size_t _stride_in, _stride_out;

private:

	IplImage *_image_in, *_image_out;
};

}

#endif
