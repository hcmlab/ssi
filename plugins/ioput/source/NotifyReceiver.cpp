// NotifyReceiver.cpp
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

#include "NotifyReceiver.h"
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

ssi_char_t *NotifyReceiver::ssi_log_name = "notifyrecv";

NotifyReceiver::NotifyReceiver (const ssi_char_t *file)
	: _file (0),
	_socket (0),
	_buffer (0),
	_n_targets(0),
	_targets(0),
	_target_ids(0),
	ssi_log_level(SSI_LOG_LEVEL_DEFAULT)
{

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
};

NotifyReceiver::~NotifyReceiver() {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

void NotifyReceiver::enter () {

	_n_targets = Factory::GetObjectIds(&_target_ids, _options.id);
	if (_n_targets > 0) {
		_targets = new IObject *[_n_targets];		
		for (ssi_size_t i = 0; i < _n_targets; i++) {
			ssi_strtrim(_target_ids[i]);			
			ssi_msg(SSI_LOG_LEVEL_BASIC, "try to access object '%s'", _target_ids[i]);			
			_targets[i] = Factory::GetObjectFromId(_target_ids[i]);			
		}
	}

	_socket = Socket::CreateAndConnect (_options.url, Socket::MODE::SERVER);
	_buffer = new ssi_byte_t[_options.size];
	
	ssi_char_t string[SSI_MAX_CHAR];
	ssi_sprint (string, "NotifyReceiver@%s", _socket->getUrl()); 	
	Thread::setName (string);

	ssi_msg(SSI_LOG_LEVEL_BASIC, "start receiving notifications from %s", _socket->getUrl());
}

void NotifyReceiver::run () {

	int result = 0;
	
	if (_socket && _socket->isConnected()) {
		result = _socket->recv (_buffer, _options.size, _options.timeout); 
		if (result > 0) 
		{			
			INotify::COMMAND::List command;
			memcpy(&command, _buffer, sizeof(command));

			ssi_char_t *message = 0;
			if (result - sizeof(command) > 0)
			{
				message = (ssi_char_t *)_buffer + sizeof(command);
			}

			for (ssi_size_t i = 0; i < _n_targets; i++)
			{
				if (_targets[i])
				{
					if (command == INotify::COMMAND::MESSAGE && ssi_strcmp(message, "enable"))
					{
						_targets[i]->setEnabled(true);
					}
					else if (command == INotify::COMMAND::MESSAGE && ssi_strcmp(message, "disable"))
					{
						_targets[i]->setEnabled(false);
					}
					else
					{
						_targets[i]->notify(command, message);
					}					
				}
			}

			ssi_msg(SSI_LOG_LEVEL_DEBUG, "receive notification %d'%s' from %s", command, message ? message : "", _socket->getUrl());
		}
	} else 
	{
		Sleep(10);
	}
};

void NotifyReceiver::flush () {

	ssi_msg(SSI_LOG_LEVEL_BASIC, "stop receiving notifications from %s", _socket->getUrl());

	delete[] _targets; _targets = 0;	
	for (ssi_size_t i = 0; i < _n_targets; i++) {
		delete[] _target_ids[i];
	}
	delete[] _target_ids;
	_target_ids = 0;

	delete _socket; _socket = 0;
	delete[] _buffer; _buffer = 0;
}

}
