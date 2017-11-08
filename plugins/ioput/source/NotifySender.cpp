// NotifySender.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2017/11/06
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

#include "NotifySender.h"
#include "ioput/xml/tinyxml.h"
#include "ioput/file/File.h"
#include "base/Factory.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif


namespace ssi {

ssi_char_t *NotifySender::ssi_log_name = "notifysend";

NotifySender::NotifySender (const ssi_char_t *file)
	: _file (0),
	_socket (0),
	_n_buffer(0),
	_buffer(0),
	ssi_log_level(SSI_LOG_LEVEL_DEFAULT)
{

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
};

NotifySender::~NotifySender () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

bool NotifySender::start()
{
	_socket = Socket::CreateAndConnect(_options.url, Socket::MODE::CLIENT);

	ssi_msg(SSI_LOG_LEVEL_BASIC, "start sending notifications to %s", _socket->getUrl());

	return true;
}

bool NotifySender::stop()
{
	ssi_msg(SSI_LOG_LEVEL_BASIC, "stop sending notifications to %s", _socket->getUrl());

	delete _socket; _socket = 0;
	delete[] _buffer; _buffer = 0;
	_n_buffer = 0;

	return true;
}

void NotifySender::setEnabled(bool enabled)
{
	notify(INotify::COMMAND::MESSAGE, enabled ? "enable" : "disable");
}

bool NotifySender::notify(INotify::COMMAND::List command, const ssi_char_t *message)
{
	if (_socket && _socket->isConnected())
	{
		ssi_size_t n = sizeof(command) + (message ? ssi_strlen(message) + 1 : 0);
		if (n > _n_buffer)
		{
			delete[] _buffer;
			_n_buffer = n; 
			_buffer = new ssi_byte_t[_n_buffer];
		}
		memcpy(_buffer, &command, sizeof(command));
		if (message)
		{
			memcpy(_buffer + sizeof(command), message, ssi_strlen(message) + 1);
		}		
		_socket->send(_buffer, n);

		ssi_msg(SSI_LOG_LEVEL_DEBUG, "send notification %d'%s' to %s", command, message ? message : "", _socket->getUrl());

		return true;
	}

	return false;
}

}
