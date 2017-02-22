// WekaWrapper.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2014/03/04
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

#include "WekaWrapper.h"
#include "WekaServerJNI.h"
#include "WekaServerProcess.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ssi_char_t *WekaWrapper::ssi_log_name = "wekawrap__";

WekaWrapper::WekaWrapper (const ssi_char_t *file) 
	: _file (0),
	_server (0),
	_socket (0),
	_server_ip (0),
	_local_ip (0),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
} 

WekaWrapper::~WekaWrapper () { 

	release ();
	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}

bool WekaWrapper::forward (ssi_stream_t &stream,
	ssi_size_t n_probs,
	ssi_real_t *probs) {

	if (!_server) {		
		ssi_msg (SSI_LOG_LEVEL_BASIC, "create weka server..");
		if (_options.use_jni) {
			_server = new WekaServerJNI (_options.server_port, 
				_options.n_features * sizeof (ssi_real_t),
				_options.model, 
				_options.class_path,
				_options.lib_path);
		} else {
			_server = new WekaServerProcess (_options.server_port, 
				_options.n_features * sizeof (ssi_real_t),
				_options.model, 
				_options.class_path,
				_options.lib_path);
		}
		ssi_msg (SSI_LOG_LEVEL_BASIC, "weka server created");
		_server->start ();
		ssi_msg (SSI_LOG_LEVEL_BASIC, "weka server started");
		Sleep (500); // give server some time to come up
		_socket = new UdpSocket ();		
		_server_ip = new IpEndpointName ("", _options.server_port);
		_local_ip = new IpEndpointName ("", _options.local_port);
		_socket->Bind (*_local_ip);
		
	}

	int result = 0;

	result = _socket->SendTo (*_server_ip, stream.ptr, stream.dim * stream.byte);			
	result = _socket->ReceiveFrom (*_server_ip, ssi_pcast (char, probs), n_probs * sizeof (ssi_real_t), 1000);	

	if (result > 0) {
		ssi_msg (SSI_LOG_LEVEL_DEBUG, "sent feature vector (%d bytes).. %f", result, probs[0]);
	} else {
		ssi_msg (SSI_LOG_LEVEL_DEBUG, "sent feature vector (%d bytes).. no answer", result);
	}

	return true;
}

bool WekaWrapper::load (const ssi_char_t *filepath) {
	
	FILE *fp = fopen (filepath, "r");
	OptionList::LoadXML (fp, _options);
	fclose (fp);

	ssi_msg (SSI_LOG_LEVEL_BASIC, "weka model loaded");

	return true;
}

bool WekaWrapper::save (const ssi_char_t *filepath) {

	FILE *fp = fopen (filepath, "w");
	OptionList::SaveXML  (fp, _options);
	fclose (fp);

	return true;
}

bool WekaWrapper::train (ISamples &samples,
	ssi_size_t stream_index) {

	_options.n_features = samples.getStream (stream_index).dim;
	_options.n_classes = samples.getClassSize ();

	return isTrained ();
}

void WekaWrapper::release () {

	if (_server) {
		char quit_msg[] = "quit";	
		ssi_msg (SSI_LOG_LEVEL_BASIC, "stop weka server");
		_socket->SendTo (*_server_ip, quit_msg, 4);
		_server->stop ();
		ssi_msg (SSI_LOG_LEVEL_BASIC, "weka server stopped");
	}

	delete _server;
	_server = 0;
	delete _socket;
	_socket = 0;
	delete _server_ip;
	_server_ip = 0;
	delete _local_ip;
	_local_ip = 0;
}


}
