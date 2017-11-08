// CameraScreen.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/11/25
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

#include "CameraScreen.h"
#include "CameraTools.h"
#include "base/Factory.h"
#include "image/include/CVResize.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi
{

int CameraScreen::ssi_log_level_static = SSI_LOG_LEVEL_DEFAULT;
static char ssi_log_name_static[] = "camscreen_";
char *CameraScreen::ssi_log_name = "camscreen_";

CameraScreen::CameraScreen (const ssi_char_t *file)
	: _provider (0),
	_buffer (0),
	_buffer_size (0),
	_org_buffer (0),
	_org_buffer_size (0),
	_rsz_filter (0),
	_rsz_buffer (0),
	_rsz_buffer_size (0),
	_flip_buffer (0),
	_timer (0),
	_file (0),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
}

CameraScreen::~CameraScreen() {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}

	delete[] _org_buffer;
	delete[] _rsz_buffer;
	delete _rsz_filter;
}

bool CameraScreen::connect() {

	ssi_msg (SSI_LOG_LEVEL_BASIC, "try to connect sensor...");
	_timer = 0;
	ssi_msg (SSI_LOG_LEVEL_BASIC, "connected");

	if (ssi_log_level >= SSI_LOG_LEVEL_DETAIL) {
		ssi_print ("\
             video\n\
             rate\t= %.2lf\t\n\
             dim\t= %d\n\
             bytes\t= %d\n",
			_options.fps, 		
			1, 
			_binfo.bmiHeader.biSizeImage);
	}

	// set thread name
	ssi_char_t string[SSI_MAX_CHAR];
	ssi_sprint (string, "%s@%ux%u", getName (), _region.width, _region.height);
	Thread::setName (string);

	return true;
}

bool CameraScreen::setProvider (const ssi_char_t *name, IProvider *provider) {

	if (strcmp (name, SSI_CAMERASCREEN_PROVIDER_NAME) == 0) {
		setProvider (provider);
		return true;
	}

	ssi_wrn ("unkown provider name '%s'", name);

	return false;
}


void CameraScreen::setProvider (IProvider *provider) {

	if (!provider) {
		return;
	}

	_provider = provider;

	_region = CameraScreen::FullScreen ();
	if (!_options.full) {

		ssi_size_t n_reg;
		int *reg = ssi_parse_indices (_options.region, n_reg);
		if(n_reg != 4)
		{
			ssi_wrn("cannot parse region: %s", _options.region);
			return;
		}

		_region = CameraScreen::Region (reg[0], reg[1], reg[2], reg[3]);
		region_s full = CameraScreen::FullScreen ();
		if (_region.left + _region.width > full.width || _region.top + _region.height > full.height || _region.width == 0|| _region.height == 0) {
			ssi_wrn ("invalid region (%u %u %u %u)", _region.left, _region.top, _region.width, _region.height);
		}
	}

	::ZeroMemory(&_binfo,sizeof(_binfo)); 

	BITMAPINFOHEADER &bih = _binfo.bmiHeader;
	bih.biSize = sizeof(bih);
	bih.biWidth = _region.width;
	bih.biHeight = _region.height;
	bih.biPlanes =1;
	bih.biBitCount = 24;
	bih.biCompression = BI_RGB;
	bih.biSizeImage = ((bih.biWidth*bih.biBitCount/8+3)&0xFFFFFFFC)*bih.biHeight;
	bih.biXPelsPerMeter = 10000;
	bih.biYPelsPerMeter = 10000;
	bih.biClrUsed = 0;
	bih.biClrImportant = 0;

	ssi_video_params (_org_params, _region.width, _region.height, _options.fps, 8, 3, SSI_GUID_NULL, true);

	_org_buffer_size = bih.biSizeImage;
	_org_buffer = new BYTE[_org_buffer_size];

	if (_options.resize) {

		ssi_char_t *file = 0;
		if (_file) {
			file = ssi_strcat (_file, ".resize");
		}
		_rsz_filter = ssi_pcast (CVResize, Factory::Create (CVResize::GetCreateName (), file, false));
		delete[] file;

		if (!_rsz_filter) {
			ssi_wrn ("could not create resize filter");
			_options.resize = false;
		} else {

			_rsz_params = _org_params;
			_rsz_params.widthInPixels = _options.resize_width;
			_rsz_params.heightInPixels = _options.resize_height;
			_rsz_buffer_size = ssi_video_size (_rsz_params);
			_rsz_buffer = new BYTE[_rsz_buffer_size];

			_params = _rsz_params;
			_buffer = _rsz_buffer;
			_buffer_size = _rsz_buffer_size;

			_rsz_stream_in.byte = _org_buffer_size;
			_rsz_stream_in.dim = 1;
			_rsz_stream_in.num_real = _rsz_stream_in.num = 1;
			_rsz_stream_in.tot_real = _rsz_stream_in.tot = _org_buffer_size;
			_rsz_stream_in.sr = _options.fps;
			_rsz_stream_in.time = 0;
			_rsz_stream_in.type = SSI_IMAGE;

			_rsz_stream_out.byte = _rsz_buffer_size;
			_rsz_stream_out.dim = 1;
			_rsz_stream_out.num_real = _rsz_stream_out.num = 1;
			_rsz_stream_out.tot_real = _rsz_stream_out.tot = _rsz_buffer_size;
			_rsz_stream_out.sr = _options.fps;
			_rsz_stream_out.time = 0;
			_rsz_stream_out.type = SSI_IMAGE;
	
			ssi_pcast (CVResize, _rsz_filter)->getOptions ()->setResize (ssi_real_t(_options.resize_width), ssi_real_t(_options.resize_height), ssi_cast (CVResize::METHOD, _options.resize_method));
			ssi_pcast (CVResize, _rsz_filter)->setFormat (_org_params);
			_rsz_filter->transform_enter (_rsz_stream_in, _rsz_stream_out);

			ssi_msg (SSI_LOG_LEVEL_BASIC, "added resize filter [%ux%u]", _options.resize_width, _options.resize_height);
		}

	} 
	
	if (!_options.resize) {
		_params = _org_params;
		_buffer = _org_buffer;
		_buffer_size = _org_buffer_size;
	}

	if (_options.flip) {
		_flip_buffer = new BYTE[_buffer_size];		
	}

	_provider->setMetaData (sizeof (_params), &_params);
	ssi_stream_init (_video_channel.stream, 0, 1, _buffer_size, SSI_IMAGE, _options.fps);
	provider->init (&_video_channel);

}

void CameraScreen::run() {

	if (!_timer) {
		_timer = new Timer (1.0 / _options.fps);
	}
	
	HANDLE hDib = CopyScreenToBitmap (_region, _org_buffer, (BITMAPINFO *) &(_binfo.bmiHeader));

	if (!hDib) {
		ssi_wrn ("screen capture failed");
		return;
	}

	if (_options.mouse) {
		AddCursorToBitmap (_region, _org_buffer, _org_params, _options.mouse_size);
	}

	if (_rsz_filter) {		

		ITransformer::info info;
		info.delta_num = 0;
		info.frame_num = 1;
		info.time = 0;

		_rsz_stream_in.ptr = ssi_pcast (ssi_byte_t, _org_buffer);
		_rsz_stream_out.ptr = ssi_pcast (ssi_byte_t, _rsz_buffer);
		
		_rsz_filter->transform (info, _rsz_stream_in, _rsz_stream_out);
	}

	if (_options.flip) {
		CameraTools::FlipImage (_flip_buffer, _buffer, _params);
		_provider->provide (ssi_pcast (ssi_byte_t, _flip_buffer), 1);
	} else {
		_provider->provide (ssi_pcast (ssi_byte_t, _buffer), 1);
	}
	SSI_DBG (SSI_LOG_LEVEL_DEBUG, "screen capture provided");

	DeleteObject(hDib);
	
	_timer->wait ();

	return;
}

bool CameraScreen::disconnect () {	

	ssi_msg (SSI_LOG_LEVEL_BASIC, "try to disconnect sensor...");

	if (_rsz_filter) {
		_rsz_filter->transform_flush (_rsz_stream_in, _rsz_stream_out);
	}
	delete _timer; _timer = 0;
	delete[] _flip_buffer; _flip_buffer = 0;

	ssi_msg (SSI_LOG_LEVEL_BASIC, "sensor disconnected");

	return true;
}

HBITMAP CameraScreen::CopyScreenToBitmap (region_s &region, BYTE *pData, BITMAPINFO *pHeader)
{
    HDC         hScrDC, hMemDC;         // screen DC and memory DC
    HBITMAP     hBitmap, hOldBitmap;    // handles to deice-dependent bitmaps
    int         nX, nY, nX2, nY2;       // coordinates of rectangle to grab
    int         nWidth, nHeight;        // DIB width and height

    // create a DC for the screen and create
    // a memory DC compatible to screen DC   
    hScrDC = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL);
    hMemDC = CreateCompatibleDC(hScrDC);

    // get points of rectangle to grab
    nX  = region.left;
    nY  = region.top;
    nX2 = region.left + region.width;
    nY2 = region.top + region.height;
	nWidth  = region.width;
	nHeight = region.height;

    // create a bitmap compatible with the screen DC
    hBitmap = CreateCompatibleBitmap(hScrDC, nWidth, nHeight);

    // select new bitmap into memory DC
    hOldBitmap = (HBITMAP) SelectObject(hMemDC, hBitmap);

    // bitblt screen DC to memory DC
    BitBlt(hMemDC, 0, 0, nWidth, nHeight, hScrDC, nX, nY, SRCCOPY);

    // select old bitmap back into memory DC and get handle to
    // bitmap of the screen   
    hBitmap = (HBITMAP)  SelectObject(hMemDC, hOldBitmap);

    // Copy the bitmap data into the provided BYTE buffer
    GetDIBits(hScrDC, hBitmap, 0, nHeight, pData, pHeader, DIB_RGB_COLORS);

    // clean up
    DeleteDC(hScrDC);
    DeleteDC(hMemDC);

    // return handle to the bitmap
    return hBitmap;
}

void CameraScreen::AddCursorToBitmap (region_s &region, BYTE *image, ssi_video_params_t &params, int border) {

	POINT pos;
	BOOL result = ::GetCursorPos (&pos);
	short lbutton = ::GetKeyState (VK_LBUTTON) >> 15;

	if (pos.x >= ssi_cast (long, region.left) && pos.x <= ssi_cast (long, region.left + region.width) && pos.y >= ssi_cast (long, region.top) && pos.y <= ssi_cast (long, region.top + region.height)) {
		
		ssi_size_t x = pos.x - region.left;
		ssi_size_t y = pos.y - region.top;

		paint_rect (image, params, x - border, y - border, x + border, y + border, 1, 0, 0, 0);
		paint_point (image, params, x, y, border, 255, lbutton ? 0 : 255, lbutton ? 0 : 255);		
	}
}

void CameraScreen::paint_rect (BYTE *image, 
	ssi_video_params_t &params, 
	int left, 
	int top,
	int right,
	int bottom,
	int border, 
	int r_value, 
	int g_value, 
	int b_value) {

	int stride = ssi_video_stride (params);
	int width = params.widthInPixels;
	int height = params.heightInPixels;
	int bottom_tmp = bottom;
	bottom = params.flipImage ? bottom : height - top;
	top = params.flipImage ? top : height - bottom_tmp;

	for (int x = max (0, left - border); x < min (width, right + border); ++x) {
		for (int y = max (0, top - border); y < min (height, bottom + border); ++y) {
			if (! (x > left + border && x < right - border && y > top + border && y < bottom - border)) {
				unsigned char *pixel = image + 3 * x + y * stride;
				pixel[0] = b_value;
				pixel[1] = g_value;
				pixel[2] = r_value;
			}			
		}
	}
}

void CameraScreen::paint_point (BYTE *image, 
	ssi_video_params_t &params, 
	int point_x, 
	int point_y, 
	int border, 
	int r_value, 
	int g_value, 
	int b_value) {

	int stride = ssi_video_stride (params);
	int width = params.widthInPixels;
	int height = params.heightInPixels;
	point_y = params.flipImage ? point_y : height - point_y;

	for (int x = max (0, point_x - border); x < min (width, point_x + border); ++x) {
		for (int y = max (0, point_y - border); y < min (height, point_y + border); ++y) {
			BYTE *pixel = image + 3 * x + y * stride;
			pixel[0] = b_value;
			pixel[1] = g_value;
			pixel[2] = r_value;
		}
	}
}

CameraScreen::region_s CameraScreen::Region (ssi_size_t left,
	ssi_size_t top,
	ssi_size_t width,
	ssi_size_t height) {

	region_s r;
	r.left = left;
	r.top = top;
	r.width = width;
	r.height = height;

	return r;
}

CameraScreen::region_s CameraScreen::FullScreen () {

	HWND desktop = GetDesktopWindow ();
	RECT rect;
	GetWindowRect (desktop, &rect);
	
	return Region (rect.left, rect.top, rect.right, rect.bottom);
}

}
