// SocketOscEventWriter.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/06/02
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute itand/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#pragma once

#ifndef SSI_SOCKET_SOCKETOSCEVENTWRITER_H
#define SSI_SOCKET_SOCKETOSCEVENTWRITER_H

#include "base/IConsumer.h"
#include "ioput/socket/SocketOsc.h"

namespace ssi {

class SocketOscEventWriter : public IConsumer {

public:

	SocketOscEventWriter (const ssi_char_t *sender_id,
		const ssi_char_t *event_id,
		SocketOsc &socket,
		ssi_size_t n_events,
		ssi_char_t **names);

	virtual ~SocketOscEventWriter ();

	void consume_enter (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);

	void consume (IConsumer::info consume_info,
		ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);

	void consume_flush (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);

	static void SetLogLevel (int level) {
		ssi_log_level = level;
	}
	
protected:

	char *ssi_log_name;
	static int ssi_log_level;

	SocketOsc &_socket;
	ssi_char_t *_event_id;
	ssi_char_t *_sender_id;
	ssi_size_t _bytes_per_sample;

	ssi_char_t **_event_name;
	ssi_size_t _event_size;
};

}

#endif
