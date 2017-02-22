// IColormap.h
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

#ifndef SSI_ICOLORMAP_H
#define SSI_ICOLORMAP_H

#include "base/IPainter.h"
/**
 * @brief a ssi colormap sets a canvas' color mode and lets you get a canvas' current tool.
 */
namespace ssi {

class IColormap
{

public:

	struct COLORMAP {
		enum List : unsigned char {
			BLACKANDWHITE = 0,
			COLOR16,
			COLOR64,
			GRAY64
		};
	};

	virtual ~IColormap() {};

	virtual ssi_rgb_t getColor (ssi_size_t index) = 0;
	virtual IPainter::ITool *getPen(ssi_size_t index) = 0;
	virtual IPainter::ITool *getBrush(ssi_size_t index) = 0;
	virtual ssi_rgb_t getColor(ssi_real_t index) = 0;
	virtual IPainter::ITool *getPen(ssi_real_t index) = 0;
	virtual IPainter::ITool *getBrush(ssi_real_t index) = 0;
};

}

#endif // _THECOLORMAP_H
