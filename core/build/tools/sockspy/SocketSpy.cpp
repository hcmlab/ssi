// SocketSpy.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/01/18
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

#include "SocketSpy.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

int SocketSpy::ssi_log_level = SSI_LOG_LEVEL_DEFAULT;
ssi_char_t *SocketSpy::ssi_log_name = "spy";

SocketSpy::SocketSpy (options opts)
	: _opts (opts),
	_afile (0),
	_bfile (0),
	_cfile (0),
	_socket (0),
	_socket_osc (0),
	_socket_img (0),
	_painter (0),
	_buffer_size (0),
	_buffer (0),
	_n_fork (0),
	_fork (0) {
}

SocketSpy::~SocketSpy () {	
}

void SocketSpy::enter () {

	if (_opts.host && _opts.host[0] != '\0') {
		_socket = Socket::CreateAndConnect (_opts.protocol, Socket::SERVER, _opts.port, _opts.host);
	} else {
		_socket = Socket::CreateAndConnect (_opts.protocol, Socket::SERVER, _opts.port);
	}

	switch (_opts.format) {
		
		case FORMAT::BINARY:

			_buffer_size = _opts.buffer_size;
			_buffer = new ssi_byte_t[_buffer_size];
			_cfile = File::Create (File::ASCII, File::WRITE, 0, stdout);
			_cfile->setType (_opts.type);

			break;

		case FORMAT::ASCII:

			_buffer_size = _opts.buffer_size;
			_buffer = new ssi_byte_t[_buffer_size];

			break;

		case FORMAT::OSC:
	
			_socket_osc = new SocketOsc (*_socket);	

			_sample_rate = 0;
			_sample_dimension = 0;
			_sample_bytes = 0;
			_sample_type = SSI_UNDEF;
			_first_call = true;

			break;

		case FORMAT::IMAGE:

			_buffer_size = ssi_video_size (_opts.vparams);
			_buffer = new ssi_byte_t[_buffer_size];
			_socket_img = new SocketImage (*_socket, _opts.buffer_size);

			break;

		case FORMAT::FORK:

			_buffer_size = _opts.buffer_size;
			_buffer = new ssi_byte_t[_buffer_size];

			_n_fork = ssi_split_string_count (_opts.fork_urls, ';');
			if (_n_fork > 0) {
				ssi_char_t **urls = new ssi_char_t *[_n_fork];
				_fork = new Socket *[_n_fork];
				ssi_split_string (_n_fork, urls, _opts.fork_urls, ';');
				for (ssi_size_t i = 0; i < _n_fork; i++) {
					ssi_char_t *url = urls[i];
					if (ssi_split_string_count (url, ':') == 2) {
						ssi_char_t **url_parts = new ssi_char_t *[2];
						ssi_split_string (2, url_parts, url, ':');
						ssi_char_t *host = url_parts[0];
						ssi_size_t port = 0;
						sscanf (url_parts[1], "%u", &port);
						_fork[i] = Socket::CreateAndConnect (_opts.protocol, Socket::CLIENT, port, host);
					} else {
						ssi_err ("found invalid url '%s'", url);
					}

				}
			}

			break;

		case FORMAT::PEEK:

			_buffer_size = _opts.buffer_size;
			_buffer = new ssi_byte_t[_buffer_size];
	} 
};

void SocketSpy::run () {

	int result = 0;

	switch (_opts.format) {
		
		case FORMAT::BINARY:

			_buffer[0] = '\0';
			result = _socket->recv (_buffer, _buffer_size, 2000);
			if (result > 0) {
				_cfile->write (_buffer, _opts.dim, result / ssi_type2bytes (_opts.type));
			}
			
			break;

		case FORMAT::ASCII:

			_buffer[0] = '\0';
			result = _socket->recv (_buffer, _buffer_size, 2000);
			if (result > 0) {
				ssi_print ("%s\n", _buffer);
			}

			break;

		case FORMAT::OSC:

			result = _socket_osc->recv (this, 2000);

			break;	

		case FORMAT::IMAGE:
						
			result = _socket_img->recvImage (_opts.vparams, _buffer, _buffer_size, 2000);

			break;	

		case FORMAT::FORK:

			_buffer[0] = '\0';
			result = _socket->recv (_buffer, _buffer_size, 2000);
			if (result > 0) {
				for (ssi_size_t i = 0; i < _n_fork; i++) {
					_fork[i]->send (_buffer, result);
				}
				if (_opts.log_console) {
					ssi_print ("fork %u bytes\n", result);
				}
			}

			break;

		case FORMAT::PEEK:

			_buffer[0] = '\0';
			result = _socket->recv (_buffer, _buffer_size, 2000);
			if (result > 0) {
				if (_opts.log_console) {
					ssi_print ("peek %u bytes\n", result);
				}
			}

			break;
	}
};

void SocketSpy::flush () {

	delete _socket; _socket = 0;
	delete _socket_osc; _socket_osc = 0;
	delete _socket_img; _socket_img = 0;
	delete _afile; _afile = 0;
	delete _bfile; _bfile = 0;
	delete _cfile; _cfile = 0;
	delete[] _buffer; _buffer = 0;

	for (ssi_size_t i = 0; i < _n_fork; i++) {
		delete _fork[i];		
	}
	delete[] _fork; _fork = 0;		
	_n_fork = 0;

	//ssi_pcast (ThePainter, _painter)->Clear (_plot_id);
	//delete _plot; _plot = 0;	
	//delete _painter; _painter = 0;
}

void SocketSpy::message (const char *from,
	const ssi_char_t *sender_id,
	const ssi_char_t *event_id,
	osc_int32 time,
	osc_int32 dur,
	const ssi_char_t *msg) {

	ssi_msg (SSI_LOG_LEVEL_BASIC, "%s -> /text %s@%s %dms %dms %s", from, sender_id, event_id, time, dur, msg);
};

void SocketSpy::stream (const ssi_char_t *from,
	const ssi_char_t *id,
	osc_int32 time,
	float sr,
	osc_int32 num, 		
	osc_int32 dim,
	osc_int32 bytes,
	osc_int32 type,		
	void *data) {

	if (_first_call) {
		_sample_rate = sr;
		_sample_dimension = dim;
		_sample_bytes = bytes;
		_sample_type = ssi_cast (ssi_type_t, type);
		init_log ();
		_first_call = false;
		if (_opts.log_console) {
			_cfile->setType (_sample_type);
		}
	}
	
	ssi_sprint (_string, "%s -> /strm %s %dms %.2fhz %d %d %d %s", from, id, time, sr, num, dim, bytes, SSI_TYPE_NAMES[type]);
	ssi_print ("%s\n", _string);

	if (_opts.log_ascii) {
		if (_opts.log_header) {
			_afile->writeLine (_string);
		}
		_afile->write (data, _sample_dimension, _sample_dimension * num);
	}
	if (_opts.log_binary) {
		if (_opts.log_header) {
			_bfile->writeLine (_string);
		}
		_bfile->write (data, _sample_bytes, _sample_dimension * num);
	}
	if (_opts.log_console) {

		_cfile->write (data, _sample_dimension, _sample_dimension * num);
	}
	
};

void SocketSpy::event (const char *from,
	const ssi_char_t *sender_id,
	const ssi_char_t *event_id,
	osc_int32 time,
	osc_int32 dur,
	osc_int32 state,
	osc_int32 event_size,
	const ssi_char_t **event_name,
	const ssi_real_t *event_value) {

	if (_first_call) {
		if (_opts.log_ascii) {
			ssi_char_t string[SSI_MAX_CHAR];
			ssi_sprint (string, "%s\\log_%s@%d.txt", _opts.work_dir, _opts.now, _opts.port);
			_afile = File::CreateAndOpen (File::ASCII, File::WRITE, string);
			_afile->setFormat (_opts.log_delim, _opts.log_format);
			_afile->setType (SSI_FLOAT);
		}
		if (_opts.log_binary) {
			ssi_char_t string[SSI_MAX_CHAR];
			ssi_sprint (string, "%s\\log_%s@%d.dat", _opts.work_dir, _opts.now, _opts.port);
			_bfile = File::CreateAndOpen (File::BINARY, File::WRITE, string);
		}
		if (_opts.log_console) {
			_cfile = File::Create (File::ASCII, File::WRITE, 0, stdout);
		}

		_first_call = false;
	}

	ssi_sprint (_string, "%s -> /evnt %s@%s %dms %.2dms (%s) n=%d", from, sender_id, event_id, time, dur, state == SSI_ESTATE_COMPLETED ? "completed" : "continued", event_size);
	ssi_print ("%s\n", _string);

	if (_opts.log_header) {
		if (_opts.log_ascii) {
			_afile->writeLine (_string);
		}
		if (_opts.log_binary) {
			_bfile->writeLine (_string);
		}
	}

	_string[0] = '\0';
	for (osc_int32 i = 0; i < event_size; i++) {
		ssi_sprint (_string, "%s%s@%.2f\n", _string, event_name[i], event_value[i]);	
	}
	if (_opts.log_ascii) {
		_afile->writeLine (_string);
	}
	if (_opts.log_binary) {
		_bfile->write (event_value, sizeof (ssi_real_t), event_size);
	}
	if (_opts.log_console) {
		_cfile->writeLine (_string);
	}
};

void SocketSpy::init_log () {

	ssi_char_t string[SSI_MAX_CHAR];

	if (_opts.log_ascii) {
		ssi_sprint (string, "%s\\log_%s@%d.txt", _opts.work_dir, _opts.now, _opts.port);		
		_afile = File::CreateAndOpen (File::ASCII, File::WRITE, string);
	}

	if (_opts.log_binary) {
		ssi_sprint (string, "%s\\log_%s@%d.dat", _opts.work_dir, _opts.now, _opts.port);
		_bfile = File::CreateAndOpen (File::BINARY, File::WRITE, string);
	}

	if (_opts.log_console) {
		_cfile = File::Create (File::ASCII, File::WRITE, string, stdout);		
	}
}

