// GraphicTools.h
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

#ifndef SSI_GRAPHIC_GRAPHICTOOLS_H
#define SSI_GRAPHIC_GRAPHICTOOLS_H

#include "base/IPainter.h"
#include "base/IColormap.h"

namespace ssi {

class GraphicTools {

public:

	static void PaintAsSignal(ssi_stream_t &stream, IPainter &painter, IPainter::ITool &pen);
	static void PaintAsSignal(ssi_stream_t &stream, ssi_size_t index, IPainter &painter, IPainter::ITool &pen);

	static void PaintAsAudio(ssi_stream_t &stream, IPainter &painter, IPainter::ITool &pen);
	static void PaintAsAudio(ssi_stream_t &stream, ssi_size_t index, IPainter &painter, IPainter::ITool &pen);
	
	static void PaintAsPath(ssi_stream_t &stream, ssi_size_t index_x, ssi_size_t index_y, IPainter &painter, IPainter::ITool &pen);
	static void PaintAsScatter(ssi_stream_t &stream, ssi_size_t index_x, ssi_size_t index_y, IPainter &painter, IPainter::ITool &pen, IPainter::ITool &brush, ssi_real_t diameter);

	static void PaintAsImage(ssi_stream_t &stream, IPainter &painter, IColormap &colormap);

	static void PaintYAxis(ssi_real_t ymin, ssi_real_t ymax, IPainter &painter, IPainter::ITool &font, IPainter::ITool &pen, IPainter::ITool &brush, ssi_size_t precision);
	static void PaintAxis(ssi_size_t n_tracks, ssi_time_t xmin, ssi_time_t xmax, ssi_real_t *ymin, ssi_real_t *ymax, IPainter &painter, IPainter::ITool &font, IPainter::ITool &pen, IPainter::ITool &brush, ssi_size_t precision);

protected:

	static ssi_char_t *ssi_log_name_static;

};

}

#endif
