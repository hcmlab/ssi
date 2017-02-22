// SDL_WindowManager.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2016/02/15
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

#ifdef SSI_USE_SDL

#include "SDL/include/ssisdl.h"
#include "SDL/include/SDL_syswm.h"
#include "graphic/SDL_WindowManager.h"
#include "graphic/SDL_Window.h"

namespace ssi{

	ssi_char_t *SDL_WindowManager::ssi_log_name = "winmanager";
	int SDL_WindowManager::ssi_log_level = SSI_LOG_LEVEL_DEFAULT;

	SDL_WindowManager::SDL_WindowManager()
		: Thread(true),
		_quit(false)
	{
		setName("SDLWindowManager");
		start();
	}

	SDL_WindowManager::~SDL_WindowManager()
	{
		_quit = true; 			
		SDL_Event event;
		event.type = SDL_QUIT;
		SDL_PushEvent(&event);
		while (!_quit) {
			ssi_sleep(10);
		}
		TTF_Quit();
		SDL_Quit();		
	}

	void SDL_WindowManager::handleUserEvent(SDL_Event *event) {

		//printf("SDL_USEREVENT %d\n", event.window.event);

		switch (event->user.code)
		{
		case EVENTS::CREATE_WINDOW:
		{
			createWindow((Window*)event->user.data1);
			break;
		}
		case EVENTS::CLOSE_WINDOW:
		{
			closeWindow((Window*)event->user.data1);
			break;
		}
		case EVENTS::UPDATE_WINDOW: 
		{
			updateWindow((Window*)event->user.data1);
			break;
		}
		}
	}

	void SDL_WindowManager::handleWindowEvent(SDL_Event *event) {

		//printf("SDL_WINDOWEVENT %d\n", event.window.event);

		switch (event->window.event)
		{
		case SDL_WINDOWEVENT_SHOWN:
		case SDL_WINDOWEVENT_EXPOSED:
		case SDL_WINDOWEVENT_SIZE_CHANGED:
		case SDL_WINDOWEVENT_MAXIMIZED:
		case SDL_WINDOWEVENT_RESTORED:
		{
			if (_windows.find(event->window.windowID) != _windows.end()) {
				_windows[event->window.windowID]->update_safe();
			}
			break;
		}
		case SDL_WINDOWEVENT_CLOSE:
		{
			if (_windows.find(event->window.windowID) != _windows.end()) {
				closeWindow(_windows[event->window.windowID]);
			}
			break;
		}
		}
	}

	void SDL_WindowManager::run()
	{
		_quit = false;

		if (init()) 
		{
			SDL_Event event;
			while (1)
			{		
				SDL_WaitEvent(&event);

				if (event.type == SDL_QUIT) {
					break;
				}
				else if (event.type == SDL_USEREVENT)
				{
					handleUserEvent(&event);
				}
				else if (event.type == SDL_WINDOWEVENT)
				{
					handleWindowEvent(&event);
				}
			}
		}

		_quit = true;
	}

	void SDL_WindowManager::updateWindow(Window *window) 
	{		
		if (window)
		{
			Uint32 id = SDL_GetWindowID((SDL_Window*)window->getHandle());
			if (_windows.find(id) != _windows.end())
			{
				_windows[id]->update_safe();
			}
		}
	}

	///adds a window to the windowmanager
	void SDL_WindowManager::createWindow(Window *window)
	{		

		if (window)
		{
			SDL_Window *handle = SDL_CreateWindow(window->_title,
				window->_position.left,
				window->_position.top,
				window->_position.width,
				window->_position.height,
				SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE);
			
			if (handle)
			{
				window->setHandle(handle);

#if _WIN32|_WIN64
				SDL_SysWMinfo info;
				SDL_VERSION(&info.version);				
				if (SDL_GetWindowWMInfo(handle, &info)) {					
					RECT rect;
					::GetWindowRect(info.info.win.window, &rect);																				
					window->_position_offset.left = window->_position.left - rect.left;
					window->_position_offset.top = window->_position.top - rect.top;
					window->_position_offset.width = window->_position.width - (rect.right - rect.left);
					window->_position_offset.height = window->_position.height - (rect.bottom - rect.top);
				}
#endif								
				window->setLimits(window->_min_width, window->_max_width, window->_min_height, window->_max_height);
				window->setPosition(window->_position);
				Uint32 id = SDL_GetWindowID(handle);
				_windows[id] = window;

				ssi_msg(SSI_LOG_LEVEL_DETAIL, "created window 'id=%d'", id);
			}
			else
			{
				ssi_wrn("could not create window '%s'", SDL_GetError());
			}
		}
	}

	void SDL_WindowManager::closeWindow(Window *window) {
	
		if (window)
		{
			SDL_Window *handle = (SDL_Window *)window->getHandle();
			if (handle)
			{
				Uint32 id = SDL_GetWindowID(handle);
				SDL_DestroyWindow(handle);
				std::map<ssi_int_t, Window *>::iterator it;
				it = _windows.find(id);
				_windows.erase(it);				

				ssi_msg(SSI_LOG_LEVEL_DETAIL, "removed window 'id=%d'", id);
			}

			window->close_safe();
			window->setHandle(0);
		}
	}
		

	bool SDL_WindowManager::init()
	{
		bool success = true;

		if (SDL_Init(SDL_INIT_VIDEO) < 0)
		{
			ssi_wrn("could not initialize SDL '%s'", SDL_GetError());
			success = false;
		}
		else
		{
			if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
			{
				ssi_wrn("could not enalbe linear texture filtering '%s'", SDL_GetError());
			}
		}

		if (TTF_Init() < 0) {
			ssi_wrn("could not initialize TTF '%s'", SDL_GetError());			
		}

		return success;
	}

}

#endif


	

