// Socket.h
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

#ifndef SSI_IOPUT_SOCKET_H
#define SSI_IOPUT_SOCKET_H

#include "SSI_Cons.h"
#include "ioput/socket/ip/IpEndpointName.h"
#if __gnu_linux__
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

#define INVALID_SOCKET -1
//#define SOCK_STREAM 1
#define SOCKET_ERROR -1

#else
#include <winsock2.h>
#endif

#ifdef _MSC_VER
#pragma comment (lib, "ws2_32.lib")
#pragma comment (lib, "Winmm.lib")
#endif

namespace ssi {

class Socket {

public:

enum TYPE {
	UDP = 0,
	TCP
};

enum MODE {
	CLIENT = 0,
	SERVER
};

static const ssi_size_t DEFAULT_MTU_SIZE = 1472;
static const ssi_size_t MAX_MTU_SIZE	 = 65507;
static const long INFINITE_TIMEOUT       = 0xFFFFFFFF;

public:

	static Socket *Create (TYPE type,
		MODE mode,
		IpEndpointName ip);
	static Socket *Create (TYPE type,
		MODE mode,
		int port = IpEndpointName::ANY_PORT,
		const char *host = 0);
	static Socket *CreateAndConnect (TYPE type,
		MODE mode,
		int port = IpEndpointName::ANY_PORT,
		const char *host = 0);
	virtual ~Socket ();

public:

	virtual bool connect () = 0;
	virtual bool disconnect () = 0;
	virtual int recv (void *ptr, ssi_size_t size, long timeout_in_ms = INFINITE_TIMEOUT) = 0;
	virtual int send (const void *ptr, ssi_size_t size) = 0;
	virtual int sendTo (const void *ptr, ssi_size_t size, const char* host = 0, int port = IpEndpointName::ANY_PORT) = 0;
	virtual const char *getRecvAddress () = 0;

	bool sendFile (const ssi_char_t *filepath);
	bool recvFile (ssi_char_t **filepath, long timeout_in_ms = INFINITE_TIMEOUT);

	bool isConnected () { return _is_connected; };
	TYPE getType () { return _type; };
	MODE getMode () { return _mode; };
	IpEndpointName getIp () { return _ip; };
	const ssi_char_t *getIpString () { return _ipstr; };

	static void SetLogLevel (int level) {
		Socket::ssi_log_level = level;
	}

protected:

	Socket ();

	TYPE _type;
	MODE _mode;
	IpEndpointName _ip;
	ssi_char_t _ipstr[IpEndpointName::ADDRESS_AND_PORT_STRING_LENGTH];
	bool _is_connected;

	static int ssi_log_level;
	static const ssi_char_t *ssi_log_name;

};

}

#endif
