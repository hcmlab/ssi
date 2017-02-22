// Tab.cpp
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

#include "graphic/Tab.h"
#include "graphic/Button.h"
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

ssi_char_t *Tab::ssi_log_name = "tab_______";

Tab::Tab (const ssi_char_t *name)
	: _parent(0),
	_n_tabs(0),
	_callback (0),
	_current(0),
	_hTab (0) {

	_name = ssi_strcpy(name);
}

Tab::~Tab () {

	delete[] _name;

	if (_hTab) {
		::DestroyWindow(_hTab);
	}
}

ssi_handle_t Tab::getHandle() {
	return _hTab;
}

void Tab::create(IWindow *parent) {
	_parent = parent;
}

void Tab::create(HWND hWnd, HMENU id) {

	_hTab = ::CreateWindow(WC_TABCONTROL,
		_name,
		WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE,
		0, 
		0, 
		0, 
		0,
		 hWnd,
		id,
		::GetModuleHandle(NULL),
		0);

	if (!_hTab) {
		//PrintLastError();
		ssi_wrn("could not create tab");
	}
}

void Tab::setPosition(ssi_rect_t rect) {

	::MoveWindow(_hTab, 0, 0, rect.width, rect.height, SWP_NOZORDER);

	RECT rectangle;
	::GetClientRect(_hTab, &rectangle);
	::SendMessage(_hTab, TCM_ADJUSTRECT, FALSE, (LPARAM)&rectangle);

	std::vector<ITabClient *>::iterator it;
	for (it = _clients.begin(); it != _clients.end(); it++) {		
		::SetWindowPos((HWND) (*it)->getHandle(), NULL, rectangle.left, rectangle.top, rectangle.right-rectangle.left, rectangle.bottom-rectangle.top, SWP_NOZORDER);
	}

	//::InvalidateRect(_hTab, NULL, TRUE);
	//::InvalidateRect(_hWnd, NULL, TRUE);
}

void Tab::update() {	
}

void Tab::addClient(const ssi_char_t *name, ITabClient *child) {

	if (!_hTab) {
		_clients.push_back(child);
		_client_names.push_back(String(name));
	} else {
		ssi_wrn("you cannot add new tabs once the window has been created");
	}

}


LRESULT CALLBACK Tab::TabSubWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

	Tab *me = (Tab *) ::GetWindowLongPtr(hWnd, GWLP_USERDATA);

	switch (msg) {
	
	case WM_COMMAND: {

		int wmId = LOWORD(wParam);
		int wmEvent = HIWORD(wParam);

		if (wmId > 0) {
			return me->_clients[wmId - 1]->windowProc(hWnd, msg, wParam, lParam);
		}

		break;
	}
	}

	return ::CallWindowProc(me->tabWindowProc, hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK Tab::windowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

	switch (msg) {

	case WM_CREATE: {

		create(hWnd,0);

		if (_hTab) {

			std::vector<ITabClient *>::iterator it;
			ssi_size_t i = 0;
			for (it = _clients.begin(); it != _clients.end(); it++) {

				TCITEM tie;
				tie.mask = TCIF_TEXT | TCIF_IMAGE;
				tie.iImage = -1;
				tie.pszText = (LPSTR)_client_names[i].str();

				if (TabCtrl_InsertItem(_hTab, _n_tabs++, &tie) == -1) {
					ssi_wrn("could not add tab '%s'", _client_names[i].str());
				} else {
					(*it)->create(_hTab,(HMENU) (i+1));
				}

				::ShowWindow((HWND) (*it)->getHandle(), it == _clients.begin() ? SW_SHOW : SW_HIDE);
				::SetParent((HWND)(*it)->getHandle(), _hTab);

				i++;
			}

			::SetWindowLongPtr(_hTab, GWLP_USERDATA, (long)this);

			// setting subclass function to handle key down events
			tabWindowProc = (WNDPROC) ::SetWindowLongPtr(_hTab, GWLP_WNDPROC, (LONG_PTR)TabSubWindowProc);
		}		

		return 0;
	}

	case WM_NOTIFY:
	{
		ssi_size_t result = (ssi_size_t) ::SendMessage( _hTab, TCM_GETCURSEL, 0, 0);

		if (result >= 0 && result < _clients.size()) {		
			if (_current != result) {
				::ShowWindow((HWND) _clients[_current]->getHandle(), SW_HIDE);
				::ShowWindow((HWND)_clients[result]->getHandle(), SW_SHOW);
				_current = result;
				if (_callback) {
					_callback->update(_current);
				}
			}
		}

		return 0;
	}

	case WM_SIZE: {
		return 0;
	}

	}

	return DefWindowProc(hWnd, msg, wParam, lParam);

}

}

#endif