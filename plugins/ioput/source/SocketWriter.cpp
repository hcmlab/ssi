// SocketWriter.cpp
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

#include "SocketWriter.h"
#include "base/Factory.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif


namespace ssi {

int SocketWriter::ssi_log_level = SSI_LOG_LEVEL_DEFAULT;
ssi_char_t *SocketWriter::ssi_log_name = "sockwrite_";

SocketWriter::SocketWriter (const ssi_char_t *file)
	: _file (0),
	_socket (0),
	_socket_osc (0),
	_socket_img (0),
	_frame (0),
	_memory (0) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}

	_frame = Factory::GetFramework ();
};

SocketWriter::~SocketWriter () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

void SocketWriter::consume_enter (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	_bytes_per_sample = stream_in[0].byte * stream_in[0].dim;

	_socket = Socket::CreateAndConnect (_options.type, Socket::CLIENT, _options.port, _options.host);

	if (_options.osc) {
		ssi_wrn ("using deprecated option 'osc', use 'format' instead");
		_options.format = Options::FORMAT::OSC;
	}

	switch (_options.format) {
		case Options::FORMAT::BINARY:
			break;
		case Options::FORMAT::ASCII:
			_memory = FileMem::Create (FileMem::ASCII);
			_memory->make (Socket::MAX_MTU_SIZE);
			_memory->setType (stream_in[0].type);
			_memory->setFormat (_options.delim, "");
			break;
		case Options::FORMAT::OSC:
			_socket_osc = new SocketOsc (*_socket);
			break;
		case Options::FORMAT::IMAGE:
			_socket_img = new SocketImage (*_socket, Socket::MAX_MTU_SIZE, _options.packet_delay);
			break;
	}

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

void SocketWriter::consume (IConsumer::info consume_info,
	ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	int result = 0;

	if (_socket->isConnected ()) {

		switch (_options.format) {
			case Options::FORMAT::BINARY: {
					result = _socket->send (stream_in[0].ptr, stream_in[0].tot);
				}
				break;
			case Options::FORMAT::ASCII: {
					_memory->seek (0);
					_memory->write (stream_in[0].ptr, stream_in[0].dim, stream_in[0].num * stream_in[0].dim);										
					result = _socket->send (_memory->getMemory (), _memory->getPosition () + 1);
				}
				break;
			case Options::FORMAT::OSC: {
					ssi_size_t time = ssi_cast (ssi_size_t, consume_info.time * 1000.0 + 0.5);		
					if (_options.reltime) {
						time = _frame->GetElapsedTimeMs () - time;
					}
					result = _socket_osc->send_stream (_options.id,
						time,
						ssi_cast (float, stream_in[0].sr),
						stream_in[0].num,
						stream_in[0].dim,
						stream_in[0].byte,
						stream_in[0].type,
						stream_in[0].ptr);
				}
				break;
			case Options::FORMAT::IMAGE: {
					result = _socket_img->sendImage (_video_format, stream_in[0].ptr, stream_in[0].tot, _options.compression);
				 }
				break;		
		}
	}

	SSI_DBG (SSI_LOG_LEVEL_DEBUG, "sent %d bytes", result);
};

void SocketWriter::consume_flush (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	delete _socket_osc; _socket_osc = 0;
	delete _socket_img; _socket_img = 0;
	delete _socket; _socket = 0;
	delete _memory; _memory = 0;
};

}
