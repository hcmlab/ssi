// XMLEventSender.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2014/10/27
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

#include "XMLEventSender.h"
#include "SSI_Tools.h"
#include "ioput/file/FileTools.h"
#include "base/Factory.h"
#include "ioput/file/FilePath.h"
#include "XMLEventHelper.h"
#include "ioput/xml/tinyxml.h"
#include "graphic/Monitor.h"
#if __gnu_linux__
	#ifndef SSI_USE_SDL    
    #include "graphic/WindowFallback.h"
    #else
    #include "graphic/Window.h"
    #endif
#else
#include "graphic/Window.h"
#endif
#include "event/EventAddress.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ssi_char_t *XMLEventSender::ssi_log_name = "xmleventw_";

XMLEventSender::XMLEventSender (const ssi_char_t *file)
	: _file (0),
	_path(0),
	_helper(0),
	_monitor(0),
	_window(0),
	_listener(0),
	_address(0),
	_loaded(false),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}

	_frame = Factory::GetFramework ();
    _doc = new TiXmlDocument();
	ssi_event_init(_event, SSI_ETYPE_STRING);
}

XMLEventSender::~XMLEventSender () {

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}

	delete _doc;
	delete _address;
	ssi_event_destroy(_event);
}

bool XMLEventSender::load() {

	ssi_msg(SSI_LOG_LEVEL_DETAIL, "open xml template '%s'", _options.path);

	if (_options.path == 0 || _options.path[0] == '\0') {
		ssi_wrn("'%s' is not a valid path", _options.path);
		return false;
	}

	FilePath fp(_options.path);
	if (!ssi_strcmp(fp.getExtension(), ".xml", false)) {
		_path = ssi_strcat(_options.path, ".xml");
	} else {
		_path = ssi_strcpy(_options.path);
	}

	if (!_doc->LoadFile(_path)) {
		ssi_wrn("could not load template '%s' (r:%d,c:%d)", _path, _doc->ErrorRow(), _doc->ErrorCol());
		return false;
	}

	_helper = new XMLEventHelper(this);

	if (!_helper->parse()) {
		ssi_wrn("could not parse template '%s'", _path);
		return false;
	}

#if _WIN32|_WIN64
	if (_options.monitor) {

		ssi_real_t max_x, max_y;

		ssi_rect_t rect;
		if (_options.relative) {
			if (_options.screen[0] <= 0 && _options.screen[1] <= 0) {
				HWND desktop = GetDesktopWindow();
				RECT rect;
				GetWindowRect(desktop, &rect);
				max_x = ssi_cast(ssi_real_t, rect.right);
				max_y = ssi_cast(ssi_real_t, rect.bottom);
			}
			else
			{
				max_x = _options.screen[0];
				max_y = _options.screen[1];
			}
			rect.left = ssi_cast(int, _options.mpos[0] * max_x);
			rect.top = ssi_cast(int, _options.mpos[1] * max_y);
			rect.width = ssi_cast(int, _options.mpos[2] * max_x);
			rect.height = ssi_cast(int, _options.mpos[3] * max_y);			
		}
		else {
			rect.left = ssi_cast(int, _options.mpos[0]);
			rect.top = ssi_cast(int, _options.mpos[1]);
			rect.width = ssi_cast(int, _options.mpos[2]);
			rect.height = ssi_cast(int, _options.mpos[3]);			
		}
		_window = new Window();
		_monitor = new Monitor(_options.mbuf);
		_window->setClient(_monitor);
		_window->setPosition(rect);
		_window->setTitle(_options.mname);
		_window->create();
		_window->show();
	}
#endif

	return true;
}

void XMLEventSender::close() {

	if (_monitor) {
		_window->close();
		delete _window; _window = 0;
		delete _monitor; _monitor = 0;
	}

	delete[] _path; _path = 0;
	delete _helper; _helper = 0;
	_doc->Clear();

}

void XMLEventSender::consume_enter (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	if (!_loaded) {
		_loaded = load();
		if (_loaded && _options.update > 0) {
			_helper->start();
		}
	}
}

void XMLEventSender::consume (IConsumer::info consume_info,
	ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	if (!_loaded) {
		return;
	}

	ssi_size_t time = ssi_sec2ms (consume_info.time);
	ssi_size_t dur = ssi_sec2ms(consume_info.dur);

	bool result = _helper->forward(stream_in_num, stream_in, time);
	if (result && _options.update == 0) {
		_helper->send(time, dur);
	}
}

void XMLEventSender::consume_flush (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	if (_loaded) {
		if (_options.update > 0) {
			_helper->stop();
		}
		close();
		_loaded = false;
	}
}

void XMLEventSender::listen_enter() {

	if (!_loaded) {
		_loaded = load();
		if (_loaded && _options.update > 0) {
			_helper->start();
		}
	}
}

bool XMLEventSender::update(IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms) {

	if (n_new_events == 0) {
		return true;
	}

	if (!_loaded) {
		return false;
	}

	bool result = _helper->forward(events, n_new_events, time_ms);
	if (result && _options.update == 0) {
		_helper->send(time_ms, 0);
	}

	return result;
}

void XMLEventSender::listen_flush() {

	if (_loaded) {
		if (_options.update > 0) {
			_helper->stop();
		}
		close();
		_loaded = false;
	}
}

bool XMLEventSender::setEventListener(IEventListener *listener) {

	_listener = listener;

	_address = new EventAddress();

	if (_options.address[0] != '\0') {

		SSI_OPTIONLIST_SET_ADDRESS(_options.address, *_address, _event);

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

		_address->setSender(_options.sname);
		_address->setEvents(_options.ename);
	}

	return true;
}

const ssi_char_t *XMLEventSender::getEventAddress() {

	return _address->getAddress();
}


bool XMLEventSender::notify(INotify::COMMAND::List command, const ssi_char_t *message) {

	switch (command) {
	case INotify::COMMAND::WINDOW_HIDE:
	{
		if (_window) {
			_window->hide();
			return true;
		}
		break;
	}
	case INotify::COMMAND::WINDOW_SHOW:
	{
		if (_window) {
			_window->show();
			return true;
		}
		break;
	}
	case INotify::COMMAND::WINDOW_MOVE:
	{
		if (_window) {
			return _window->setPosition(message);
		}
		break;
	}
	}

	return false;
}


}
