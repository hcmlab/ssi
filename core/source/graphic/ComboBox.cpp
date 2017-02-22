// ComboBox.cpp
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

#include "graphic/ComboBox.h"
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

ssi_char_t *ComboBox::ssi_log_name = "combobox__";

ComboBox::ComboBox (ssi_size_t n_items,
	const ssi_char_t **items,
	ssi_size_t current)
	: _parent(0),
	_n_items(n_items),
	_items(0),
	_current(current),
	_callback (0),
	_hComboBox (0) {

	if (_n_items > 0) {
		_items = new ssi_char_t *[n_items];
		for (ssi_size_t i = 0; i < _n_items; i++) {
			_items[i] = ssi_strcpy(items[i]);
		}
	}
}

ComboBox::~ComboBox () {

	for (ssi_size_t i = 0; i < _n_items; i++) {
		delete[] _items[i];
	}
	_items = 0;
	_n_items = 0;

	if (_hComboBox) {
		::DestroyWindow(_hComboBox);
	}
}

void ComboBox::create(IWindow *parent) {
	_parent = parent;
}

void ComboBox::set(const ssi_char_t *item) {

	for (ssi_size_t i = 0; i < _n_items; i++) {
		if (ssi_strcmp(item, _items[i])) {
			set(i);
		}
	}

	ssi_wrn("an item %s does not exist", item);	
}

void ComboBox::set(ssi_size_t index) {

	if (index >= _n_items) {
		ssi_wrn ("index %u out of range [0..%u]", index, _n_items-1);
		return;
	}
	_current = index;

	::SendMessage(_hComboBox, CB_SETCURSEL, (WPARAM)_current, (LPARAM)0);
}

const ssi_char_t *ComboBox::get() {

	return _items[_current];
}

ssi_handle_t ComboBox::getHandle() {
	return _hComboBox;
}

void ComboBox::create(HWND hWnd, HMENU id) {

	_hComboBox = ::CreateWindow(WC_COMBOBOX,		
		"",
		CBS_DROPDOWN | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
		0,
		0,
		0,
		0,
		hWnd,
		id,
		::GetModuleHandle(NULL),
		0);

	if (!_hComboBox) {
		//PrintLastError();
		ssi_wrn("could not create combo box");
	} else {	

		for (ssi_size_t i = 0; i < _n_items; i++)
		{			
			// Add string to combobox.
			::SendMessage(_hComboBox, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)_items[i]);
		}

		set(_current);
	}
}

void ComboBox::setPosition(ssi_rect_t rect) {

	::MoveWindow(_hComboBox, 0, 0, rect.width, rect.height, TRUE);
}

void ComboBox::update() {
}

LRESULT CALLBACK ComboBox::windowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

	switch (msg) {

	case WM_CREATE: {
		create(hWnd, 0);
		return 0;
	}

	case WM_COMMAND: {

		if (HIWORD(wParam) == CBN_SELCHANGE)
			// If the user makes a selection from the list:
			//   Send CB_GETCURSEL message to get the index of the selected list item.
			//   Send CB_GETLBTEXT message to get the item.
			//   Display the item in a messagebox.
		{
			_current = (ssi_size_t) ::SendMessage((HWND)lParam, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);

			if (_callback) {
				_callback->update(_items[_current]);
			}
			
			ssi_msg(SSI_LOG_LEVEL_DETAIL, "selection changed '%s'", _items[_current]);
		}

		return 0;
	}
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);;
}

}

#endif