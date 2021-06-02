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
		_last_recv_ms = 0;
		
		_msg_start = false;
		_msg_start_checking = false;
		_msg_start_counter = 0;
		_msg_start_flag = false;

		_msg_stop = true;
		_msg_stop_checking = false;
		_msg_stop_counter = 0;
		_msg_stop_flag = false;
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

		_frame.clear();
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
			/*if (_socket->isConnected())
			{
				ssi_print("\npyExit created and connected ... \n");
			}
			else
			{
				ssi_print("\npyExit created, but not connected ... \n");
			}*/
		}

		if (_socket->isConnected())
		{
			_last_recv_ms = getElapsedTime();

			_result_recv = 0;
			memcpy(_buffer_store, _buffer_recv, _last_recv);
			_result_recv = _socket->recv(_buffer_recv, Socket::MAX_MTU_SIZE);
			
			if (_result_recv > 0)
			{
				memcpy(_buffer_msg + _last_recv, _buffer_recv, _result_recv);
				memcpy(_buffer_msg, _buffer_store, _last_recv);
				
				int counter = 0;
				for (int i_msg = _last_recv; i_msg < _result_recv + _last_recv; i_msg++)
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
							}
						}
					}
					else
					{
						_msg_start_counter = 0;
						_msg_start_checking = false;
					}
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
								_msg_start = false;
								_msg_start_flag = false;
							}
						}
					}
					else
					{
						_msg_stop_counter = 0;
						_msg_stop_checking = false;
					}
					
					counter++;

					if (_msg_start)
					{
						if (_msg_start_flag)
						{
							ssi_byte_t tmp = _buffer_msg[i_msg];
							_frame.push_back(tmp);
						}
						else
						{
							_msg_start_flag = true;
						}
					}
					else
					{
						if (_frame.size() > 0)
						{
							if (_msg_stop)
							{
								_frame.pop_back();
								_frame.pop_back();

								// ssi_print("\n");
								for (int i_frame = 0; i_frame < _frame.size(); i_frame += _options.size)
								{
									ssi_byte_t* value = new ssi_byte_t[_options.size];
									for (int i_byte = 0; i_byte < _options.size; i_byte++)
									{
										value[i_byte] = _frame[i_frame + i_byte];
									}
									float fu = 0.0f;
									memcpy(&fu, value, 4);

									if (_listener) {
										ssi_char_t string[SSI_MAX_CHAR];
										_event.dur = 100;
										_event.time = _last_recv_ms + 100 * (i_frame / _options.size);
										ssi_event_map_t* e = ssi_pcast(ssi_event_map_t, _event.ptr);
										ssi_sprint(string, "pyexit");
										e[0].id = Factory::AddString(string); // TODO: einmalig in listen_enter registrieren und id als variable speichern
										e[0].value = fu;
										_listener->update(_event);
									}

									// _last_recv_ms = getElapsedTime();

									// ssi_print("\n%.2f ", fu);
									delete value;
								}
								// ssi_print("\n");
								_frame.clear();
							}
						}
					}
				}
				_last_recv = _result_recv;

			}

			//if (_listener) {
			//	ssi_char_t string[SSI_MAX_CHAR];
			//	_event.dur = getElapsedTime() - _last_recv_ms;
			//	_event.time = getElapsedTime();
			//	ssi_event_map_t* e = ssi_pcast(ssi_event_map_t, _event.ptr);
			//	ssi_sprint(string, "pyexit");
			//	e[0].id = Factory::AddString(string); // TODO: einmalig in listen_enter registrieren und id als variable speichern
			//	e[0].value = 0.0f;
			//	_listener->update(_event);
			//}

			//_last_recv_ms = getElapsedTime();
		}
		else
		{
			// ssi_print("\npyExit not connected ... \n");
			
			/*_socket->disconnect();
			delete _socket; _socket = 0;
			try {
				ssi_print("\nTry to reconnect ... \n");
				_socket = Socket::CreateAndConnect(Socket::TYPE::TCP, Socket::MODE::SERVER, _options.port, _options.host);
			}
			catch (...) {

			}*/
		}

		::Sleep(_options.wait);
	}

	void PythonBridgeExit::flush() {
		
		/*if (_socket) {
			_socket->disconnect();
			{
				delete _socket; _socket = 0;
			}
		}*/
	}

}