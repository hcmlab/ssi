// SocketTcp.h
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

#ifndef SSI_IOPUT_SOCKETTCP_H
#define SSI_IOPUT_SOCKETTCP_H

#include "ioput/socket/Socket.h"
#include "ioput/socket/ip/TcpSocket.h"
#include "ioput/socket/ip/IpEndpointName.h"
#include "thread/Lock.h"
#include "thread/RunAsThread.h"

#if _WIN32|_WIN64
#include <winsock2.h>

#else

#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#define SOCKADDR_IN sockaddr_in
#define BYTE char


#endif


#ifdef _MSC_VER 
#pragma comment (lib, "ws2_32.lib")
#pragma comment (lib, "Winmm.lib")
#endif

namespace ssi {

class SocketTcp : public Socket {

friend class Socket;

public:

	bool connect ();
	bool disconnect ();
	int recv (void *ptr, ssi_size_t size, long timeout_in_ms = INFINITE_TIMEOUT);
	int send (const void *ptr, ssi_size_t size);
	int sendTo (const void *ptr, ssi_size_t size, const char* host = 0, int port = IpEndpointName::ANY_PORT);
	const char *getRecvAddress();

	void setBlockingMode(bool block);

protected:

	SocketTcp ();
	virtual ~SocketTcp ();

	static void waitForServer(void *ptr);
	static void waitForClient(void *ptr);

	TcpSocket *_client;
	TcpSocket *_socket;	
	ssi_char_t *_recv_from_host;
	IpEndpointName _recv_from;
	Mutex _send_mutex;
	RunAsThread *_thread;

	static ssi_char_t *ssi_log_name;
	static int ssi_log_level;
	static ssi_char_t *ssi_log_name_static;
	static int ssi_log_level_static;
};

}

#endif
