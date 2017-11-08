// SocketUdp.cpp
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

#include "ioput/socket/SocketUdp.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ssi_char_t *SocketUdp::ssi_log_name = "socketudp_";
int SocketUdp::ssi_log_level = SSI_LOG_LEVEL_DEFAULT;

SocketUdp::SocketUdp () 
	: _socket (0),
	_last_recv_from_host (0) {

	_last_recv_from_host = new ssi_char_t[IpEndpointName::ADDRESS_AND_PORT_STRING_LENGTH];
	_last_recv_from_host[0] = '\0';
}

SocketUdp::~SocketUdp () {

	if (_is_connected) {
		disconnect ();
	}

	delete[] _last_recv_from_host;
}

bool SocketUdp::connect () {

	if (_socket) {
		ssi_wrn ("trying to connect a socket that is already connected");
		return true;
	}

	_socket = new UdpSocket();

	switch (_mode)
	{
	case Socket::MODE::CLIENT:
		_socket->Connect(_ip); //configures broadcast if ip is ANY_ADDRESS and allows the use of send()
		ssi_msg(SSI_LOG_LEVEL_BASIC, "connect to '%s' [udp]", _ipstr);
		break;
	case Socket::MODE::SERVER:
		_socket->Bind(_ip);
		ssi_msg(SSI_LOG_LEVEL_BASIC, "bind to '%s' [udp]", _ipstr);
		break;
	}

	_is_connected = true;

	return true;
}

bool SocketUdp::disconnect () {

	if (!_socket) {
		ssi_wrn ("trying to disconnected a socket that is not connected");
		return false;
	} 
	
	delete _socket; _socket = 0;
	_is_connected = false; 

	ssi_msg (SSI_LOG_LEVEL_BASIC, "close '%s' [udp]", _ipstr);
	return true;
}

int SocketUdp::recv (void *ptr, ssi_size_t size, long timeout) {

	if (!_is_connected) {
		ssi_wrn ("socket not connected");
		return -1;
	}

	if (_mode == SocketUdp::MODE::CLIENT) {
		ssi_wrn("UDP clients cannot receive packages");
		return -1;
	}

	int result = _socket->ReceiveFrom (_last_recv_from_ip, ssi_pcast (char, ptr), size, timeout);

	if (result == SOCKET_ERROR) {
		ssi_wrn ("receive() failed\n");
		return -1;
	} else {
		SSI_DBG (SSI_LOG_LEVEL_DEBUG, "receive %d bytes", result);
	}

	return result;
}

int SocketUdp::send (const void *ptr, ssi_size_t size) {

	if (!_is_connected) {
		ssi_wrn("socket not connected");
		return -1;
	}

	if (_mode == SocketUdp::MODE::SERVER) {
		ssi_wrn("sending package to %s (use sendTo to send to different address)", _ipstr);
	}

	int result;
	{
		Lock lock (_send_mutex);		
		result = _socket->SendTo(_ip, ssi_pcast(const char, ptr), size);
	}

	if (result == SOCKET_ERROR) {
		

#if _WIN32|_WIN64
		ssi_wrn ("send() failed\n");
#else
		
		int tmp= errno;
		if(tmp=ECONNREFUSED)
		{
			result=size;

        }else
        {
			ssi_wrn ("send() failed with %d \n",tmp);
        }

#endif
		
		return -1;

	} else {
		SSI_DBG (SSI_LOG_LEVEL_DEBUG, "send %d bytes", result);
	}

	return result;
}

int SocketUdp::sendTo(const void *ptr, ssi_size_t size, const char* host, int port) {

	if (!_is_connected) {
		ssi_wrn("socket not connected");
		return -1;
	}

	long address = IpEndpointName::ANY_ADDRESS;
	if (host && host[0] != '\0')
	{
		address = IpEndpointName::GetHostByName(host);
		_socket->SetBroadcast(false);
	}
	else
	{	
		_socket->SetBroadcast(true);
	}

	IpEndpointName dest = IpEndpointName(address, port);

	int result;
	{
		Lock lock(_send_mutex);
		result = _socket->SendTo(dest, ssi_pcast(const char, ptr), size);
	}

	if (result == SOCKET_ERROR) {
		ssi_wrn("send() failed\n");
		return -1;
	}
	else {
		SSI_DBG(SSI_LOG_LEVEL_DEBUG, "send %d bytes", result);
	}

	return result;
}

const char *SocketUdp::getRecvAddress () {

	_last_recv_from_ip.AddressAndPortAsString (_last_recv_from_host);
	return _last_recv_from_host;
}

}
