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

#include "ImageCanvas.h"
#include "base/String.h"

#include <ole2.h>
#include <commdlg.h>
#include <gdiplus.h>

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

char ImageCanvas::ssi_log_name[] = "imgcanvas_";

ImageCanvas::ImageCanvas()
	: _parent(0),
	_bitmap(0),
	_directory (0),
	_extension (0) {

	Gdiplus::GdiplusStartupInput gdiplusStartupInput;		
	Gdiplus::GdiplusStartup(&_gdiplusToken, &gdiplusStartupInput, NULL);
	_backBrush = ::CreateSolidBrush(RGB(0, 0, 0));
}

ImageCanvas::~ImageCanvas () {

	::DeleteObject(_backBrush);
	delete _bitmap;
	Gdiplus::GdiplusShutdown(_gdiplusToken);	

	delete[] _directory; _directory = 0;
	delete[] _extension; _extension = 0;
}

void ImageCanvas::create(ICanvas *parent) {
	_parent = parent;
}

void ImageCanvas::close() {
}

void ImageCanvas::paint(ssi_handle_t context, ssi_rect_t area) {

	RECT rect;
	rect.left = area.left;
	rect.right = area.left + area.width;
	rect.top = area.top;
	rect.bottom = area.top + area.height;

	::FillRect((HDC)context, &rect, _backBrush);

	if (_bitmap) {
		
		Gdiplus::Graphics graphics((HDC)context);
		graphics.SetInterpolationMode(Gdiplus::InterpolationMode::InterpolationModeNearestNeighbor); //faster

		float canvas_width = (float)(rect.right - rect.left);
		float canvas_height = (float)(rect.bottom - rect.top);
		float bitmap_width = (float)_bitmap->GetWidth();
		float bitmap_height = (float)_bitmap->GetHeight();

		float scale_width = canvas_width / bitmap_width;
		float scale_height = canvas_height / bitmap_height;
		float scale = min(scale_width, scale_height);

		float width = scale * bitmap_width;
		float height = scale * bitmap_height;
		float left = (canvas_width - width) / 2.0f;
		float top = (canvas_height - height) / 2.0f;
		
		graphics.DrawImage(_bitmap, ssi_cast(int, left + 0.5f), ssi_cast(int, top + 0.5f), ssi_cast(int, width + 0.5f), ssi_cast(int, height + 0.5f));
	}
}

void ImageCanvas::setDefaultDirectory(const ssi_char_t *dir) {
	
	delete[] _directory;

	if (dir[ssi_strlen(dir)] == '\\') {
		_directory = ssi_strcpy(dir);
	} else {
		_directory = ssi_strcat(dir, "\\");
	}

}
void ImageCanvas::setDefaultExtension(const ssi_char_t *ext) {

	delete[] _extension;

	if (ext[0] == '.') {
		_extension = ssi_strcpy(ext);
	}
	else {
		_extension = ssi_strcat(".", ext);
	}
}

void ImageCanvas::navigate(const ssi_char_t *img) {

	if (!img || img[0] == '\0') {
		delete _bitmap; _bitmap = 0;
		return;
	}

	String path(img);

	if (!ssi_exists(path.str())) {
		if (_directory) {
			path = _directory;
			path += img;
			if (!ssi_exists(path.str())) {
				if (_extension) {
					path += _extension;
				}
			}
		}
	}

	if (ssi_exists(path.str())) {
		delete _bitmap; _bitmap = 0;
		wchar_t *wimg = ssi_char2wchar(path.str());
		_bitmap = Gdiplus::Bitmap::FromFile(wimg);
		delete[] wimg;		
	} else {
		ssi_wrn("image not found '%s'", path.str());
	}

}


}
