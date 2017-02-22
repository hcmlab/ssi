// GraphicTools.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 30/03/15
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

#include "graphic/GraphicTools.h"
#include "graphic/Painter.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif


namespace ssi {

ssi_char_t *GraphicTools::ssi_log_name_static = "gtools____";

void GraphicTools::PaintAsSignal(ssi_stream_t &stream, IPainter &painter, IPainter::ITool &pen) {

	if (stream.type != SSI_REAL) {
		ssi_wrn_static("type is not supported '%s'", SSI_TYPE_NAMES[stream.type]);
		return;
	}

	ssi_rect_t area = painter.getArea();
	ssi_size_t width = ssi_cast(ssi_size_t, area.width);

	ssi_stream_t *resample = &stream;
	if (width != stream.num) {
		resample = new ssi_stream_t;
		ssi_stream_init(*resample, width, stream.dim, stream.byte, stream.type, stream.sr);
		ssi_resample(stream.num, resample->num, stream.dim, ssi_pcast(ssi_real_t, stream.ptr), ssi_pcast(ssi_real_t, resample->ptr), ssi_mean);
	}

	ssi_size_t d = stream.dim;
	ssi_real_t height = ssi_cast(ssi_real_t, area.height) / d;
	ssi_rect_t a;	
	a.left = area.left;
	a.top = 0;
	a.width = area.width;

	for (ssi_size_t i = 0; i < d; i++) {		
		a.height = ssi_cast(ssi_size_t, height * (i + 1) + 0.5) - ssi_cast(ssi_size_t, height * i + 0.5);		
		painter.setArea(a);
		PaintAsSignal(*resample, i, painter, pen);
		a.top += a.height;
	}

	painter.setArea(area);

	if (width != stream.num) {
		ssi_stream_destroy(*resample);
		delete resample;
	}
}

void GraphicTools::PaintAsSignal(ssi_stream_t &stream, ssi_size_t index, IPainter &painter, IPainter::ITool &pen) {

	if (stream.type != SSI_REAL) {
		ssi_wrn_static("type is not supported '%s'", SSI_TYPE_NAMES[stream.type]);
		return;
	}	

	ssi_rect_t area = painter.getArea();
	ssi_size_t width = ssi_cast(ssi_size_t, area.width);

	ssi_stream_t *resample = &stream;
	if (width != stream.num) {
		resample = new ssi_stream_t;
		ssi_stream_init(*resample, width, stream.dim, stream.byte, stream.type, stream.sr);
		ssi_resample(stream.num, resample->num, stream.dim, ssi_pcast(ssi_real_t, stream.ptr), ssi_pcast(ssi_real_t, resample->ptr), ssi_mean);		
	}
	
	ssi_size_t n = resample->num;
	ssi_size_t d = resample->dim;
	ssi_pointf_t from, to;
	ssi_real_t delta = 1.0f / width;
	
	ssi_real_t *ptr = ssi_pcast(ssi_real_t, resample->ptr) + index;
	to.x = 0;
	to.y = *ptr;
	for (ssi_size_t j = 1; j < n; j++) {
		from = to;
		to.x += delta;
		ptr += d;		
		to.y = 1.0f - *ptr;
		painter.line(pen, from, to, true);
	}
	
	if (width != stream.num) {
		ssi_stream_destroy(*resample);
		delete resample;
	}
}


void GraphicTools::PaintAsAudio(ssi_stream_t &stream, IPainter &painter, IPainter::ITool &pen) {

	if (stream.type != SSI_REAL) {
		ssi_wrn_static("type is not supported '%s'", SSI_TYPE_NAMES[stream.type]);
		return;
	}

	ssi_rect_t area = painter.getArea();
	ssi_size_t width = ssi_cast(ssi_size_t, area.width);

	ssi_stream_t *resample = &stream;
	if (width != stream.num) {
		resample = new ssi_stream_t;
		ssi_stream_init(*resample, width, stream.dim, stream.byte, stream.type, stream.sr);
		ssi_resample(stream.num, resample->num, stream.dim, ssi_pcast(ssi_real_t, stream.ptr), ssi_pcast(ssi_real_t, resample->ptr), ssi_max);
	}

	ssi_size_t d = stream.dim;
	ssi_real_t height = ssi_cast(ssi_real_t, area.height) / d;
	ssi_rect_t a;
	a.left = area.left;
	a.top = 0;
	a.width = area.width;

	for (ssi_size_t i = 0; i < d; i++) {
		a.height = ssi_cast(ssi_size_t, height * (i + 1) + 0.5) - ssi_cast(ssi_size_t, height * i + 0.5);
		painter.setArea(a);
		PaintAsAudio(*resample, i, painter, pen);
		a.top += a.height;
	}

	painter.setArea(area);

	if (width != stream.num) {
		ssi_stream_destroy(*resample);
		delete resample;
	}
}

void GraphicTools::PaintAsAudio(ssi_stream_t &stream, ssi_size_t index, IPainter &painter, IPainter::ITool &pen) {

	if (stream.type != SSI_REAL) {
		ssi_wrn_static("type is not supported '%s'", SSI_TYPE_NAMES[stream.type]);
		return;
	}

	ssi_rect_t area = painter.getArea();
	ssi_size_t width = ssi_cast(ssi_size_t, area.width);

	ssi_stream_t *resample = &stream;
	if (width != stream.num) {
		resample = new ssi_stream_t;
		ssi_stream_init(*resample, width, stream.dim, stream.byte, stream.type, stream.sr);
		ssi_resample(stream.num, resample->num, stream.dim, ssi_pcast(ssi_real_t, stream.ptr), ssi_pcast(ssi_real_t, resample->ptr), ssi_max);
	}

	ssi_size_t n = resample->num;
	ssi_size_t d = resample->dim;
	ssi_pointf_t from, to;
	ssi_real_t delta = 1.0f / width;

	ssi_real_t *ptr = ssi_pcast(ssi_real_t, resample->ptr) + index;
	from.x = to.x = 0;
	for (ssi_size_t j = 0; j < n; j++) {		
		from.y = *ptr;//(*ptr * 0.5f) + 0.5f;
		to.y = 1.0f - from.y;		
		painter.line(pen, from, to, true); 
		ptr += d;
		to.x = from.x = from.x + delta;
	}
	painter.line(pen, ssi_pointf(0.0f,0.5f), ssi_pointf(1.0f,0.5f), true);

	if (width != stream.num) {
		ssi_stream_destroy(*resample);
		delete resample;
	}
}

void GraphicTools::PaintAsPath(ssi_stream_t &stream, ssi_size_t index_x, ssi_size_t index_y, IPainter &painter, IPainter::ITool &pen) {

	if (stream.type != SSI_REAL) {
		ssi_wrn_static("type is not supported '%s'", SSI_TYPE_NAMES[stream.type]);
		return;
	}

	ssi_rect_t area = painter.getArea();
	ssi_size_t width = ssi_cast(ssi_size_t, area.width);
	ssi_size_t height = ssi_cast(ssi_size_t, area.height);

	ssi_size_t n = stream.num;
	ssi_size_t d = stream.dim;
	ssi_pointf_t from, to;
	ssi_real_t delta = 1.0f / width;

	ssi_real_t *ptr = ssi_pcast(ssi_real_t, stream.ptr);
	from.x = ptr[index_x];
	from.y = ptr[index_y];
	for (ssi_size_t i = 1; i < n; i++) {
		to.x = ptr[index_x];
		to.y = ptr[index_y];
		painter.line(pen, from, to, true);
		from = to;
		ptr += d;
	}
}

void GraphicTools::PaintAsScatter(ssi_stream_t &stream, ssi_size_t index_x, ssi_size_t index_y, IPainter &painter, IPainter::ITool &pen, IPainter::ITool &brush, ssi_real_t diameter) {

	if (stream.type != SSI_REAL) {
		ssi_wrn_static("type is not supported '%s'", SSI_TYPE_NAMES[stream.type]);
		return;
	}

	ssi_rect_t area = painter.getArea();
	ssi_size_t width = ssi_cast(ssi_size_t, area.width);
	ssi_size_t height = ssi_cast(ssi_size_t, area.height);

	ssi_size_t n = stream.num;
	ssi_size_t d = stream.dim;
	ssi_rectf_t rect;
	ssi_real_t delta = 1.0f / width;

	ssi_real_t *ptr = ssi_pcast(ssi_real_t, stream.ptr);	
	rect.width = diameter / width;
	rect.height = diameter / height;
	for (ssi_size_t i = 0; i < n; i++) {
		rect.left = ptr[index_x];
		rect.top = ptr[index_y];
		painter.ellipse(pen, brush, rect, true);		
		ptr += d;
	}
}

void GraphicTools::PaintAsImage(ssi_stream_t &stream, IPainter &painter, IColormap &colormap) {
	
	if (stream.type != SSI_REAL) {
		ssi_wrn_static("type is not supported '%s'", SSI_TYPE_NAMES[stream.type]);
		return;
	}

	ssi_rect_t area = painter.getArea();
	ssi_size_t width = ssi_cast (ssi_size_t, area.width);
	ssi_size_t height = ssi_cast (ssi_size_t, area.height);
	ssi_size_t d = stream.dim;
	ssi_size_t n = stream.num;
	ssi_real_t *ptr = ssi_pcast(ssi_real_t, stream.ptr);

	// calculate scale factors
	ssi_real_t scale_x = ssi_cast(ssi_real_t, n) / width;
	ssi_real_t scale_y = ssi_cast(ssi_real_t, d) / height;

	// plot _data_scaled
	if (scale_x >= 1) {
		if (scale_y >= 1) {

			// more points than pixels in x and y direction

			// determine row indices
			int *x_inds = new int[width];
			for (ssi_size_t x = 0; x < width; x++) {
				x_inds[x] = (int)(scale_x * x);
			}
			// determine col indices
			int *y_inds = new int[height];
			for (ssi_size_t y = 0; y < height; y++) {
				y_inds[y] = (int)(scale_y * y);
			}
			// now print
			for (ssi_size_t x = 0; x < width; x++) {
				ssi_real_t *tmp = ptr + x_inds[x] * d;
				for (ssi_size_t y = 0; y < height; y++) {
					painter.pixel(*colormap.getPen(*(tmp + y_inds[y])), ssi_point(x, y));
				}
			}
			delete[] x_inds;
			delete[] y_inds;

		}
		else {

			// more points in x direction but less in y
			scale_y = 1 / scale_y;

			// determine row indices
			int *x_inds = new int[width];
			for (ssi_size_t x = 0; x < width; x++) {
				x_inds[x] = (int)(scale_x * x);
			}
			// determine col indices
			int *y_inds = new int[d + 1];
			for (ssi_size_t y = 0; y < d; y++) {
				y_inds[y] = (int)(scale_y * y);
			}
			y_inds[d] = height;
			// now print
			ssi_rect_t r;
			for (ssi_size_t x = 0; x < width; x++) {
				ssi_real_t *tmp = ptr + x_inds[x] * d;
				r.left = x;
				r.width = 1;
				for (ssi_size_t y = 0; y < d; y++) {
					r.top = y_inds[y];
					r.height = y_inds[y + 1] - r.top;
					painter.rect(*colormap.getPen(*(tmp + y)), *colormap.getBrush(*(tmp + y)), r);
				}
			}
			delete[] x_inds;
			delete[] y_inds;

		}
	}
	else {

		if (scale_y >= 1) {

			// more points than pixels in y direction but less in x
			scale_x = 1 / scale_x;

			// determine col indices
			int *x_inds = new int[n + 1];
			for (ssi_size_t x = 0; x < n; x++) {
				x_inds[x] = (int)(scale_x * x);
			}
			x_inds[n] = width;
			// determine col indices
			int *y_inds = new int[height];
			for (ssi_size_t y = 0; y < height; y++) {
				y_inds[y] = (int)(scale_y * y);
			}
			// now print
			ssi_rect_t r;
			for (ssi_size_t x = 0; x < n; x++) {
				ssi_real_t *tmp = ptr + x * d;
				r.left = x_inds[x];
				r.width = x_inds[x + 1] - r.left;
				for (ssi_size_t y = 0; y < height; y++) {
					r.top = y;
					r.height = 1;
					painter.rect(*colormap.getPen(*(tmp + y_inds[y])), *colormap.getBrush(*(tmp + y_inds[y])), r);
				}
			}
			delete[] x_inds;
			delete[] y_inds;

		}
		else {

			// less points in x and y direction
			scale_x = 1 / scale_x;
			scale_y = 1 / scale_y;

			// determine col indices
			int *x_inds = new int[n + 1];
			for (ssi_size_t x = 0; x < n; x++) {
				x_inds[x] = (int)(scale_x * x);
			}
			x_inds[n] = width;
			// determine col indices
			int *y_inds = new int[d + 1];
			for (ssi_size_t y = 0; y < d; y++) {
				y_inds[y] = (int)(scale_y * y);
			}
			y_inds[d] = height;
			// now print
			ssi_rect_t r;
			for (ssi_size_t x = 0; x < n; x++) {
				ssi_real_t *tmp = ptr + x * d;
				r.left = x_inds[x];
				r.width = x_inds[x + 1] - r.left;
				for (ssi_size_t y = 0; y < d; y++) {
					r.top = y_inds[y];
					r.height = y_inds[y + 1] - r.top;
					painter.rect(*colormap.getPen(*(tmp + y)), *colormap.getBrush(*(tmp + y)), r);
				}
			}
			delete[] x_inds;
			delete[] y_inds;

		}
	}
}

void GraphicTools::PaintYAxis(ssi_real_t ymin, ssi_real_t ymax, IPainter &painter, IPainter::ITool &font, IPainter::ITool &pen, IPainter::ITool &brush, ssi_size_t precision) {

	ssi_char_t str[SSI_MAX_CHAR];

	ssi_rect_t area = painter.getArea();
	ssi_pointf_t pos;

	ssi_sprint(str, "%.*f", precision, ymax);
	pos.x = 0;
	pos.y = 0;
	painter.text(font, pen, brush, pos, str, true, IPainter::TEXT_ALIGN_HORZ::LEFT, IPainter::TEXT_ALIGN_VERT::TOP);
	
	ssi_sprint(str, "%.*f", precision, ymin);
	pos.x = 0;
	pos.y = 1;
	painter.text(font, pen, brush, pos, str, true, IPainter::TEXT_ALIGN_HORZ::LEFT, IPainter::TEXT_ALIGN_VERT::BOTTOM);
}

void GraphicTools::PaintAxis(ssi_size_t n_tracks, ssi_time_t xmin, ssi_time_t xmax, ssi_real_t *ymin, ssi_real_t *ymax, IPainter &painter, IPainter::ITool &font, IPainter::ITool &pen, IPainter::ITool &brush, ssi_size_t precision) {

	ssi_char_t str[SSI_MAX_CHAR];

	ssi_rect_t area = painter.getArea();
	ssi_pointf_t pos;
	ssi_size_t d = n_tracks;
	ssi_real_t height = ssi_cast(ssi_real_t, area.height) / d;

	ssi_sprint(str, "%.*lfs", precision, xmax);
	pos.x = 1;
	pos.y = 0;
	painter.text(font, pen, brush, pos, str, true, IPainter::TEXT_ALIGN_HORZ::RIGHT, IPainter::TEXT_ALIGN_VERT::TOP);

	ssi_sprint(str, "%.*lfs", precision, xmin);
	pos.x = 1;
	pos.y = 1;
	painter.text(font, pen, brush, pos, str, true, IPainter::TEXT_ALIGN_HORZ::RIGHT, IPainter::TEXT_ALIGN_VERT::BOTTOM);

	ssi_rect_t a;
	a.left = area.left;
	a.top = 0;
	a.width = area.width;

	for (ssi_size_t i = 0; i < d; i++) {
		a.height = ssi_cast(ssi_size_t, height * (i + 1) + 0.5) - ssi_cast(ssi_size_t, height * i + 0.5);
		painter.setArea(a);		
		PaintYAxis(ymin[i], ymax[i], painter, font, pen, brush, precision);
		if (i != 0){			
			painter.line(pen, ssi_pointf(0, 0), ssi_pointf(1, 0), true);
		}
		a.top += a.height;
	}

	painter.setArea(area);
}

}

