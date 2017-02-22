// SocketImage.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2013/06/09
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

#ifndef SSI_IOPUT_SOCKETIMAGE_H
#define SSI_IOPUT_SOCKETIMAGE_H

#include "ioput/socket/Socket.h"


namespace ssi {

#define SSI_SOCKETIMAGE_HEADER_ID 0
#define SSI_SOCKETIMAGE_DATA_ID 1

class SocketImage {

public:

	struct COMPRESSION {
		enum VALUE {
			NONE = 0,
			JPG
		};
	};

public:

	// 4 byte int: SSI_SOCKETIMAGE_HEADER_ID
	// 4 byte uint: unique image id>
	// 4 byte int: compression (0=NONE,1=JPG)
	// 4 byte int: total image size in bytes
	// 4 byte uint: # of following packets> 
	// ssi_video_params_t params
	#pragma pack (1)
	struct HEADER {
		ssi_size_t master;
		ssi_size_t id;		
		ssi_size_t compression;
		ssi_size_t bytes;
		ssi_size_t n_packets;
		ssi_video_params_t params;
	};
	#pragma pack ()

	// 4-byte int: SSI_SOCKETIMAGE_DATA_ID>
	// 4 byte uint: unique image id
	// 4 byte uint: packet number (0..n_packets-1)
	// 4 byte uint: data size in bytes
	#pragma pack (1)
	struct DATA {
		ssi_size_t master;
		ssi_size_t id;
		ssi_size_t num;
		ssi_size_t bytes;		
	};
	#pragma pack ()

public:

	SocketImage (Socket &socket,
		ssi_size_t capacity = Socket::MAX_MTU_SIZE,
		ssi_size_t packet_send_delay_ms = 3 /* delay between sending packets to give receiver time to pick it up */); 
	virtual ~SocketImage ();
	
	int recvImage (ssi_video_params_t params, void *ptr, ssi_size_t size, long timeout_in_ms = Socket::INFINITE_TIMEOUT);
	int sendImage (ssi_video_params_t params, const void *ptr, ssi_size_t size, COMPRESSION::VALUE compression = COMPRESSION::JPG);
	const char *getRecvAddress ();

	void setLogLevel (int level) {
		ssi_log_level = level;
	}


protected:

	int ssi_log_level;
	static const ssi_char_t *ssi_log_name;

	Socket &_socket;
	ssi_size_t _counter;
	HEADER _header;
	bool _early_header;
	ssi_size_t _n_buffer;	
	ssi_byte_t *_buffer;
	ssi_size_t _n_packet;
	ssi_size_t _n_compressed;
	ssi_byte_t *_compressed;
	ssi_size_t _packet_send_delay_ms;
};

}

#endif
