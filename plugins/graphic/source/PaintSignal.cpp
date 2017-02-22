// PaintSignal.cpp
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

#include "PaintSignal.h"

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

PaintSignal::PaintSignal(ssi_time_t sr,
	ssi_size_t dim,
	ssi_time_t size_in_s,
	PaintSignalType::TYPE type,
	ssi_size_t x_ind,
	ssi_size_t y_ind,
	bool static_image)
	: _painter(0),
	_parent(0),
	data_buffer_scaled(0),
	data_buffer(0),
	scale(0),
	scale_tot(0),
	data_buffer_position(0),
	sample_rate(sr),
	sample_dimension(dim),
	window_size(size_in_s),
	sample_number_total(0),
	data_min(0),
	data_min_tot(0),
	data_max(0),
	data_max_tot(0),
	time_in_s(0),
	fixed_min_max(false),
	_colormap (0),
	_pen(0),
	_brush(0),
	_font(0),
	_font_pen(0),
	_font_brush(0),
	_back_brush(0),
	reset_min_max(true),
	discrete_plot(size_in_s <= 0),
	_type(type),
	_x_ind(x_ind),
	_y_ind(_y_ind),
	_static_image(static_image),
	axis_precision(3),
	_draw_stream(false) {

	if (sr == 0) {
		ssi_err("sample rate must not be zero");
	}	

	setColormap(IColormap::COLORMAP::COLOR64);

	ssi_stream_init(_convert, 0, 0, sizeof(ssi_real_t), SSI_REAL, sr);
	sample_number_total_real = sample_number_total = 0;
	data_buffer_size_real = data_buffer_size = 0;
	data_buffer_scaled = 0;
	data_buffer = 0;
	scale = new ssi_real_t[sample_dimension];
	data_min = new ssi_real_t[sample_dimension];
	data_max = new ssi_real_t[sample_dimension];
}

PaintSignal::~PaintSignal() {

	ssi_stream_destroy(_convert);

	delete _colormap; _colormap = 0;

	delete[] data_buffer;
	delete[] data_buffer_scaled;
	delete[] data_min;
	delete[] data_max;
	delete[] scale;
}

void PaintSignal::create(ICanvas *canvas) {

	_parent = canvas;

	_painter = new Painter();
	setPen(IPainter::ITool::COLORS::GREEN, 1, IPainter::ITool::LINE_STYLES::SOLID);
	setBrush(IPainter::ITool::COLORS::BLACK);
	setBackground(IPainter::ITool::COLORS::BLACK);
	setFont(SSI_DEFAULT_FONT_NAME, SSI_DEFAULT_FONT_COLOR_FORE, SSI_DEFAULT_FONT_COLOR_BACK, SSI_DEFAULT_FONT_SIZE, IPainter::ITool::FONT_STYLES::NORMAL);
	setPointSize(5);
}

void PaintSignal::close() {

	delete _pen; _pen = 0;
	delete _brush; _brush = 0;
	delete _font; _font = 0;
	delete _font_pen; _font_pen = 0;
	delete _font_brush; _font_brush = 0;
	delete _painter; _painter = 0;
}

void PaintSignal::clear() {
	_draw_stream = false;
}

void PaintSignal::setData(ssi_stream_t &stream, ssi_time_t time) {

	ssi_size_t sample_number = stream.num;
	ssi_stream_adjust(_convert, stream.num);
	ssi_stream_convert(stream, _convert);

	_draw_stream = true;

	bool scale_change = false;

	// in continuous case init buffer at first call
	if (!discrete_plot && data_buffer == 0) {

		sample_number_total_real = sample_number_total = ssi_cast(ssi_size_t, sample_rate * window_size + 0.5);
		data_buffer_size_real = data_buffer_size = sample_dimension * sample_number_total_real;
		data_buffer_scaled = new ssi_real_t[data_buffer_size_real];
		data_buffer = new ssi_real_t[data_buffer_size_real];

		ssi_real_t *dstptr = data_buffer;
		ssi_real_t *dstptr_scaled = data_buffer_scaled;
		ssi_real_t *srcptr = ssi_pcast(ssi_real_t, _convert.ptr);
		for (ssi_size_t i = 0; i < sample_number_total_real; i++) {
			for (ssi_size_t j = 0; j < sample_dimension; j++) {
				*dstptr++ = srcptr[j];
				*dstptr_scaled++ = 0;
			}
		}
	}

	// in continuous case check if new data fits in buffer
	if (!discrete_plot && sample_number > sample_number_total) {
		ssi_wrn("crop stream (%u -> %u)", sample_number, sample_number_total);
		sample_number = sample_number_total;
	}

	// in discrete case check if it is neccessary to adjust the buffer size
	if (discrete_plot) {
		if (sample_number > sample_number_total_real) {

			sample_number_total_real = sample_number;
			data_buffer_size_real = sample_number_total_real * sample_dimension;
			delete[] data_buffer;
			data_buffer = new ssi_real_t[data_buffer_size_real];
			delete[] data_buffer_scaled;
			data_buffer_scaled = new ssi_real_t[data_buffer_size_real];

			ssi_real_t *dstptr = data_buffer;
			ssi_real_t *dstptr_scaled = data_buffer_scaled;
			ssi_real_t *srcptr = ssi_pcast(ssi_real_t, _convert.ptr);
			for (ssi_size_t i = 0; i < sample_number; i++) {
				for (ssi_size_t j = 0; j < sample_dimension; j++) {
					*dstptr++ = srcptr[j];
					*dstptr_scaled++ = 0;
				}
			}
		}
		sample_number_total = sample_number;
		data_buffer_size = sample_number_total * sample_dimension;
		data_buffer_position = 0;
	}

	if (!fixed_min_max) {

		// initialize min, max arrays if necessary
		if (reset_min_max) {
			ssi_real_t *dataptr = ssi_pcast(ssi_real_t, _convert.ptr);
			for (ssi_size_t i = 0; i < sample_dimension; i++) {
				data_min[i] = *dataptr;
				data_max[i] = *dataptr++;
			}
			reset_min_max = false;
		}

		// adjust min, max arrays
		ssi_real_t *dataptr = ssi_pcast(ssi_real_t, _convert.ptr);
		for (ssi_size_t i = 0; i < sample_number; i++) {
			for (ssi_size_t j = 0; j < sample_dimension; j++) {
				if (data_min[j] > *dataptr) {
					data_min[j] = *dataptr;
					scale_change = true;
				}
				else if (data_max[j] < *dataptr) {
					data_max[j] = *dataptr;
					scale_change = true;
				}
				dataptr++;
			}
		}

		if (_type == PaintSignalType::AUDIO) {
			for (ssi_size_t j = 0; j < sample_dimension; j++) {
				if (abs(data_min[j]) > abs(data_max[j])) {
					data_max[j] = abs(data_min[j]);
				}
				data_min[j] = 0 - abs(data_max[j]);
			}
		}

		// find total min, max
		data_min_tot = data_min[0];
		data_max_tot = data_max[0];
		for (ssi_size_t i = 1; i < sample_dimension; i++) {
			if (data_min_tot > data_min[i]) {
				data_min_tot = data_min[i];
			}
			else if (data_max_tot < data_max[i]) {
				data_max_tot = data_max[i];
			}
		}
		scale_tot = ssi_cast(ssi_real_t, data_max_tot - data_min_tot);
	}

	// adjust scale array
	for (ssi_size_t i = 0; i < sample_dimension; i++) {
		scale[i] = ssi_cast(ssi_real_t, data_max[i] - data_min[i]);
	}

	{

		Lock lock(_mutex);

		// update time
		time_in_s = time + sample_number / sample_rate;

		// fill in new data
		ssi_real_t *srcptr = ssi_pcast(ssi_real_t, _convert.ptr);
		ssi_real_t *dstptr = data_buffer + data_buffer_position;
		ssi_real_t *dstptr_scaled = data_buffer_scaled + data_buffer_position;
		ssi_real_t value;
		for (ssi_size_t i = 0; i < sample_number; i++) {
			for (ssi_size_t j = 0; j < sample_dimension; j++) {
				value = *srcptr++;
				*dstptr++ = value;
				if (_type == PaintSignalType::IMAGE && !fixed_min_max) {
					*dstptr_scaled++ = ssi_cast(ssi_real_t, value - data_min_tot) / scale_tot;
				}
				else {
					*dstptr_scaled++ = ssi_cast(ssi_real_t, value - data_min[j]) / scale[j];
				}
				++data_buffer_position;
				if (data_buffer_position == data_buffer_size) {
					data_buffer_position = 0;
					dstptr_scaled = data_buffer_scaled;
					dstptr = data_buffer;
				}
			}
		}

		// if scale has changed do a rescale
		if (scale_change) {
			srcptr = data_buffer;
			dstptr_scaled = data_buffer_scaled;
			for (ssi_size_t i = 0; i < sample_number_total; i++) {
				for (ssi_size_t j = 0; j < sample_dimension; j++) {
					value = *srcptr++;
					if (_type == PaintSignalType::IMAGE) {
						*dstptr_scaled++ = ssi_cast(ssi_real_t, value - data_min_tot) / scale_tot;
					}
					else {
						*dstptr_scaled++ = ssi_cast(ssi_real_t, value - data_min[j]) / scale[j];
					}
				}
			}
		}
	}


}

void PaintSignal::resetLimits() {

	Lock lock(_mutex);

	if (!fixed_min_max) {
		reset_min_max = true;
	}
}

void PaintSignal::keyDown(IWindow::KEY key, IWindow::KEY vkey) {

	resetLimits();
}

void PaintSignal::paint(ssi_handle_t context, ssi_rect_t area) {

	Lock lock(_mutex);

	if (!data_buffer) {
		return;
	}

	_painter->begin(context, area);
	_painter->fill(*_back_brush, area);

	if (_draw_stream) {

		switch (_type) {
		case PaintSignalType::IMAGE:
			paintAsImage(area);
			break;
		case PaintSignalType::SIGNAL:
			paintAsSignal(area);
			break;
		case PaintSignalType::AUDIO:
			paintAsAudio(area);
			break;
		case PaintSignalType::PATH:
			paintAsPath(area);
			break;
		}

		if (_type != PaintSignalType::PATH) {
			paintAxis(area);
		}
	}

    _painter->end();
}

void PaintSignal::paintAxis(ssi_rect_t area) {

	ssi_size_t dim = _type == PaintSignalType::IMAGE ? 1 : sample_dimension;

	ssi_char_t str[128];

	// get dim of the canvas
	ssi_int_t width = area.width;
	ssi_int_t height_total = area.height;
	ssi_int_t height = height_total / dim;

	// draw
	for (ssi_size_t i = 0; i < dim; i++) {

		ssi_real_t min_val = data_min[i];
		ssi_real_t max_val = data_max[i];
		ssi_sprint(str, "%.*f", axis_precision, max_val);
		_painter->text(*_font, *_font_pen, *_font_brush, ssi_point(0, i*height), str, IPainter::TEXT_ALIGN_HORZ::LEFT, IPainter::TEXT_ALIGN_VERT::TOP);
		ssi_sprint(str, "%.*f", axis_precision, min_val);
		_painter->text(*_font, *_font_pen, *_font_brush, ssi_point(0, (i + 1)*height), str, IPainter::TEXT_ALIGN_HORZ::LEFT, IPainter::TEXT_ALIGN_VERT::BOTTOM);
		
		if (_type == PaintSignalType::SIGNAL || _type == PaintSignalType::AUDIO) {
			ssi_sprint(str, "%.*f", axis_precision, (ssi_real_t)data_buffer[(data_buffer_position < dim) ? (data_buffer_size - dim + i) : (data_buffer_position - dim + i)]);
			_painter->text(*_font, *_font_pen, *_font_brush, ssi_point(0, i*height + (height >> 1)), str, IPainter::TEXT_ALIGN_HORZ::LEFT, IPainter::TEXT_ALIGN_VERT::CENTER);
		}

		if (i > 0) {
			_painter->line(*_font_pen, ssi_point(0, height*i), ssi_point(width, height*i));
		}
	}

	ssi_sprint(str, "%.3lf", time_in_s - sample_number_total / sample_rate);
	_painter->text(*_font, *_font_pen, *_font_brush, ssi_point(width, 0), str, IPainter::TEXT_ALIGN_HORZ::RIGHT, IPainter::TEXT_ALIGN_VERT::TOP);	
	ssi_sprint(str, "%.3lf", time_in_s);	
	_painter->text(*_font, *_font_pen, *_font_brush, ssi_point(width, height_total), str, IPainter::TEXT_ALIGN_HORZ::RIGHT, IPainter::TEXT_ALIGN_VERT::BOTTOM);
}

ssi_real_t PaintSignal::mymax(ssi_real_t *data, ssi_size_t size, ssi_size_t dim, ssi_int_t old_ind, ssi_int_t new_ind) {

	ssi_real_t *ptr = data + old_ind;
	ssi_real_t result = abs(*ptr);
	while (old_ind != new_ind) {
		if (abs(*ptr) > result) {
			result = abs(*ptr);
		}
		ptr += dim;
		old_ind += dim;
		if (old_ind >= (ssi_int_t)size) {
			old_ind -= size;
			ptr = data + old_ind;
		}
	}

	return result;
}

ssi_real_t PaintSignal::mean(ssi_real_t *data, ssi_size_t size, ssi_size_t dim, ssi_int_t old_ind, ssi_int_t new_ind) {

	ssi_real_t result = 0;

	ssi_size_t len = 0;
	ssi_real_t *ptr = data + old_ind;
	while (old_ind != new_ind) {
		result += *ptr;
		ptr += dim;
		old_ind += dim;
		++len;
		if (old_ind >= (ssi_int_t)size) {
			old_ind -= size;
			ptr = data + old_ind;
		}
	}

	return result / len;
}

void PaintSignal::paintAsPath(ssi_rect_t area) {

	// get dimension of the canvas
	ssi_int_t width = area.width;
	ssi_int_t height = area.height;

	// plot data
	ssi_real_t *data_scaledptr = data_buffer_scaled + data_buffer_position;
	ssi_real_t x = *(data_scaledptr + _x_ind);
	ssi_real_t y = *(data_scaledptr + _y_ind);
	data_scaledptr += sample_dimension;
	ssi_point_t from, to;
	from.x = (ssi_int_t)(x * width);
	from.y = (ssi_int_t)((1.0f - y) * height);	
	for (ssi_size_t i = sample_dimension; i < sample_number_total; i++) {
		if (data_buffer_position + i == data_buffer_size) {
			data_scaledptr = data_buffer_scaled;
		}
		x = *(data_scaledptr + _x_ind);
		y = *(data_scaledptr + _y_ind);
		data_scaledptr += sample_dimension;
		to.x = (ssi_int_t)(x * width);
		to.y = (ssi_int_t)((1.0f - y) * height);
		_painter->line(*_pen, from, to);
		from = to;		
	}
}

void PaintSignal::paintAsSignal(ssi_rect_t area) {

	// get sample_dimension of the canvas
	ssi_int_t width = area.width;
	ssi_int_t height_total = area.height;

	// calculate y scale factor
	ssi_real_t scale_y = (ssi_real_t)(sample_number_total) / width;
	ssi_int_t height = height_total / sample_dimension;
	ssi_int_t *offset = new ssi_int_t[sample_dimension];
	for (ssi_size_t i = 0; i < sample_dimension; i++) {
		offset[i] = height * i;
	}

	// plot data
	if (scale_y >= 1) {

		ssi_int_t old_y;
		ssi_int_t next_y;
		ssi_int_t old_ind, new_ind;
		ssi_real_t value;
		//ssi_size_t data_buffer_head = ((ssi_int_t) (data_buffer_position / scale_y)) / sample_dimension;
		ssi_int_t data_buffer_head = (ssi_int_t)data_buffer_position;

		ssi_point_t from, to;

		for (ssi_size_t i = 0; i < sample_dimension; i++) {
			bool found_data_buffer_head = false;
			//new_ind = (0 + i + data_buffer_position) % data_buffer_size;
			new_ind = i;
			next_y = offset[i] + (ssi_int_t)((1.0f - data_buffer_scaled[new_ind]) * height);
			from.x = 0;
			from.y = next_y;			
			for (ssi_int_t x = 1; x < width; x++) {
				old_ind = new_ind;
				//new_ind = (data_buffer_position + sample_dimension * (ssi_int_t) (scale_y * x) + i) % data_buffer_size;
				new_ind = sample_dimension * (ssi_int_t)(scale_y * x) + i;

				if (!found_data_buffer_head && new_ind >= data_buffer_head) {
					//value = mean (data_buffer_scaled, data_buffer_size, sample_dimension, old_ind, new_ind);
					value = data_buffer_scaled[new_ind];
					old_y = next_y;
					next_y = offset[i] + (ssi_int_t)((1.0f - value) * height);
					to.x = x;
					to.y = old_y;
					_painter->line(*_pen, from, to);
					from.x = x;
					from.y = offset[i];
					to.x = x;
					to.y = offset[i] + height;
					_painter->line(*_font_pen, from, to);					
					from.x = x;
					from.y = next_y;					
					found_data_buffer_head = true;
				}
				else {
					value = mean(data_buffer_scaled, data_buffer_size, sample_dimension, old_ind, new_ind);
					old_y = next_y;
					next_y = offset[i] + (ssi_int_t)((1.0f - value) * height);
					to.x = x;
					to.y = next_y;
					_painter->line(*_pen, from, to);
					from = to;					
				}
			}
		}

	}
	else {

		ssi_int_t old_y;
		ssi_int_t next_x, next_y;
		ssi_point_t from, to;
		scale_y = 1 / scale_y;
		ssi_size_t data_buffer_head = data_buffer_position / sample_dimension;
		for (ssi_size_t i = 0; i < sample_dimension; i++) {
			next_x = 0;
			//next_y = offset[i] + (ssi_int_t) ((1.0f - data_buffer_scaled[(data_buffer_position + i) % data_buffer_size]) * height);
			next_y = offset[i] + (ssi_int_t)((1.0f - data_buffer_scaled[i]) * height);
			from.x = next_x;
			from.y = next_y;
			for (ssi_size_t y = 1; y < sample_number_total; y++) {
				next_x = (ssi_int_t)(scale_y * y);
				//next_y = offset[i] + (ssi_int_t) ((1.0f - data_buffer_scaled[(data_buffer_position + sample_dimension*y+i) % data_buffer_size]) * height);
				old_y = next_y;
				next_y = offset[i] + (ssi_int_t)((1.0f - data_buffer_scaled[sample_dimension*y + i]) * height);
				if (y == data_buffer_head) {
					to.x = next_x;
					to.y = old_y;
					_painter->line(*_pen, from, to);
					from.x = next_x;
					from.y = offset[i];
					to.x = next_x;
					to.y = offset[i] + height;
					_painter->line(*_font_pen, from, to);
					from.x = next_x;
					from.y = next_y;
				}
				else {
					to.x = next_x;
					to.y = next_y;
					_painter->line(*_pen, from, to);
					from = to;
				}
			}
		}

	}
	delete[] offset;
}

void PaintSignal::paintAsAudio(ssi_rect_t area) {

	// get sample_dimension of the canvas
	ssi_int_t width = area.width;
	ssi_int_t height_total = area.height;

	// calculate y scale factor
	ssi_real_t scale_y = (ssi_real_t)(sample_number_total) / width;
	ssi_int_t height = height_total / sample_dimension;
	ssi_int_t *offset = new ssi_int_t[sample_dimension];
	for (ssi_size_t i = 0; i < sample_dimension; i++) {
		offset[i] = height * i;
	}

	// plot data
	if (scale_y >= 1) {

		ssi_int_t next_y;
		ssi_int_t old_ind, new_ind;
		ssi_point_t from, to;
		ssi_real_t value;
		ssi_int_t data_buffer_head = (ssi_int_t)data_buffer_position;
		for (ssi_size_t i = 0; i < sample_dimension; i++) {
			bool found_data_buffer_head = false;
			//new_ind = (0 + i + data_buffer_position) % data_buffer_size;		
			new_ind = i;
			for (ssi_int_t x = 0; x < width; x++) {
				old_ind = new_ind;
				//new_ind = (data_buffer_position + sample_dimension * (ssi_int_t) (scale_y * x) + i) % data_buffer_size;
				new_ind = sample_dimension * (ssi_int_t)(scale_y * x) + i;

				if (!found_data_buffer_head && new_ind >= data_buffer_head) {
					from.x = x;
					from.y = offset[i];
					to.x = x;
					to.y = offset[i] + height;
					_painter->line(*_font_pen, from, to);
					found_data_buffer_head = true;
				}
				else {
					value = mymax(data_buffer_scaled, data_buffer_size, sample_dimension, old_ind, new_ind);
					next_y = offset[i] + (ssi_int_t)((1.0f - value) * height);
					from.x = x;
					from.y = next_y;
					to.x = x;
					to.y = offset[i] + (ssi_int_t)(value * height);
					_painter->line(*_pen, from, to);
				}
			}
		}

	}
	else {

		ssi_int_t next_x, next_y;
		ssi_point_t from, to;
		scale_y = 1 / scale_y;
		ssi_size_t data_buffer_head = data_buffer_position / sample_dimension;
		for (ssi_size_t i = 0; i < sample_dimension; i++) {
			for (ssi_size_t y = 1; y < sample_number_total; y++) {
				next_x = (ssi_int_t)(scale_y * y);
				if (y == data_buffer_head) {
					from.x = next_x; 
					from.y = offset[i];
					to.x = next_x;
					to.y = offset[i] + height;
					_painter->line(*_font_pen, from, to);
				}
				else {
					next_y = offset[i] + (ssi_int_t)((1.0f - data_buffer_scaled[sample_dimension*y + i]) * height);
					from.x = next_x;
					from.y = next_y;
					to.x = next_x;
					to.y = offset[i] + (ssi_int_t)((data_buffer_scaled[sample_dimension*y + i]) * height);
					_painter->line(*_pen, from, to);
				}
			}
		}

	}
	delete[] offset;
}

void PaintSignal::paintAsImage(ssi_rect_t area) {

	// get sample_dimension of the canvas
	ssi_int_t width = area.width;
	ssi_int_t height = area.height;

	ssi_int_t vals_x = sample_number_total;
	ssi_int_t vals_y = sample_dimension;

	if (_static_image) {
		vals_x = _x_ind;
		vals_y = _y_ind;
	}

	// calculate y scale factor
	ssi_real_t scale_x = ssi_cast(ssi_real_t, vals_x) / width;
	ssi_real_t scale_y = ssi_cast(ssi_real_t, vals_y) / height;

	// plot data_buffer_scaled
	if (scale_x >= 1) {
		if (scale_y >= 1) {

			// more data_buffer_scaled points than pixels in x and y direction

			// determine row indices
			ssi_int_t *x_inds = new ssi_int_t[width];
			for (ssi_int_t x = 0; x < width; x++) {
				x_inds[x] = (ssi_int_t)(scale_x * x);
			}
			// determine col indices
			ssi_int_t *y_inds = new ssi_int_t[height];
			for (ssi_int_t y = 0; y < height; y++) {
				y_inds[y] = (ssi_int_t)(scale_y * y);
			}
			// now print
			for (ssi_int_t x = 0; x < width; x++) {
				ssi_real_t *data_scaled_ptr = data_buffer_scaled + x_inds[x] * vals_y;
				for (ssi_int_t y = 0; y < height; y++) {
					_painter->pixel(*_colormap->getPen(*(data_scaled_ptr + y_inds[y])), ssi_point(x, y));
				}
			}
			delete[] x_inds;
			delete[] y_inds;

		}

		else {

			// more data_buffer_scaled points in x direction but less in y
			scale_y = 1 / scale_y;

			// determine row indices
			ssi_int_t *x_inds = new ssi_int_t[width];
			for (ssi_int_t x = 0; x < width; x++) {
				x_inds[x] = (ssi_int_t)(scale_x * x);
			}
			// determine col indices
			ssi_int_t *y_inds = new ssi_int_t[vals_y + 1];
			for (ssi_int_t y = 0; y < vals_y; y++) {
				y_inds[y] = (ssi_int_t)(scale_y * y);
			}
			y_inds[vals_y] = height;
			// now print			
			ssi_rect_t r;
			for (ssi_int_t x = 0; x < width; x++) {
				ssi_real_t *data_scaled_ptr = data_buffer_scaled + x_inds[x] * vals_y;
				r.left = x;
				r.width = 1;
				for (ssi_int_t y = 0; y < vals_y; y++) {
					r.top = y_inds[y];
					r.height = y_inds[y + 1] - r.top;
					_painter->fill(*_colormap->getBrush(*(data_scaled_ptr + y)), r);
				}
			}
			delete[] x_inds;
			delete[] y_inds;

		}
	}
	else {

		if (scale_y >= 1) {

			// more data_buffer_scaled points than pixels in y direction but less in x
			scale_x = 1 / scale_x;

			// determine col indices
			ssi_int_t *x_inds = new ssi_int_t[vals_x + 1];
			for (ssi_int_t x = 0; x < vals_x; x++) {
				x_inds[x] = (ssi_int_t)(scale_x * x);
			}
			x_inds[vals_x] = width;
			// determine col indices
			ssi_int_t *y_inds = new ssi_int_t[height];
			for (ssi_int_t y = 0; y < height; y++) {
				y_inds[y] = (ssi_int_t)(scale_y * y);
			}
			// now print
			ssi_rect_t r;
			for (ssi_int_t x = 0; x < vals_x; x++) {
				ssi_real_t *data_scaled_ptr = data_buffer_scaled + x * vals_y;
				r.left = x_inds[x];
				r.width = x_inds[x + 1] - r.left;
				for (ssi_int_t y = 0; y < height; y++) {
					r.top = y;
					r.height = 1;
					_painter->fill(*_colormap->getBrush(*(data_scaled_ptr + y_inds[y])), r);
				}
			}
			delete[] x_inds;
			delete[] y_inds;

		}
		else {

			// less data_buffer_scaled points in x and y direction
			scale_x = 1 / scale_x;
			scale_y = 1 / scale_y;

			// determine col indices
			ssi_int_t *x_inds = new ssi_int_t[vals_x + 1];
			for (ssi_int_t x = 0; x < vals_x; x++) {
				x_inds[x] = (ssi_int_t)(scale_x * x);
			}
			x_inds[vals_x] = width;
			// determine col indices
			ssi_int_t *y_inds = new ssi_int_t[vals_y + 1];
			for (ssi_int_t y = 0; y < vals_y; y++) {
				y_inds[y] = (ssi_int_t)(scale_y * y);
			}
			y_inds[vals_y] = height;
			// now print
			ssi_rect_t r;
			for (ssi_int_t x = 0; x < vals_x; x++) {
				ssi_real_t *data_scaled_ptr = data_buffer_scaled + x * vals_y;
				r.left = x_inds[x];
				r.width = x_inds[x + 1] - r.left;
				for (ssi_int_t y = 0; y < vals_y; y++) {
					r.top = y_inds[y];
					r.height = y_inds[y + 1] - r.top;
					_painter->fill(*_colormap->getBrush(*(data_scaled_ptr + y)), r);
				}
			}
			delete[] x_inds;
			delete[] y_inds;

		}
	}
}

void PaintSignal::setColormap(IColormap::COLORMAP::List colormap) {

	Lock lock(_mutex);

	delete _colormap;
	_colormap = new Colormap(colormap);
}

void PaintSignal::setPen(ssi_rgb_t color, IPainter::ITool::WIDTH width, IPainter::ITool::LINE_STYLES::List style) {

	Lock lock(_mutex);

	delete _pen;
	_pen = new Painter::Pen(color, width, style);
}

void PaintSignal::setBrush(ssi_rgb_t color) {

	Lock lock(_mutex);

	delete _brush;
	_brush = new Painter::Brush(color);
}

void PaintSignal::setBackground(ssi_rgb_t color) {

	Lock lock(_mutex);

	delete _back_brush;
	_back_brush = new Painter::Brush(color);
}

void PaintSignal::setFont(const ssi_char_t *name, ssi_rgb_t fore, ssi_rgb_t back, IPainter::ITool::WIDTH size, IPainter::ITool::FONT_STYLE style) {

	Lock lock(_mutex);

	delete _font;
	_font = new Painter::Font(name, size, style);
	delete _font_pen;
	_font_pen = new Painter::Pen(fore);
	delete _font_brush;
	_font_brush = new Painter::Brush(back);
}

void PaintSignal::setPointSize(ssi_real_t point) {

	Lock lock(_mutex);

	_point_size = point;
}

void PaintSignal::setLimits(ssi_real_t minval, ssi_real_t maxval) {

	Lock lock(_mutex);

	delete[] data_min;
	delete[] data_max;
	data_min = new ssi_real_t[sample_dimension];
	data_max = new ssi_real_t[sample_dimension];

	for (ssi_size_t i = 0; i < sample_dimension; i++) {
		data_min[i] = minval;
		data_max[i] = maxval;
	}

	fixed_min_max = true;
}

void PaintSignal::setPrecision(ssi_size_t precision) {

	Lock lock(_mutex);

	axis_precision = precision;
}

void PaintSignal::setIndices(ssi_size_t indx, ssi_size_t indy) {

	Lock lock(_mutex);

	_x_ind = indx;
	_y_ind = indy;
}


}
