// CWebBrowser.h
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

#pragma once

#ifndef SSI_BROWSER_CWEBBROWSER_H
#define SSI_BROWSER_CWEBBROWSER_H

#include "graphic/Window.h"

typedef long WINAPI EmbedBrowserObjectPtr(HWND);
typedef long WINAPI UnEmbedBrowserObjectPtr(HWND);
typedef long WINAPI DisplayHTMLPagePtr(HWND, const char *);
typedef long WINAPI DisplayHTMLStrPtr(HWND, const char *);
typedef void WINAPI ResizeBrowserPtr(HWND, DWORD, DWORD);

namespace ssi {

class CWebBrowser : public Thread {

public:

	CWebBrowser (const ssi_char_t *name,
		ssi_size_t maxlen = SSI_MAX_CHAR);
	virtual ~CWebBrowser ();
		
	void show();
	void hide();
	void navigate(const ssi_char_t *url);
	void navigateString(const ssi_char_t *string);
	bool setPosition(const ssi_char_t *position);
	void setPosition(ssi_rect_t rect);
	void close();

public:
	
	void enter();	
	void run();
	
	static void PrintLastError();
	static HINSTANCE _hDll;
	static EmbedBrowserObjectPtr *lpEmbedBrowserObject;
	static UnEmbedBrowserObjectPtr *lpUnEmbedBrowserObject;
	static DisplayHTMLPagePtr *lpDisplayHTMLPage;
	static DisplayHTMLStrPtr *lpDisplayHTMLStr;
	static ResizeBrowserPtr	*lpResizeBrowser;
	static void LoadDLL();

	static const char *_wClassName;
	static HMODULE _hInstance;	
	static HWND _hParent;
	static void RegisterWindowClass();
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	
	HWND _hWnd;
	LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	
	ssi_rect_t _position;
	ssi_char_t *_name;
	
};

}

#endif
