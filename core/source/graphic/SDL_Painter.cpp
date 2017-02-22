// SDL_Painter.cpp
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

#include "graphic/SDL_Painter.h"
#include "SDL/include/ssisdl.h"

namespace ssi {

ssi_char_t *Painter::ssi_log_name = "painter___";

Painter::Painter()
	: _device(0) {

	_area.height = _area.left = _area.top = _area.width = 0;

	_back_pen = new Painter::Pen(IPainter::ITool::COLORS::BLACK);
	_back_brush = new Painter::Pen(IPainter::ITool::COLORS::BLACK);	
}

Painter::~Painter() {

	if (_device) {
		end();
	}

	delete _back_pen;
	delete _back_brush;
};

Painter::Pen::Pen(ssi_rgb_t color, WIDTH width, LINE_STYLES::List style) {
	_handle = 0;
	_color = color;
}

Painter::Brush::Brush() {
	_handle = 0;
	_color = -1;
}

Painter::Brush::Brush(ssi_rgb_t color) {
	_handle = 0;
	_color = color;
}

Painter::Font::Font(const ssi_char_t *name, WIDTH size, FONT_STYLE font_style) {
	
	_handle = 0;
	_color = -1;

	TTF_Font *font = TTF_OpenFont(name ? name : SSI_DEFAULT_FONT_NAME, size);	

	if (font)
	{
		int style = TTF_STYLE_NORMAL;
		if (font_style & IPainter::ITool::FONT_STYLES::BOLD)
		{
			style |= TTF_STYLE_BOLD;
		}
		if (font_style & IPainter::ITool::FONT_STYLES::ITALIC)
		{
			style |= TTF_STYLE_ITALIC;
		}
		if (font_style & IPainter::ITool::FONT_STYLES::UNDERLINE)
		{
			style |= TTF_STYLE_UNDERLINE;
		}
		if (font_style & IPainter::ITool::FONT_STYLES::STRIKEOUT)
		{
			style |= TTF_STYLE_STRIKETHROUGH;
		}
		TTF_SetFontStyle(font, style);
		_handle = font;
	}

	if (!_handle) {
		ssi_wrn("could not create font '%s'", name);
	}

}

Painter::Tool::~Tool() {
	if (_handle) 
	{
		TTF_CloseFont((TTF_Font*)_handle); _handle = 0;
	}
}

ssi_handle_t Painter::Tool::getHandle() {
	return _handle;
}

ssi_rgb_t Painter::Tool::getColor() {
	return _color;
}

ssi_rect_t Painter::rectf2rect(ssi_rectf_t rect) {

	ssi_rect_t r;
	r.left = ssi_cast(ssi_int_t, rect.left + 0.5f);
	r.top = ssi_cast(ssi_int_t, rect.top + 0.5f);
	r.width = ssi_cast(ssi_int_t, rect.width + 0.5f);
	r.height = ssi_cast(ssi_int_t, rect.height + 0.5f);
	return r;
}

ssi_rect_t Painter::rectf2rect(ssi_rectf_t rect, ssi_rect_t area) {

	ssi_rect_t r;
	r.left = ssi_cast(ssi_int_t, rect.left * area.width + 0.5f);
	r.top = ssi_cast(ssi_int_t, rect.top * area.height + 0.5f);
	r.width = ssi_cast(ssi_int_t, rect.width * area.width + 0.5f);
	r.height = ssi_cast(ssi_int_t, rect.height * area.height + 0.5f);
	return r;
}

ssi_point_t Painter::pointf2point(ssi_pointf_t point) {

	ssi_point_t p;
	p.x = ssi_cast(ssi_int_t, point.x + 0.5f);
	p.y = ssi_cast(ssi_int_t, point.y + 0.5f);
	return p;
}

ssi_point_t Painter::pointf2point(ssi_pointf_t point, ssi_rect_t area) {

	ssi_point_t p;
	p.x = ssi_cast(ssi_int_t, point.x * area.width + 0.5f);
	p.y = ssi_cast(ssi_int_t, point.y * area.height + 0.5f);
	return p;
}

void Painter::begin(ssi_handle_t device, ssi_rect_t area) {

	if (_device) {
		end();
	}

	_device = (SDL_Renderer*)device;
	_area = area;
}

ssi_rect_t Painter::getArea() {
	return _area;
}

void Painter::setArea(ssi_rect_t area) {
	_area = area;	
}

SDL_Color Painter::getColor(ssi_rgb_t color) {
	
	SDL_Color c;
	
	c.a = 0;
	c.r = (Uint8)color;
	c.g = (Uint8)(color >> 8);
	c.b = (Uint8)(color >> 16);

	return c;
}

void Painter::setColor(ssi_rgb_t color) {

	Uint8 r = (Uint8)color;
	Uint8 g = (Uint8)(color >> 8);
	Uint8 b = (Uint8)(color >> 16);
	
	SDL_SetRenderDrawColor(_device, r, g, b, 0);
}

void Painter::setBackground(ssi_rgb_t color) {

	delete _back_pen;
	delete _back_brush;
	_back_pen = new Painter::Pen(color);
	_back_brush = new Painter::Pen(color);
}

void Painter::blank() {

	rect(*_back_pen, *_back_brush, _area);
}

void Painter::pixel(ITool &pen, ssi_pointf_t point, bool relative) {

	if (relative) {
		Painter::pixel(pen, pointf2point(point, _area));
	} else {
		Painter::pixel(pen, pointf2point(point));
	}
}

void Painter::pixel(ITool &pen, ssi_point_t point) {
	
	setColor(pen.getColor());	
	SDL_RenderDrawPoint(_device, point.x, point.y);
}

void Painter::fill(ITool &brush, ssi_rectf_t rect, bool relative) {

	if (relative) {
		Painter::fill(brush, rectf2rect(rect, _area));
	}
	else {
		Painter::fill(brush, rectf2rect(rect));
	}
}

void Painter::fill(ITool &brush, ssi_rect_t rect) {

	setColor(brush.getColor());

	if (rect.width == 1 && rect.height == 1) {
		
		SDL_RenderDrawPoint(_device, _area.left + rect.left, _area.top + rect.top);

	} else {

		SDL_Rect r;
		r.x = _area.left + rect.left;
		r.y = _area.top + rect.top;
		r.w = rect.width;
		r.h = rect.height;

		SDL_RenderFillRect(_device, &r);
	}
}

void Painter::rect(ITool &pen, ssi_rectf_t rect, bool relative) {

	if (relative) {
		Painter::rect(pen, rectf2rect(rect, _area));
	}
	else {
		Painter::rect(pen, rectf2rect(rect));
	}
}

void Painter::rect(ITool &pen, ssi_rect_t rect) {

	if (rect.width == 1 && rect.height == 1) {

		setColor(pen.getColor());
		SDL_RenderDrawPoint(_device, _area.left + rect.left, _area.top + rect.top);

	}
	else {

		SDL_Rect r;
		r.x = _area.left + rect.left;
		r.y = _area.top + rect.top;
		r.w = rect.width;
		r.h = rect.height;

		setColor(pen.getColor());
		SDL_RenderDrawRect(_device, &r);
	}
}

void Painter::rect(ITool &pen, ITool &brush, ssi_rectf_t rect, bool relative) {

	if (relative) {
		Painter::rect(pen, brush, rectf2rect(rect, _area));
	} else {
		Painter::rect(pen, brush, rectf2rect(rect));
	}
}

void Painter::rect(ITool &pen, ITool &brush, ssi_rect_t rect) {
	
	if (rect.width == 1 && rect.height == 1) {

		setColor(pen.getColor());
		SDL_RenderDrawPoint(_device, _area.left + rect.left, _area.top + rect.top);

	} else {

		SDL_Rect r;
		r.x = _area.left + rect.left;
		r.y = _area.top + rect.top;
		r.w = rect.width;
		r.h = rect.height;

		setColor(brush.getColor());
		SDL_RenderFillRect(_device, &r);
		setColor(pen.getColor());
		SDL_RenderDrawRect(_device, &r);
	}
}

void Painter::line(ITool &pen, ssi_pointf_t from, ssi_pointf_t to, bool relative) {

	if (relative) {
		Painter::line(pen, pointf2point(from, _area), pointf2point(to, _area));
	} else {
		Painter::line(pen, pointf2point(from), pointf2point(to));
	}
}

void Painter::line(ITool &pen, ssi_point_t from, ssi_point_t to) {

	setColor(pen.getColor());
	SDL_RenderDrawLine(_device, _area.left + from.x, _area.top + from.y, _area.left + to.x, _area.top + to.y);
}

void Painter::ellipse(ITool &pen, ssi_rectf_t rect, bool relative) {

	if (relative) {
		Painter::ellipse(pen, rectf2rect(rect, _area));
	}
	else {
		Painter::ellipse(pen, rectf2rect(rect));
	}
}

void Painter::ellipse(ITool &pen, ssi_rect_t rect) {

	this->rect(pen, rect);
}

void Painter::circle(ITool &pen, ssi_point_t center, int radius) {

	ssi_rect_t rect;
	rect.left = center.x - radius;
	rect.top = center.y - radius;
	rect.height = 2 * radius;
	rect.width = 2 * radius;
	ellipse(pen, rect);
}

void Painter::circle(ITool &pen, ssi_pointf_t center, ssi_real_t radius, bool relative) {

	ssi_rectf_t rect;
	rect.left = center.x - radius;
	rect.top = center.y - radius;
	rect.height = 2 * radius;
	rect.width = 2 * radius;
	ellipse(pen, rect, relative);
}

void Painter::ellipse(ITool &pen, ITool &brush, ssi_rectf_t rect, bool relative) {

	if (relative) {
		Painter::ellipse(pen, brush, rectf2rect(rect, _area));
	}
	else {
		Painter::ellipse(pen, brush, rectf2rect(rect));
	}
}

void Painter::ellipse(ITool &pen, ITool &brush, ssi_rect_t rect) {

	this->rect(pen, brush, rect);
}

void Painter::circle(ITool &pen, ITool &brush, ssi_point_t center, int radius) {

	ssi_rect_t rect;
	rect.left = center.x - radius;
	rect.top = center.y - radius;
	rect.height = 2 * radius;
	rect.width = 2 * radius;
	ellipse (pen, brush, rect);
}

void Painter::circle(ITool &pen, ITool &brush, ssi_pointf_t center, ssi_real_t radius, bool relative) {
	
	ssi_rectf_t rect;
	rect.left = center.x - radius;
	rect.top = center.y - radius;
	rect.height = 2 * radius;
	rect.width = 2 * radius;
	ellipse(pen, brush, rect, relative);
}

void Painter::text(ITool &font, ITool &pen, ssi_pointf_t position, const ssi_char_t *text, bool relative, TEXT_ALIGN_HORZ::List align_horz, TEXT_ALIGN_VERT::List align_vert) {

	if (relative) {
		Painter::text(font, pen, pointf2point(position, _area), text, align_horz, align_vert);
	} else {
		Painter::text(font, pen, pointf2point(position), text, align_horz, align_vert);
	}
}

void Painter::text(ITool &font, ITool &pen, ssi_point_t position, const ssi_char_t *text, TEXT_ALIGN_HORZ::List align_horz, TEXT_ALIGN_VERT::List align_vert) {

	if (!text || text[0] == '\0') {
		return;
	}

	if (font.getHandle())
	{
		SDL_Color fcolor = getColor(pen.getColor());
		SDL_Surface *surface = TTF_RenderText_Blended_Wrapped((TTF_Font *)font.getHandle(), text, fcolor, _area.width - position.x);
		if (surface)
		{
			SDL_Texture *texture = SDL_CreateTextureFromSurface(_device, surface);
			SDL_Rect rect;
			switch (align_horz) {
			case TEXT_ALIGN_HORZ::LEFT:
				rect.x = position.x;
				break;
			case TEXT_ALIGN_HORZ::RIGHT:
				rect.x = position.x - surface->w;
				break;
			case TEXT_ALIGN_HORZ::CENTER:
				rect.x = position.x - surface->w / 2;
				break;
			}
			switch (align_vert) {
			case TEXT_ALIGN_VERT::BOTTOM:
				rect.y = position.y - surface->h;
				break;
			case TEXT_ALIGN_VERT::TOP:
				rect.y = position.y;
				break;
			case TEXT_ALIGN_VERT::CENTER:
				rect.y = position.y - surface->h / 2;
				break;
			}
			rect.w = surface->w;
			rect.h = surface->h;
			SDL_RenderCopy(_device, texture, 0, &rect);
			SDL_FreeSurface(surface);
			SDL_DestroyTexture(texture);
		}
		else
		{
			ssi_wrn("could not create surface '%s' (%s)", SDL_GetError(), text);
		}
	}
}

void Painter::text(ITool &font, ITool &pen, ITool &brush, ssi_pointf_t position, const ssi_char_t *text, bool relative, TEXT_ALIGN_HORZ::List align_horz, TEXT_ALIGN_VERT::List align_vert) {

	if (relative) {
		Painter::text(font, pen, brush, pointf2point(position, _area), text, align_horz, align_vert);
	}
	else {
		Painter::text(font, pen, brush, pointf2point(position), text, align_horz, align_vert);
	}
}


void Painter::text(ITool &font, ITool &pen, ITool &brush, ssi_point_t position, const ssi_char_t *text, TEXT_ALIGN_HORZ::List align_horz, TEXT_ALIGN_VERT::List align_vert) {

	if (!text || text[0] == '\0') {
		return;
	}

	if (font.getHandle()) 
	{
		SDL_Color fcolor = getColor(pen.getColor());
		SDL_Color bcolor = getColor(brush.getColor());
		SDL_Surface *surface = TTF_RenderText_Shaded ((TTF_Font *)font.getHandle(), text, fcolor, bcolor);
		if (surface)
		{
			SDL_Texture *texture = SDL_CreateTextureFromSurface(_device, surface);
			SDL_Rect rect;
			switch (align_horz) {
			case TEXT_ALIGN_HORZ::LEFT:
				rect.x = position.x;
				break;
			case TEXT_ALIGN_HORZ::RIGHT:
				rect.x = position.x - surface->w;
				break;
			case TEXT_ALIGN_HORZ::CENTER:
				rect.x = position.x - surface->w / 2;
				break;
			}
			switch (align_vert) {
			case TEXT_ALIGN_VERT::BOTTOM:
				rect.y = position.y - surface->h;
				break;
			case TEXT_ALIGN_VERT::TOP:
				rect.y = position.y;
				break;
			case TEXT_ALIGN_VERT::CENTER:
				rect.y = position.y - surface->h / 2;
				break;
			}			
			rect.w = surface->w;
			rect.h = surface->h;
			SDL_RenderCopy(_device, texture, 0, &rect);
			SDL_FreeSurface(surface);
			SDL_DestroyTexture(texture);
		}
		else
		{
			ssi_wrn("could not create surface '%s'", SDL_GetError());
		}
	}
}

void Painter::image(ssi_video_params_t params, ssi_byte_t *buffer, bool scale) {

	int stride = ssi_video_stride(params);

	SDL_Surface *surface = SDL_CreateRGBSurfaceFrom(buffer,
		params.widthInPixels,
		params.heightInPixels,
		params.depthInBitsPerChannel * params.numOfChannels,
		stride,
		0,
		0,
		0,
		0);

	SDL_Texture *texture = SDL_CreateTextureFromSurface(_device, surface);
	SDL_Rect rect;
	rect.x = _area.left;
	rect.y = _area.top;
	if (scale) 
	{
		rect.w = _area.width;
		rect.h = _area.height;		
	}
	else
	{
		rect.w = params.widthInPixels;
		rect.h = params.heightInPixels;
	}
	SDL_RenderCopy(_device, texture, 0, &rect);
	SDL_FreeSurface(surface);
	SDL_DestroyTexture(texture);
}

void Painter::end() {

	_device = 0;
	_area.height = _area.left = _area.top = _area.width = 0;
}

}

#endif