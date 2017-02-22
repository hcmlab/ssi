// Socket.cpp
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

#include "ioput/socket/Socket.h"
#include "ioput/socket/SocketUdp.h"
#include "ioput/socket/SocketTcp.h"
#include "ioput/file/FilePath.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

int Socket::ssi_log_level = SSI_LOG_LEVEL_DEFAULT;
const ssi_char_t *Socket::ssi_log_name = "socket____";

Socket *Socket::Create (TYPE type,
	MODE mode,
	IpEndpointName ip) {

	Socket *socket = 0;
	switch (type) {
		case UDP:
			socket = new SocketUdp ();
			break;
		case TCP:
			#if (defined(ANDROID))
			#else
			socket = new SocketTcp ();
			#endif
			break;
		default:
			ssi_err ("type not supported");
	}	

	socket->_type = type;
	socket->_mode = mode;
	socket->_ip = ip;
	socket->_ip.AddressAndPortAsString (socket->_ipstr);

	ssi_msg (SSI_LOG_LEVEL_DETAIL, "created socket (type=%d, mode=%d, ip=%s)", socket->_type, socket->_mode, socket->_ipstr);
	return socket;
}

Socket *Socket::Create (TYPE type,
	MODE mode,
	int port,
	const char *host) {

	long address = IpEndpointName::ANY_ADDRESS;
	if (host && host[0] != '\0') {	
		address = IpEndpointName::GetHostByName(host);
	};

	return Socket::Create (type, mode, IpEndpointName (address, port));
}

Socket *Socket::CreateAndConnect (TYPE type,
	MODE mode,
	int port,
	const char *host) {

	Socket *socket = Socket::Create (type, mode, port, host);
	socket->connect ();

	return socket;
}

Socket::Socket ()
	: _is_connected (false) {
}

Socket::~Socket () {

	ssi_msg (SSI_LOG_LEVEL_DETAIL, "destroyed socket (type=%d, mode=%d, ip=%s)", _type, _mode, _ipstr);
}


bool Socket::sendFile (const ssi_char_t *filepath) {

	ssi_msg (SSI_LOG_LEVEL_BASIC, "send file (path=%s)", filepath);

	if (!ssi_exists (filepath)) {
		ssi_wrn ("file not found (path=%s)", filepath);
		return false;
	}

	// open file
	FILE *fp = fopen (filepath, "rb");
	fseek (fp, 0, SEEK_END);
	long n_bytes = ftell (fp);
	fseek (fp, 0, SEEK_SET);
	ssi_size_t n_bytes_per_packet = DEFAULT_MTU_SIZE;
	ssi_char_t *buffer = new ssi_char_t[n_bytes_per_packet];
	ssi_size_t n_packets = n_bytes / n_bytes_per_packet;
	ssi_size_t n_bytes_last_packet = n_bytes - n_packets * n_bytes_per_packet;
	if (n_bytes_last_packet > 0) {
		++n_packets;
	}

	// send file name
	FilePath fpath (filepath);
	ssi_size_t len = ssi_strlen (fpath.getNameFull ()) + 1;
	send (&len, sizeof (len));
	send (fpath.getNameFull (), len);

	// send file content
	send (&n_bytes, sizeof (n_bytes));
	send (&n_packets, sizeof (n_packets));
	send (&n_bytes_per_packet, sizeof (n_bytes_per_packet));
	buffer = new ssi_char_t[n_bytes_per_packet];
	for (ssi_size_t i = 0; i < n_packets-1; i++) {
		fread (buffer, n_bytes_per_packet, 1, fp);
		send (buffer, n_bytes_per_packet);
	}
	if (n_bytes_last_packet > 0) {
		fread (buffer, n_bytes_last_packet, 1, fp);
		send (buffer, n_bytes_last_packet);
	} else {
		fread (buffer, n_bytes_per_packet, 1, fp);
		send (buffer, n_bytes_per_packet);
	}
	
	fclose (fp);
	delete[] buffer;

	return true;
}

bool Socket::recvFile (ssi_char_t **filepath, long timeout_in_ms) {

	int result = 0;

	// receive file name
	ssi_size_t n_filepath = 0;	
	result = recv (&n_filepath, sizeof (n_filepath), timeout_in_ms);
	if (result != sizeof (n_filepath)) {
		ssi_wrn ("could not receive size of filepath");
		return false;
	}

	*filepath = new ssi_char_t[n_filepath];
	(*filepath)[0] = '\0';
	result = recv (*filepath, n_filepath, timeout_in_ms);
	if (result != n_filepath) {
		ssi_wrn ("could not receive filepath");
		return false;
	}

	FILE *fp = fopen (*filepath, "wb");
	if (!fp) {
		ssi_wrn ("could not open file (path=%s)", *filepath);
		return false;
	}
		
	long n_bytes = 0;
	ssi_size_t n_bytes_per_packet = 0;
	ssi_size_t n_packets = 0;
	ssi_size_t n_bytes_last_packet = 0;
	result = recv (&n_bytes, sizeof (n_bytes), timeout_in_ms);
	if (result != sizeof (n_bytes)) {
		ssi_wrn ("could not receive size of file");
		return false;
	}
	result = recv (&n_packets, sizeof (n_packets), timeout_in_ms);
	if (result != sizeof (n_packets)) {
		ssi_wrn ("could not receive number of packets");
		return false;
	}
	result = recv (&n_bytes_per_packet, sizeof (n_bytes_per_packet), timeout_in_ms);		
	if (result != sizeof (n_bytes_per_packet)) {
		ssi_wrn ("could not receive size of packet");
		return false;
	}
		
	ssi_char_t *buffer = new ssi_char_t[n_bytes_per_packet];
	for (ssi_size_t i = 0; i < n_packets; i++) {
		result = recv (buffer, n_bytes_per_packet, timeout_in_ms);
		fwrite (buffer, result, 1, fp);
	}
	
	ssi_msg (SSI_LOG_LEVEL_BASIC, "received file (path=%s)", *filepath);
	fclose (fp);

	delete[] buffer;

	return true;
}

}
