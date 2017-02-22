// SDL_Window.cpp
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
#include "graphic/SDL_Canvas.h"
#include "graphic/SDL_Window.h"
#include "graphic/SDL_WindowManager.h"
#include "base/Factory.h"

namespace ssi{

Window::~Window()
{

}
Window::Window()
	: _manager(0),
	_client(0),
	_handle(0),
	_min_width(0),
	_max_width(0),
	_min_height(0),
	_max_height(0),
	_icons(ICONS::NONE),
	_event(false, false)
{

	// start window manager and get an instance
	//_manager = &SDL_WindowManager::Instance();
	_manager = (SDL_WindowManager *) Factory::GetWindowManager();

	_position = ssi_rect(0, 0, 100, 100);
	_position_offset = ssi_rect(0, 0, 0, 0);

	_title[0] = '\0';
}

void Window::create()
{	
    _event.block();

	// send request to create a window
	SDL_Event event;
	event.type = SDL_USEREVENT;
	event.user.code = SDL_WindowManager::EVENTS::CREATE_WINDOW;
	event.user.data1 = this;
	SDL_PushEvent(&event);

	// wait until handle is received
    _event.wait();

	if (_client)
	{
		_client->create(this);
	}
}

void Window::close()
{
    _event.block();

	// send request to close window
	SDL_Event event;
	event.type = SDL_USEREVENT;
	event.user.code = SDL_WindowManager::EVENTS::CLOSE_WINDOW;
	event.user.data1 = this;
	SDL_PushEvent(&event);

	// wait until empty handle is received
    _event.wait();
}

void Window::close_safe() {

	if (_client) {
		_client->close();
	}
}

void Window::setHandle(ssi_handle_t handle) {

	_handle = (SDL_Window*)handle;

	// signal that handle was received
    _event.release();
}

ssi_handle_t Window::getHandle()
{
	return _handle;
}

void Window::hide()
{
	if (_handle)
	{
		SDL_HideWindow(_handle);
	}
}

void Window::show()
{
	if (_handle)
	{
		SDL_ShowWindow(_handle);
	}
}

bool Window::isVisible() 
{

	if (_handle) 
	{
		return SDL_GetWindowFlags(_handle) & SDL_WINDOW_SHOWN ? true : false;
	}

	return false;
}

void Window::update()
{
	if (_handle)
	{
		// send request to update window
		SDL_Event event;
		event.type = SDL_USEREVENT;
		event.user.code = SDL_WindowManager::EVENTS::UPDATE_WINDOW;
		event.user.data1 = this;
		SDL_PushEvent(&event);
	}
}

void Window::update_safe()
{
	if (_handle)
	{
		if (_client)
		{
			_client->update_safe();
		}
	}
}

void Window::setTitle(const ssi_char_t *str)
{
	ssi_strcpy(_title, str);
	
	if (_handle)
	{
		SDL_SetWindowTitle((SDL_Window*)_handle, _title);
	}
}

void Window::setPosition(ssi_rect_t rect)
{
	_position = rect;

	_position.left += _position_offset.left;
	_position.top += _position_offset.top;
	_position.width += _position_offset.width;
	_position.height += _position_offset.height;
	
	if (_handle) 
	{
		SDL_SetWindowPosition(_handle, _position.left, _position.top);
		SDL_SetWindowSize(_handle, _position.width, _position.height);
	}

	if (_client)
	{
		_client->setPosition(rect);
	}
}

bool Window::setPosition(const ssi_char_t* message)
{
    ssi_real_t posf[4] = { 0, 0, 0, 0 };
    ssi_size_t n = ssi_string2array_count(message, ',');
    if (n == 4) {
        ssi_string2array(n, posf, message, ',');
    } else {
        ssi_wrn("could not parse position '%s'", message);
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

void Window::setLimits(ssi_size_t min_width, ssi_size_t max_width, ssi_size_t min_height, ssi_size_t max_height) {

	_min_width = min_width += _position_offset.width;
	_max_width = max_width += _position_offset.width;
	_min_height = min_height += _position_offset.height;
	_max_height = max_height += _position_offset.height;

	if (_handle)
	{
		SDL_SetWindowMinimumSize(_handle, _min_width, _min_height);
		SDL_SetWindowMaximumSize(_handle, _max_width, _max_height);
	}
}

void Window::setIcons(ICON icons) {

	_icons = icons;
}


void Window::setClient(IWindowClient *client)
{	
	_client = client;
}

}

#endif




