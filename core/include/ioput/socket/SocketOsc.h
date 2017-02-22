// SocketOsc.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/09/11
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

#ifndef SSI_IOPUT_SOCKETOSC_H
#define SSI_IOPUT_SOCKETOSC_H

#include "ioput/socket/Socket.h"
#include "ioput/socket/SocketOscListener.h"
#include "ioput/socket/osc/OscOutboundPacketStream.h"
#include "ioput/socket/osc/OscReceivedElements.h"
#include "thread/Lock.h"

namespace ssi {

class SocketOsc {

public:

static ssi_char_t STREAM_PATTERN[];
static ssi_char_t EVENT_PATTERN[];
static ssi_char_t MESSAGE_PATTERN[];
static const ssi_size_t MAX_EVENT_SIZE = 128;

public:

	SocketOsc (Socket &socket,
		ssi_size_t capacity = Socket::DEFAULT_MTU_SIZE);
	virtual ~SocketOsc ();
	
	int recv (SocketOscListener *listener, long timeout_in_ms);
	
	int send_stream (const ssi_char_t *id,
		osc_int32 time,
		float sr,
		osc_int32 num, 		
		osc_int32 dim,
		osc_int32 bytes,
		osc_int32 type,		
		void *data);
	
	int send_event(const ssi_char_t *sender_id,
		const ssi_char_t *event_id,
		osc_int32 time,
		osc_int32 dur,
		osc_int32 state,
		osc_int32 n_events,
		ssi_char_t **names, 
		float *values);

	int send_message(const ssi_char_t *sender_id,
		const ssi_char_t *event_id,
		osc_int32 time,
		osc_int32 dur,
		const ssi_char_t *msg);

	void connect ();
	void disconnect ();

protected:

	bool parse_stream (ReceivedMessageArgumentStream &args,
		const ssi_char_t *&id,
		osc_int32 &time,
		float &sr,
		osc_int32 &num,
		osc_int32 &dim,
		osc_int32 &bytes,
		osc_int32 &type,
		void *&data);

	bool parse_event (ReceivedMessageArgumentStream &args,
		const ssi_char_t *&sender_id,
		const ssi_char_t *&event_id,
		osc_int32 &start,
		osc_int32 &dur,
		osc_int32 &state,
		osc_int32 &n_events,
		const ssi_char_t **names,
		float *values);

	bool parse_message (ReceivedMessageArgumentStream &args,
		const ssi_char_t *&sender_id,
		const ssi_char_t *&event_id,
		osc_int32 &time,
		osc_int32 &dur,
		const ssi_char_t *&msg);

	Socket &_socket;

	ssi_size_t _buffer_size;
	OutboundPacketStream *_stream;
	ssi_byte_t *_send_buffer;
	ssi_byte_t *_recv_buffer;
	const ssi_char_t *_event_name_buffer[SocketOsc::MAX_EVENT_SIZE];
	ssi_real_t _event_values_buffer[SocketOsc::MAX_EVENT_SIZE];

	Mutex _send_mutex;
};

}

#endif
