// Button.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2015/11/23
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

#include "graphic/Button.h"
#include "base/Factory.h"

#include <shlwapi.h> 
#include <commctrl.h>

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif


namespace ssi {

ssi_char_t *Button::ssi_log_name = "button____";

Button::Button (const ssi_char_t *name)
	: _parent(0),
	_callback(0),
	_hButton (0) {

	_name = ssi_strcpy(name);
}

Button::~Button () {

	delete[] _name;

	if (_hButton) {
		::DestroyWindow(_hButton);
	}
}

void Button::create(IWindow *parent) {
	_parent = parent;
}

ssi_handle_t Button::getHandle() {
	return _hButton;
}

void Button::create(HWND hWnd, HMENU id) {

	_hButton = ::CreateWindow("BUTTON",
		_name ? _name : "Button",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		0, 
		0, 
		0, 
		0,
		hWnd,
		id,
		::GetModuleHandle(NULL),
		0);

	if (!_hButton) {
		//PrintLastError();
		ssi_wrn("could not create button");
	}
}

void Button::setPosition(ssi_rect_t rect) {

	::MoveWindow(_hButton, 0, 0, rect.width, rect.height, TRUE);
}

void Button::update() {
}

LRESULT CALLBACK Button::windowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

	switch (msg) {

	case WM_CREATE: {
		create(hWnd,0);
		return 0;
	}

	case WM_COMMAND: {

		if (HIWORD(wParam) == BN_CLICKED) {

			if (_callback) {
				_callback->update();
			}

			ssi_msg(SSI_LOG_LEVEL_DETAIL, "click");
		}
	
		return 0;
		
	}

	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

}

#endif