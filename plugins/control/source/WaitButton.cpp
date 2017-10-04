// WaitButton.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2017/09/26
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

#include "SSI_Define.h"

#ifndef SSI_USE_SDL

#include "WaitButton.h"
#include "base/Factory.h"
#include "graphic/Window.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif


namespace ssi {

ssi_char_t *WaitButton::ssi_log_name = "ctrlbutton";

WaitButton::WaitButton (const ssi_char_t *file)
: _file (0),
	_button (0),
	_window(0),
	_interrupted(true),
	_event(true, true),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
}

WaitButton::~WaitButton () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

bool WaitButton::start () {

	_button = new Button(_options.label);
	_button->setCallback(this);

	_window = new Window();
	_window->setClient(_button);
	_window->setPosition(ssi_rect(_options.pos[0], _options.pos[1], _options.pos[2], _options.pos[3]));	
	_window->setTitle(_options.title);
	_window->create();
	_window->show();	

	_interrupted = true;

	return true;
}

bool WaitButton::wait()
{
	_event.wait();

	return !_interrupted;
}

bool WaitButton::cancel()
{
	_interrupted = true;
	_event.release();

	return true;
}

void WaitButton::update() {
			
	_interrupted = false;
	_event.release();
}

bool WaitButton::stop() {

	_window->close();

	delete _window; _window = 0;
	delete _button; _button = 0;

	return true;
}

bool WaitButton::notify(INotify::COMMAND::List command, const ssi_char_t *message) {

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

#endif