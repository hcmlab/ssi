// PaintRandomPoints.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2007/11/06
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

#ifndef _PAINTRANDOMPOINTS_H
#define _PAINTRANDOMPOINTS_H

#include "graphic/Painter.h"
#include "base/ICanvas.h"
#include "thread/Lock.h"

namespace ssi {

class PaintRandomPoints : public ICanvasClient {

public:

	PaintRandomPoints(int n) : n_tools(n) {

		tools = new IPainter::ITool *[n_tools];
		for (ssi_size_t i = 0; i < n_tools; i++) {
			ssi_rgb_t color = ssi_rgb(ssi_random(256u), ssi_random(256u), ssi_random(256u));
			tools[i] = new Painter::Pen(color);
		}
	}

	~PaintRandomPoints() {
		for (ssi_size_t i = 0; i < n_tools; i++) {
			delete tools[i];
		}
		delete[] tools;
	}

	void create(ICanvas *parent) {
	}

	void close() {
	}

	void paint(ssi_handle_t hdc, ssi_rect_t rect) {

		Lock lock(mutex);

		ssi_point_t point;

		painter.begin(hdc, rect);
		for (ssi_size_t i = 0; i < n_tools; i++)
		{
			point.x = ssi_random(rect.width + 1u);
			point.y = ssi_random(rect.height + 1u);
			painter.pixel(*tools[i], point);
		}
		painter.end();
	};

private:

	ssi_size_t n_tools;
	IPainter::ITool **tools;
	Painter painter;
	Mutex mutex;
};

}

#endif
