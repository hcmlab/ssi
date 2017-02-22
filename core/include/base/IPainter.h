// IPainter.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2015/01/06
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

#ifndef SSI_IPAINTER_H
#define SSI_IPAINTER_H

#include "SSI_Cons.h"

namespace ssi {

class IPainter {

public:

	class ITool {

	public:

		struct COLORS {
			enum List : ssi_rgb_t {
				WHITE = ssi_rgb(255, 255, 255),
				BLACK = ssi_rgb(0, 0, 0),
				RED = ssi_rgb(255, 0, 0),
				GREEN = ssi_rgb(0, 255, 0),
				BLUE = ssi_rgb(0, 0, 255)
			};
		};

		typedef ssi_int_t WIDTH;

		struct LINE_STYLES {
			enum List {
				SOLID,
				DASH,
				DOT,
				DASHDOT,
				DASTDOTDOT
			};
		};

		typedef ssi_size_t FONT_STYLE;
		struct FONT_STYLES {
			enum List : ssi_size_t {
				NORMAL = 0,
				ITALIC = 1 << 1,
				BOLD = 1 << 2,
				UNDERLINE = 1 << 3,
				STRIKEOUT = 1 << 4
			};
		};

	public:

		virtual ~ITool() {}
		virtual ssi_handle_t getHandle() = 0;		
		virtual ssi_rgb_t getColor() = 0;		
	};

public:

	struct TEXT_ALIGN_HORZ {
		enum List {
			LEFT = 0,
			RIGHT,
			CENTER
		};
	};

	struct TEXT_ALIGN_VERT {
		enum List {
			BOTTOM = 0,
			TOP,
			CENTER
		};
	};

	virtual ~IPainter() {};

	virtual void begin(ssi_handle_t device, ssi_rect_t area) = 0;	
	virtual void setBackground(ssi_rgb_t color) = 0;
	virtual void blank() = 0;
	virtual void setArea(ssi_rect_t area) = 0;
	virtual ssi_rect_t getArea() = 0;
	virtual void pixel(ITool &pen, ssi_point_t point) = 0;
	virtual void pixel(ITool &pen, ssi_pointf_t point, bool relative) = 0;	
	virtual void fill(ITool &brush, ssi_rect_t rect) = 0;
	virtual void fill(ITool &brush, ssi_rectf_t rect, bool relative) = 0;
	virtual void rect(ITool &pen, ssi_rect_t rect) = 0;
	virtual void rect(ITool &pen, ssi_rectf_t rect, bool relative) = 0;
	virtual void rect(ITool &pen, ITool &brush, ssi_rect_t rect) = 0;
	virtual void rect(ITool &pen, ITool &brush, ssi_rectf_t rect, bool relative) = 0;
	virtual void line(ITool &pen, ssi_point_t from, ssi_point_t to) = 0;
	virtual void line(ITool &pen, ssi_pointf_t from, ssi_pointf_t to, bool relative) = 0;
	virtual void ellipse(ITool &pen, ssi_rect_t rect) = 0;
	virtual void ellipse(ITool &pen,ssi_rectf_t rect, bool relative) = 0;
	virtual void circle(ITool &pen, ssi_point_t center, int radius) = 0;
	virtual void circle(ITool &pen, ssi_pointf_t center, ssi_real_t radius, bool relative) = 0;
	virtual void ellipse(ITool &pen, ITool &brush, ssi_rect_t rect) = 0; 
	virtual void ellipse(ITool &pen, ITool &brush, ssi_rectf_t rect, bool relative) = 0;
	virtual void circle(ITool &pen, ITool &brush, ssi_point_t center, int radius) = 0;
	virtual void circle(ITool &pen, ITool &brush, ssi_pointf_t center, ssi_real_t radius, bool relative) = 0;
	virtual void text(ITool &font, ITool &pen, ssi_point_t position, const ssi_char_t *text, TEXT_ALIGN_HORZ::List align_horz = TEXT_ALIGN_HORZ::LEFT, TEXT_ALIGN_VERT::List align_vert = TEXT_ALIGN_VERT::BOTTOM) = 0;
	virtual void text(ITool &font, ITool &pen, ssi_pointf_t position, const ssi_char_t *text, bool relative, TEXT_ALIGN_HORZ::List align_horz = TEXT_ALIGN_HORZ::LEFT, TEXT_ALIGN_VERT::List align_vert = TEXT_ALIGN_VERT::BOTTOM) = 0;
	virtual void text(ITool &font, ITool &pen, ITool &brush, ssi_point_t position, const ssi_char_t *text, TEXT_ALIGN_HORZ::List align_horz = TEXT_ALIGN_HORZ::LEFT, TEXT_ALIGN_VERT::List align_vert = TEXT_ALIGN_VERT::BOTTOM) = 0;
	virtual void text(ITool &font, ITool &pen, ITool &brush, ssi_pointf_t position, const ssi_char_t *text, bool relative, TEXT_ALIGN_HORZ::List align_horz = TEXT_ALIGN_HORZ::LEFT, TEXT_ALIGN_VERT::List align_vert = TEXT_ALIGN_VERT::BOTTOM) = 0;
	virtual void image(ssi_video_params_t params, ssi_byte_t *buffer, bool scale) = 0;
	
	virtual void end() = 0;

};

}

#endif
