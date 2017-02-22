// Painter.h
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

#pragma once

#ifndef SSI_GRAPHIC_SDL_PAINTER_H
#define SSI_GRAPHIC_SDL_PAINTER_H

#include "SSI_Define.h"

#ifdef SSI_USE_SDL

#include "base/IPainter.h"

struct SDL_Renderer;
struct SDL_Color;

namespace ssi {

class Painter : public IPainter {

public:

	class Tool : public ITool {

	public:

		~Tool();

		ssi_handle_t getHandle();
		ssi_rgb_t getColor();

	protected:

		ssi_handle_t _handle;
		ssi_rgb_t _color;
	};

	class Pen : public Tool {

	public:

		Pen(ssi_rgb_t color, WIDTH width = 1, LINE_STYLES::List style = LINE_STYLES::SOLID);
	};

	class Brush : public Tool {

	public:
		
		Brush(); // hollow brush
		Brush(ssi_rgb_t color); 		
	};

	class Font : public Tool {

	public:

		Font(const ssi_char_t *name = SSI_DEFAULT_FONT_NAME, WIDTH size = SSI_DEFAULT_FONT_SIZE, FONT_STYLE font_style = FONT_STYLES::NORMAL);

	};

public:

	Painter();
	virtual ~Painter();

	void begin(ssi_handle_t surface, ssi_rect_t area);

	void setArea(ssi_rect_t area);
	ssi_rect_t getArea();

	void setBackground(ssi_rgb_t color);
	void blank();
	
	void pixel(ITool &pen, ssi_point_t point);
	void pixel(ITool &pen, ssi_pointf_t point, bool relative);

	void fill(ITool &brush, ssi_rect_t rect);
	void fill(ITool &brush, ssi_rectf_t rect, bool relative);

	void rect(ITool &pen, ssi_rect_t rect);
	void rect(ITool &pen, ssi_rectf_t rect, bool relative);

	void rect(ITool &pen, ITool &brush, ssi_rect_t rect);
	void rect(ITool &pen, ITool &brush, ssi_rectf_t rect, bool relative);

	void line(ITool &pen, ssi_point_t from, ssi_point_t to);
	void line(ITool &pen, ssi_pointf_t from, ssi_pointf_t to, bool relative);

	void ellipse(ITool &pen, ssi_rect_t rect);
	void ellipse(ITool &pen, ssi_rectf_t rect, bool relative);

	void circle(ITool &pen, ssi_point_t center, int radius);
	void circle(ITool &pen, ssi_pointf_t center, ssi_real_t radius, bool relative);

	void ellipse(ITool &pen, ITool &brush, ssi_rect_t rect);
	void ellipse(ITool &pen, ITool &brush, ssi_rectf_t rect, bool relative);
	
	void circle(ITool &pen, ITool &brush, ssi_point_t center, int radius);
	void circle(ITool &pen, ITool &brush, ssi_pointf_t center, ssi_real_t radius, bool relative);

	void text(ITool &font, ITool &pen, ssi_point_t position, const ssi_char_t *text, TEXT_ALIGN_HORZ::List align_horz = TEXT_ALIGN_HORZ::LEFT, TEXT_ALIGN_VERT::List align_vert = TEXT_ALIGN_VERT::BOTTOM);
	void text(ITool &font, ITool &pen, ssi_pointf_t position, const ssi_char_t *text, bool relative, TEXT_ALIGN_HORZ::List align_horz = TEXT_ALIGN_HORZ::LEFT, TEXT_ALIGN_VERT::List align_vert = TEXT_ALIGN_VERT::BOTTOM);

	void text(ITool &font, ITool &pen, ITool &brush, ssi_point_t position, const ssi_char_t *text, TEXT_ALIGN_HORZ::List align_horz = TEXT_ALIGN_HORZ::LEFT, TEXT_ALIGN_VERT::List align_vert = TEXT_ALIGN_VERT::BOTTOM);
	void text(ITool &font, ITool &pen, ITool &brush, ssi_pointf_t position, const ssi_char_t *text, bool relative, TEXT_ALIGN_HORZ::List align_horz = TEXT_ALIGN_HORZ::LEFT, TEXT_ALIGN_VERT::List align_vert = TEXT_ALIGN_VERT::BOTTOM);

	void image(ssi_video_params_t params, ssi_byte_t *buffer, bool scale);

	void end();

protected:

	static ssi_char_t *ssi_log_name;

	ssi_rect_t rectf2rect(ssi_rectf_t rect);
	ssi_rect_t rectf2rect(ssi_rectf_t rect, ssi_rect_t area);
	ssi_point_t pointf2point(ssi_pointf_t point);
	ssi_point_t pointf2point(ssi_pointf_t point, ssi_rect_t area);

	void setColor(ssi_rgb_t color);
	SDL_Color getColor(ssi_rgb_t color);

	SDL_Renderer *_device;
	ssi_rect_t _area;

	IPainter::ITool *_back_pen;
	IPainter::ITool *_back_brush;
};

}

#endif

#endif