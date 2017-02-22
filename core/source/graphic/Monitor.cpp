// Monitor.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2007/10/15
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

#include "graphic/Monitor.h"
#include "base/Factory.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif


namespace ssi {

Monitor::Monitor (ssi_size_t maxchar)
	: _parent(0),
	_hWnd (0),
	_hEdit (0),
	_hFont (0),
	_fontName(0),
	_fontSize(SSI_DEFAULT_FONT_SIZE) {

	_n_buffer = maxchar + 1;
	_buffer = new ssi_char_t[_n_buffer];
	_buffer[0] = '\0';
	_buffer_copy = new ssi_char_t[_n_buffer];
	_buffer_copy[0] = '\0';

	clear();	
}

Monitor::~Monitor () {
	
	delete[] _fontName; _fontName;
	delete[] _buffer; _buffer = 0;	
	delete[] _buffer_copy; _buffer_copy = 0;
	if (_hEdit) {
		::DestroyWindow(_hWnd);
	}

}

void Monitor::create(IWindow *parent) {

	_parent = parent;	
}

void Monitor::close() {

	if (_hFont) {
		::DeleteObject(_hFont);
		_hFont = 0;
	}
}

void Monitor::setFont(const ssi_char_t *name, ssi_size_t size) {

	delete[] _fontName; 
	_fontName = ssi_strcpy(name);
	_fontSize = size;

	if (_hEdit) {

		HFONT hFont = ::CreateFont(size, 
			0,
			0,
			0, 
			0, 
			FALSE, 
			0, 
			0, 
			OEM_CHARSET, 
			OUT_RASTER_PRECIS,
			CLIP_DEFAULT_PRECIS, 
			DEFAULT_QUALITY,
			FIXED_PITCH, 
			TEXT(_fontName ? _fontName : SSI_DEFAULT_FONT_NAME));
		
		if (!hFont) {
			ssi_wrn("could not create font '%s'", name)
		} else {
			if (_hFont) {
				::DeleteObject(_hFont);
			}
			_hFont = hFont;
			::SendMessage(_hEdit, WM_SETFONT, (WPARAM)_hFont, MAKELPARAM(TRUE, 0));
		}		
	}
}

LRESULT CALLBACK Monitor::windowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

	if (!_hWnd) {
		_hWnd = hWnd;
	}

	switch (msg) {
	
	case WM_CREATE: {
		create(hWnd, 0);
		return 0;
	}

	case WM_CTLCOLORSTATIC:
	{
		HDC hEdit = (HDC)wParam;

		::SetTextColor(hEdit, SSI_DEFAULT_FONT_COLOR_FORE);
		::SetBkColor(hEdit, SSI_DEFAULT_FONT_COLOR_BACK);

		return (INT_PTR)GetStockObject(BLACK_BRUSH);
	}

	}

	return DefWindowProc(hWnd, msg, wParam, lParam);;;
}

ssi_handle_t Monitor::getHandle() {
	return _hEdit;
}

void Monitor::create(HWND hWnd, HMENU id) {

	_hEdit = CreateWindowEx(WS_EX_CLIENTEDGE,
		"EDIT",
		"",
		WS_CHILD | WS_VISIBLE | ES_READONLY | ES_AUTOVSCROLL | ES_AUTOHSCROLL |
		ES_MULTILINE /*| WS_VSCROLL | WS_HSCROLL*/,
		0,
		0,
		0,
		0,
		 hWnd,
		id,
		::GetModuleHandle(NULL),
		0);

	if (!_hEdit) {
		ssi_wrn("could not create edit");
		ssi_PrintLastError();
	} else {		
		setFont(_fontName, _fontSize);
		update();
	}
}

void Monitor::clear() {

	_buffer[0] = '\0';
	_buffer_count = 1;
}

void Monitor::print(const ssi_char_t *str) {

	ssi_size_t len = ssi_cast(ssi_size_t, strlen(str));
	if (_buffer_count + len >= _n_buffer - 1) {
		//ssi_wrn ("max buffer size reached '%u', crop string", _n_buffer);
		len = _n_buffer - _buffer_count - 1;
	}
	if (len > 0) {
		memcpy(_buffer + _buffer_count - 1, str, len);
		_buffer_count += len;
		_buffer[_buffer_count - 1] = '\0';
	}
}

void Monitor::setPosition(ssi_rect_t rect) {

	::MoveWindow(_hEdit, 0, 0, rect.width, rect.height, TRUE);

	update();
}


void Monitor::update() {

	if (_hEdit) {

		if (ssi_strcmp(_buffer, _buffer_copy)) {
			return;
		}
		ssi_strcpy(_buffer_copy, _buffer);

		::SendMessage(_hEdit, WM_SETREDRAW, FALSE, 0);
		::SendMessage(_hEdit, WM_SETTEXT, _buffer_count, (LPARAM)_buffer);
		::SendMessage(_hEdit, EM_SCROLL, _scroll_v, 0);
		::SendMessage(_hEdit, WM_VSCROLL, _scroll_v, NULL);
		::SendMessage(_hEdit, WM_HSCROLL, _scroll_h, NULL);
		::SendMessage(_hEdit, WM_SETREDRAW, TRUE, 0);

		RECT rect;
		::GetClientRect(_hWnd, &rect);
		::MoveWindow(_hEdit, 0, 0, rect.right - rect.left, rect.bottom - rect.top, TRUE);
		::InvalidateRect(_hEdit, NULL, FALSE);
		::UpdateWindow(_hEdit);
	}
}

}

#endif
