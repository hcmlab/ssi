// SocketTcp.cpp
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

#include "ioput/socket/SocketTcp.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

#if __gnu_linux__
#define SD_BOTH 2
#include <chrono>
#include <thread>

#endif

namespace ssi {

ssi_char_t *SocketTcp::ssi_log_name = "sockettcp_";
int SocketTcp::ssi_log_level = SSI_LOG_LEVEL_DEFAULT;
ssi_char_t *SocketTcp::ssi_log_name_static = "sockettcp_";
int SocketTcp::ssi_log_level_static = SSI_LOG_LEVEL_DEFAULT;

SocketTcp::SocketTcp () 
	: _recv_from_host (0),
	_socket (0),
	_client (0),
	_thread (0) {

	_recv_from_host = new ssi_char_t[IpEndpointName::ADDRESS_AND_PORT_STRING_LENGTH];
	_recv_from_host[0] = '\0';
}

SocketTcp::~SocketTcp () {

	if (_is_connected) {
		disconnect ();
	}

	delete[] _recv_from_host;
}

bool SocketTcp::connect () {

	if (_socket) {
		ssi_wrn ("trying to connect a socket that is already connected");
		return false;
	}

	_socket = new TcpSocket ();
	if (!_socket->create ()) {
		ssi_wrn ("could not create socket [tcp]");
		return false;
	}

	switch (_mode) {

		case SocketTcp::CLIENT: {

			_thread = new RunAsThread (&waitForServer, this, true);
			_thread->start ();

			break;
		}
		case SocketTcp::SERVER: {

			if (!_socket->listen (_ip.port)) {
				ssi_wrn ("listening on port %d failed [tcp]", _ip.port);
				return false;
			}

			_thread = new RunAsThread(&waitForClient, this, true);
			_thread->start ();

			break;
		}
	}

	return true;
}

void SocketTcp::waitForServer(void *ptr) {

	SocketTcp *me = ssi_pcast (SocketTcp, ptr);

	while (!me->_socket->connect (me->_ip.address, me->_ip.port)) {
		ssi_msg_static (SSI_LOG_LEVEL_DETAIL, "waiting for server '%s' [tcp]", me->_ipstr);

        ssi_sleep (1000);

	}
			
	ssi_msg_static (SSI_LOG_LEVEL_BASIC, "connected to '%s' [tcp]", me->_ipstr);

	me->_is_connected = true;
}

void SocketTcp::waitForClient(void *ptr) {

	SocketTcp *me = ssi_pcast (SocketTcp, ptr);

	ssi_msg_static (SSI_LOG_LEVEL_BASIC, "waiting for client on port %d", me->_ip.port);

	if (!me->_socket->accept (&me->_client)) {
		ssi_wrn_static ("could not accept client [tcp]");		
	} else {
			
		#if _WIN32|_WIN64
		me->_client->getpeer (&me->_recv_from.address, &me->_recv_from.port);
		#else
		me->_client->getpeer ((unsigned int*)&me->_recv_from.address, &me->_recv_from.port);
		#endif
		me->_recv_from.AddressAndPortAsString (me->_recv_from_host);
		me->_is_connected = true;

		ssi_msg_static (SSI_LOG_LEVEL_BASIC, "receiving from '%s'", me->_recv_from_host);		
	}
}

bool SocketTcp::disconnect () {

	if (!_socket) {
		ssi_wrn ("trying to disconnected a socket that is not connected");
		return false;
	} 

	_socket->shutdown (SD_BOTH);
	_socket->close();
	delete _client; _client = 0;
	delete _socket; _socket = 0;
	_is_connected = false;
	delete _thread; _thread = 0;

	ssi_msg (SSI_LOG_LEVEL_BASIC, "closed [tcp]");
	return true;
}

int SocketTcp::recv (void *ptr, ssi_size_t size, long timeout) {

	if (!_is_connected) {
		ssi_wrn ("socket not connected");
		return -1;
	}

	int result = -1;

	switch (_mode) {
		case SocketTcp::CLIENT:
			result = _socket->recv(ptr, size, timeout);
			break;		
		case SocketTcp::SERVER:
			result = _client->recv(ptr, size, timeout);
			break;
	}
#if _WIN32|_WIN64
	if (result == SOCKET_ERROR) {
		int error = WSAGetLastError();
		if (error != WSAEWOULDBLOCK) // WSAWOULDBLOCK happens, if there is nothing more to receive
		{
			ssi_wrn("receive() failed\n");
			return - 1;
		}
	} else {
		SSI_DBG (SSI_LOG_LEVEL_DEBUG, "received %d bytes", result);
	}
#else
	if (result == SOCKET_ERROR) {

		if (errno != EWOULDBLOCK) // WSAWOULDBLOCK happens, if there is nothing more to receive
		{
			ssi_wrn("receive() failed\n");
			return 0;
		}
	} else {
		SSI_DBG (SSI_LOG_LEVEL_DEBUG, "received %d bytes", result);
	}
	
#endif

	return result;
}

int SocketTcp::send (const void *ptr, ssi_size_t size) {

	if (!_is_connected) {
		ssi_wrn ("socket not connected");
		return -1;
	}

	int result = -1;
	{
		Lock lock (_send_mutex);		

		switch (_mode) {
			case SocketTcp::CLIENT:
				result = _socket->send(ptr, size);
				break;
			case SocketTcp::SERVER:
				result = _client->send(ptr, size);
				break;
		}
	}

	if (result == SOCKET_ERROR) {		
		ssi_wrn("send() failed\n");
		return -1;
	} else {
		SSI_DBG (SSI_LOG_LEVEL_DEBUG, "sent %d bytes", result);
	}

	return result;
}

int SocketTcp::sendTo(const void *ptr, ssi_size_t size, const char* host, int port)
{
	ssi_wrn("TCP socket can only send data to the connected client");
	return send(ptr, size);
}

const char *SocketTcp::getRecvAddress () {

	return _recv_from_host;
}


void SocketTcp::setBlockingMode(bool block)
{
	switch (_mode) {
		case SocketTcp::CLIENT:
			_socket->setBlockingMode(block);
			break;
		case SocketTcp::SERVER:
			_client->setBlockingMode(block);
			break;
	}
}

}
