// Monitor.cpp
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

#include "SSI_Define.h"

#ifdef SSI_USE_SDL

#include "SDL/include/ssisdl.h"
#include "graphic/SDL_Monitor.h"
#include "graphic/SDL_Painter.h"
#include "base/Factory.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif


namespace ssi {

Monitor::Monitor (ssi_size_t maxchar)
	: _painter(0),
	_parent(0),
	_renderer (0),
	_texture (0),
	_pen(0),
	_brush(0),
	_font (0),
	_font_name(0),
	_font_size(SSI_DEFAULT_FONT_SIZE),
	_width(0),
	_height(0) {

	_n_buffer = maxchar + 1;
	_buffer = new ssi_char_t[_n_buffer];
	_buffer_safe = new ssi_char_t[_n_buffer];
	_buffer_safe[0] = '\0';

	clear();
}

Monitor::~Monitor () {
	
	delete[] _buffer; _buffer = 0;
}

void Monitor::create(IWindow *parent) {

	_parent = parent;

	_painter = new Painter();
	_font = new Painter::Font(_font_name, _font_size);
	_brush = new Painter::Brush(IPainter::ITool::COLORS::WHITE);
	_pen = new Painter::Pen(IPainter::ITool::COLORS::BLACK);

	if (_renderer) {
		SDL_DestroyRenderer(_renderer);
		_renderer = 0;
	}
	if (_texture) {
		SDL_DestroyTexture(_texture);
		_texture = 0;
	}
}

void Monitor::close() {

	_parent = 0;

	delete _font; _font = 0;
	delete _brush; _brush = 0;
	delete _pen; _pen = 0;
	delete _painter; _painter = 0;
}

void Monitor::setFont(const ssi_char_t *name, ssi_size_t size) {

	delete _font_name;
	if (name && name[0] != '\0') 
	{
		_font_name = ssi_strcpy(name);
	}
	else 
	{
		_font_name = 0;
	}
	_font_size = size;

	if (_parent) {
		Lock lock(_mutex);
		delete _font;
		_font = new Painter::Font(_font_name, _font_size);
	}
}

void Monitor::clear() {

	_buffer[0] = '\0';
	_buffer_count = 1;
}

ssi_handle_t Monitor::getHandle() {
	return _renderer;
}

void Monitor::print(const ssi_char_t *str) {

	ssi_size_t len = ssi_cast(ssi_size_t, strlen(str));
	if (_buffer_count + len >= _n_buffer - 1) {
		//ssi_wrn ("max buffer size reached '%u', crop string", _n_buffer);
		len = _n_buffer - _buffer_count - 1;
	}
	if (len > 0) {
		memcpy(_buffer + _buffer_count - 1, str, len);
		_buffer_count += len;
		_buffer[_buffer_count - 1] = '\0';
	}
}

void Monitor::setPosition(ssi_rect_t rect) {

	//update();
}

void Monitor::update() {

	if (_parent) {

		{
			Lock lock(_mutex);
			ssi_strcpy(_buffer_safe, _buffer);
		}

		_parent->update();
	}
}

void Monitor::update_safe ()
{
	if (_parent) {

		if (!_renderer)
		{
			_renderer = SDL_CreateRenderer((SDL_Window*)_parent->getHandle(), -1, SDL_RENDERER_ACCELERATED);
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
		SDL_SetRenderDrawColor(_renderer, (Uint8)_brush->getColor(), (Uint8)(_brush->getColor() >> 8), (Uint8)(_brush->getColor() >> 16), 0);
		SDL_RenderClear(_renderer);

		ssi_rect_t rect = ssi_rect(0, 0, width, height);
		_painter->begin(_renderer, rect);
		{
			Lock lock(_mutex);
			_painter->text(*_font, *_pen, ssi_point(0, 0), _buffer_safe, IPainter::TEXT_ALIGN_HORZ::LEFT, IPainter::TEXT_ALIGN_VERT::TOP);
		}
		_painter->end();

		SDL_SetRenderTarget(_renderer, 0);
		SDL_RenderCopy(_renderer, _texture, NULL, NULL);
		SDL_RenderPresent(_renderer);
	}
}

}

#endif