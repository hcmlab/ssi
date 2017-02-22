// PaintBars.h
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

#pragma once

#ifndef SSI_GRAPHIC_PAINTEVENT_H
#define SSI_GRAPHIC_PAINTEVENT_H

#include "base/IObject.h"
#include "thread/Thread.h"
#include "thread/Lock.h"
#include "base/ICanvas.h"
#include "base/IPainter.h"

namespace ssi {

class PaintBars : public ICanvasClient {

public:

	struct TYPE {
		enum List {
			BAR = 0,
			BAR_POS
		};
	};

	PaintBars (TYPE::List type);
	~PaintBars ();

	void create(ICanvas *parent);
	void close();
	void paint(ssi_handle_t hdc, ssi_rect_t rect);
	void setData (ssi_event_t &e);
	void setData (ssi_stream_t &s);
	void reset();
	void setWindowCaption (ssi_char_t *caption);
	void setExternalAxisCaptions(ssi_size_t n_captions, ssi_char_t **captions);
	void setFixedLimit(ssi_real_t fix);
	void setGlobalLimit(bool global);
	void setPrecision(ssi_size_t precision);
	void keyDown(IWindow::KEY key, IWindow::KEY vkey);

protected:

	static ssi_char_t *ssi_log_name;
	static int ssi_log_level;

	void setPen(ssi_rgb_t color, IPainter::ITool::WIDTH width = 1, IPainter::ITool::LINE_STYLES::List style = IPainter::ITool::LINE_STYLES::SOLID);
	void setBrush(ssi_rgb_t color);
	void setBackground(ssi_rgb_t color);
	void setFont(const ssi_char_t *name, ssi_rgb_t fore, ssi_rgb_t back, IPainter::ITool::WIDTH size, IPainter::ITool::FONT_STYLE style);

	void resetValues();
	void initAxisCaption();
	void releaseAxisCaption();
	void releaseExternalAxisCaption();
	void setAxisCaption(ssi_size_t dim, const ssi_char_t* caption);
	void setEventCaption(ssi_size_t event_id, ssi_size_t sender_id);

	void paintBack(ssi_handle_t context, ssi_rect_t area);
	void paintGrid(ssi_handle_t context, ssi_rect_t area);
	void paintBars(ssi_handle_t context, ssi_rect_t area);

	Mutex _mutex;	
	int _p_perdim;	
	TYPE::List _type;

	ssi_size_t _dim;
	ssi_real_t *_values;
	ssi_real_t *_values_real;
	ssi_real_t *_limits;
	bool _fixed_limit;	
	bool _global_limit;
	ssi_real_t _global_limit_value;
	ssi_size_t _precision;
	ssi_char_t *_event_caption;	
	ssi_char_t **_axis_captions;
	ssi_size_t _n_external_axis_captions;
	ssi_char_t **_external_axis_captions;
	ssi_char_t *_window_caption;

	ICanvas *_parent;
	IPainter *_painter;
	IPainter::ITool *_pen;
	IPainter::ITool *_brush;
	IPainter::ITool *_back_brush;
	IPainter::ITool *_font;
	IPainter::ITool *_font_pen;
	IPainter::ITool *_font_brush;

};

};

#endif

