// SocketMessage.h
// author: Florian Lingenfelsr <florian.lingenfelser@informatik.uni-augsburg.de>
// created: 2012/06/12
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

#ifndef SSI_FILE_SOCKETMESSAGE_H
#define SSI_FILE_SOCKETMESSAGE_H

#include "base/IMessage.h"
#include "ioput/socket/SocketTcp.h"
#include "ioput/socket/SocketUdp.h"
#include "thread/Lock.h"

namespace ssi {

class SocketMessage :  public IMessage {

public:

	SocketMessage (Socket::TYPE::List type, ssi_size_t port, const ssi_char_t *host) {
		_socket = Socket::CreateAndConnect (type, Socket::MODE::CLIENT, port, host);
	}

	~SocketMessage () {
		delete _socket;
	}

	void print (const char* text, ...) {
		Lock lock(_mutex);
		{
			va_list args;
			va_start(args, text);
			vsprintf (_string2, text, args);
			va_end(args);
			sprintf (_string, "%s", _string2);
			_socket->send (_string, ssi_strlen (_string) + 1);
		}
	}

	void err (const char* logname, const char* file, int line, const char* text, ...){
	
		Lock lock(_mutex);
		{
			va_list args;
			va_start(args, text);
			vsprintf (_string2, text, args);
			va_end(args);
			sprintf (_string, "[%s] # !ERROR! # %s\nlocation: %s (%d)", logname, _string2, file, line);
			_socket->send (_string, ssi_strlen (_string) + 1);
		}
	};

	void wrn (const char* logname, const char* file, int line, const char* text, ...) {

		Lock lock(_mutex);
		{
			va_list args;
			va_start(args, text);
			vsprintf (_string2, text, args);
			va_end(args);
			sprintf (_string, "[%s] # !WARNING! # %s\nlocation: %s (%d)", logname, _string2, file, line);	
			_socket->send (_string, ssi_strlen (_string) + 1);
		}
	
	};

	void msg (const char* logname, const char* text, ...) {

		Lock lock(_mutex);
		{
			va_list args;
			va_start(args, text);
			vsprintf (_string2, text, args);
			va_end(args);
			sprintf (_string, "[%s] %s", logname, _string2);
			_socket->send (_string, ssi_strlen (_string) + 1);
		}
	
	};

protected:

	Mutex _mutex;
	Socket *_socket;
	ssi_char_t _string[SSI_MAX_CHAR];
	ssi_char_t _string2[SSI_MAX_CHAR];
};

}

#endif
