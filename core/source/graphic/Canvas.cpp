// Canvas.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2007/11/13
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

#include "graphic/Canvas.h"
#include "graphic/Colormap.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

Canvas::Canvas() :
	_parent(0),
	_hWnd(0),		
	_bitmap(0),
	_context(0),
	_context_mem(0),
	_width(0),
	_height(0),
	_rect(ssi_rect(0,0,0,0)){
}

Canvas::~Canvas () {

	Lock lock(_mutex);
	_objects.clear();
}

void Canvas::create(IWindow *parent) 
{
	_parent = parent;

	std::vector<ICanvasClient *>::iterator iter;
	for (iter = _objects.begin(); iter != _objects.end(); iter++) {
		(*iter)->create(this);
	}
}

void Canvas::close() {

	if (_context_mem) {
		::DeleteDC((HDC) _context_mem);
		_context_mem = 0;
	}
	if (_bitmap) {
		::DeleteObject(_bitmap);
		_bitmap = 0;
	}	
}

void Canvas::update() {

	Lock lock(_mutex);

	::InvalidateRect(_hWnd, 0, 0);
}

void Canvas::setPosition(ssi_rect_t rect) {
	update();
}

ssi_handle_t Canvas::getHandle() {
	return _hWnd;
}

void Canvas::mouseDown(ssi_point_t position, IWindow::KEY vkey) {

	std::vector<ICanvasClient *>::iterator iter;
	for (iter = _objects.begin(); iter != _objects.end(); iter++) {
		(*iter)->mouseDown(position, vkey);
	}
	//ssi_print("clicked@(%d,%d)\n", position.x, position.y);
}

void Canvas::keyDown(IWindow::KEY key, IWindow::KEY vkey) {

	std::vector<ICanvasClient *>::iterator iter;
	for (iter = _objects.begin(); iter != _objects.end(); iter++) {
		(*iter)->keyDown(key, vkey);
	}
	//ssi_print("key@%d\n", key);
}

void Canvas::addClient(ICanvasClient *client) {

	Lock lock(_mutex);

	_objects.push_back(client);
}

void Canvas::paint(){

	// get drawing context
	PAINTSTRUCT ps;
	_context = ::BeginPaint((HWND)_hWnd, &ps);
	
	// get dimension of the canvas
	RECT r;
	::GetClientRect((HWND)_hWnd, &r);	
	_rect.height = r.bottom - r.top;
	_rect.left = r.left;
	_rect.width = r.right - r.left;
	_rect.top = r.top;	
	
	// reallocate memory bitmap if necessary
	if (_width != _rect.width || _height != _rect.height) {
		_width = _rect.width;
		_height = _rect.height;
		if (_context_mem) {
			::DeleteDC((HDC) _context_mem);
		}
		_context_mem = ::CreateCompatibleDC((HDC)_context);
		if (_bitmap) {
			::DeleteObject(_bitmap);
		}
		_bitmap = ::CreateCompatibleBitmap((HDC)_context, _width, _height);
	}
	
	// switch to memory bitmap
	HBITMAP bitmap_tmp = (HBITMAP) ::SelectObject((HDC)_context_mem, _bitmap);

	// paint objects
	paint_objects();

	// copy the memory bitmap to the hdc
	BitBlt((HDC)_context, 0, 0, _width, _height, (HDC)_context_mem, 0, 0, SRCCOPY);

	// select old bitmap back into the device context
	::SelectObject((HDC)_context_mem, bitmap_tmp);

	// close drawing context
	::EndPaint((HWND)_hWnd, &ps);
}

void Canvas::paint_objects() {

	Lock lock(_mutex);
	std::vector<ICanvasClient *>::iterator iter;
	for (iter = _objects.begin(); iter != _objects.end(); iter++) {
		(*iter)->paint(_context_mem, _rect);
	}
}

LRESULT CALLBACK Canvas::windowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

	if (!_hWnd) {
		_hWnd = hWnd;
	}

	switch (msg) {

	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_LBUTTONDOWN:
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

		mouseDown(ssi_point(x, y), vkey);

		return 0;
	}

	case WM_KEYDOWN: 
	{

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

		keyDown(key, vkey);

		return 0;
	}		

	case WM_PAINT: 
		
		paint();

		return 0;

	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

}

#endif