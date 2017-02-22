// Colormap.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2007/11/15
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

#ifndef SSI_GRAPHIC_COLORMAP_H
#define SSI_GRAPHIC_COLORMAP_H

#include "base/IColormap.h"

namespace ssi {

class Colormap : public IColormap {

public:

	Colormap(ssi_rgb_t color);
	Colormap(ssi_size_t n_colors, ssi_rgb_t *colors);
	Colormap(COLORMAP::List type);
	~Colormap ();

	ssi_rgb_t getColor(ssi_size_t index);
	IPainter::ITool *getPen(ssi_size_t index);
	IPainter::ITool *getBrush(ssi_size_t index);
	ssi_rgb_t getColor(ssi_real_t index);
	IPainter::ITool *getPen(ssi_real_t index);
	IPainter::ITool *getBrush(ssi_real_t index);

private:

	void loadDefault(COLORMAP::List type);
	void load (const unsigned char *colormap);

	ssi_size_t _n_colors;
	IPainter::ITool **_pens;
	IPainter::ITool **_brushes;
	ssi_rgb_t *_colors;
};

}

#endif
