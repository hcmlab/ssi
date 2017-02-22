// Tab.h
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

#pragma once

#ifndef SSI_GRAPHIC_TAB_H
#define SSI_GRAPHIC_TAB_H

#include "SSI_Define.h"

#ifndef SSI_USE_SDL

#include "base/IWindow.h"
#include "base/String.h"

namespace ssi {

class ITabClient : public IWindowClient
{

public:

	virtual void create(HWND hwnd, HMENU id) = 0;
};

class Tab : public IWindowClient {

public:
	
	class ICallback {

	public:
	
		virtual void update(ssi_size_t index) = 0;
	};

public:

	Tab (const ssi_char_t *name);
	~Tab ();

	void create(IWindow *parent);	
	void close() {};
	ssi_handle_t getHandle();
	void update();
	void setPosition(ssi_rect_t rect);

	void addClient(const ssi_char_t *name, ITabClient *child);

	void setCallback (ICallback *callback) {
		_callback = callback;
	};

protected:

	static ssi_char_t *ssi_log_name;

	LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	WNDPROC tabWindowProc;
	static LRESULT CALLBACK TabSubWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void create(HWND hwnd, HMENU id);
	HWND _hTab;
	IWindow *_parent;

	ssi_size_t _current;
	std::vector<ITabClient *> _clients;
	std::vector<String> _client_names;

	ssi_char_t *_name;	
	ssi_size_t _n_tabs;

	ICallback *_callback;
};

}

#endif

#endif