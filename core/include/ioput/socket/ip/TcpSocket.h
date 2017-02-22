// TcpSocket.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/11/02
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

#ifndef SSI_IOPUT_SOCKET_TCPSOCKET_H
#define SSI_IOPUT_SOCKET_TCPSOCKET_H

#include "ioput/socket/Socket.h"
#include "ioput/socket/ip/NetworkingUtils.h"
#include "ioput/socket/ip/IpEndpointName.h"

#if __gnu_linux__
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#define SOCKADDR_IN sockaddr_in
#define BYTE char
#define DWORD uint32_t
#define SOCKET uint32_t
//#define INADDR_ANY 0x0
#define INFINITE 0x0

#endif

namespace ssi {

typedef bool (*acceptproc)(DWORD ip, DWORD param);

class TcpSocket {

public:

	TcpSocket (SOCKET sock) : m_sock (sock) {}
	TcpSocket();
	~TcpSocket();

	bool create();
	bool create(SOCKET sock);

	bool shutdown(int how);

	bool connect(DWORD ip,             int port);
	bool connect(const char *hostname, int port);

	bool close();

	bool listen(int port, DWORD ip = INADDR_ANY);
	bool accept (TcpSocket **newsock, DWORD *clientip = 0);

	int recv      (void *buf, int num_bytes, long timeout = INFINITE);
	int recv_exact(void *buf, int num_bytes, long timeout = INFINITE);
	int send	  (const void *buf, int num_bytes);

	bool connected();

	bool getpeer(DWORD *ip, int *port);
	bool getself(DWORD *ip, int *port);

	void allow(DWORD ip = INADDR_ANY, acceptproc ap = 0, DWORD param = 0);

	void setBlockingMode(bool block);

private:

	int  select(long time);

	SOCKET m_sock;
	DWORD  m_allow;
	bool   m_connected;

	acceptproc m_accept_callback;
	DWORD  m_accept_param;

	TcpSocket(TcpSocket&);
	TcpSocket& operator = (TcpSocket &);

	NetworkInitializer m_networkInitializer;
};

}

#endif
