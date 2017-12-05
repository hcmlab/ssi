// MemoryWriter.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2017/11/28
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

#ifndef SSI_IOPUT_MEMORYWRITER_H
#define SSI_IOPUT_MEMORYWRITER_H

#include "base/IConsumer.h"
#include "ioput/option/OptionList.h"
#include "base/ITheFramework.h"

namespace ssi {

class MemoryWriter : public IConsumer {

public:

	class Options : public OptionList {

	public:

		Options ()
		{
			setSize("10.0s");

			addOption("size", size, SSI_MAX_CHAR, SSI_CHAR, "buffer size (add s/ms if seconds/milliseconds, otherwise number of samples)"); 			
		};

		void setSize(const ssi_char_t *size) {			
			if (size) {
				ssi_strcpy(this->size, size);
			}
		}

		ssi_char_t size[SSI_MAX_CHAR];
	};

public: 	

	static const ssi_char_t *GetCreateName () { return "MemoryWriter"; };
	static IObject *Create (const ssi_char_t *file) { return new MemoryWriter (file); };
	~MemoryWriter ();

	MemoryWriter::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Stores a stream to memory."; };

	void consume_enter (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);
	void consume (IConsumer::info consume_info,
		ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);
	void consume_flush (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);

	virtual void setStream(ssi_stream_t &stream);
	virtual ssi_stream_t &getStream();

	void setLogLevel (int level) {
		ssi_log_level = level;
	}

protected:

	MemoryWriter (const ssi_char_t *file = 0);
	MemoryWriter::Options _options;
	ssi_char_t *_file;

	static ssi_char_t *ssi_log_name;
	int ssi_log_level;

	ITheFramework *_frame;

	bool _ready;	
	bool _borrowed;
	ssi_stream_t _stream;
	ssi_byte_t *_stream_ptr;
	ssi_size_t _stream_num;
};

}

#endif
