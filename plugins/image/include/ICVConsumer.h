// ICVConsumer.h
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

#ifndef SSI_IMAGE_ICVCONSUMER_H
#define SSI_IMAGE_ICVCONSUMER_H

#include "base/IConsumer.h"
typedef struct _IplImage IplImage;

namespace ssi {

class ICVConsumer : public IConsumer {

public:

	virtual ~ICVConsumer ();

	virtual void consume_enter (ssi_time_t frame_rate,
		const IplImage *_image_in) {};
	virtual void consume (ssi_time_t frame_rate,
		const IplImage *_image_in) = 0;
	virtual void consume_flush (ssi_time_t frame_rate,
		const IplImage *_image_in) {};
	virtual void setFormat (ssi_video_params_t format_in) = 0;

	void consume_enter (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);
	void consume (IConsumer::info consume_info,
		ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);
	void consume_flush (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);

	ssi_video_params_t getFormatIn () { return _format_in; };
	void setMetaData (ssi_size_t size, const void *meta) {
		if (sizeof (_format_in) != size) {
			ssi_err ("invalid meta size");
		}
		memcpy (&_format_in, meta, size);
		setFormat (_format_in);
	};

protected:

	ICVConsumer ();

	void setFormatIn (ssi_video_params_t format);

	ssi_video_params_t _format_in;
	ssi_size_t _stride_in;
	IplImage *_image_in;
};

}

#endif
