// PaintData.h
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

#ifndef SSI_GRAPHIC_PAINTDATA_H
#define SSI_GRAPHIC_PAINTDATA_H

#include "base/ICanvas.h"
#include "thread/Lock.h"
#include "base/IPainter.h"

namespace ssi {

class PaintData : public ICanvasClient {

public:

	struct TYPE {
		enum List {
			SIGNAL = 0,
			AUDIO,
			PATH,
			SCATTER,
			IMAGE,
		};
	};

public:

	PaintData ();
	virtual ~PaintData ();

	void create(ICanvas *parent);
	void paint (ssi_handle_t context, ssi_rect_t area);
	void setData (ssi_stream_t &stream, TYPE::List type);
	void resetLimits();
	void close();

	void keyDown(IWindow::KEY key, IWindow::KEY vkey);
	void setColormap(IColormap::COLORMAP::List colormap);
	void setLimits(ssi_real_t minval, ssi_real_t maxval);
	void setLabel(const ssi_char_t *label);
	void setPrecision(ssi_size_t precision);
	void setIndices(ssi_size_t indx, ssi_size_t indy);

	void setPen(ssi_rgb_t color, IPainter::ITool::WIDTH width = 1, IPainter::ITool::LINE_STYLES::List style = IPainter::ITool::LINE_STYLES::SOLID);
	void setBrush(ssi_rgb_t color);
	void setBackground(bool toogle, ssi_rgb_t color);
	void setFont(const ssi_char_t *name, ssi_rgb_t fore, ssi_rgb_t back, IPainter::ITool::WIDTH size, IPainter::ITool::FONT_STYLE style);
	void setPointSize(ssi_real_t point);

protected:

	TYPE::List _type;
	ssi_stream_t _stream; 
	ssi_stream_t _resample;
	Mutex _mutex;	
	ssi_size_t _dim;
	ssi_real_t *_mins;
	ssi_real_t *_maxs;
	ssi_real_t _fix_min, _fix_max;
	ssi_size_t _precision;
	bool _fix_limits;
	ssi_size_t _indx, _indy;
	
	ICanvas *_parent;
	IPainter *_painter;
	IColormap *_colormap;
	IPainter::ITool *_pen;
	IPainter::ITool *_brush;
	IPainter::ITool *_back_brush;
	bool _back_toggle;
	IPainter::ITool *_font;
	IPainter::ITool *_font_pen;
	IPainter::ITool *_font_brush;
	ssi_real_t _point_size;

	bool _reset_limits;
	bool _stream_has_changed;

	char _label[SSI_MAX_CHAR];
};

}

#endif
