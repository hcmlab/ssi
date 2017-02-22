// PaintPoints.h
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

#ifndef SSI_GRAPHIC_PAINTPOINTS_H
#define SSI_GRAPHIC_PAINTPOINTS_H

#include "base/ICanvas.h"
#include "thread/Lock.h"
#include "base/IPainter.h"

namespace ssi {

struct PaintPointsType {
	enum TYPE : unsigned char {
		DOTS = 0,
		LINES
	};
};

class PaintPoints : public ICanvasClient {

public:

	PaintPoints(PaintPointsType::TYPE type,
		bool relative,
		bool swap,
		bool draw_labels,
		bool draw_background);
	virtual ~PaintPoints ();

	void create(ICanvas *parent);
	void close();
	void paint (ssi_handle_t context, ssi_rect_t area);
	void clear();
	// remember that a point has two coordinates, i.e. dimension of points has to be 2*n_points!!
	void setData(ssi_size_t n_points, ssi_real_t *points, ssi_time_t time);
	void setData(ssi_stream_t &stream, ssi_time_t time);
	void setPrecision(ssi_size_t precision);

protected:

	void paintPointsAsDots(ssi_rect_t area);	
	void paintPointsAsLines(ssi_rect_t area);
	void paintPositionLabels(ssi_rect_t area);

	void setPen(ssi_rgb_t color, IPainter::ITool::WIDTH width = 1, IPainter::ITool::LINE_STYLES::List style = IPainter::ITool::LINE_STYLES::SOLID);
	void setBrush(ssi_rgb_t color);
	void setBackground(ssi_rgb_t color);
	void setFont(const ssi_char_t *name, ssi_rgb_t fore, ssi_rgb_t back, IPainter::ITool::WIDTH size, IPainter::ITool::FONT_STYLE style);
	void setPointSize(ssi_real_t point);

	Mutex _mutex;
	PaintPointsType::TYPE _type;

	ssi_size_t _n_points;
	ssi_pointf_t *_points;

	ICanvas *_parent;
	IPainter *_painter;
	IPainter::ITool *_pen;
	IPainter::ITool *_brush;
	IPainter::ITool *_back_brush;
	IPainter::ITool *_font;
	IPainter::ITool *_font_pen;
	IPainter::ITool *_font_brush;
	ssi_real_t _point_size;
	ssi_size_t _precision;

	bool _relative;
	bool _swap;
	bool _draw_back;
	bool _draw_points;	
	bool _draw_labels;

	static const ssi_char_t *ssi_log_name;
};

}

#endif
