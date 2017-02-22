// NamedPipe.cpp
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

#include "ioput/pipe/NamedPipe.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

int NamedPipe::ssi_log_level = SSI_LOG_LEVEL_DEFAULT;
const ssi_char_t *NamedPipe::ssi_log_name = "pipe______";
const ssi_char_t *NamedPipe::TYPE_NAMES[] = { "server", "client" };
const ssi_char_t *NamedPipe::MODE_NAMES[] = { "r", "w", "rw" };

NamedPipe::~NamedPipe () {	
	
	ssi_msg (SSI_LOG_LEVEL_DETAIL, "destroy (type=%s, mode=%s, name=%s)", TYPE_NAMES[_type], MODE_NAMES[_mode], _name);

	if (_is_open) {
		close ();
	}
	delete[] _name;
}

NamedPipe::NamedPipe ()
	: _is_open (false),
	_has_client (false),
	_listen_thread (0),
	_name (0) {
}

NamedPipe *NamedPipe::Create (const ssi_char_t *pipe_name,
	TYPE connection_type,
	MODE connection_mode,
	ssi_size_t time_out_ms,
	ssi_size_t buffer_size) {

	ssi_msg (SSI_LOG_LEVEL_BASIC, "create (type=%s, mode=%s, name=%s)", TYPE_NAMES[connection_type], MODE_NAMES[connection_mode], pipe_name);

	NamedPipe *pipe = new NamedPipe ();
	pipe->_name = ssi_strcpy (pipe_name);
	pipe->_type = connection_type;
	pipe->_mode = connection_mode;
	pipe->_buffer_size = buffer_size;
	pipe->_time_out_ms = time_out_ms;

	return pipe;
}

NamedPipe *NamedPipe::CreateAndOpen (const ssi_char_t *pipe_name,
	TYPE connection_type,
	MODE connection_mode,
	ssi_size_t time_out_ms,
	ssi_size_t buffer_size) {
		


	NamedPipe *pipe = Create (pipe_name, connection_type, connection_mode, time_out_ms, buffer_size);
	pipe->open ();

	return pipe;
}

bool NamedPipe::open () {

	ssi_msg (SSI_LOG_LEVEL_BASIC, "connect (type=%s, mode=%s, name=%s)", TYPE_NAMES[_type], MODE_NAMES[_mode], _name);

	if (_is_open) {
		ssi_wrn ("already connected (type=%s, mode=%s, name=%s)", TYPE_NAMES[_type], MODE_NAMES[_mode], _name);		
		return true;
	}
	


	DWORD access = 0;
	switch (_type) {
		case NamedPipe::CLIENT:					
			switch (_mode) {
				case NamedPipe::READ:
					access = GENERIC_READ;
					break;
				case NamedPipe::WRITE:
					access = GENERIC_WRITE;
					break;
			}
			#if _WIN32|_WIN64
			_handle = ::CreateFile (_name, access, 0, 0, OPEN_EXISTING, 0, 0);
			#else
			mkfifo (_name, S_IRUSR| S_IWUSR);
			_handle=open(_name,_mode);

			#endif
			break;
		case NamedPipe::SERVER:
			switch (_mode) {
				case NamedPipe::READ:
					access = PIPE_ACCESS_INBOUND;
					break;
				case NamedPipe::WRITE:
					access = PIPE_ACCESS_OUTBOUND;
					break;
			}
			#if _WIN32|_WIN64
			DWORD wait_mode = _time_out_ms == INFINITE_TIMEOUT ? PIPE_WAIT : PIPE_NOWAIT;
			_handle = ::CreateNamedPipe (_name, access, PIPE_TYPE_BYTE | wait_mode, 255, _buffer_size, _buffer_size, 0, 0);
			#else
			mkfifo (_name, S_IRUSR| S_IWUSR);
			_handle = open(_name,_mode);
			#endif
			break;
	}

	/*COMMTIMEOUTS timeouts;
	BOOL r = ::GetCommTimeouts (_handle, &timeouts);
	DWORD err = ::GetLastError ();
	timeouts.ReadIntervalTimeout = MAXDWORD;
	timeouts.ReadTotalTimeoutMultiplier = MAXDWORD;
	timeouts.ReadTotalTimeoutConstant = _time_out_ms;
	timeouts.WriteTotalTimeoutConstant = 0;
	timeouts.WriteTotalTimeoutMultiplier = 0;
	r = ::SetCommTimeouts (_handle, &timeouts);
	err = ::GetLastError ();*/

	if (_handle == INVALID_HANDLE_VALUE) {
		ssi_wrn ("open failed (type=%s, mode=%s, name=%s)", TYPE_NAMES[_type], MODE_NAMES[_mode], _name);		
	} else {		
		_is_open = true;
	}

	return _is_open;
}

bool NamedPipe::waitForClient () {

	ssi_msg (SSI_LOG_LEVEL_BASIC, "wait for client (type=%s, mode=%s, name=%s)", TYPE_NAMES[_type], MODE_NAMES[_mode], _name);

	if (_handle == INVALID_HANDLE_VALUE) {
		ssi_wrn ("invalid handle (type=%s, mode=%s, name=%s)", TYPE_NAMES[_type], MODE_NAMES[_mode], _name);	
		return false;
	}

	if (_type != NamedPipe::SERVER) {
		ssi_wrn ("only server can wait for client (type=%s, mode=%s, name=%s)", TYPE_NAMES[_type], MODE_NAMES[_mode], _name);	
		return false;
	}

	if (_has_client) {
		ssi_wrn ("already has client (type=%s, mode=%s, name=%s)", TYPE_NAMES[_type], MODE_NAMES[_mode], _name);	
		return false;
	}

	bool found = false;
	if (_time_out_ms == INFINITE_TIMEOUT) {
		found = ::ConnectNamedPipe (_handle, 0) != 0;
	} else {
		ssi_size_t time = ssi_time_ms ();
		do {
			::ConnectNamedPipe (_handle, 0);
			found = ::GetLastError () == ERROR_PIPE_CONNECTED;
			if (!found) {
				Sleep (DEFAULT_WAIT_MS);
			}
		} while (!found && (ssi_time_ms () - time <= _time_out_ms));
	}

	if (!found) {
		ssi_wrn ("wait for client timeout reached");
	} else {
		ssi_msg (SSI_LOG_LEVEL_DETAIL, "found client (type=%s, mode=%s, name=%s)", TYPE_NAMES[_type], MODE_NAMES[_mode], _name);
	}

	_has_client = found;
	return _has_client;
}

bool NamedPipe::startListening (NamedPipeListener *listener) {

	ssi_msg (SSI_LOG_LEVEL_BASIC, "start listening (type=%s, mode=%s, name=%s)", TYPE_NAMES[_type], MODE_NAMES[_mode], _name);

	if (!_is_open) {
		ssi_wrn ("not connected (type=%s, mode=%s, name=%s)", TYPE_NAMES[_type], MODE_NAMES[_mode], _name);		
		return false;
	}

	if (_mode != NamedPipe::READ) {
		ssi_wrn ("cannot listen in write mode (type=%s, mode=%s, name=%s)", TYPE_NAMES[_type], MODE_NAMES[_mode], _name);	
		return false;
	}

	_listen_thread = new NamedPipe::ListenThread (_handle, listener); 
	_listen_thread->start ();

	return true;
}

bool NamedPipe::stopListening () {
	
	ssi_msg (SSI_LOG_LEVEL_BASIC, "stop listening (type=%s, mode=%s, name=%s)", TYPE_NAMES[_type], MODE_NAMES[_mode], _name);

	if (!_is_open) {
		ssi_wrn ("not connected (type=%s, mode=%s, name=%s)", TYPE_NAMES[_type], MODE_NAMES[_mode], _name);		
		return false;
	}

	if (!_listen_thread) {
		ssi_wrn ("not listening (type=%s, mode=%s, name=%s)", TYPE_NAMES[_type], MODE_NAMES[_mode], _name);	
		return false;
	}

	_listen_thread->stop ();
	delete _listen_thread;
	_listen_thread = 0;

	return true;
}

bool NamedPipe::close () {

	ssi_msg (SSI_LOG_LEVEL_BASIC, "close (type=%s, mode=%s, name=%s)", TYPE_NAMES[_type], MODE_NAMES[_mode], _name);

	if (!_is_open) {
		ssi_wrn ("not connected (type=%s, mode=%s, name=%s)", TYPE_NAMES[_type], MODE_NAMES[_mode], _name);		
		return false;
	}

	if (_listen_thread) {
		stopListening ();
	}

	::CloseHandle (_handle);	
	_is_open = false;
	_has_client = false;

	return true;
}

bool NamedPipe::send (const ssi_char_t *str) {

	if (_mode != NamedPipe::WRITE) {
		ssi_wrn ("cannot send in read mode (type=%s, mode=%s, name=%s)", TYPE_NAMES[_type], MODE_NAMES[_mode], _name);	
		return false;
	}

	SSI_DBG (SSI_LOG_LEVEL_DEBUG, "send string %s", str);
	return send (ssi_cast (ssi_size_t, strlen (str)) + 1, str);
}

bool NamedPipe::send (ssi_size_t n_bytes, const ssi_byte_t *bytes) {

	ssi_msg (SSI_LOG_LEVEL_DETAIL, "send (type=%s, mode=%s, name=%s)", TYPE_NAMES[_type], MODE_NAMES[_mode], _name);

	if (!_is_open) {
		ssi_wrn ("not connected (type=%s, mode=%s, name=%s)", TYPE_NAMES[_type], MODE_NAMES[_mode], _name);		
		return false;
	}

	DWORD written;
	BOOL r = ::WriteFile (_handle, bytes, n_bytes, &written, 0);
	SSI_DBG (SSI_LOG_LEVEL_DEBUG, "sent %u bytes", written);

	return r != 0;
}

void NamedPipe::ListenThread::run () {
	
	DWORD n_recv_bytes = 0;
	DWORD n_total_bytes = 0;
	BOOL r = ::PeekNamedPipe (_handle, _recv_buffer, NamedPipe::DEFAULT_BUFFER_SIZE, &n_recv_bytes, &n_total_bytes, 0);
	if (r != 0 && n_recv_bytes > 0) {
		r = ::ReadFile (_handle, _recv_buffer, NamedPipe::DEFAULT_BUFFER_SIZE, &n_recv_bytes, 0);
		ssi_msg (SSI_LOG_LEVEL_DETAIL, "read %u bytes", n_recv_bytes);
		if (_listener) {
			_listener->receive (n_recv_bytes, _recv_buffer);
		}
	}
	
	Thread::sleep_ms (DEFAULT_WAIT_MS);
}

}
