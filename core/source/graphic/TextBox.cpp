// TextBox.cpp
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

#include "graphic/TextBox.h"
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

ssi_char_t *TextBox::ssi_log_name = "textbox___";

TextBox::TextBox (const ssi_char_t *text,
	bool multi_line,
	ssi_size_t n_buffer)
	: _parent(0),
	_n_buffer(n_buffer),
	_multi_line(multi_line),
	_callback (0),
	_hTextBox (0),
	editWindowProc(0) {

	_text = new ssi_char_t[_n_buffer+1];
	ssi_size_t n_copy = min(_n_buffer, ssi_strlen(text));
	memcpy(_text, text, n_copy);
	_text[n_copy] = '\0';
}

TextBox::~TextBox () {

	delete[] _text;

	if (_hTextBox) {
		::DestroyWindow(_hTextBox);
	}
}

void TextBox::create(IWindow *parent) {
	_parent = parent;
}

void TextBox::add(const ssi_char_t *text) {

	ssi_size_t n_used = ssi_strlen(_text);
	ssi_size_t n_remain = _n_buffer - n_used;
	ssi_size_t n_copy = min(n_remain, (WPARAM)ssi_strlen(_text) + ssi_strlen(text));	

	memcpy(_text + n_used, text, n_copy);
	_text[n_used + n_copy] = '\0';

	ssi_msg(SSI_LOG_LEVEL_DEBUG, "new text: %s", _text);

	::SendMessage(_hTextBox, WM_SETTEXT, (WPARAM)n_used + n_copy, (LPARAM)_text);
	::SendMessage(_hTextBox, EM_SETSEL, (WPARAM)n_used + n_copy, (LPARAM)n_copy);
}

void TextBox::set(const ssi_char_t *text) {

	ssi_size_t n_copy = min(_n_buffer, ssi_strlen(text));
	memcpy(_text, text, n_copy);
	_text[n_copy] = '\0';

	ssi_msg(SSI_LOG_LEVEL_DEBUG, "new text: %s", _text);

	::SendMessage(_hTextBox, WM_SETTEXT, (WPARAM)n_copy, (LPARAM)_text);
	::SendMessage(_hTextBox, EM_SETSEL, (WPARAM)n_copy, (LPARAM)n_copy);
}

const ssi_char_t *TextBox::get() {

	::SendMessage(_hTextBox, WM_GETTEXT, (WPARAM)_n_buffer, (LPARAM)_text);

	return _text;
}

LRESULT CALLBACK TextBox::EditSubWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

	TextBox *me = (TextBox *) ::GetWindowLongPtr(hWnd, GWLP_USERDATA);

	switch (msg) {

		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		case WM_LBUTTONUP:
		{
			IWindow::KEY vkey = IWindow::VIRTUAL_KEYS::NONE;

			if (wParam & MK_CONTROL) {
				vkey |= IWindow::VIRTUAL_KEYS::CONTROL_LEFT;
				vkey |= IWindow::VIRTUAL_KEYS::CONTROL_RIGHT;
			}
			if (wParam & MK_SHIFT) {
				vkey |= IWindow::VIRTUAL_KEYS::SHIFT_LEFT;
				vkey |= IWindow::VIRTUAL_KEYS::SHIFT_RIGHT;
			}
			if (wParam & MK_LBUTTON) {
				vkey |= IWindow::VIRTUAL_KEYS::MOUSE_LEFT;
			}
			if (wParam & MK_RBUTTON) {
				vkey |= IWindow::VIRTUAL_KEYS::MOUSE_RIGHT;
			}
			if (wParam & MK_MBUTTON) {
				vkey |= IWindow::VIRTUAL_KEYS::MOUSE_MIDDLE;
			}

			int x = LOWORD(lParam);
			int y = HIWORD(lParam);

			DWORD position;
			LRESULT result = ::SendMessage(me->_hTextBox, EM_GETSEL, (WPARAM)&position, 0);

			if (me->_callback) {
				me->_callback->mouseUp(position, vkey);
			}

			break;
		}
	
		case WM_CHAR: {

			IWindow::KEY key = ssi_cast(IWindow::KEY, wParam);
			IWindow::KEY vkey = IWindow::VIRTUAL_KEYS::NONE;
			if (::GetKeyState(VK_LSHIFT) & 0x8000) {
				vkey |= IWindow::VIRTUAL_KEYS::SHIFT_LEFT;
			}
			if (::GetKeyState(VK_RSHIFT) & 0x8000) {
				vkey |= IWindow::VIRTUAL_KEYS::SHIFT_RIGHT;
			}
			if (::GetKeyState(VK_LCONTROL) & 0x8000) {
				vkey |= IWindow::VIRTUAL_KEYS::CONTROL_LEFT;
			}
			if (::GetKeyState(VK_RCONTROL) & 0x8000) {
				vkey |= IWindow::VIRTUAL_KEYS::CONTROL_RIGHT;
			}
			if (::GetKeyState(VK_LMENU) & 0x8000) {
				vkey |= IWindow::VIRTUAL_KEYS::ALT_LEFT;
			}
			if (::GetKeyState(VK_RMENU) & 0x8000) {
				vkey |= IWindow::VIRTUAL_KEYS::ALT_RIGHT;
			}

			bool forward = true;
			if (me->_callback) {
				forward = me->_callback->keyDown(key, vkey);
			}

			if (!forward) {
				return 0;
			}

			break;

		}

		case WM_KEYDOWN: {

			IWindow::KEY key = ssi_cast(IWindow::KEY, wParam);
			IWindow::KEY vkey = IWindow::VIRTUAL_KEYS::NONE;
			if (::GetKeyState(VK_LSHIFT) & 0x8000) {
				vkey |= IWindow::VIRTUAL_KEYS::SHIFT_LEFT;
			}
			if (::GetKeyState(VK_RSHIFT) & 0x8000) {
				vkey |= IWindow::VIRTUAL_KEYS::SHIFT_RIGHT;
			}
			if (::GetKeyState(VK_LCONTROL) & 0x8000) {
				vkey |= IWindow::VIRTUAL_KEYS::CONTROL_LEFT;
			}
			if (::GetKeyState(VK_RCONTROL) & 0x8000) {
				vkey |= IWindow::VIRTUAL_KEYS::CONTROL_RIGHT;
			}
			if (::GetKeyState(VK_LMENU) & 0x8000) {
				vkey |= IWindow::VIRTUAL_KEYS::ALT_LEFT;
			}
			if (::GetKeyState(VK_RMENU) & 0x8000) {
				vkey |= IWindow::VIRTUAL_KEYS::ALT_RIGHT;
			}

			bool forward = true;
			if (me->_callback) {
				forward = me->_callback->keyDown(key, vkey);
			}

			if (key == VK_RETURN) {

				me->get();

				if (me->_callback) {
					me->_callback->update (me->_text);
				}
			}

			if (!forward) {
				return 0;
			}

			break;
		}
	}

	return ::CallWindowProc(me->editWindowProc, hWnd, msg, wParam, lParam);
}

ssi_handle_t TextBox::getHandle() {
	return _hTextBox;
}

void TextBox::create(HWND hWnd, HMENU id) {

	DWORD style = WS_CHILD | WS_VISIBLE;

	if (_multi_line) {
		style |= ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL;
	}

	_hTextBox = ::CreateWindowEx(WS_EX_CLIENTEDGE,
		"EDIT",
		"",
		style,
		0,
		0,
		0,
		0,
		hWnd,
		id,
		::GetModuleHandle(NULL),
		0);

	if (!_hTextBox) {

		//PrintLastError();
		ssi_wrn("could not create comboBox");

	} else {	

		::SendMessage( _hTextBox, EM_LIMITTEXT, _n_buffer + 1, 0);
		::SetWindowLongPtr(_hTextBox, GWLP_USERDATA, (long)this);
		
		// setting subclass function to handle key down events
		editWindowProc = (WNDPROC) ::SetWindowLongPtr(_hTextBox, GWLP_WNDPROC, (LONG_PTR)EditSubWindowProc);

		set(_text);
	}
}

void TextBox::setPosition(ssi_rect_t rect) {

	::MoveWindow(_hTextBox, 0, 0, rect.width, rect.height, TRUE);
}

void TextBox::update() {
}

LRESULT CALLBACK TextBox::windowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

	switch (msg) {	

	case WM_CREATE: {
		create(hWnd,0);
		return 0;
	}

	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

}

#endif