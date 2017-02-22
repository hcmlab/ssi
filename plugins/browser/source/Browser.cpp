// Browser.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 17/11/2014
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#include "Browser.h"
#include "CWebBrowser.h"

namespace ssi {

char Browser::ssi_log_name[] = "browser___";

Browser::Browser(const ssi_char_t *file)
	: _browser(0),
	_file(0) {

	if (file) {
		if (!OptionList::LoadXML(file, _options)) {
			OptionList::SaveXML(file, _options);
		}
		_file = ssi_strcpy(file);
	}
}

Browser::~Browser() {

	delete _browser; _browser = 0;

	if (_file) {
		OptionList::SaveXML(_file, _options);
		delete[] _file;
	}
}

void Browser::listen_enter() {

	_browser = new CWebBrowser(_options.mname);
	_browser->setPosition(ssi_rect(_options.mpos[0], _options.mpos[1], _options.mpos[2], _options.mpos[3]));
	_browser->start();

	if (_options.url[0] != '\0') {
		_browser->navigate(_options.url);
	}

	_browser->show();
}

bool Browser::update(IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms)
{
	if (n_new_events > 0) {

		ssi_event_t *e = events.next();
		if (e->type == SSI_ETYPE_STRING) 
		{			
			if (e->ptr[0] != '\0') {
				if (_options.HTMLstr) {
					ssi_msg(SSI_LOG_LEVEL_BASIC, "load html string");
					_browser->navigateString(e->ptr);
				} else {
					ssi_msg(SSI_LOG_LEVEL_BASIC, "load page '%s'", e->ptr);
					_browser->navigate(e->ptr);
				}
			} else {
				_browser->navigate("about:blank");
			}

			return true;
		}
	}

	return false;
}

void Browser::listen_flush() {

	_browser->close();
}

bool Browser::notify(INotify::COMMAND::List command, const ssi_char_t *message) {

	switch (command) {
	case INotify::COMMAND::WINDOW_HIDE:
	{
		if (_browser) {
			_browser->hide();
			return true;
		}
		break;
	}
	case INotify::COMMAND::WINDOW_SHOW:
	{
		if (_browser) {
			_browser->show();
			return true;
		}
		break;
	}
	case INotify::COMMAND::WINDOW_MOVE:
	{
		if (_browser) {
			return _browser->setPosition(message);
		}
		break;
	}

	}

	return false;
}

}
