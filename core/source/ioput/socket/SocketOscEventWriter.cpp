// SocketOscEventWriter.cpp
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

#include "ioput/socket/SocketOscEventWriter.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif


namespace ssi {

int SocketOscEventWriter::ssi_log_level = SSI_LOG_LEVEL_DEFAULT;

SocketOscEventWriter::SocketOscEventWriter (const ssi_char_t *sender_id,
	const ssi_char_t *event_id,
	SocketOsc &socket,
	ssi_size_t event_size,
	ssi_char_t **event_name)
	: _socket (socket),
	_event_size (event_size),
	_event_name (0) {

	_event_name = new ssi_char_t *[_event_size];
	for (ssi_size_t i = 0; i < _event_size; i++) {
		_event_name[i] = ssi_strcpy (event_name[i]);
	}

	static int device_counter = 1;
	ssi_log_name = new char[SSI_MAX_CHAR];
	sprintf (ssi_log_name, "sendosc_%s%d", device_counter > 9 ? "" : "_", device_counter); 
	++device_counter;

	_sender_id = ssi_strcpy (sender_id);
	_event_id = ssi_strcpy(event_id);
};

SocketOscEventWriter::~SocketOscEventWriter () {

	delete[] ssi_log_name;
	delete[] _sender_id;
	delete[] _event_id;
	for (ssi_size_t i = 0; i < _event_size; i++) {
		delete[] _event_name[i];
	}
	delete[] _event_name;
}

void SocketOscEventWriter::consume_enter (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	_bytes_per_sample = stream_in[0].byte * stream_in[0].dim;

	_socket.connect ();

	ssi_msg (SSI_LOG_LEVEL_BASIC, "started");
	if (ssi_log_level >= SSI_LOG_LEVEL_DETAIL) {
		ssi_print ("\
             rate:\t= %.2lf\n\
             dim:\t= %u\n\
             bytes:\t= %u\n",
			stream_in[0].sr,
			stream_in[0].dim,
			stream_in[0].byte
		);
	}
}

void SocketOscEventWriter::consume (IConsumer::info consume_info,
	ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	SSI_ASSERT (stream_in[0].dim == _event_size &&		
		stream_in[0].byte == sizeof (ssi_real_t));

	if (stream_in[0].num > 1) {
		ssi_wrn ("the input stream has %d samples, anything but the first sample will be ignored ", stream_in[0].num);
	}

	ssi_real_t *event_value = ssi_pcast (ssi_real_t, stream_in[0].ptr);

	ssi_size_t time = ssi_cast (ssi_size_t, consume_info.time * 1000.0 + 0.5);
	ssi_size_t dur = ssi_cast (ssi_size_t, consume_info.dur * 1000.0 + 0.5);
	int result = _socket.send_event (_sender_id,
		_event_id,
		time,
		dur,
		SSI_ESTATE_COMPLETED,
		_event_size,
		_event_name,
		event_value);

	SSI_DBG (SSI_LOG_LEVEL_DEBUG, "sent %d bytes", result);

};

void SocketOscEventWriter::consume_flush (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	_socket.disconnect ();
};

}
