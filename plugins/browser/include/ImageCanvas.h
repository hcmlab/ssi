// ImageCanvas.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 29/11/2015
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

#pragma once

#ifndef SSI_BROWSER_IMAGECANVAS_H
#define SSI_BROWSER_IMAGECANVAS_H

#include "graphic/Canvas.h"

namespace Gdiplus {
	class Bitmap;
}

namespace ssi {

class ImageCanvas : public ICanvasClient {

public:

	ImageCanvas();
	~ImageCanvas ();

	void create(ICanvas *parent);
	void close();
	void paint(ssi_handle_t context, ssi_rect_t area);
	void navigate(const ssi_char_t *img);

	void setDefaultDirectory(const ssi_char_t *dir);
	void setDefaultExtension(const ssi_char_t *ext);

protected:

	static char ssi_log_name[];

	Gdiplus::Bitmap *_bitmap;
	ULONG_PTR _gdiplusToken;
	HBRUSH _backBrush;
	ICanvas *_parent;

	ssi_char_t *_directory;
	ssi_char_t *_extension;
};

}

#endif // _CANVASTHREAD_H
