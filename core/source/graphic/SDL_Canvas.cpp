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
#include "graphic/SDL_Canvas.h"
#include "graphic/SDL_WindowManager.h"

namespace ssi{
	
Canvas::Canvas() :
	_parent(0),
	_renderer(0),
	_texture(0),
	_width(0),
	_height(0)
{
}

Canvas::~Canvas()
{
}

ssi_handle_t Canvas::getHandle() 
{
	return _texture;
}

void Canvas::create(IWindow *parent)
{
	_parent = parent;	

	std::vector<ICanvasClient *>::iterator iter;
	for (iter = _objects.begin(); iter != _objects.end(); iter++) {
		(*iter)->create(this);
	}
}

void Canvas::close()
{
	if (_renderer) {
		SDL_DestroyRenderer(_renderer);
		_renderer = 0;
	}
	if (_texture) {
		SDL_DestroyTexture(_texture);
		_texture = 0;
	}
}

void Canvas::paint() {	

	if (_parent) {
		_parent->update();
	}
}

void Canvas::paint_safe()
{
	if (_parent) {

		if (!_renderer)
		{
#if _WIN32||_WIN64
            _renderer = SDL_CreateRenderer((SDL_Window*)_parent->getHandle(), -1, SDL_RENDERER_ACCELERATED);
#else
            _renderer = SDL_CreateRenderer( (SDL_Window*)_parent->getHandle(), -1, SDL_RENDERER_SOFTWARE );
#endif
		}
		
		int width, height;
		SDL_GetRendererOutputSize(_renderer, &width, &height);
		if (width == 0 || height == 0)
		{
			return;
		}

		if (width != _width || height != _height) {

			if (_texture) {
				SDL_DestroyTexture(_texture);
			}
			_width = width;
			_height = height;
			_texture = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET, _width, _height);
		}
	
		SDL_SetRenderTarget(_renderer, _texture);

		std::vector<ICanvasClient *>::iterator iter;
		ssi_rect_t rect = ssi_rect(0, 0, _width, _height);
		for (iter = _objects.begin(); iter != _objects.end(); iter++) {
			(*iter)->paint(_renderer, rect);
		}

		SDL_SetRenderTarget(_renderer, 0);
		SDL_RenderCopy(_renderer, _texture, NULL, NULL);
		SDL_RenderPresent(_renderer);
	
	}
}

void Canvas::update_safe()
{
	paint_safe();
}

void Canvas::update()
{
	paint();
}

void Canvas::setPosition(ssi_rect_t rect)
{
	//update();
}

void Canvas::addClient(ICanvasClient *client) {

	_objects.push_back(client);
}

}

#endif
