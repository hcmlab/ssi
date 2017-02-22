// PaintEvent.cpp
// author: Florian Lingenfelser <florian.lingenfelser@informatik.uni-augsburg.de>
// created: 2012/04/25
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute itand/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#include "PaintEvent.h"
#include "base/Factory.h"
#if HAS_CAIRO
#include "graphic/CairoPainter.h"
#else
#include "graphic/Painter.h"
#endif
#include "event/EventAddress.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ssi_char_t *PaintEvent::ssi_log_name = "paintevent";
int PaintEvent::ssi_log_level = SSI_LOG_LEVEL_DEFAULT;

PaintEvent::PaintEvent(TYPE::List type) 
	: _type (type),
	_pen(0),
	_brush(0),
	_font(0),
	_font_pen(0),
	_font_brush(0),
	_back_brush(0),
	_dim (0),	
	_axis_captions (0),
	_n_external_axis_captions(0),
	_external_axis_captions(0),
	_event_caption(0),
	_window_caption (0),
	_p_perdim (0),
	_fixed_limit(false),
	_global_limit(false),
	_global_limit_value(0),
	_limits(0),
	_values(0),
	_values_real(0),
	_precision (2) {

	ssi_event_init(_event);

	_painter = new Painter();
	_hollow_brush = new Painter::Brush();

	setPen(IPainter::ITool::COLORS::WHITE, 1, IPainter::ITool::LINE_STYLES::SOLID);
	setBrush(IPainter::ITool::COLORS::RED);
	setBackground(IPainter::ITool::COLORS::BLACK);
	setFont("Courier New", IPainter::ITool::COLORS::WHITE, IPainter::ITool::COLORS::BLACK, 15, IPainter::ITool::FONT_STYLES::NORMAL);
}

PaintEvent::~PaintEvent(){

	Lock lock(_mutex);

	delete _pen;
	delete _hollow_brush;
	delete _brush;
	delete _font;
	delete _font_pen;
	delete _font_brush;
	delete _painter;

	delete[] _values;
	delete[] _values_real;

	releaseAxisCaption();
	releaseExternalAxisCaption();
	delete[] _event_caption;
	delete[] _window_caption;

	ssi_event_destroy(_event);
}

void PaintEvent::setData(ssi_event_t &e){

	Lock lock(_mutex);

	ssi_event_copy(e, _event);
}

void PaintEvent::setPrecision(ssi_size_t precision) {

	Lock lock(_mutex); 
	_precision = precision;
}

void PaintEvent::initAxisCaption(){

	_axis_captions = new ssi_char_t*[_dim];
	for(ssi_size_t i = 0; i < _dim; i++){
		_axis_captions[i] = 0;
	}	
}

void PaintEvent::releaseAxisCaption() {

	for (ssi_size_t i = 0; i < _dim; i++){
		delete[] _axis_captions[i];
		_axis_captions[i] = 0;
	}
	delete[] _axis_captions; _axis_captions = 0;
}

void PaintEvent::releaseExternalAxisCaption() {

	for (ssi_size_t i = 0; i < _n_external_axis_captions; i++) {
		delete[] _external_axis_captions[i];
	}
	delete[] _external_axis_captions; _external_axis_captions = 0;
	_n_external_axis_captions = 0;
}

void PaintEvent::setExternalAxisCaptions(ssi_size_t n_captions, ssi_char_t **captions){
	releaseExternalAxisCaption();
	_n_external_axis_captions = n_captions;
	_external_axis_captions = new ssi_char_t *[_n_external_axis_captions];
	for (ssi_size_t i = 0; i < _n_external_axis_captions; i++) {
		_external_axis_captions[i] = ssi_strcpy(captions[i]);
	}
}

void PaintEvent::setAxisCaption(ssi_size_t dim, const ssi_char_t* caption){
	_axis_captions[dim] = ssi_strcpy (caption);
}

void PaintEvent::setWindowCaption (ssi_char_t *caption) {
	_window_caption = ssi_strcpy (caption);
}

void PaintEvent::setEventCaption(ssi_size_t event_id, ssi_size_t sender_id) {

	_event_caption = ssi_strcat(Factory::GetString(event_id), "@", Factory::GetString(sender_id));
}

void PaintEvent::setPen(ssi_rgb_t color, IPainter::ITool::WIDTH width, IPainter::ITool::LINE_STYLES::List style) {

	Lock lock(_mutex);

	delete _pen;
	_pen = new Painter::Pen(color, width, style);
}

void PaintEvent::setBrush(ssi_rgb_t color) {

	Lock lock(_mutex);

	delete _brush;
	_brush = new Painter::Brush(color);
}

void PaintEvent::setBackground(ssi_rgb_t color) {

	Lock lock(_mutex);

	delete _back_brush;
	_back_brush = new Painter::Brush(color);
}

void PaintEvent::setFont(const ssi_char_t *name, ssi_rgb_t fore, ssi_rgb_t back, IPainter::ITool::WIDTH size, IPainter::ITool::FONT_STYLE style) {

	Lock lock(_mutex);

	delete _font;
	_font = new Painter::Font(name, size, style);
	delete _font_pen;
	_font_pen = new Painter::Pen(fore);
	delete _font_brush;
	_font_brush = new Painter::Brush(back);
}

void PaintEvent::paint(ICanvas *canvas, ssi_handle_t hdc, ssi_rect_t rect){

	Lock lock(_mutex);

	switch (_event.type) {

	case SSI_ETYPE_NTUPLE: {

		paintAsTupel(canvas, hdc, rect);

		break;
	}
		
	}
}

void PaintEvent::reset() {

	Lock lock(_mutex);

	ssi_event_tuple_t *tuple = ssi_pcast(ssi_event_tuple_t, _event.ptr);
	ssi_size_t n_events = _event.tot / sizeof(ssi_event_tuple_t);
	for (ssi_size_t i = 0; i < n_events; i++) {
		tuple[i].value = 0;
	}
	
	resetLimit();
}

void PaintEvent::setFixedLimit(ssi_real_t fix) {

	Lock lock(_mutex);

	_global_limit = true;
	_fixed_limit = true;
	_global_limit_value = fix;
}

void PaintEvent::setGlobalLimit(bool global) {

	Lock lock(_mutex);
	
	_global_limit = global;	
	_fixed_limit = false;
	_global_limit_value = 0;
}

void PaintEvent::keyDown(ICanvas *canvas, IWindow::KEY key, IWindow::KEY vkey) {

	Lock lock(_mutex);

	if (!_fixed_limit) {
		resetLimit();
	}
}

void PaintEvent::resetLimit() {

	delete[] _values; _values = new ssi_real_t[_dim];
	delete[] _values_real; _values_real = new ssi_real_t[_dim];
	delete[] _limits; _limits = new ssi_real_t[_dim];
	
	if (!_fixed_limit) {
		_global_limit_value = 0;
	}
	for (ssi_size_t i = 0; i < _dim; i++) {
		_values[i] = 0;
		_values_real[i] = 0;
		_limits[i] = 0;		
	}
}

void PaintEvent::paintAsTupel(ICanvas *canvas, ssi_handle_t hdc, ssi_rect_t rect) {

	ssi_event_tuple_t *tuple = ssi_pcast(ssi_event_tuple_t, _event.ptr);
	ssi_size_t dim = _event.tot / sizeof(ssi_event_tuple_t);

	if (dim != _dim) {
		releaseAxisCaption();
		_dim = dim;
		resetLimit();
		setEventCaption(_event.event_id, _event.sender_id);
		initAxisCaption();
		for (ssi_size_t i = 0; i < _dim; i++) {
			if (i < _n_external_axis_captions) {
				setAxisCaption(i, _external_axis_captions[i]);
			} else {
				setAxisCaption(i, Factory::GetString(tuple[i].id));
			}
		}
	}

	if (_dim != 0){

		if (!_fixed_limit) {
			for (ssi_size_t i = 0; i < _dim; i++) {
				if (_limits[i] < abs(tuple[i].value)) {
					_limits[i] = abs(tuple[i].value);
				}
				if (_global_limit_value < abs(tuple[i].value)) {
					_global_limit_value = abs(tuple[i].value);
				}
			}
		}

		for (ssi_size_t i = 0; i < _dim; i++) {
			_values_real[i] = tuple[i].value;
			if (_global_limit) {
				_values[i] = _global_limit == 0 ? 0 : tuple[i].value / _global_limit_value;
			} else {
				_values[i] = _limits[i] == 0 ? 0 : tuple[i].value / _limits[i];
			}
		}

		paintBack(canvas, hdc, rect);
		paintBars(canvas, hdc, rect);
		paintGrid(canvas, hdc, rect);
		
	}
}

void PaintEvent::paintBack(ICanvas *canvas, ssi_handle_t context, ssi_rect_t area){

	_painter->begin(context, area);
	_painter->fill(*_back_brush, area);
	_painter->end();
}

void PaintEvent::paintGrid(ICanvas *canvas, ssi_handle_t context, ssi_rect_t area){

	_painter->begin(context, area);

	_painter->text(*_font, *_font_pen, *_font_brush, ssi_point(0, 0), _event_caption, IPainter::TEXT_ALIGN_HORZ::LEFT, IPainter::TEXT_ALIGN_VERT::TOP);

	int x1, x2, y1, y2, w, h = 0;

	int width = area.width;
	int height = area.height;

	_p_perdim = (int) width / _dim;

	for(ssi_size_t ndim = 0; ndim < _dim; ndim++){

		x1 = (int) ( (_p_perdim*ndim) + (_p_perdim/3) );
		y1 = (int) (height/2-height/4);
		w  = (int) (_p_perdim/3);
		h  = (int) (height/2 + 1);

		_painter->rect(*_pen, *_hollow_brush, ssi_rect(x1, y1, w, h));
		if (_axis_captions[ndim]) {
			_painter->text(*_font, *_font_pen, *_font_brush, ssi_point(x1, y1 + h), _axis_captions[ndim], IPainter::TEXT_ALIGN_HORZ::LEFT, IPainter::TEXT_ALIGN_VERT::TOP);
		}

		if (_type == TYPE::BAR) {

			x2 = x1 + w;
			y1 = y2 = (int)(height / 2);
			_painter->line(*_pen, ssi_point(x1, y1), ssi_point(x2, y2));

		}

	}

	_painter->end();
}


void PaintEvent::paintBars(ICanvas *canvas, ssi_handle_t context, ssi_rect_t area){

	ssi_char_t string[100];

	_painter->begin(context, area);

	int x1, y1, w, h = 0;

	int width = area.width;
	int height = area.height;

	_p_perdim = (int)width / _dim;
	
	for (ssi_size_t ndim = 0; ndim < _dim; ndim++){				

		switch (_type) {

		case TYPE::BAR: {

			ssi_real_t value = _values[ndim];
			value = min(1.0f, value);
			value = max(-1.0f, value);

			if (value > 0){

				x1 = ssi_size_t(_p_perdim * ndim + _p_perdim / 3 + 1);
				y1 = ssi_size_t(height / 2 - (value * (height / 4)) + 1);
				w = ssi_size_t(_p_perdim / 3 - 2);
				h = ssi_size_t(value * (height / 4) + 1);

			} else {

				x1 = ssi_size_t(_p_perdim * ndim + _p_perdim / 3 + 1);
				y1 = ssi_size_t(height / 2);
				w = ssi_size_t(_p_perdim / 3 - 2);
				h = ssi_size_t(std::abs((double)value)*(height / 4));

			}

			_painter->fill(*_brush, ssi_rect(x1, y1, w, h));			

			break;
		}

		case TYPE::BAR_POS: {

			ssi_real_t value = _values[ndim];
			value = min(1.0f, value);
			value = max(0.0f, value);

			x1 = ssi_size_t(_p_perdim * ndim + _p_perdim / 3 + 1);
			y1 = ssi_size_t(3 * height / 4 - (value * (height / 2)) + 1);
			w = ssi_size_t(_p_perdim / 3 - 2);
			h = ssi_size_t(std::abs((double)value) * (height / 2) + 1);

			if (y1 <= 3 * height / 4) {
				_painter->fill(*_brush, ssi_rect(x1, y1, w, h));
			}

			break;
		}

		}

		ssi_sprint(string, "%.*f", _precision, _values_real[ndim]);
		_painter->text(*_font, *_font_pen, *_font_brush, ssi_point(x1, height / 4), string, IPainter::TEXT_ALIGN_HORZ::LEFT, IPainter::TEXT_ALIGN_VERT::BOTTOM);

	}

	_painter->end();

}


};

