// SystemTray.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2016/02/02
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

#include "graphic/SystemTray.h"

namespace ssi {	

SystemTray::SystemTray(HWND hWnd, HICON icon, const ssi_char_t *title)
: _hWnd(hWnd) {

	_title = ssi_strcpy(title);

	if (_hWnd) {

		_nid.cbSize = sizeof(NOTIFYICONDATA);
		_nid.hWnd = _hWnd;
		_nid.uID = 100;
#ifdef NOTIFYICON_VERSION_4
		_nid.uVersion = NOTIFYICON_VERSION_4;
#else 
		_nid.uVersion = NOTIFYICON_VERSION;
#endif
		_nid.uCallbackMessage = SSI_GRAPHIC_SYSTEMTRAY_MSG;
		_nid.hIcon = icon;
		ssi_strcpy(_nid.szTip, _title);
		_nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;

	}
}

SystemTray::~SystemTray() {
	delete _title; _title = 0;
}

void SystemTray::create() {

	if (!_hWnd) {
		return;
	}

	::Shell_NotifyIcon(NIM_ADD, &_nid);
}

void SystemTray::close() {

	if (!_hWnd) {
		return;
	}

	::Shell_NotifyIcon(NIM_DELETE, &_nid);
}

void SystemTray::showMenu(bool isVisible, bool minMaxVisible)
{
	if (!_hWnd) {
		return;
	}

	POINT pt;
	GetCursorPos(&pt);
	HMENU hMenu = CreatePopupMenu();
	if (hMenu)
	{
		::InsertMenu(hMenu, -1, MF_BYPOSITION | MF_GRAYED, 0, _title);
		::InsertMenu(hMenu, -1, MF_BYPOSITION | MF_SEPARATOR, 0, _title);
		::InsertMenu(hMenu, -1, MF_BYPOSITION, SSI_GRAPHIC_SYSTEMTRAY_ABOUT, "About");
		if (isVisible) {
			::InsertMenu(hMenu, -1, MF_BYPOSITION, SSI_GRAPHIC_SYSTEMTRAY_HIDE, "Hide windows");
		} else {
			::InsertMenu(hMenu, -1, MF_BYPOSITION, SSI_GRAPHIC_SYSTEMTRAY_SHOW, "Show windows");
		}
		if (minMaxVisible) {
			::InsertMenu(hMenu, -1, MF_BYPOSITION, SSI_GRAPHIC_MINMAX_HIDE, "Hide titlebar icons");
		}
		else {
			::InsertMenu(hMenu, -1, MF_BYPOSITION, SSI_GRAPHIC_MINMAX_SHOW, "Show titlebar icons");
		}
		::InsertMenu(hMenu, -1, MF_BYPOSITION, SSI_GRAPHIC_SYSTEMTRAY_EXIT, "Exit");

		// note:	must set window to the foreground or the
		//			menu won't disappear when it should
		::SetForegroundWindow(_hWnd);
		::TrackPopupMenu(hMenu, TPM_BOTTOMALIGN, pt.x, pt.y, 0, _hWnd, NULL);
		::DestroyMenu(hMenu);
	}
}

void SystemTray::showAbout()
{
	if (_hWnd) {
		int msgboxID = ::MessageBox(_hWnd,			
			SSI_COPYRIGHT,
			SSI_VERSION,
			MB_ICONINFORMATION | MB_OK);
	}
}
	
}

#endif