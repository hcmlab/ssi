// PaintSomeText.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2007/11/08
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

#ifndef _PAINTRANDOMTEXT_H
#define _PAINTRANDOMTEXT_H

#include "base/ICanvas.h"
#include "graphic/Painter.h"
#include "thread/Lock.h"

namespace ssi {

class PaintSomeText : public ICanvasClient {

public:

	PaintSomeText(ssi_size_t n, const char *f, const char *t)
		: n_font (n) {		
		
		text = new char[strlen(t) + 1];
		strcpy(text, t);		
		font = new IPainter::ITool *[n_font];
		pen = new IPainter::ITool *[n_font];
		brush = new IPainter::ITool *[n_font];
		for (ssi_size_t i = 0; i < n_font; i++) {
			ssi_rgb_t fore_color = ssi_rgb(ssi_random(256u), ssi_random(256u), ssi_random(256u));
			ssi_rgb_t back_color = ssi_rgb(ssi_random(256u), ssi_random(256u), ssi_random(256u));
			ssi_int_t size = 10 + ssi_random(20);
			ssi_int_t style = Painter::Tool::FONT_STYLES::NORMAL;
			if (ssi_random() > 0.5) {
				style |= Painter::Tool::FONT_STYLES::UNDERLINE;
			}
			if (ssi_random() > 0.5) {
				style |= Painter::Tool::FONT_STYLES::STRIKEOUT;
			}
			if (ssi_random() > 0.5) {
				style |= Painter::Tool::FONT_STYLES::ITALIC;
			}
			if (ssi_random() > 0.5) {
				style |= Painter::Tool::FONT_STYLES::BOLD;
			}			
			font[i] = new Painter::Font(f, size, style);
			pen[i] = new Painter::Pen(fore_color);
			if (i % 2) {
				brush[i] = new Painter::Brush(back_color);
			} else {
				brush[i] = new Painter::Brush();
			}
		}
	};

	~PaintSomeText() {

		delete[] text;
		for (ssi_size_t i = 0; i < n_font; i++) {
			delete font[i];
			delete pen[i];
			delete brush[i];
		}
		delete[] font;
		delete[] pen;
		delete[] brush;
	}

	void create(ICanvas *parent) {
	}

	void close() {
	}

	void paint(ssi_handle_t hdc, ssi_rect_t rect) {

		Lock lock(mutex);

		painter.begin(hdc, rect);
		for (ssi_size_t i = 0; i < n_font; i++) {	
			ssi_point_t position;
			position.x = ssi_random(rect.width + 1u);
			position.y = ssi_random(rect.height + 1u);
			painter.text(*font[i], *pen[i], *brush[i], position, text);
		}
		painter.end();
	};

private:

	char *text;
	ssi_size_t n_font;
	Painter::ITool **font;
	Painter::ITool **pen;
	Painter::ITool **brush;	
	Painter painter;
	Mutex mutex;
};

}

#endif


