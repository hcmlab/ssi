// SDL_WindowManager.h
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

#ifndef SSI_GRAPHIC_SDL_WINDOWMANAGER_H
#define SSI_GRAPHIC_SDL_WINDOWMANAGER_H

#include "SSI_Define.h"

#ifdef SSI_USE_SDL

#include "base/IWindowManager.h"
#include "thread/Thread.h"
#include "thread/Lock.h"

union SDL_Event;

namespace ssi {

class Window;

class SDL_WindowManager : public IWindowManager, public Thread
{

friend class Factory;

public:

	struct EVENTS {
		enum List {
			CREATE_WINDOW = 0,
			CLOSE_WINDOW,
			UPDATE_WINDOW
		};
	};

protected:

	SDL_WindowManager();
	~SDL_WindowManager();

	static ssi_char_t *ssi_log_name;
	static int ssi_log_level;
	
	bool init();
	void run();
	void updateWindow(Window *window);
	void createWindow(Window *window);
	void closeWindow(Window *window);

	void handleUserEvent(SDL_Event *event);
	void handleWindowEvent(SDL_Event *event);

	std::map<ssi_int_t, Window *> _windows;

	bool _quit;
};

} 

#endif

#endif