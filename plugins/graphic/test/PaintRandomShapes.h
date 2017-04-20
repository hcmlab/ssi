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
		Randomi random255(0, 255);
		Randomi random9(0, 9);
		Randomi random4(0, 4);
		Randomf random(0, 0.5f);
		for (ssi_size_t i = 0; i < n_tools; i++) {
			ssi_rgb_t pen_color = ssi_rgb(random255.next(), random255.next(), random255.next());
			ssi_rgb_t brush_color = ssi_rgb(random255.next(), random255.next(), random255.next());
			IPainter::ITool::WIDTH width = 1 + random9.next();
			IPainter::ITool::LINE_STYLES::List style = (IPainter::ITool::LINE_STYLES::List) random4.next();
			pen[i] = new Painter::Pen(pen_color, width, style);
			if (random.next() > 0.5f) {
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

		Randomi randomw(0, rect.width / 2);
		Randomi randomh(0, rect.height / 2);

		for (ssi_size_t i = 0; i < n_tools; i++) {

			r.left = randomw.next();
			r.width = randomw.next();

			r.top = randomh.next();
			r.height = randomh.next();

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
