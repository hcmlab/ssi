// pythonbridge.cpp
// author: Florian Lingenfelser <florian.lingenfelser@informatik.uni-augsburg.de>
// created: 2020 / 09 / 08
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

#include "../include/PythonBridge.h"
#include "ioput/file/FileTools.h"
#include "base/ITheFramework.h"
#include "base/Factory.h"
#include "SSI_Tools.h"

namespace ssi {

char PythonBridge::ssi_log_name[] = "__pybridge";

PythonBridge::PythonBridge(const ssi_char_t *file)
	:	_file (0),
		_listener (0),
		/*_msg_start_sequence (0),
		_msg_end_sequence (0),*/
		_socket (0)
		{

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}

	ssi_event_init (_event, SSI_ETYPE_MAP);	

}

PythonBridge::~PythonBridge() {
	
	ssi_event_destroy (_event);

}

bool PythonBridge::setEventListener (IEventListener *listener) {

	_listener = listener;

	if (_options.address[0] != '\0') {

		_event_address.setAddress(_options.address);
		_event.sender_id = Factory::AddString(_event_address.getSender(0));
		_event.event_id = Factory::AddString(_event_address.getEvent(0));

	}
	else {

		ssi_wrn("use of deprecated option 'sname' and 'ename', use 'address' instead")

			_event.sender_id = Factory::AddString(_options.sname);
		if (_event.sender_id == SSI_FACTORY_STRINGS_INVALID_ID) {
			return false;
		}
		_event.event_id = Factory::AddString(_options.ename);
		if (_event.event_id == SSI_FACTORY_STRINGS_INVALID_ID) {
			return false;
		}

		_event_address.setSender(_options.sname);
		_event_address.setEvents(_options.ename);
	}

	return true;

}

void PythonBridge::listen_enter () {

	if (_listener) {
		ssi_event_adjust (_event, 1 * sizeof (ssi_event_map_t));
	}
	
	_socket = Socket::CreateAndConnect(Socket::TYPE::TCP, Socket::MODE::CLIENT, _options.port, _options.host);

	/*_msg_start_sequence = new ssi_byte_t[3];
	_msg_start_sequence[0] = '+';
	_msg_start_sequence[1] = '+';
	_msg_start_sequence[2] = '+';
	_msg_end_sequence = new ssi_byte_t[3];
	_msg_start_sequence[0] = '#';
	_msg_start_sequence[1] = '#';
	_msg_start_sequence[2] = '#';*/
	

}

bool PythonBridge::update (IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms) {

	ssi_event_t *e = 0;
	//events.reset ();
	int counter = 0;
	for(ssi_size_t nevent = 0; nevent < n_new_events; nevent++){
		// ssi_print("\npyBridge received %d events", n_new_events);
		e = events.next();
		int result = 0;
		if (e != 0)
		{
			{
				if ((int)e->type == (int)SSI_ETYPE_UNDEF) {
					//std::string _test_string = "hal";
					
					if (_socket->isConnected())
					{
						// ssi_print("\npyBridge connected. \n")
						result = _socket->send(&_msg_start_sequence, _msg_start_sequence.size());
						result = _socket->send(e->ptr, e->tot);
						result = _socket->send(&_msg_end_sequence, _msg_end_sequence.size());
					}
					else
					{
						ssi_print("\npyBridge not connected. Trying to reconnect ... \n")
						_socket->connect();
					}
					//result = _socket->send(&_test_string, _test_string.size());
					// result = _socket->send(&_msg_end_sequence, _msg_end_sequence.size());
					//ssi_print("\n start_%d, msg_%d, end_%d", _msg_start_sequence.size(), e->tot, _msg_end_sequence.size());
				}
				else {
					counter++;
				}
			}

			/*if (_listener) {
				_listener->update(_event);
			}*/
		}
	}

	if (counter != 0) {
		ssi_wrn("Please use PythonbridgeEntry or SSI_ETYPE_UNDEF");
	}

	return true;
}

void PythonBridge::listen_flush (){

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}

	if (_listener) {
		ssi_event_reset (_event);
	}

	//if (_socket) {
	//	if (_socket->disconnect()) {
	//		// delete _socket; _socket = 0;
	//	}
	//}

	
	/*delete _msg_start_sequence; _msg_start_sequence = 0;
	delete _msg_end_sequence; _msg_end_sequence = 0;*/
}
}
