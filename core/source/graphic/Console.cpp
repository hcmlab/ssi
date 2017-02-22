// Console.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2016/01/29
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

#include "graphic/Console.h"

namespace ssi {

ssi_char_t *Console::ssi_log_name = "console___";
int Console::ssi_log_level = SSI_LOG_LEVEL_DEFAULT;
ssi_handle_t Console::_hWnd = 0;

Console::Console() {

#if _WIN32|_WIN64
	static HWND hWnd = ::GetConsoleWindow();
	if (!_hWnd) {
		_hWnd = hWnd;
	}
#endif

}

Console::~Console() {
	_hWnd = 0;
}

void Console::show() {

#if _WIN32|_WIN64
	if (_hWnd) {
		::ShowWindow((HWND)_hWnd, SW_RESTORE);
	}
#endif

}
void Console::hide() {

#if _WIN32|_WIN64
	if (_hWnd) {
		::ShowWindow((HWND)_hWnd, SW_HIDE);
	}
#endif

}

void Console::setPosition(ssi_rect_t rect) {

#if _WIN32|_WIN64
	if (_hWnd) {
		::MoveWindow((HWND) _hWnd, rect.left, rect.top, rect.width, rect.height, true);
	}
#endif

}

bool Console::setPosition(const ssi_char_t *position) {

	ssi_real_t posf[4] = { 0, 0, 0, 0 };
	ssi_size_t n = ssi_string2array_count(position, ',');
	if (n == 4) {
		ssi_string2array(n, posf, position, ',');
	}
	else {
		ssi_wrn("could not parse position '%s'", position);
		return false;
	}

	ssi_rect_t pos;
	pos.left = (int)(posf[0] + 0.5f);
	pos.top = (int)(posf[1] + 0.5f);
	pos.width = (int)(posf[2] + 0.5f);
	pos.height = (int)(posf[3] + 0.5f);

	setPosition(pos);

	return true;
}

bool Console::notify(INotify::COMMAND::List command, const ssi_char_t *message) {

	switch (command) {

	case INotify::COMMAND::WINDOW_HIDE:
	{
		hide();
		return true;
	}
	case INotify::COMMAND::WINDOW_SHOW:
	{
		show();
		return true;
	}
	case INotify::COMMAND::WINDOW_MOVE:
	{
		setPosition(message);
		return true;
	}
	}

	return false;
}

}
