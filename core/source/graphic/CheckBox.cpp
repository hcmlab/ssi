// CheckBox.cpp
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

#include "graphic/CheckBox.h"
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

ssi_char_t *CheckBox::ssi_log_name = "checkbox__";

CheckBox::CheckBox (const ssi_char_t *name, 
	bool checked)
	: _parent(0),
	_checked (checked),
	_callback (0),
	_hWnd (0),
	_hCheckBox (0) {

	_name = ssi_strcpy(name);
}

CheckBox::~CheckBox () {

	delete[] _name;

	if (_hCheckBox) {
		::DestroyWindow(_hCheckBox);
	}
}

void CheckBox::create(IWindow *parent) {
	_parent = parent;
}

void CheckBox::set(bool checked) {

	_checked = checked;

	::SendMessage(_hCheckBox, (UINT)BM_SETCHECK, (WPARAM) (_checked ? BST_CHECKED : BST_UNCHECKED), (LPARAM)0);

	ssi_msg(SSI_LOG_LEVEL_DETAIL, _checked ? "checked" : "unchecked");
}

bool CheckBox::get() {

	return _checked;
}

ssi_handle_t CheckBox::getHandle() {
	return _hCheckBox;
}

void CheckBox::create(HWND hWnd, HMENU id) {

	_hCheckBox = ::CreateWindow("BUTTON",
		_name,
		WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
		0, 
		0, 
		0, 
		0,
		hWnd,
		id, 
		::GetModuleHandle(NULL),
		0);

	if (!_hCheckBox) {
		//PrintLastError();
		ssi_wrn("could not create comboBox");
	} else {	
		set (_checked);
	}
}

void CheckBox::setPosition(ssi_rect_t rect) {

	::MoveWindow(_hCheckBox, 0, 0, rect.width, rect.height, TRUE);
}

void CheckBox::update() {
}

LRESULT CALLBACK CheckBox::windowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

	if (!_hWnd) {
		_hWnd = hWnd;
	}

	switch (msg) {

	case WM_CREATE: {
		create(hWnd, 0);
		return 0;
	}

	case WM_COMMAND: {

		bool result = ::SendMessage(_hCheckBox, (UINT)BM_GETCHECK, (WPARAM)0, (LPARAM)0) == BST_CHECKED;
		
		if (result) {
			::SendMessage(_hCheckBox, (UINT)BM_SETCHECK, (WPARAM)BST_UNCHECKED, (LPARAM)0);
			_checked = false;
		} else {
			::SendMessage(_hCheckBox, (UINT)BM_SETCHECK, (WPARAM)BST_CHECKED, (LPARAM)0);
			_checked = true;
		}

		if (_callback) {
			_callback->update(_checked);
		}

		ssi_msg(SSI_LOG_LEVEL_DETAIL, _checked ? "checked" : "unchecked");
	
		return 0;
		
	}

	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

}

#endif