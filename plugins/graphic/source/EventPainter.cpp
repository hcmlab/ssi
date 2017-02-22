// EventPainter.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/02/04
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

#include "EventPainter.h"
#include "base/Factory.h"
#include "graphic/Window.h"
#include "graphic/Canvas.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

const ssi_char_t *EventPainter::ssi_log_name = "sigpainter_";
int EventPainter::ssi_log_level = SSI_LOG_LEVEL_DEFAULT;

EventPainter::EventPainter (const ssi_char_t *file)
	: _window(0),
	_canvas(0),
	_client(0),
	_file (0) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
}

EventPainter::~EventPainter () {

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}

void EventPainter::listen_enter() {

	_client = new PaintBars(_options.type);
	_client->setWindowCaption(_options.name);
	_client->setPrecision(_options.axisPrecision);	
	_client->setGlobalLimit(_options.global);	
	if (_options.global && !_options.autoscale && !_options.reset){
		_client->setFixedLimit(_options.fix);
	}

	if (_options.barNames[0] != '\0') {
		ssi_size_t n = ssi_split_string_count(_options.barNames, ',');
		ssi_char_t **tokens = new ssi_char_t *[n];
		ssi_split_string(n, tokens, _options.barNames, ',');
		_client->setExternalAxisCaptions(n, tokens);
		for (ssi_size_t i = 0; i < n; i++) {
			delete[] tokens[i];
		}
		delete[] tokens;
	}

	_canvas = new Canvas();
	_canvas->addClient(_client);

	_window = new Window();
	_window->setClient(_canvas);
	_window->setTitle(_options.name);
	_window->setPosition(ssi_rect(_options.pos[0], _options.pos[1], _options.pos[2], _options.pos[3]));
	_window->create();
	_window->show();
}

bool EventPainter::update(IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms) {

	if (n_new_events > 0) {

		if (_options.reset && _options.global) {
			_client->reset();
		}

		_client->setData(*events.next());
		_window->update();
	}

	return true;
}

void EventPainter::listen_flush() {
	
	_window->close();
	delete _window; _window = 0;
	delete _canvas; _canvas = 0;
	delete _client; _client = 0;
}

bool EventPainter::notify(INotify::COMMAND::List command, const ssi_char_t *message) {

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
	case INotify::COMMAND::RESET:
	{
		if (_window) {
			_client->reset();
			_window->update();
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
	case INotify::COMMAND::MINMAX_SHOW:
	{
		if (_window) {
			_window->setIcons(_window->getIcons() | IWindow::ICONS::MINIMIZE | IWindow::ICONS::MAXIMIZE);
			return true;
		}
		break;
	}
	case INotify::COMMAND::MINMAX_HIDE:
	{
		if (_window) {
			_window->setIcons(_window->getIcons() & ~IWindow::ICONS::MINIMIZE & ~IWindow::ICONS::MAXIMIZE);
			return true;
		}
		break;
	}

	}

	return false;
}

}
