// MyConsumer.cpp
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

#include "MyConsumer.h"

namespace ssi {

char MyConsumer::ssi_log_name[] = "myconsume_";

MyConsumer::MyConsumer(const ssi_char_t *file)
	: _file(0) {
}

MyConsumer::~MyConsumer() {
}

void MyConsumer::consume_enter(ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	_file = File::Create(File::ASCII, File::WRITE, 0);
	_file->setFormat(" ", "6.2");
	_file->setType(stream_in[0].type);

	ssi_msg(SSI_LOG_LEVEL_BASIC, "enter()..ok");
}

void MyConsumer::consume(IConsumer::info consume_info,
	ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	ssi_char_t string[20];
	for (ssi_size_t i = 0; i < stream_in_num; i++) {
		ssi_sprint(string, "stream#%u", i);
		_file->writeLine(string);
		_file->write(stream_in[i].ptr, stream_in[i].dim, stream_in[i].dim * stream_in[i].num);
	}
}

void MyConsumer::consume_flush(ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	delete _file; _file = 0;

	ssi_msg(SSI_LOG_LEVEL_BASIC, "flush()..ok");
}

}
