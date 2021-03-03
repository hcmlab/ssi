// pythonbridgeexit.cpp
// author: Florian Lingenfelser <lingenfelser@hcm-lab.de>
// created: 2020/09/08
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

#include "ioput/file/FileTools.h"
#include "base/ITheFramework.h"
#include "base/Factory.h"
#include "ioput/socket/Socket.h"

#include "../include/pythonbridgeexit.h"

namespace ssi {

	ssi_char_t *PythonBridgeExit::ssi_log_name = "____pyexit";

	PythonBridgeExit::PythonBridgeExit(const ssi_char_t *file)
		: _file(0),
		_socket(0),
		_buffer_recv(0),
		_buffer_store(0),
		_buffer_msg (0) {

		if (file) {
			if (!OptionList::LoadXML(file, &_options)) {
				OptionList::SaveXML(file, &_options);
			}
			_file = ssi_strcpy(file);
		}

		ssi_event_init(_event, SSI_ETYPE_MAP);

		if (_buffer_recv == 0)
		{
			_buffer_recv = new ssi_byte_t[Socket::MAX_MTU_SIZE];
		}
		if (_buffer_store == 0)
		{
			_buffer_store = new ssi_byte_t[Socket::MAX_MTU_SIZE];
		}
		if (_buffer_msg == 0)
		{
			_buffer_msg = new ssi_byte_t[Socket::MAX_MTU_SIZE * 2];
		}

		_result_recv = 0;
		_last_recv = 0;
		
		_msg_start = false;
		_msg_start_checking = false;
		_msg_start_counter = 0;

		_msg_stop = true;
		_msg_stop_checking = false;
		_msg_stop_counter = 0;
	}

	bool PythonBridgeExit::setEventListener(IEventListener *listener) {

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

	PythonBridgeExit::~PythonBridgeExit() {

		if (_file) {
			OptionList::SaveXML(_file, &_options);
			delete[] _file;
		}

		ssi_event_destroy(_event);
		if (_buffer_recv) {
			delete _buffer_recv;
			_buffer_recv = 0;
		}
		if (_buffer_store) {
			delete _buffer_store;
			_buffer_store = 0;
		}
		if (_buffer_msg) {
			delete _buffer_msg;
			_buffer_msg = 0;
		}
	}

	void PythonBridgeExit::enter() {
		
		if (_listener) {
			ssi_event_adjust(_event, 1 * sizeof(ssi_event_map_t));
		}
	}

	void PythonBridgeExit::run() {
		
		if (_socket == 0)
		{
			_socket = Socket::CreateAndConnect(Socket::TYPE::TCP, Socket::MODE::SERVER, _options.port, _options.host);
			if (_socket->isConnected())
			{
				ssi_print("\npyExit created and connected ... \n");
			}
			else
			{
				ssi_print("\npyExit created, but not connected ... \n");
			}
		}

		if (_socket->isConnected())
		{
			_result_recv = 0;
			
			memcpy(_buffer_store, _buffer_recv, _last_recv);

			/*ssi_print("\n\nBytes stored: %d\n", _last_recv);
			for (int i_stored = 0; i_stored < _last_recv; i_stored++)
			{
				ssi_print("%c", (char)_buffer_store[i_stored]);
			}*/

			_result_recv = _socket->recv(_buffer_recv, Socket::MAX_MTU_SIZE);
			
			if (_result_recv > 0)
			{
				ssi_print("\n\nBytes received: %d\n", _result_recv);
				
				/*for (int i_recv = 0; i_recv < _result_recv; i_recv++)
				{
					ssi_print("%c", (char)_buffer_recv[i_recv]);
				}*/

				memcpy(_buffer_msg + _last_recv, _buffer_recv, _result_recv);

				memcpy(_buffer_msg, _buffer_store, _last_recv);
				
				/* decipher msg */

				ssi_print("\n\nBytes msg: %d\n", _result_recv + _last_recv);
				for (int i_msg = 0; i_msg < _result_recv + _last_recv; i_msg++)
				{
					ssi_print("%c", (char)_buffer_msg[i_msg]);
				}

				ssi_print("\n\nDecipher msg:\n");
				for (int i_msg = 0; i_msg < _result_recv + _last_recv; i_msg++)
				{
					if (_msg_stop)
					{
						if ((char)_buffer_msg[i_msg] == '&')
						{
							if (_msg_start_counter < 3)
							{
								if (_msg_start_checking)
								{
									_msg_start_counter++;
								}
								else
								{
									_msg_start_counter = 1;
									_msg_start_checking = true;
								}

								if (_msg_start_counter == 3)
								{
									_msg_start = true;
									_msg_stop = false;
									// ssi_print("\nmsg start at %d", i_msg);
								}
							}
						}
						else
						{
							_msg_start_counter = 0;
							_msg_start_checking = false;
						}
					}
					else
					{
						if ((char)_buffer_msg[i_msg] == '#')
						{
							if (_msg_stop_counter < 3)
							{
								if (_msg_stop_checking)
								{
									_msg_stop_counter++;
								}
								else
								{
									_msg_stop_counter = 1;
									_msg_stop_checking = true;
								}

								if (_msg_stop_counter == 3)
								{
									_msg_stop = true;
									// ssi_print("\nmsg stop at %d", i_msg);
								}
							}
						}
						else
						{
							_msg_stop_counter = 0;
							_msg_stop_checking = false;
						}
					}

					if (_msg_start)
					{
						ssi_print("%c", (char)_buffer_msg[i_msg]);
					}
				}

				/* decipher msg */

				_last_recv = _result_recv;

				/*if ((char)_buffer[0] == '&' && (char)_buffer[1] == '&' && (char)_buffer[2] == '&') {
				ssi_print("\nStart Message received:\t%c%c%c\n", (char)_buffer[0], (char)_buffer[1], (char)_buffer[2]);
				}*/
			}
			

			if (_listener) {

				if (true) {
					ssi_char_t string[SSI_MAX_CHAR];
					_event.dur = 20;
					_event.time = getElapsedTime();
					ssi_event_map_t* e = ssi_pcast(ssi_event_map_t, _event.ptr);
					ssi_sprint(string, "pypexit");
					e[0].id = Factory::AddString(string); // TODO: einmalig in listen_enter registrieren und id als variable speichern
					e[0].value = 0.0f;
					_listener->update(_event);
				}
			}
		}
		else
		{
			ssi_print("\npyExit not connected ... \n");
			/*_socket->disconnect();
			delete _socket; _socket = 0;
			try {
				ssi_print("\nTry to reconnect ... \n");
				_socket = Socket::CreateAndConnect(Socket::TYPE::TCP, Socket::MODE::SERVER, _options.port, _options.host);
			}
			catch (...) {

			}*/
		}

		::Sleep(20);
	}

	void PythonBridgeExit::flush() {
		
		//if (_socket) {
		//	if (_socket->disconnect()) {
		//		// delete _socket; _socket = 0;
		//	}
		//}
	}

}