// IWindow.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2014/01/28
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

#ifndef SSI_IWINDOW_H
#define SSI_IWINDOW_H

#include "SSI_Cons.h"

struct ssi_rect_t;

namespace ssi {

class IWindowClient;

class IWindow {

public:

	typedef unsigned int KEY;
	struct VIRTUAL_KEYS {
		enum List : KEY {
			NONE = 0,
			SHIFT_LEFT = 1 << 0,
			SHIFT_RIGHT = 1 << 1,
			CONTROL_LEFT = 1 << 2,
			CONTROL_RIGHT = 1 << 3,
			ALT_LEFT = 1 << 4,
			ALT_RIGHT = 1 << 5,
			MOUSE_LEFT = 1 << 6,
			MOUSE_MIDDLE = 1 << 7,
			MOUSE_RIGHT = 1 << 8
		};
	};

	typedef unsigned int ICON;
	struct ICONS {
		enum List : ICON {
			NONE = 0,
			CLOSE = 1 << 0,
			MINIMIZE = 1 << 1,
			MAXIMIZE = 1 << 2,
			SYSTEMTRAY = 1 << 3
		};
	};


public:

	virtual ~IWindow() {};

	virtual void create() = 0;
	virtual ssi_handle_t getHandle() = 0;
	virtual bool isVisible() = 0;
	virtual void show() = 0;	
	virtual void hide() = 0;
	virtual void update() = 0;
	virtual void close() = 0;

	virtual void setClient(IWindowClient *client) = 0;
	virtual void setTitle(const ssi_char_t *title) = 0;
	virtual bool setPosition(const ssi_char_t *position) = 0;
	virtual void setPosition(ssi_rect_t rect) = 0;
	virtual void setLimits(ssi_size_t min_width, ssi_size_t max_width, ssi_size_t min_height, ssi_size_t max_height) = 0;

	virtual ICON getIcons() = 0;

	virtual void setIcons(ICON icons) = 0;
};

class IWindowClient {

public:

	virtual ~IWindowClient() {};

	virtual void create(IWindow *parent) = 0;
	virtual ssi_handle_t getHandle() = 0;
	virtual void update() = 0;
	virtual void setPosition(ssi_rect_t rect) = 0;
	virtual void close() = 0;

#ifdef SSI_USE_SDL
	virtual void update_safe() = 0;
#else
#if __gnu_linux__

#else
        virtual LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) = 0;
#endif
#endif

};

}

#endif
