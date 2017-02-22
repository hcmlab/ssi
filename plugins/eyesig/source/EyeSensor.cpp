// EyeSensor.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/10/08
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

#include "EyeSensor.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

static char ssi_log_name[] = "eye_______";

EyeSensor::EyeSensor (const ssi_char_t *file) 
	: _buffer_size (0),
	_provider (0), 
	_socket (INVALID_SOCKET), 
	_counter (0),
	_file (0) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}

	// Initialize Winsock
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != 0) {
		ssi_err ("WSAStartup failed");
	}
}

EyeSensor::~EyeSensor () {

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}

bool EyeSensor::setProvider (const ssi_char_t *name, IProvider *provider) {

	if (_provider) {
		ssi_wrn ("already set");
		return false;
	}

	_provider = provider;
	if (_provider) {

		_buffer_size = static_cast<ssi_size_t> (_eye_channel.stream.sr * _options.size + 0.5 * _eye_channel.stream.dim);

		_provider->init (&_eye_channel);

		ssi_msg (SSI_LOG_LEVEL_DETAIL, "eye provider set");

		return true;
	}

	return false;
}


bool EyeSensor::connect () {

	ssi_msg (SSI_LOG_LEVEL_BASIC, "try to connect sensor...");

	struct sockaddr_in serverAddress;

	ssi_msg (SSI_LOG_LEVEL_DETAIL, "create socket...");
	// Create socket for listening for client connection requests.
	_socket = socket (AF_INET, SOCK_DGRAM, 0);
	if (_socket < 0) {
		ssi_err ("cannot create listen socket");
		return false;
	 }
	  
	// Bind listen socket to listen port.  First set various fields in
	// the serverAddress structure, then call bind().
	// htonl() and htons() convert long integers and short integers
	// (respectively) from host byte order (on x86 this is Least
	// Significant Byte first) to network byte order (Most Significant
	// Byte first).
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port = htons (_options.port);
	
	ssi_msg (SSI_LOG_LEVEL_DETAIL, "connect socket...");
	if (bind(_socket, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
		ssi_err ("cannot bind socket");
		return false;
	}

	// Wait for connections from clients.
	// This is a non-blocking call; i.e., it registers this program with
	// the system as expecting connections on this socket, and then
	// this thread of execution continues on.
	listen(_socket, 5);

	_buffer = new int[_buffer_size];

	ssi_msg (SSI_LOG_LEVEL_BASIC, "sensor connected");

	// set thread name
	ssi_char_t thread_name[SSI_MAX_CHAR];	
	ssi_sprint (thread_name, "%s@%u", getName (), _options.port);
	Thread::setName (thread_name);

	return true;

}

bool EyeSensor::disconnect () {

	ssi_msg (SSI_LOG_LEVEL_BASIC, "try to disconnect sensor...");

	if (INVALID_SOCKET != _socket) {

		delete _buffer;
		_buffer = 0;

		// cleanup
		closesocket (_socket);
		WSACleanup ();

		ssi_msg(SSI_LOG_LEVEL_BASIC, "sensor disconnected");
	} else {
		ssi_wrn ("socket not connected");
		return false;
	}

	return true;
}

bool EyeSensor::strtoken (char *&str, char token, char *substr, int maxlen) {

	while (*str == token)
		str++;

	if (*str == '\0') {
		return false;
	}

	int i = 0;
	while (*str != '\0' && *str != token && i++ < maxlen-1) {
		*substr++ = *str++;
	}
	*substr = '\0';

	return true;
}

void EyeSensor::run () {

	struct sockaddr_in fromaddr;
    socklen_t fromlen = sizeof (fromaddr);
	int iResult = recvfrom (_socket, _socket_buffer, SSI_EYE_PACKET_SIZE, 0, (struct sockaddr *) &fromaddr, &fromlen);

	_socket_buffer[iResult] = '\0';
	SSI_DBG (SSI_LOG_LEVEL_DEBUG, "%d,%s", iResult, _socket_buffer);

	if (iResult > 0) {

		int x1, x2, y1, y2;
		char *token = new char[64];
		char *ptr = _socket_buffer;
		if (strtoken (ptr, ' ', token, 64)) {
			if (strcmp (token, "ET_SPL") == 0) {
				if (!strtoken (ptr, ' ', token, 64))
					goto skip;
				if (!strtoken (ptr, ' ', token, 64))
					goto skip;
				x1 = atoi (token);				
				if (!strtoken (ptr, ' ', token, 64))
					goto skip;
				x2 = atoi (token);
				if (!strtoken (ptr, ' ', token, 64))
					goto skip;
				y1 = atoi (token);
				if (!strtoken (ptr, ' ', token, 64))
					goto skip;
				y2 = atoi (token);
				_buffer[_counter++] = x1 == 0 ? x2 : x1;
				_buffer[_counter++] = y1 == 0 ? y2 : y1;				
				SSI_DBG (SSI_LOG_LEVEL_DEBUG, "x1=%d,x2=%d,y1=%d,y2=%d -> x=%d,y=%d", x1,x2,y1,y2,_buffer[_counter-2],_buffer[_counter-1]);
			}
		}
		skip:
			delete token;

		if (_counter == _buffer_size) {

			/*for	(ssi_size_t i = 0; i < _buffer_size/2; i++) {
				ssi_print ("x:%d,y:%d\n", _buffer[i*2], _buffer[i*2+1]);
			}*/

			_provider->provide (reinterpret_cast<char *> (_buffer), _buffer_size/2);
			_counter = 0;

			SSI_DBG (SSI_LOG_LEVEL_DEBUG, "provide data");
		}

	} else if (iResult == 0) {
		ssi_wrn ("could not receive data");
		return;
	} else  {
		ssi_wrn ("recv failed (%d)", ::WSAGetLastError ());
        closesocket(_socket);
        WSACleanup();
        return;
    }

}

}
