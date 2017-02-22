// BufferWriter.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/02/08
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

#ifndef SSI_IOPUT_BUFFERWRITER_H
#define SSI_IOPUT_BUFFERWRITER_H

#include "base/IConsumer.h"
#include "buffer/TimeBuffer.h"
#include "ioput/option/OptionList.h"
#include "thread/Lock.h"

namespace ssi {

class IBufferWriter : public IConsumer {

public:

	virtual bool get (ssi_stream_t &stream, IConsumer::info consume_info) = 0;
};

class BufferWriter : public IBufferWriter {

public:

	class Options : public OptionList {

	public:

		Options () 
			: size (10.0) {
		
			addOption ("size", &size, 1, SSI_TIME, "buffer capacity in seconds");
		};	

		ssi_time_t size;
	};

public: 	

	static const ssi_char_t *GetCreateName () { return "BufferWriter"; };
	static IObject *Create (const ssi_char_t *file) { return new BufferWriter (file); };
	~BufferWriter ();

	BufferWriter::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "writes stream to a buffer"; };

	void consume_enter (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);
	void consume (IConsumer::info consume_info,
		ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);
	void consume_flush (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);

	bool get (ssi_stream_t &stream, IConsumer::info consume_info);

	void setLogLevel (int level) {
		ssi_log_level = level;
	}

protected:

	BufferWriter (const ssi_char_t *file = 0);
	BufferWriter::Options _options;
	ssi_char_t *_file;

	static ssi_char_t *ssi_log_name;
	int ssi_log_level;

	TimeBuffer *_buffer;
	Mutex _mutex;
};

}

#endif
