// PaintRandomLines.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2015/03/30
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

#ifndef _MYCANVASCLIENT_H
#define _MYCANVASCLIENT_H

#include "base/ICanvas.h"
#include "graphic/GraphicTools.h"
#include "graphic/Painter.h"
#include "graphic/Colormap.h"

namespace ssi {

class MyCanvasClient : public ICanvasClient {

public:

	struct TYPE {
		enum List {
			SIGNAL = 0,
			PATH,
			SCATTER,
			IMAGE
		};
	};

	MyCanvasClient(ssi_stream_t stream, TYPE::List type) 
	: _pen(IPainter::ITool::COLORS::WHITE),
	_brush(IPainter::ITool::COLORS::BLACK), 
	_font(0),
	_colormap(IColormap::COLORMAP::BLACKANDWHITE) {
		_type = type;
		ssi_stream_clone(stream, _stream);
		_mins = new ssi_real_t[_stream.dim];
		_maxs = new ssi_real_t[_stream.dim];
		ssi_minmax(_stream.num, _stream.dim, ssi_pcast (ssi_real_t, _stream.ptr), _mins, _maxs);
		ssi_norm(_stream.num, _stream.dim, ssi_pcast(ssi_real_t, _stream.ptr));
	}

	~MyCanvasClient() {
		ssi_stream_destroy(_stream);
	}

	void create(ICanvas *parent) {
		_font = new Painter::Font();
	}

	void close() {
		delete _font;
	}

	void paint(ssi_handle_t context, ssi_rect_t area) {
		_painter.begin(context, area);
				
		_painter.fill(_brush, area);

		switch (_type) {
		case TYPE::SIGNAL:
			GraphicTools::PaintAsSignal(_stream, _painter, _pen);
			GraphicTools::PaintAxis(_stream.dim, 0, _stream.num/_stream.sr, _mins, _maxs, _painter, *_font, _pen, _brush, 2);
			break;
		case TYPE::PATH:
			GraphicTools::PaintAsPath(_stream, 0, 1, _painter, _pen);			
			break;
		case TYPE::SCATTER:
			GraphicTools::PaintAsScatter(_stream, 0, 1, _painter, _pen, _brush, 10.0f);
			break;
		case TYPE::IMAGE:
			GraphicTools::PaintAsImage(_stream, _painter, _colormap);
			GraphicTools::PaintAxis(1, 0, _stream.num / _stream.sr, _mins, _maxs, _painter, *_font, _pen, _brush, 2);
			break;
		}		
		_painter.end();
	}

protected:

	ssi_stream_t _stream;
	ssi_real_t *_mins, *_maxs;
	Painter _painter;
	TYPE::List _type;
	Painter::Pen _pen;
	Painter::Brush _brush;
	Painter::Font *_font;
	Colormap _colormap;
};

}

#endif
