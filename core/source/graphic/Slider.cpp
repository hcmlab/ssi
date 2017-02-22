// Slider.cpp
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

#include "graphic/Slider.h"
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

ssi_char_t *Slider::ssi_log_name = "slider____";

Slider::Slider (const ssi_char_t *name, 
	ssi_real_t value,
	ssi_real_t min,
	ssi_real_t max,
	ssi_size_t steps,
	ORIENTATION::List orientation)
	: _parent(0),
	_steps (steps),
	_value (value),
	_min (min),
	_max (max),
	_orientation(orientation),
	_callback (0),
	_hWnd (0),
	_hSlider (0) {

	_name = ssi_strcpy(name);
}

Slider::~Slider () {

	delete[] _name;

	if (_hSlider) {
		::DestroyWindow(_hSlider);
	}
}

void Slider::create(IWindow *parent) {
	_parent = parent;
}

void Slider::set (ssi_real_t value) {

	ssi_real_t new_value = (value - _min) / (_max - _min); 

	if (new_value < 0 || new_value > 1.0f) {
		ssi_wrn ("ignore value %.2f out of range [0..1]", value);
		return;
	}

	_value = value;

	ssi_char_t name[SSI_MAX_CHAR];
	ssi_sprint(name, "%s (%f)", _name, _value);
	::SendMessage(_hWnd, WM_SETTEXT, 0, (LPARAM)name);
	::SendMessage(_hSlider, TBM_SETPOS, true, ssi_cast(ssi_size_t, new_value * _steps + 0.5f));
}

ssi_real_t Slider::get() {

	return _value;
}

ssi_handle_t Slider::getHandle() {
	return _hSlider;
}

void Slider::create(HWND hWnd, HMENU id) {

	DWORD style = WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_ENABLESELRANGE | TBS_BOTH;
	if (_orientation == ORIENTATION::VERTICAL) {
		style |= TBS_VERT;
	}

	_hSlider = ::CreateWindowEx(WS_EX_CLIENTEDGE,
		TRACKBAR_CLASS,
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

	if (!_hSlider) {
		//PrintLastError();
		ssi_wrn("could not create slider");
	} else {	

		::SendMessage(_hSlider, TBM_SETRANGEMAX, (WPARAM)TRUE, _steps);
		::SendMessage(_hSlider, TBM_SETTICFREQ, _steps / 10, 0);
		
		set(_value);
	}
}

void Slider::setPosition(ssi_rect_t rect) {

	::MoveWindow(_hSlider, 0, 0, rect.width, rect.height, TRUE);
}

void Slider::update() {
}

LRESULT CALLBACK Slider::windowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

	if (!_hWnd) {
		_hWnd = hWnd;
	}

	switch (msg) {

	case WM_CREATE: {
		create(hWnd,0);
		return 0;
	}	

	case WM_HSCROLL:
	case WM_VSCROLL: {

		ssi_size_t pos = static_cast<ssi_size_t>(::SendMessage(_hSlider, TBM_GETPOS, 0, 0));

		if (pos > _steps) {
			::SendMessage(_hSlider, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)_steps);
			pos = _steps;
		}

		if (pos < 0) {
			::SendMessage(_hSlider, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)0);
			pos = 0;
		}

		_value = _min + (_max - _min) * pos / (1.0f * _steps);

		ssi_msg(SSI_LOG_LEVEL_DEBUG, "value=%.2f", _value);

		ssi_char_t name[SSI_MAX_CHAR];
		ssi_sprint(name, "%s (%f)", _name, _value);
		::SendMessage(_hWnd, WM_SETTEXT, 0, (LPARAM)name);

		if (_callback && wParam == SB_ENDSCROLL) {
			_callback->update(_value);
		}

		return 0;
	}
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);;
}

}

#endif