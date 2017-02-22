// NamedPipe.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/05/26
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

#ifndef SSI_IOPUT_NAMEDPIPE_H
#define SSI_IOPUT_NAMEDPIPE_H

#include "SSI_Cons.h"
#include "ioput/pipe/NamedPipeListener.h"
#include "thread/Thread.h"

#if __gnu_linux__

#define OVERLAPPED int

#endif
namespace ssi {

class NamedPipe {

friend class ListenThread;

public:

/*
linux open defines:
#define O_RDONLY         00
#define O_WRONLY         01
#define O_RDWR           02
*/

enum TYPE {
	SERVER = 0,
	CLIENT
};
static const ssi_char_t *TYPE_NAMES[];

enum MODE {
	READ = 0,
	WRITE
};
static const ssi_char_t *MODE_NAMES[];
static const ssi_size_t DEFAULT_BUFFER_SIZE = 4096;
static const ssi_size_t DEFAULT_TIMEOUT_MS = 5000;
static const ssi_size_t INFINITE_TIMEOUT = -1;
static const ssi_size_t DEFAULT_WAIT_MS = 100;

public:

	static NamedPipe *Create (const ssi_char_t *pipe_name,
		TYPE connection_type,
		MODE connection_mode,
		ssi_size_t time_out_ms = DEFAULT_TIMEOUT_MS,
		ssi_size_t buffer_size = DEFAULT_BUFFER_SIZE);	
	static NamedPipe *CreateAndOpen (const ssi_char_t *pipe_name,
		TYPE connection_type,
		MODE connection_mode,
		ssi_size_t time_out_ms = DEFAULT_TIMEOUT_MS,
		ssi_size_t buffer_size = DEFAULT_BUFFER_SIZE);	
	virtual ~NamedPipe ();

public:

	bool open ();
	bool close ();
	bool waitForClient ();
	bool startListening (NamedPipeListener *listener);
	bool stopListening ();
	bool isOpen () { return _is_open; };
	bool send (const ssi_char_t *str);
	bool send (ssi_size_t n_bytes, const ssi_byte_t *bytes);

	static void SetLogLevel (int level) {
		NamedPipe::ssi_log_level = level;
	}

protected:

	NamedPipe ();

	class ListenThread : public Thread {
	public:

		ListenThread (ssi_handle_t handle,
			NamedPipeListener *listener) 
			: _handle (handle), _listener (listener) {	
				Thread::setName ("pipe listen thread");
		};
		~ListenThread  () {};

		void run ();

	protected:

		ssi_handle_t _handle;
		NamedPipeListener *_listener;
		ssi_byte_t _recv_buffer[DEFAULT_BUFFER_SIZE];
		OVERLAPPED _overlap; 
	};

	ssi_char_t *_name;
	TYPE _type;
	MODE _mode;	
	ssi_size_t _buffer_size;
	ssi_size_t _time_out_ms;

	bool _is_open;	
	bool _has_client;	
	ssi_handle_t _handle;
	
	
	
	NamedPipe::ListenThread *_listen_thread;
	
	static int ssi_log_level;
	static const ssi_char_t *ssi_log_name;

};

}

#endif
