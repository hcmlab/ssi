// Window.cpp
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

#include "SSI_Cons.h"

#ifndef SSI_USE_SDL

#include "graphic/Window.h"
#include "base/Factory.h"
#include "graphic/SystemTray.h"
#include "resource.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif


namespace ssi {

const char *Window::_wClassName = "SSIWindow";
ssi_handle_t Window::_hParent = 0;
ssi_handle_t Window::_hInstance = 0;
ssi_handle_t Window::_hIcon = 0;

Window::Window ()
: Thread (true),
	_client (0),
	_min_width (0),
	_max_width (0),
	_min_height (0),
	_max_height(0),
	_icons(ICONS::NONE),
	_hWnd(0),
	_systemTray(0),
	_showAndHideCallback(0) {

	RegisterWindowClass();

	_position.left = 0;
	_position.top = 0;
	_position.height = 100;
	_position.width = 100;

	_title[0] = '\0';

	Thread::setName("Window");
}

Window::~Window () {

	if (_hWnd) {
		::DestroyWindow((HWND) _hWnd);
	}
}

ssi_handle_t Window::getHandle() {
	return _hWnd;
}

void Window::setClient(IWindowClient *client) {
	_client = client;
}

void Window::setLimits(ssi_size_t min_width, ssi_size_t max_width, ssi_size_t min_height, ssi_size_t max_height) {
	_min_width = min_width;
	_max_width = max_width;
	_min_height = min_height;
	_max_height = max_height;
}

void Window::PrintLastError()
{
	ssi_PrintLastError();
}

void Window::RegisterWindowClass() {

	_hParent = GetConsoleWindow();
	_hInstance = ::GetModuleHandle(NULL);
	_hIcon = LoadIcon((HINSTANCE)_hInstance, MAKEINTRESOURCE(IDI_ICON1));
	if (!_hIcon) {
		_hIcon = ::LoadIcon(NULL, IDI_APPLICATION);
	}

	WNDCLASS wndcls;
	BOOL result = ::GetClassInfo((HINSTANCE) _hInstance, _wClassName, &wndcls);

	if (result == 0) {

		WNDCLASSEX wClass;
		wClass.cbSize = sizeof(WNDCLASSEX);
		wClass.style = 0;
		wClass.lpfnWndProc = WindowProc;
		wClass.cbClsExtra = 0;
		wClass.cbWndExtra = 0;
		wClass.hInstance = (HINSTANCE) _hInstance;
		wClass.hIcon = (HICON) _hIcon;
		wClass.hCursor = ::LoadCursor(NULL, IDC_ARROW);
		wClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wClass.lpszMenuName = NULL;
		wClass.lpszClassName = _wClassName;
		wClass.hIconSm = ::LoadIcon(NULL, IDI_APPLICATION);

		if (!RegisterClassEx(&wClass))
		{
			PrintLastError();
			ssi_err("window registration failed");
		}
	}
}

LRESULT CALLBACK Window::WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_NCCREATE) {
		LPCREATESTRUCT cs = (LPCREATESTRUCT)lParam;
		::SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)cs->lpCreateParams);
	}

	Window *me = (Window *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if (me) {	
		return me->windowProc(hWnd, msg, wParam, lParam);
	} else {
		return ::DefWindowProc(hWnd, msg, wParam, lParam);
	}
}

LRESULT CALLBACK Window::windowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

	switch (msg) {

	case WM_GETMINMAXINFO:
	{
		MINMAXINFO *lpMinMaxInfo = (MINMAXINFO *)lParam;

		if (_min_width > 0) {
			lpMinMaxInfo->ptMinTrackSize.x = _min_width;
		}
		if (_min_height > 0) {
			lpMinMaxInfo->ptMinTrackSize.y = _min_height;
		}
		if (_max_width > 0) {
			lpMinMaxInfo->ptMaxTrackSize.x = _max_width;
		}
		if (_max_height > 0) {
			lpMinMaxInfo->ptMaxTrackSize.y = _max_height;
		}

		break;
	}

	case WM_WINDOWPOSCHANGED: {

		if (_client) {
			RECT rect;
			::GetClientRect((HWND)_hWnd, &rect);
			ssi_rect_t r;
			r.left = rect.left;
			r.top = rect.top;
			r.width = rect.right - rect.left;
			r.height = rect.bottom - rect.top;
			_client->setPosition(r);
		}

		return 0;
	}

	case SSI_GRAPHIC_SYSTEMTRAY_MSG: {

		switch (lParam)
		{
		case WM_LBUTTONDBLCLK:			
			if (isVisible()) {
				hide();
			} else {
				show();
			}
			break;
		case WM_LBUTTONDOWN:
			if (_systemTray) {				
				_systemTray->showMenu(isVisible(), _showAndHideCallback ? _showAndHideCallback->isMinMaxVisible() : false);
			}
			break;
		};

		return 0;
	}

	case WM_COMMAND: {

		int wmId = LOWORD(wParam);
		int wmEvent = HIWORD(wParam);

		switch (wmId)
		{
		case SSI_GRAPHIC_SYSTEMTRAY_SHOW:
			show();
			return 0;

		case SSI_GRAPHIC_SYSTEMTRAY_HIDE:
			hide();
			return 0;

		case SSI_GRAPHIC_MINMAX_SHOW:
			minmax_show();
			return 0;
		case SSI_GRAPHIC_MINMAX_HIDE:
			minmax_hide();
			return 0;

		case SSI_GRAPHIC_SYSTEMTRAY_ABOUT:
			if (_systemTray) {
				_systemTray->showAbout();
			}
			return 0;

		case SSI_GRAPHIC_SYSTEMTRAY_EXIT:
			ITheFramework *frame = Factory::GetFramework();
			if (frame) {
				frame->CancelWait();
			}
			return 0;
		}

		break;
	}

	case WM_CLOSE: {

		::DestroyWindow(hWnd);

		return 0;
	}

	case WM_DESTROY: {

		if (_icons & ICONS::SYSTEMTRAY) {
			if (_systemTray) {
				_systemTray->close();
				delete _systemTray; _systemTray = 0;
			}
		}

		::PostQuitMessage(0);

		return 0;
	}

	}

	if (_client) {
		return _client->windowProc(hWnd, msg, wParam, lParam);
	} else {
		return ::DefWindowProc(hWnd, msg, wParam, lParam);
	}
}

void Window::enter() {
	
	_hWnd = ::CreateWindowEx(
		WS_EX_CLIENTEDGE,
		_wClassName,
		_title,
		WS_CLIPCHILDREN | WS_SIZEBOX,
		0, 0, 0, 0,
		(HWND)_hParent,
		0,
		(HINSTANCE)_hInstance,
		this);

	if (!_hWnd)
	{
		PrintLastError();
		ssi_wrn("could not create window");
	} else {
		setIcons(_icons);
		setTitle(_title);
		setPosition(_position);
	}
}

void Window::run () {

	MSG msg;
	while (::GetMessage(&msg, NULL, NULL, NULL) > 0) {
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);	
	}
}

void Window::create() {

	start();

	if (_client) {
		_client->create(this);
	}
}

bool Window::isVisible() {

	return  _showAndHideCallback ? _showAndHideCallback->isVisible() : IsWindowVisible((HWND)_hWnd) == TRUE;
}



void Window::show() {

	if (_showAndHideCallback) {
		_showAndHideCallback->show(this);
	} else {
		if (_hWnd) {
			::ShowWindow((HWND)_hWnd, SW_RESTORE);
			::UpdateWindow((HWND)_hWnd);
		}
	}
}

void Window::hide() {

	if (_showAndHideCallback) {
		_showAndHideCallback->hide(this);
	}
	else {
		if (_hWnd) {
			::ShowWindow((HWND)_hWnd, SW_HIDE);
			::UpdateWindow((HWND)_hWnd);
		}
	}
}

bool Window::isMinMaxVisible() {

	return  _showAndHideCallback ? _showAndHideCallback->isVisible() : IsWindowVisible((HWND)_hWnd) == TRUE;
}

void Window::minmax_show() {

	if (_showAndHideCallback) {
		_showAndHideCallback->minmax_show(this);
	}
}

void Window::minmax_hide() {

	if (_showAndHideCallback) {
		_showAndHideCallback->minmax_hide(this);
	}
}

void Window::update() {

	if (_client) {
		_client->update();
	}	
}

void Window::setTitle(const ssi_char_t *title) {

	Thread::setName(title);
	ssi_strcpy(_title, title);

	if (_hWnd) {
		::SendMessage((HWND)_hWnd, WM_SETTEXT, ssi_strlen(title) + 1, (LPARAM)title);
	}
}

bool Window::setPosition(const ssi_char_t *position) {

	ssi_real_t posf[4] = { 0, 0, 0, 0 };
	ssi_size_t n = ssi_string2array_count(position, ',');
	if (n == 4) {
		ssi_string2array(n, posf, position, ',');
	} else {
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

void Window::setPosition(ssi_rect_t rect) {

	_position = rect;

 	if (_hWnd) {
		::SetWindowPos((HWND)_hWnd, HWND_TOP, rect.left, rect.top, rect.width, rect.height, 0);
	}
}

unsigned int Window::getIcons(){
	return _icons;
}

void Window::setIcons(ICON icons) {

	_icons = icons;

	if (_hWnd) {

		HWND hWnd = (HWND)_hWnd;

		if (_icons == ICONS::NONE || _icons == ICONS::SYSTEMTRAY) {
			::SetWindowLong(hWnd, GWL_STYLE, ::GetWindowLong(hWnd, GWL_STYLE) & ~WS_SYSMENU);
		} else {
			::SetWindowLong(hWnd, GWL_STYLE, ::GetWindowLong(hWnd, GWL_STYLE) | WS_SYSMENU);
			::EnableMenuItem(::GetSystemMenu(hWnd, false), SC_CLOSE, MF_BYCOMMAND | MF_GRAYED);
			::SetWindowLong(hWnd, GWL_STYLE, ::GetWindowLong(hWnd, GWL_STYLE) & ~WS_MINIMIZEBOX);
			::SetWindowLong(hWnd, GWL_STYLE, ::GetWindowLong(hWnd, GWL_STYLE) & ~WS_MAXIMIZEBOX);
			if (_icons & ICONS::CLOSE) {
				::EnableMenuItem(::GetSystemMenu(hWnd, false), SC_CLOSE, MF_BYCOMMAND | MF_ENABLED);
			}
			if (_icons & ICONS::MINIMIZE) {
				::SetWindowLong(hWnd, GWL_STYLE, ::GetWindowLong(hWnd, GWL_STYLE) | WS_MINIMIZEBOX);
			}
			if (_icons & ICONS::MAXIMIZE) {
				::SetWindowLong(hWnd, GWL_STYLE, ::GetWindowLong(hWnd, GWL_STYLE) | WS_MAXIMIZEBOX);
			}
		}		

		if (_icons & ICONS::SYSTEMTRAY) {
			if (!_systemTray) {
				_systemTray = new SystemTray(hWnd, (HICON) _hIcon, _title);				
				_systemTray->create();
			}
		} else {
			if (_systemTray) {
				_systemTray->close();
				delete _systemTray; _systemTray = 0;
			}
		}
	}
}

void Window::close() {

	if (_hWnd) {
		::SendMessage((HWND)_hWnd, WM_CLOSE, 0, 0);
	}

	if (_client) {
		_client->close();
	}
}

void Window::setCallback(Window::ShowAndHideCallback *callback) {
	_showAndHideCallback = callback;
}

}

#endif