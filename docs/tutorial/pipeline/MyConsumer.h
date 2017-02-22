// MyConsumer.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/09/17
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

#ifndef _MYCONSUMER_H
#define _MYCONSUMER_H

#include "base/IConsumer.h"
#include "ioput/file/File.h"

namespace ssi {

class MyConsumer : public IConsumer {

public:

	static const ssi_char_t *GetCreateName() { return "MyConsumer"; };
	static IObject *Create(const ssi_char_t *file) { return new MyConsumer(file); };
	~MyConsumer();

	IOptions *getOptions() { return 0; };
	const ssi_char_t *getName() { return GetCreateName(); };
	const ssi_char_t *getInfo() { return "outputs stream on console"; };

	void consume_enter(ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);
	void consume(IConsumer::info consume_info,
		ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);
	void consume_flush(ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);

protected:

	MyConsumer(const ssi_char_t *file = 0);
	static char ssi_log_name[];
	File *_file;
};

}

#endif
