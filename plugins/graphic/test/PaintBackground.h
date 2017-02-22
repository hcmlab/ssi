// PaintBackground.h
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

#ifndef _PAINTBACKGROUND_H
#define _PAINTBACKGROUND_H

#include "graphic/Painter.h"
#include "base/ICanvas.h"
#include "thread/Lock.h"

namespace ssi {

class PaintBackground : public ICanvasClient {

public:
	
	PaintBackground() {

		_back_brush = new Painter::Brush(ssi_rgb (ssi_random(255u), ssi_random(255u), ssi_random(255u)));					
	}

	~PaintBackground() {

		delete _back_brush;
	}

	void create(ICanvas *parent) {
	}

	void close() {
	}

	void paint(ssi_handle_t hdc, ssi_rect_t rect) {

		Lock lock(mutex);

		painter.begin(hdc, rect);
		painter.fill(*_back_brush, rect);
		painter.end();
	};

protected:

	ssi_size_t n_tools;
	IPainter::ITool *_back_brush;
	Painter painter;	
	Mutex mutex;
};

}

#endif
