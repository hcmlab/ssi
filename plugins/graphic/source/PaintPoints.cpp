// PaintPoints.cpp
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

#include "PaintPoints.h"
#include "graphic/Painter.h"
#include "graphic/Colormap.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

const ssi_char_t *PaintPoints::ssi_log_name = "pointpaint";

PaintPoints::PaintPoints(PaintPointsType::TYPE type,
	bool relative,
	bool swap,
	bool draw_labels,
	bool draw_background)
	: _painter(0),
	_parent(0),
	_n_points(0),
	_points(0),
	_pen(0),
	_brush(0),
	_font(0),
	_font_pen(0),
	_font_brush(0),
	_back_brush(0),
	_type(type),
	_precision(1),
	_swap(swap),
	_relative(relative),
	_draw_points(false),
	_draw_back(draw_background),
	_draw_labels(draw_labels) {
}

PaintPoints::~PaintPoints() {

	delete[] _points;
}

void PaintPoints::create(ICanvas *canvas) {

	_parent = canvas;

	_painter = new Painter();
	setPen(IPainter::ITool::COLORS::GREEN, 1, IPainter::ITool::LINE_STYLES::SOLID);
	setBrush(IPainter::ITool::COLORS::BLACK);
	setBackground(IPainter::ITool::COLORS::BLACK);
	setFont(SSI_DEFAULT_FONT_NAME, SSI_DEFAULT_FONT_COLOR_FORE, SSI_DEFAULT_FONT_COLOR_BACK, SSI_DEFAULT_FONT_SIZE, IPainter::ITool::FONT_STYLES::NORMAL);
	setPointSize(5);
}

void PaintPoints::close() {

	delete _pen; _pen = 0;
	delete _brush; _brush = 0;
	delete _font; _font = 0;
	delete _font_pen; _font_pen = 0;
	delete _font_brush; _font_brush = 0;
	delete _painter; _painter = 0;
}

void PaintPoints::clear() {
	_draw_points = false;
}

void PaintPoints::setData(ssi_size_t n_points, ssi_real_t *points, ssi_time_t time) {

	Lock lock(_mutex);

	if (n_points != _n_points) {
		_n_points = n_points;
		delete[] _points;
		_points = new ssi_pointf_t[_n_points];
	}

	_draw_points = true;

	ssi_real_t *ptr = points;
	for (ssi_size_t i = 0; i < _n_points; i++) {
		_points[i].x = *ptr++;
		_points[i].y = *ptr++;
	}

}

void PaintPoints::setData(ssi_stream_t &stream, ssi_time_t time) {

	Lock lock(_mutex);

	if (stream.type != SSI_REAL) {
		ssi_wrn("type '%s' not supported", SSI_TYPE_NAMES[stream.type]);
		return;
	}

	ssi_size_t n_points = stream.dim / 2;
	if (n_points != _n_points) {
		_n_points = n_points;
		delete[] _points;
		_points = new ssi_pointf_t[_n_points];
	}

	_draw_points = true;

	ssi_real_t *ptr = ssi_pcast(ssi_real_t, stream.ptr);
	for (ssi_size_t i = 0; i < _n_points; i++) {
		_points[i].x = *ptr++;
		_points[i].y = *ptr++;
	}

}

void PaintPoints::paint(ssi_handle_t context, ssi_rect_t area) {

	Lock lock(_mutex);

	if (!_points) {
		return;
	}

	_painter->begin(context, area);
	if (_draw_back) {
		_painter->fill(*_back_brush, area);
	}

	if (_draw_points) {

		if (_draw_labels) {
			paintPositionLabels(area);
		}

		switch (_type) {
		case PaintPointsType::DOTS:
			paintPointsAsDots(area);
			break;
		case PaintPointsType::LINES:
			paintPointsAsLines(area);
			break;
		}
	}
	
    _painter->end();
}

void PaintPoints::paintPositionLabels(ssi_rect_t area) {

	ssi_char_t str[128];
	
	ssi_pointf_t point;
	for (ssi_size_t i = 0; i < _n_points; i++) {
		point = _points[i];
		ssi_sprint(str, "[%.*f,%.*f]", _precision, point.x, _precision, point.y);
		if (_swap) {
			point.y = _relative ? 1.0f - point.y : area.height - point.y;
		}
		_painter->text(*_font, *_font_pen, *_font_brush, point, str, _relative, IPainter::TEXT_ALIGN_HORZ::LEFT, IPainter::TEXT_ALIGN_VERT::CENTER);
	}
}

void PaintPoints::paintPointsAsDots(ssi_rect_t area) {

	ssi_rect_t rect;
	for (ssi_size_t i = 0; i < _n_points; i++) {
		rect.width = rect.height = ssi_cast (int, _point_size + 0.5f);		 
		rect.left = ssi_cast (int, _points[i].x);
		if (_relative) {
			rect.left = ssi_cast(int, _points[i].x * area.width);
			rect.top = ssi_cast(int, _points[i].y * area.height);			
		} else {
			rect.left = ssi_cast(int, _points[i].x);
			rect.top = ssi_cast(int, _points[i].y);
		}

		if (_swap) {
			rect.top = area.height - rect.top;
		}
		_painter->ellipse(*_pen, *_brush, rect);
	}
}

void PaintPoints::paintPointsAsLines(ssi_rect_t area) {

	ssi_pointf_t from, to;

	from.x = _points[_n_points-1].x;
	if (_swap) {
		if (_relative) {
			from.y = 1.0f - _points[_n_points - 1].y;
		}
		else {
			from.y = area.top - _points[_n_points - 1].y;
		}
	}
	else {
		from.y = _points[_n_points - 1].y;
	}

	for (ssi_size_t i = 0; i < _n_points; i++) {
		to.x = _points[i].x;
		if (_swap) {
			if (_relative) {
				to.y = 1.0f - _points[i].y;
			}
			else {
				to.y = area.top - _points[i].y;
			}
		} else {
			to.y = _points[i].y;
		}
		_painter->line(*_pen, from, to, _relative);
		from = to;
	}
}

void PaintPoints::setPen(ssi_rgb_t color, IPainter::ITool::WIDTH width, IPainter::ITool::LINE_STYLES::List style) {

	Lock lock(_mutex);

	delete _pen;
	_pen = new Painter::Pen(color, width, style);
}

void PaintPoints::setBrush(ssi_rgb_t color) {

	Lock lock(_mutex);

	delete _brush;
	_brush = new Painter::Brush(color);
}

void PaintPoints::setBackground(ssi_rgb_t color) {

	Lock lock(_mutex);

	delete _back_brush;
	_back_brush = new Painter::Brush(color);
}

void PaintPoints::setFont(const ssi_char_t *name, ssi_rgb_t fore, ssi_rgb_t back, IPainter::ITool::WIDTH size, IPainter::ITool::FONT_STYLE style) {

	Lock lock(_mutex);

	delete _font;
	_font = new Painter::Font(name, size, style);
	delete _font_pen;
	_font_pen = new Painter::Pen(fore);
	delete _font_brush;
	_font_brush = new Painter::Brush(back);
}

void PaintPoints::setPointSize(ssi_real_t point) {

	Lock lock(_mutex);

	_point_size = point;
}

void PaintPoints::setPrecision(ssi_size_t precision) {

	Lock lock(_mutex);

	_precision = precision;
}

}
