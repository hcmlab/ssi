// MyVideoConsumer.h
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

#ifndef _MYVIDEOCONSUMER_H
#define _MYVIDEOCONSUMER_H

#include "base/IConsumer.h"
#include "ioput/option/OptionList.h"

typedef struct _IplImage IplImage;

namespace ssi {

class MyVideoConsumer : public IConsumer {

public:

	static const ssi_char_t *GetCreateName() { return "MyVideoConsumer"; };
	static IObject *Create(const ssi_char_t *file) { return new MyVideoConsumer(file); };
	~MyVideoConsumer();

	IOptions *getOptions() { return 0; };
	const ssi_char_t *getName() { return GetCreateName(); };
	const ssi_char_t *getInfo() { return "displays video"; };

	void consume_enter (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);
	void consume (IConsumer::info consume_info,
		ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);
	void consume_flush (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);

	bool notify(INotify::COMMAND::List command, const ssi_char_t *message);

	ssi_video_params_t getFormatIn () { return _format_in; };
	void setMetaData (ssi_size_t size, const void *meta) {
		if (sizeof (_format_in) != size) {
			ssi_err ("invalid meta size");
		}
		memcpy (&_format_in, meta, size);		
	};

protected:

	MyVideoConsumer(const ssi_char_t *file);
	const ssi_char_t *_file;

	bool getPosition(const ssi_char_t *message, ssi_rect_t &pos);

	ssi_video_params_t _format_in;
	ssi_size_t _stride_in;
	IplImage *_image_in;

	static ssi_size_t _window_counter;
	ssi_char_t _window_name[100];
};

}

#endif
