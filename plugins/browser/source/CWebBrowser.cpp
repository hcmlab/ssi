// CWebBrowser.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/11/18
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

#include "cwebpage.h"
#include "CWebBrowser.h"
#include "base/Factory.h"
#include "Shlwapi.h"
#include "ioput/file/FileTools.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

const char *CWebBrowser::_wClassName = "CWebBrowserClass";
HMODULE CWebBrowser::_hInstance = 0;
HWND CWebBrowser::_hParent = 0;

HINSTANCE CWebBrowser::_hDll = 0;
EmbedBrowserObjectPtr *CWebBrowser::lpEmbedBrowserObject = 0;
UnEmbedBrowserObjectPtr *CWebBrowser::lpUnEmbedBrowserObject = 0;
DisplayHTMLPagePtr *CWebBrowser::lpDisplayHTMLPage = 0;
DisplayHTMLStrPtr *CWebBrowser::lpDisplayHTMLStr = 0;
ResizeBrowserPtr	*CWebBrowser::lpResizeBrowser = 0;

CWebBrowser::CWebBrowser (const ssi_char_t *name, 
	ssi_size_t maxchar)
: Thread (true),	
	_hWnd(0) {

	_position = ssi_rect(0, 0, 100, 100);

	LoadDLL();
	RegisterWindowClass();

	_name = ssi_strcpy(name);
	Thread::setName(_name);
}

CWebBrowser::~CWebBrowser () {

	FreeLibrary(_hDll);
}

void CWebBrowser::PrintLastError()
{
	LPTSTR lpMsgBuf;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0,
		NULL);

	ssi_wrn(lpMsgBuf)

}

void CWebBrowser::LoadDLL() {

	if (_hDll) {
		return;
	}

#ifdef _DEBUG
	if ((_hDll = LoadLibrary("cwebpaged.dll")))
#else
	if ((_hDll = LoadLibrary("cwebpage.dll")))
#endif
	{
		lpEmbedBrowserObject = (EmbedBrowserObjectPtr *)GetProcAddress((HINSTANCE)_hDll, EMBEDBROWSEROBJECTNAME);
		lpUnEmbedBrowserObject = (UnEmbedBrowserObjectPtr *)GetProcAddress((HINSTANCE)_hDll, UNEMBEDBROWSEROBJECTNAME);
		lpDisplayHTMLPage = (DisplayHTMLStrPtr *)GetProcAddress((HINSTANCE)_hDll, DISPLAYHTMLPAGENAME);
		lpDisplayHTMLStr = (DisplayHTMLStrPtr *)GetProcAddress((HINSTANCE)_hDll, DISPLAYHTMLSTRNAME);
		lpResizeBrowser = (ResizeBrowserPtr *)GetProcAddress((HINSTANCE)_hDll, RESIZEBROWSERNAME);
	}
	else {
		ssi_err("cwebpage[d].dll not found");
	}
}

void CWebBrowser::RegisterWindowClass() {

	_hParent = GetConsoleWindow();
	_hInstance = ::GetModuleHandle(NULL);

	WNDCLASS wndcls;
	BOOL result = ::GetClassInfo(_hInstance, _wClassName, &wndcls);

	if (result == 0) {
		WNDCLASSEX wc;
		ZeroMemory(&wc, sizeof(WNDCLASSEX));
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.hInstance = _hInstance;
		wc.lpfnWndProc = WindowProc;
		wc.lpszClassName = _wClassName;

		if (!RegisterClassEx(&wc))
		{
			PrintLastError();
			ssi_err("window registration failed");
		}
	}
}

LRESULT CALLBACK CWebBrowser::WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_NCCREATE) {
		LPCREATESTRUCT cs = (LPCREATESTRUCT)lParam;
		::SetWindowLongPtr(hWnd, GWLP_USERDATA, (long)cs->lpCreateParams);
	}

	CWebBrowser *me = (CWebBrowser *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if (me) {
		return me->windowProc(hWnd, msg, wParam, lParam);
	}
	else {
		return ::DefWindowProc(hWnd, msg, wParam, lParam);
	}
}

LRESULT CALLBACK CWebBrowser::windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_SIZE:
	{
		// Resize the browser object to fit the window
		(*ssi::CWebBrowser::lpResizeBrowser)(hwnd, LOWORD(lParam), HIWORD(lParam));

		return 0;
	}

	case WM_CREATE:
	{
		// Embed the browser object into our host window. We need do this only
		// once. Note that the browser object will start calling some of our
		// IOleInPlaceFrame and IOleClientSite functions as soon as we start
		// calling browser object functions in EmbedBrowserObject().
		if ((*ssi::CWebBrowser::lpEmbedBrowserObject)(hwnd)) {
			return -1;
		}
		
		return 0;
	}

	case WM_CLOSE: {

		::DestroyWindow(hwnd);

		return 0;
	}

	case WM_DESTROY:
	{
		// Detach the browser object from this window, and free resources.
		(*ssi::CWebBrowser::lpUnEmbedBrowserObject)(hwnd);
		PostQuitMessage(0);

		return 1;
	}

	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void CWebBrowser::close() 
{
	::SendMessage(_hWnd, WM_CLOSE, 0, 0);
}

void CWebBrowser::enter() {

	_hWnd = CreateWindowEx
		(0, 
		_wClassName, 
		_name, 
		WS_OVERLAPPEDWINDOW,
		0, 0, 0, 0,
		_hParent,
		0, 
		_hInstance, 
		this);

	if (_hWnd) 
	{
		::EnableMenuItem(::GetSystemMenu(_hWnd, false), SC_CLOSE, MF_BYCOMMAND | MF_GRAYED);
		setPosition(_position);
	}
	else
	{
		ssi_wrn("could not create window");
	}
}

void CWebBrowser::show() 
{
	if (_hWnd) {
		ShowWindow(_hWnd, SW_RESTORE);
		UpdateWindow(_hWnd);
	}
}

void CWebBrowser::hide() 
{
	if (_hWnd) {
		::ShowWindow((HWND)_hWnd, SW_HIDE);
		::UpdateWindow((HWND)_hWnd);
	}
}

bool CWebBrowser::setPosition(const ssi_char_t *position) {

	ssi_real_t posf[4] = { 0, 0, 0, 0 };
	ssi_size_t n = ssi_string2array_count(position, ',');
	if (n == 4) {
		ssi_string2array(n, posf, position, ',');
	}
	else {
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

void CWebBrowser::setPosition(ssi_rect_t rect) {

	_position = rect;

	if (_hWnd) {
		::SetWindowPos((HWND)_hWnd, HWND_TOP, rect.left, rect.top, rect.width, rect.height, 0);
	}
}


void CWebBrowser::run()
{
	MSG msg;

	while (GetMessage(&msg, 0, 0, 0) == 1)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void CWebBrowser::navigate(const ssi_char_t *url) {

	if (_hWnd)
	{	
		if (!::PathIsURL(url) && ::PathIsRelative(url)) {
			ssi_char_t path[SSI_MAX_CHAR];
			GetCurrentDirectory(SSI_MAX_CHAR, path);
			ssi_strcpy(path + ssi_strlen(path), "\\");
			ssi_strcpy(path + ssi_strlen(path), url);		
			(*lpDisplayHTMLPage)(_hWnd, path);
		} else {
			(*lpDisplayHTMLPage)(_hWnd, url);
		}

		::InvalidateRect(_hWnd, NULL, FALSE);
		::UpdateWindow(_hWnd);
		
	}
}

void CWebBrowser::navigateString(const ssi_char_t *string) {

	if (_hWnd)
	{
		// crashes
		//(*lpDisplayHTMLStr)(_hWnd, string);		

		char dir[SSI_MAX_CHAR];
		char url[SSI_MAX_CHAR];
		char *file = "~.html";

		::GetCurrentDirectory(SSI_MAX_CHAR, dir);
		ssi_sprint(url, "file:///%s\\%s", dir, file);
		FileTools::WriteAsciiFile(file, string);

		(*lpDisplayHTMLPage)(_hWnd, url);		

		::InvalidateRect(_hWnd, NULL, FALSE);
		::UpdateWindow(_hWnd);
	}
}

}
