// PaintRandomShapes.h
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

#ifndef _PAINTRANDOMSHAPES_H
#define _PAINTRANDOMSHAPES_H

#include "graphic/Painter.h"
#include "base/ICanvas.h"
#include "thread/Lock.h"

namespace ssi {

class PaintRandomShapes : public ICanvasClient {

public:

	PaintRandomShapes(int n) : n_tools(n) {

		pen = new IPainter::ITool *[n_tools];
		brush = new IPainter::ITool *[n_tools];
		for (ssi_size_t i = 0; i < n_tools; i++) {
			ssi_rgb_t pen_color = ssi_rgb(ssi_random(256u), ssi_random(256u), ssi_random(256u));
			ssi_rgb_t brush_color = ssi_rgb(ssi_random(256u), ssi_random(256u), ssi_random(256u));
			IPainter::ITool::WIDTH width = 1 + ssi_random(9u);
			IPainter::ITool::LINE_STYLES::List style = (IPainter::ITool::LINE_STYLES::List) ssi_random(4u);
			pen[i] = new Painter::Pen(pen_color, width, style);
			if (ssi_random() > 0.5) {				
				brush[i] = new Painter::Brush(brush_color);
			}
			else {
				brush[i] = new Painter::Brush();
			}
		}
	};

	~PaintRandomShapes() {

		for (ssi_size_t i = 0; i < n_tools; i++) {
			delete pen[i];
			delete brush[i];
		}
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
		ssi_rect_t r;

		for (ssi_size_t i = 0; i < n_tools; i++) {

			r.left = ssi_random(rect.width / 2);
			r.width = ssi_random(rect.width / 2);

			r.top = ssi_random(rect.height / 2);
			r.height = ssi_random(rect.height / 2);

			if (i % 2) {
				painter.rect(*pen[i], *brush[i], r);
			} else {
				painter.ellipse(*pen[i], *brush[i], r);
			}
		}

		painter.end();
	};

private:

	ssi_size_t n_tools;
	IPainter::ITool **pen;
	IPainter::ITool **brush;
	Painter painter;
	Mutex mutex;
};

}

#endif
