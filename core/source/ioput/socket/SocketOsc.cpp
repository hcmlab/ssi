// SocketOsc.cpp
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

#include "ioput/socket/SocketOsc.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ssi_char_t SocketOsc::STREAM_PATTERN[] = "/strm";
ssi_char_t SocketOsc::EVENT_PATTERN[] = "/evnt";
ssi_char_t SocketOsc::MESSAGE_PATTERN[] = "/text";

SocketOsc::SocketOsc (Socket &socket,
	ssi_size_t capacity) 
	: _stream (0),
	_recv_buffer (0),
	_send_buffer (0),
	_buffer_size (capacity),
	_socket (socket) {

	_recv_buffer = new ssi_byte_t[_buffer_size];	
	_stream = new OutboundPacketStream (_buffer_size);
}

SocketOsc::~SocketOsc () {

	delete[] _recv_buffer;
	delete _stream;
}

void SocketOsc::connect () {
	_socket.connect ();
}
void SocketOsc::disconnect () {
	_socket.disconnect ();
}

int SocketOsc::send_message(const ssi_char_t *sender_id,
	const ssi_char_t *event_id,
	osc_int32 time,
	osc_int32 dur,
	const ssi_char_t *string) {

	Lock lock (_send_mutex);

	// calculate message size
	long required = 4;	
	required += OutboundPacketStream::RoundUp4 (ssi_cast (long, strlen (SocketOsc::MESSAGE_PATTERN)) + 1); // pattern
	required += OutboundPacketStream::RoundUp4 (3 + 1 + 1); // #tags + \0 + comma
	//required += OutboundPacketStream::RoundUp4(ssi_cast(long, strlen(sender_id)) + 1); // sender id
	required += OutboundPacketStream::RoundUp4(ssi_cast(long, strlen(event_id)) + 1); // event id
	required += sizeof (osc_int32); // time
	required += sizeof (osc_int32); // duration
	required += OutboundPacketStream::RoundUp4 (ssi_cast (long, strlen (string)) + 1); // string
	required = OutboundPacketStream::RoundUp4 (required); // round up
	
	// send message
	_stream->Adjust (required);
	*_stream << BeginMessage (SocketOsc::MESSAGE_PATTERN)
		<< sender_id
		<< event_id
		<< time
		<< dur
        << string
		<< EndMessage;

	long stream_size = _stream->Size ();
	SSI_ASSERT (required <= stream_size);
	return _socket.send (_stream->Data(), stream_size);
}

int SocketOsc::send_stream (const ssi_char_t *id,
	osc_int32 time,	
	float sr,
	osc_int32 num, 		
	osc_int32 dim,
	osc_int32 bytes,
	osc_int32 type,		
	void *data) {

	Lock lock (_send_mutex);

	ssi_size_t data_size = num * dim * bytes;
	Blob blob (data, data_size);

	// calculate message size
	long required = 4;	
	required += OutboundPacketStream::RoundUp4 (ssi_cast (long, strlen (SocketOsc::MESSAGE_PATTERN)) + 1); // pattern
	required += OutboundPacketStream::RoundUp4 (5 + 1 + 1); // #tags + \0 + comma
	//required += OutboundPacketStream::RoundUp4(ssi_cast(long, strlen(id)) + 1); // id	
	required += sizeof (osc_int32); // time
	required += sizeof (float); // sr
	required += sizeof (osc_int32); // num
	required += sizeof (osc_int32); // dim
	required += sizeof (osc_int32); // bytes
	required += sizeof (osc_int32); // type
	required += 4 + OutboundPacketStream::RoundUp4(blob.size); // blob
	required = OutboundPacketStream::RoundUp4 (required); // round up

	// send message
	_stream->Adjust (required);
	*_stream << BeginMessage (SocketOsc::STREAM_PATTERN)
		<< id
        << time
		<< sr
		<< num
		<< dim
		<< bytes
		<< type
		<< blob
		<< EndMessage;

	long stream_size = _stream->Size ();
	SSI_ASSERT (required <= stream_size);
	return _socket.send (_stream->Data(), stream_size);
}

int SocketOsc::send_event(const ssi_char_t *sender_id,
	const ssi_char_t *event_id,
	osc_int32 time,
	osc_int32 dur,
	osc_int32 state,
	osc_int32 n_events,
	ssi_char_t **names, 
	float *values) {

	Lock lock (_send_mutex);

	// calculate message size
	long required = 4;
	required += OutboundPacketStream::RoundUp4 (ssi_cast (long, strlen (SocketOsc::MESSAGE_PATTERN)) + 1); // pattern
	required += OutboundPacketStream::RoundUp4 (4 + 2*n_events + 1 + 1); // #tags + \0 + comma
	//required += OutboundPacketStream::RoundUp4(ssi_cast(long, strlen(sender_id)) + 1); // sender id
	required += OutboundPacketStream::RoundUp4(ssi_cast(long, strlen(event_id)) + 1); // event id
	required += sizeof (osc_int32); // time
	required += sizeof (osc_int32); // dur
	required += sizeof (osc_int32); // state
	required += 4; // event size
	for (osc_int32 i = 0; i < n_events; i++) {
		required += OutboundPacketStream::RoundUp4 (ssi_cast (long, strlen (names[i])) + 1); // name
		required += 4; // value
	}
	required = OutboundPacketStream::RoundUp4 (required);

	// send message
	_stream->Adjust(required);
	*_stream << BeginMessage (SocketOsc::EVENT_PATTERN)
		<< sender_id
		<< event_id
        << time
		<< dur
		<< state
		<< n_events;
	for (osc_int32 i = 0; i < n_events; i++) {
		*_stream << names[i] << values[i];
	}
	*_stream << EndMessage;

	long stream_size = _stream->Size ();
	SSI_ASSERT (required <= stream_size);
	return _socket.send (_stream->Data(), stream_size);
}

int SocketOsc::recv (SocketOscListener *listener, long timeout) {

	int result = _socket.recv (_recv_buffer, _buffer_size, timeout);

	if (result > 0) {

		ReceivedPacket packet (_recv_buffer, result);
		ReceivedMessage message (packet);

		try {
			ReceivedMessageArgumentStream args = message.ArgumentStream();
			ReceivedMessage::const_iterator arg = message.ArgumentsBegin();
	        
			const char *pattern = message.AddressPattern();

			if (strcmp (pattern, SocketOsc::STREAM_PATTERN) == 0) {
				const ssi_char_t *id;
				osc_int32 time, num, dim, bytes, type;
				float sr;				
				void *data;
				if (parse_stream (args, id, time, sr, num, dim, bytes, type, data)) {
					listener->stream (_socket.getRecvAddress (), id, time, sr, num, dim, bytes, ssi_cast (ssi_type_t, type), data);
				} else {
					ssi_wrn ("could not parse stream");
				}
			} else if (strcmp (pattern, SocketOsc::EVENT_PATTERN) == 0) {
				const ssi_char_t *event_id;
				const ssi_char_t *sender_id;
				osc_int32 time, dur, state, n_events;
				if (parse_event (args, sender_id, event_id, time, dur, state, n_events, _event_name_buffer, _event_values_buffer)) {
					listener->event (_socket.getRecvAddress (), sender_id, event_id, time, dur, state, n_events, _event_name_buffer, _event_values_buffer);
				} else {
					ssi_wrn ("could not parse event");
				}
				
			} else if (strcmp (pattern, SocketOsc::MESSAGE_PATTERN) == 0) {
				const ssi_char_t *event_id;
				const ssi_char_t *sender_id;
				osc_int32 time, dur;
				const ssi_char_t *string;
				if (parse_message (args, sender_id, event_id, time, dur, string)) {
					listener->message (_socket.getRecvAddress (), sender_id, event_id, time, dur, string);
				} else {
					ssi_wrn ("could not parse message");
				}
			} else {
				ssi_wrn ("unkown message address %s", pattern);
			}
			
		} catch (Exception& e) {
			ssi_wrn ("error while parsing message %s: %s", message.AddressPattern(), e.what());
		}
	}
	
	return result;
}

bool SocketOsc::parse_stream (ReceivedMessageArgumentStream &args,
	const ssi_char_t *&id,
	osc_int32 &time,
	float &sr,
	osc_int32 &num,
	osc_int32 &dim,
	osc_int32 &bytes,
	osc_int32 &type,
	void *&data) {

	Blob blob;	
	args >> id >> time >> sr >> num >> dim >> bytes >> type >> blob >> EndMessage;	
	data = ssi_ccast (void*, blob.data);

	return true;
}

bool SocketOsc::parse_event (ReceivedMessageArgumentStream &args,
	const ssi_char_t *&sender_id,
	const ssi_char_t *&event_id,
	osc_int32 &time,
	osc_int32 &dur,
	osc_int32 &state,
	osc_int32 &n_events,
	const ssi_char_t **names,
	float *values) {
		
	args >> sender_id >> event_id >> time >> dur >> state >> n_events;	

	if (n_events <= MAX_EVENT_SIZE) {
		for (osc_int32 i = 0; i < n_events; i++) {
			args >> names[i];
			args >> values[i];
		}
		args >> EndMessage;
	} else {
		ssi_err ("number of events too large (%u > %u)", n_events, MAX_EVENT_SIZE);
		return false;
	}

	return true;
}

bool SocketOsc::parse_message (ReceivedMessageArgumentStream &args,
	const ssi_char_t *&sender_id, 
	const ssi_char_t *&event_id,
	osc_int32 &time,
	osc_int32 &dur,
	const ssi_char_t *&string) {

	args >> sender_id >> event_id >> time >> dur >> string >> EndMessage;
	
	return true;
}


}
