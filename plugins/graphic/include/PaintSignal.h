// PaintSignal.h
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

#ifndef SSI_GRAPHIC_PAINTSIGNAL_H
#define SSI_GRAPHIC_PAINTSIGNAL_H

#include "base/ICanvas.h"
#include "thread/Lock.h"
#include "base/IPainter.h"

namespace ssi {

struct PaintSignalType {
	enum TYPE : unsigned char {
		SIGNAL = 0,
		IMAGE,
		AUDIO,
		PATH,
		BAR,
		BAR_POS
	};
};

class PaintSignal : public ICanvasClient {

public:

	PaintSignal(ssi_time_t sr,
		ssi_size_t dim,
		ssi_time_t size_in_s,
		PaintSignalType::TYPE type,
		ssi_size_t x_ind,
		ssi_size_t y_ind,
		bool static_image);
	virtual ~PaintSignal ();

	void create(ICanvas *parent);
	void close();
	void paint (ssi_handle_t context, ssi_rect_t area);
	void clear();
	void setData(ssi_stream_t &stream, ssi_time_t time);
	void resetLimits();

	void keyDown(IWindow::KEY key, IWindow::KEY vkey);

	void setColormap(IColormap::COLORMAP::List colormap);
	void setLimits(ssi_real_t minval, ssi_real_t maxval);
	void setLabel(const ssi_char_t *label);
	void setPrecision(ssi_size_t precision);
	void setIndices(ssi_size_t indx, ssi_size_t indy);

protected:

	static ssi_real_t mymax(ssi_real_t *data, ssi_size_t size, ssi_size_t dim, int old_ind, int new_ind);
	static ssi_real_t mean(ssi_real_t *data, ssi_size_t size, ssi_size_t dim, int old_ind, int new_ind);

	void setPen(ssi_rgb_t color, IPainter::ITool::WIDTH width = 1, IPainter::ITool::LINE_STYLES::List style = IPainter::ITool::LINE_STYLES::SOLID);
	void setBrush(ssi_rgb_t color);
	void setBackground(ssi_rgb_t color);
	void setFont(const ssi_char_t *name, ssi_rgb_t fore, ssi_rgb_t back, IPainter::ITool::WIDTH size, IPainter::ITool::FONT_STYLE style);
	void setPointSize(ssi_real_t point);

	void paintAsPath(ssi_rect_t area);
	void paintAsSignal(ssi_rect_t area);
	void paintAsImage(ssi_rect_t area);
	void paintAsAudio(ssi_rect_t area);
	void paintAxis(ssi_rect_t area);

	Mutex _mutex;
	PaintSignalType::TYPE _type;
	ssi_stream_t _convert;
	ssi_real_t *data_buffer;
	ssi_real_t *data_buffer_scaled;
	ssi_real_t *scale, scale_tot;
	ssi_size_t data_buffer_position, data_buffer_size, data_buffer_size_real;
	ssi_size_t sample_dimension, sample_number_total, sample_number_total_real;
	ssi_time_t sample_rate, window_size;
	ssi_real_t *data_min, data_min_tot;
	ssi_real_t *data_max, data_max_tot;
	ssi_time_t time_in_s;
	bool fixed_min_max;
	bool reset_min_max;
	bool discrete_plot;
	ssi_size_t _x_ind;
	ssi_size_t _y_ind;
	bool _static_image;
	ssi_size_t axis_precision;
	
	ICanvas *_parent;
	IPainter *_painter;
	IColormap *_colormap;
	IPainter::ITool *_pen;
	IPainter::ITool *_brush;
	IPainter::ITool *_back_brush;
	IPainter::ITool *_font;
	IPainter::ITool *_font_pen;
	IPainter::ITool *_font_brush;
	ssi_real_t _point_size;

	bool _draw_stream;	
};

}

#endif
