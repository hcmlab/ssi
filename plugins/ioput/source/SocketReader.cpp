// SocketReader.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/06/02
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute itand/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#include "SocketReader.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif


namespace ssi {

int SocketReader::ssi_log_level = SSI_LOG_LEVEL_DEFAULT;
ssi_char_t *SocketReader::ssi_log_name = "sockrecv__";

SocketReader::SocketReader (const ssi_char_t *file)
	: _file (0),
	_provider (0),
	_bytes_per_sample (0),	
	_socket (0),
	_socket_osc (0),
	_socket_img (0),
	_buffer (0),
	_buffer2 (0),
	_memory (0) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
};

bool SocketReader::setProvider (const ssi_char_t *name, IProvider *provider) {

	if (strcmp (name, SSI_SOCKETREADER_PROVIDER_NAME) == 0) {
		setProvider (provider);
		return true;
	}

	ssi_wrn ("unkown provider name '%s'", name);

	return false;
}

void SocketReader::setProvider (IProvider *provider) {

	if (_provider) {
		ssi_wrn ("provider already set");
		return;
	}
	
	_provider = provider;	

	switch (_options.format) {	

		case Options::FORMAT::BINARY:
		case Options::FORMAT::ASCII:
		case Options::FORMAT::OSC:			

			if (_options.sdim == 0) {
				ssi_err ("option 'sdim' not set");
			}
			if (_options.sbyte == 0) {
				ssi_err ("option 'sbyte' not set");
			}
			if (_options.stype == SSI_UNDEF) {
				ssi_err ("options 'stype' not set");
			}
			if (_options.ssr == 0) {
				ssi_err ("options 'ssr' not set");
			}

			_bytes_per_sample = _options.sbyte * _options.sdim;		
			ssi_stream_init (_socket_channel.stream, 0,  _options.sdim, _options.sbyte, _options.stype, _options.ssr);
			break;

		case Options::FORMAT::IMAGE:			
			ssi_video_params (_vformat, _options.swidth, _options.sheight, _options.ssr, _options.sdepth, _options.schannels);
			_bytes_per_sample = ssi_video_size (_vformat);
			provider->setMetaData (sizeof (_vformat), &_vformat);
			ssi_stream_init (_socket_channel.stream, 0, 1, _bytes_per_sample, SSI_IMAGE, _options.ssr);
			break;
	}
	_provider->init (&_socket_channel);
}

SocketReader::~SocketReader () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

bool SocketReader::connect () {

	if (!_provider) {
		ssi_wrn ("provider not set");
		return false;
	}

	if (_options.osc) {
		ssi_wrn ("using deprecated option 'osc', use 'format' instead");
		_options.format = Options::FORMAT::OSC;
	}
	
	_socket = Socket::CreateAndConnect (_options.type, Socket::SERVER, _options.port, _options.host);	
	switch (_options.format) {
		case Options::FORMAT::BINARY:			
			_n_buffer = _options.size;
			_buffer = new ssi_byte_t[_n_buffer];
			break;
		case Options::FORMAT::ASCII:
			_n_buffer = _options.size;
			_memory = FileMem::Create (FileMem::ASCII);
			_memory->setType (_options.stype);
			_memory->setFormat (_options.delim, "");
			_buffer = new ssi_byte_t[_n_buffer];
			_buffer2 = new ssi_byte_t[_n_buffer];
			break;
		case Options::FORMAT::OSC:
			_n_buffer = _options.size;
			_socket_osc = new SocketOsc (*_socket, _n_buffer);
			break;
		case Options::FORMAT::IMAGE:
			_n_buffer = ssi_video_size (_vformat);
			_buffer = new ssi_byte_t[_n_buffer];
			_socket_img = new SocketImage (*_socket);
			break;
	}

	// set thread name
	ssi_char_t *thread_name = ssi_strcat ("ssi_sensor_SocketReader@", _socket->getIpString ());
	Thread::setName (thread_name);
	delete[] thread_name;

	return true;
};

void SocketReader::run () {

	int result = 0;

	if (_socket->isConnected ()) {
		
		switch (_options.format) {
			case Options::FORMAT::BINARY:
				result = _socket->recv (_buffer, _n_buffer, _options.timeout); 
				if (result > 0) {
					ssi_size_t num = result / _bytes_per_sample;		
					_provider->provide (_buffer, num);
				}
				break;
			case Options::FORMAT::ASCII:
				result = _socket->recv (_buffer, _n_buffer, _options.timeout);
				if (result > 0) {
					_memory->set (result, _buffer, true);
					while (_memory->ready ()) {
						_memory->read (_buffer2, _options.sdim, _options.sdim);
						_provider->provide (_buffer2, 1);
					}
				}
				break;
			case Options::FORMAT::OSC:	
				result = _socket_osc->recv (this, _options.timeout); 
				break;
			case Options::FORMAT::IMAGE:
				result = _socket_img->recvImage (_vformat, _buffer, _n_buffer, _options.timeout);
				if (result > 0) {
					_provider->provide (_buffer, 1);
				}
				break;
		}
	}

	if (result == 0) {
		::Sleep (10);
	}	

	SSI_DBG (SSI_LOG_LEVEL_DEBUG, "received %d bytes", result);
};

void SocketReader::terminate () {
	
	delete _socket; _socket = 0;
	delete _socket_osc; _socket_osc = 0;
	delete _socket_img; _socket_img = 0;
	delete[] _buffer; _buffer = 0;
	delete[] _buffer2; _buffer2 = 0;

}

bool SocketReader::disconnect () {

	return true;
};

void SocketReader::stream (const char *from,
	const ssi_char_t *id,
	osc_int32 time,
	float sr,
	osc_int32 num, 		
	osc_int32 dim,
	osc_int32 byte,
	osc_int32 type,
	void *data) {		

	if (this->_options.ssr != sr || this->_options.sdim != dim || this->_options.sbyte != byte || this->_options.stype != type) {
		ssi_wrn ("stream not compatible");
		return;
	}
	
	ssi_byte_t *ptr = ssi_pcast (ssi_byte_t, ssi_ccast (void *, data));
	_provider->provide (ptr, num);
};

}
