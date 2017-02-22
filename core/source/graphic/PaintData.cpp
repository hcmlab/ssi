// PaintData.cpp
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

#include "graphic/PaintData.h"
#include "graphic/Painter.h"
#include "graphic/Colormap.h"
#include "graphic/GraphicTools.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif


namespace ssi {

PaintData::PaintData()
	: _painter(0),
	_parent(0),
	_type(TYPE::SIGNAL),
	_mins(0),
	_maxs(0),
	_dim(0),
	_colormap(0),
	_pen(0),
	_brush(0),
	_back_brush(0),
	_font(0),
	_font_pen(0),
	_font_brush(0),
	_precision(2),
	_fix_limits(false),
	_fix_min (0),
	_fix_max (0),
	_indx (0),
	_indy (1),
	_reset_limits (true),
	_stream_has_changed (true) {

	setColormap(IColormap::COLORMAP::COLOR64);
	_painter = new Painter();
	setPen(IPainter::ITool::COLORS::GREEN, 1, IPainter::ITool::LINE_STYLES::SOLID);
	setBrush(IPainter::ITool::COLORS::BLACK);
	setBackground(true, IPainter::ITool::COLORS::BLACK);
	setFont(SSI_DEFAULT_FONT_NAME, IPainter::ITool::COLORS::WHITE, IPainter::ITool::COLORS::BLACK, SSI_DEFAULT_FONT_SIZE, IPainter::ITool::FONT_STYLES::NORMAL);
	setPointSize(5);

	_label[0] = '\0';	

	ssi_stream_init(_stream, 0, 0, sizeof(ssi_real_t), SSI_REAL, 0);
	ssi_stream_init(_resample, 0, 0, sizeof(ssi_real_t), SSI_REAL, 0);
}

PaintData::~PaintData() {

	delete _colormap;
	delete _pen; _pen = 0;
	delete _brush; _brush = 0;
	delete _font; _font = 0;
	delete _font_pen; _font_pen = 0;
	delete _font_brush; _font_brush = 0;
	delete _painter; _painter = 0;
	delete _back_brush; _back_brush = 0;

	ssi_stream_destroy(_stream);
	ssi_stream_destroy(_resample);
	delete[] _mins;
	delete[] _maxs;
}

void PaintData::setData(ssi_stream_t &stream, TYPE::List type) {

	Lock lock(_mutex);

	_type = type;
	_stream.sr = stream.sr;
	ssi_stream_convert(stream, _stream);

	_stream_has_changed = true;
}

void PaintData::resetLimits() {

	Lock lock(_mutex);

	_reset_limits = true;
	_fix_limits = false;
}

void PaintData::keyDown(IWindow::KEY key, IWindow::KEY vkey) {

	Lock lock(_mutex);

	if (!_fix_limits) {
		_reset_limits = true;		
	}

	_stream_has_changed = true;
	_parent->update();
}

void PaintData::create(ICanvas *canvas) {

	_parent = canvas;
}

void PaintData::close() {
}

void PaintData::paint(ssi_handle_t context, ssi_rect_t area) {

	Lock lock(_mutex);

	if (_stream.num == 0) {
		return;
	}

	_painter->begin(context, area);

	if (_stream_has_changed) {

		if (_stream.dim != _dim) {
			ssi_stream_destroy(_resample);
			ssi_stream_init(_resample, 0, _stream.dim, _stream.byte, _stream.type, _stream.sr, _stream.time);
			_dim = _stream.dim;
			delete[] _mins; delete[] _maxs;
			_mins = new ssi_real_t[_dim];
			_maxs = new ssi_real_t[_dim];
		}

		switch (_type) {
		case TYPE::SIGNAL:
		case TYPE::AUDIO:
		case TYPE::IMAGE: {

			ssi_stream_adjust(_resample, ssi_cast(ssi_size_t, area.width));
			ssi_resample(_stream.num, _resample.num, _stream.dim, ssi_pcast(ssi_real_t, _stream.ptr), ssi_pcast(ssi_real_t, _resample.ptr), _type == TYPE::AUDIO ? ssi_max : ssi_mean);			

			break;
		}

		case TYPE::PATH:
		case TYPE::SCATTER: {

			ssi_stream_adjust(_resample, _stream.num);
			memcpy(_resample.ptr, _stream.ptr, _stream.tot);						

			break;
		}
		}

		if (!_fix_limits) {
			if (_reset_limits) {
				ssi_minmax(_resample.num, _resample.dim, ssi_pcast(ssi_real_t, _resample.ptr), _mins, _maxs);
				_reset_limits = false;
			}
			else {
				ssi_real_t *mins = new ssi_real_t[_dim];
				ssi_real_t *maxs = new ssi_real_t[_dim];
				ssi_minmax(_resample.num, _resample.dim, ssi_pcast(ssi_real_t, _resample.ptr), mins, maxs);
				for (ssi_size_t i = 0; i < _dim; i++) {
					_mins[i] = min(_mins[i], mins[i]);
					_maxs[i] = max(_maxs[i], maxs[i]);
				}
				delete[] mins;
				delete[] maxs;
			}
			if (_type == TYPE::AUDIO) {
				for (ssi_size_t i = 0; i < _dim; i++) {
                    _maxs[i] = std::abs((ssi_real_t)_maxs[i]);
					_mins[i] = -_maxs[i];
				}
			}
		}
		else {
			for (ssi_size_t i = 0; i < _dim; i++) {
				_mins[i] = _fix_min;
				_maxs[i] = _fix_max;
			}
		}

		ssi_norm(_resample.num, _resample.dim, ssi_pcast(ssi_real_t, _resample.ptr), _mins, _maxs);

		_stream_has_changed = false;
	}

	if (_back_toggle) {
		_painter->fill(*_back_brush, area);
	}

	switch (_type) {
	case TYPE::SIGNAL:
		GraphicTools::PaintAsSignal(_resample, *_painter, *_pen);
		break;
	case TYPE::AUDIO:
		GraphicTools::PaintAsAudio(_resample, *_painter, *_pen);
		break;
	case TYPE::PATH:
		GraphicTools::PaintAsPath(_resample, _indx, _indy, *_painter, *_pen);
		break;
	case TYPE::SCATTER:
		GraphicTools::PaintAsScatter(_resample, _indx, _indy, *_painter, *_pen, *_brush, _point_size);
		break;
	case TYPE::IMAGE:
		GraphicTools::PaintAsImage(_resample, *_painter, *_colormap);
		break;
	}

	switch (_type) {
	case TYPE::SIGNAL:
	case TYPE::AUDIO:
		GraphicTools::PaintAxis(_stream.dim, _stream.time, _stream.time + (_stream.sr > 0 ? _stream.num / _stream.sr : 0), _mins, _maxs, *_painter, *_font, *_font_pen, *_font_brush, _precision);
		break;
	case TYPE::PATH:
	case TYPE::SCATTER:
	case TYPE::IMAGE:
		ssi_real_t minval, maxval, tmp;
		ssi_minmax(_dim, 1, _mins, &minval, &tmp);
		ssi_minmax(_dim, 1, _maxs, &tmp, &maxval);
		GraphicTools::PaintAxis(1, _stream.time, _stream.time + (_stream.sr > 0 ? _stream.num / _stream.sr : 0), &minval, &maxval, *_painter, *_font, *_font_pen, *_font_brush, _precision);
		break;
	}

	_painter->end();
}

void PaintData::setColormap(IColormap::COLORMAP::List colormap) {

	Lock lock(_mutex);

	delete _colormap;
	_colormap = new Colormap(colormap);
}

void PaintData::setPen(ssi_rgb_t color, IPainter::ITool::WIDTH width, IPainter::ITool::LINE_STYLES::List style) {

	Lock lock(_mutex);

	delete _pen;
	_pen = new Painter::Pen(color, width, style);
}

void PaintData::setBrush(ssi_rgb_t color) {

	Lock lock(_mutex);

	delete _brush;
	_brush = new Painter::Brush(color);
}

void PaintData::setBackground(bool toggle, ssi_rgb_t color) {

	Lock lock(_mutex);

	delete _back_brush;
	_back_brush = new Painter::Brush(color);
	_back_toggle = toggle;
}

void PaintData::setFont(const ssi_char_t *name, ssi_rgb_t fore, ssi_rgb_t back, IPainter::ITool::WIDTH size, IPainter::ITool::FONT_STYLE style) {

	Lock lock(_mutex);

	delete _font;
	_font = new Painter::Font(name, size, style);
	delete _font_pen;
	_font_pen = new Painter::Pen(fore);
	delete _font_brush;
	_font_brush = new Painter::Brush(back);
}

void PaintData::setPointSize(ssi_real_t point) {

	Lock lock(_mutex);

	_point_size = point;
}

void PaintData::setLimits(ssi_real_t minval, ssi_real_t maxval) {

	Lock lock(_mutex);

	_fix_limits = true;
	_fix_min = minval;
	_fix_max = maxval;
}

void PaintData::setPrecision(ssi_size_t precision) {

	Lock lock(_mutex);

	_precision = precision;
}

void PaintData::setIndices(ssi_size_t indx, ssi_size_t indy) {

	Lock lock(_mutex);

	_indx = indx;
	_indy = indy;
}

void PaintData::setLabel(const ssi_char_t *label) {

	Lock lock(_mutex);

	ssi_strcpy(_label, label);
}

}
