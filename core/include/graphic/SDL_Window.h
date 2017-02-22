// SDL_Window.h
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

#pragma once

#ifndef SSI_GRAPHIC_SDL_WINDOW_H
#define SSI_GRAPHIC_SDL_WINDOW_H

#include "SSI_Define.h"

#ifdef SSI_USE_SDL

#include "base/IWindow.h"
#include "thread/Event.h"

union SDL_Event;
struct SDL_Window;
struct SDL_Renderer;

namespace ssi{
	
class SDL_WindowManager;

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

class Window : public IWindow
{

friend class SDL_WindowManager;

public:

	class ShowAndHideCallback {
	public:
		virtual bool isVisible() { return true;  };
		virtual void show(Window *window) {};
		virtual void hide(Window *window) {};
	};

public:
	
	Window();
	virtual ~Window();

	void setHandle(ssi_handle_t handle);
	ssi_handle_t getHandle();

	void create();
	void show();
	bool isVisible();
	void hide();	
	void update();
	void close();

	void setLimits(ssi_size_t min_width, ssi_size_t max_width, ssi_size_t min_height, ssi_size_t max_height);
	void setPosition(ssi_rect_t rect);
	bool setPosition(const ssi_char_t* message);
	void setTitle(const ssi_char_t *str);
	void setClient(IWindowClient *client);
	void setIcons(ICON icons);
	void setCallback(ShowAndHideCallback *callback) {};
        ICON getIcons(){return 0;};
		
protected:

	SDL_WindowManager *_manager;

	void update_safe();
	void close_safe();

	SDL_Window *_handle;
	Event _event;

	ssi_char_t _title[SSI_MAX_CHAR];
	ssi_rect_t _position;
	ssi_rect_t _position_offset;
	ICON _icons;
	ssi_int_t _min_width;
	ssi_int_t _max_width;
	ssi_int_t _min_height;
	ssi_int_t _max_height;
	
	IWindowClient *_client;
};

} 

#endif

#endif
