// ICVFeature.h
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

#ifndef SSI_IMAGE_ICVFEATURE_H
#define SSI_IMAGE_ICVFEATURE_H

#include "base/IFeature.h"
typedef struct _IplImage IplImage;
typedef struct CvRect CvRect;
typedef struct CvMat CvMat;
typedef struct CvSize CvSize;

namespace ssi {

class ICVFeature : public IFeature {

public:

	virtual ~ICVFeature();

	virtual void transform_enter (ssi_time_t frame_rate,
		IplImage *image_in, 
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {};
	virtual void transform (ssi_time_t frame_rate,
		IplImage *image_in, 
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) = 0;
	virtual void transform_flush (ssi_time_t frame_rate,
		IplImage *image_in, 
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {};

	virtual void setFormat (ssi_video_params_t format_in) = 0;

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

	ssi_video_params_t getFormatIn () { return _format_in; };

	void setMetaData (ssi_size_t size, const void *meta) {
		if (sizeof (_format_in) != size) {
			ssi_err ("invalid meta size");
		}
		memcpy (&_format_in, meta, size);
		setFormat (_format_in);
	};

protected:

	ICVFeature ();

	void setFormatIn (ssi_video_params_t format);

	ssi_video_params_t _format_in;
	ssi_size_t _stride_in;

private:

	IplImage *_image_in;
};

}

#endif
