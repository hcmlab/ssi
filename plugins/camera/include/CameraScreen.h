// CameraScreen.h
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

// Provides live recording from WebCam

#pragma once

#ifndef SSI_SENSOR_CAMSCREENSENSOR_H
#define	SSI_SENSOR_CAMSCREENSENSOR_H

#include "base/ISensor.h"
#include "base/IProvider.h"
#include "base/IFilter.h"
#include "thread/Thread.h"
#include "thread/Timer.h"
#include "ioput/option/OptionList.h"
#include "CameraCons.h"

namespace ssi {

class CameraScreen : public ISensor, public Thread {

	class VideoChannel : public IChannel {

		friend class CameraScreen;

		public:

			VideoChannel () {
				ssi_stream_init (stream, 0, 0, 0, SSI_IMAGE, 0);
			}
			~VideoChannel () {
				ssi_stream_destroy (stream);
			}

			const ssi_char_t *getName () { return SSI_CAMERA_PROVIDER_NAME; };
			const ssi_char_t *getInfo () { return "Delivers a stream of uncompressed images in the Device Independent Bitmap (DIB) file format of the desired sub type."; };
			ssi_stream_t getStream () { return stream; };

		protected:

			ssi_stream_t stream;
	};

public:

	class Options : public OptionList {

	public:

		Options ()
			: mouse (false), mouse_size (4), fps (25.0), resize (false), resize_width (0), resize_height (0), resize_method (1), flip(true), full(true) {

			setRegion (0,0,0,0);

			addOption ("fps", &fps, 1, SSI_DOUBLE, "frames per second");
			addOption ("full", &full, 1, SSI_BOOL, "full screen toggle (if true region is ignored)");			
			addOption ("region", region, SSI_MAX_CHAR, SSI_CHAR, "region (left, top, width, height)");
			addOption ("mouse", &mouse, 1, SSI_BOOL, "capture mouse cursor");
			addOption ("mouse_size", &mouse_size, 1, SSI_SIZE, "size of mouse cursor in pixel");
			addOption ("resize", &resize, 1, SSI_BOOL, "resize image to widthxheight (requires ssiimage[d].dll!)");
			addOption ("resize_width", &resize_width, 1, SSI_SIZE, "resize width in pixels");		
			addOption ("resize_height", &resize_height, 1, SSI_SIZE, "resize height in pixels");		
			addOption ("resize_method", &resize_method, 1, SSI_INT, "resize interpolation method (0=NN,1=LINEAR,2=CUBIC,3=AREA)");
			addOption ("flip", &flip, 1, SSI_BOOL, "flip video image");			
		};

		void setRegion (ssi_size_t left, ssi_size_t top, ssi_size_t width, ssi_size_t height) {

			full = false;
			region[0] = '\0';
			ssi_char_t s[SSI_MAX_CHAR];

			ssi_sprint (s, "%u", left);
			strcat (region, s);
			
			ssi_sprint (s, ",%u", top);
			strcat (region, s);
				
			ssi_sprint (s, ",%u", width);
			strcat (region, s);
				
			ssi_sprint (s, ",%u", height);
			strcat (region, s);
		}		

		void setResize (ssi_size_t width, ssi_size_t height, int method = 1) {
			resize = true;
			resize_width = width;
			resize_height = height;
			resize_method = method;
		}

		bool full;
		ssi_char_t region[SSI_MAX_CHAR];
		bool mouse;	
		ssi_size_t mouse_size;
		ssi_time_t fps;
		bool resize;
		ssi_size_t resize_width, resize_height;
		int resize_method;
		bool flip;
	};

public:

	static const ssi_char_t *GetCreateName () { return "CameraScreen"; };
	static IObject *Create (const ssi_char_t *file) { return new CameraScreen (file); };
	~CameraScreen ();
	CameraScreen::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Captures the screen or parts of the screen and provides the grabbed frames as a video stream."; };

	ssi_size_t getChannelSize () { return 1; };
	IChannel *getChannel (ssi_size_t index) { return &_video_channel; };
	bool setProvider (const ssi_char_t *name, IProvider *provider);
	bool connect ();
	bool start () { return Thread::start (); };
	void run ();
	bool stop () { return Thread::stop (); };
	bool disconnect ();	

	ssi_video_params_t getFormat () { return _params; };
	const void *getMetaData (ssi_size_t &size) { size = sizeof (_params); return &_params; };

	void setLogLevel (int level) { ssi_log_level = level; };
	static void SetLogLevelStatic (int level) { ssi_log_level_static = level; };

protected:

	CameraScreen (const ssi_char_t *file = 0);
	CameraScreen::Options _options;
	ssi_char_t *_file;

	struct region_s {
		ssi_size_t left;
		ssi_size_t top;
		ssi_size_t width;
		ssi_size_t height;
	};

	static region_s Region (ssi_size_t left,
		ssi_size_t top,
		ssi_size_t width,
		ssi_size_t height);

	static region_s FullScreen ();

	static HBITMAP CopyScreenToBitmap (region_s &region, BYTE *pData, BITMAPINFO *pHeader);
	static void AddCursorToBitmap (region_s &region, BYTE *pData, ssi_video_params_t &params, int size);
	static void paint_point (BYTE *image, 
		ssi_video_params_t &params, 
		int point_x, 
		int point_y, 
		int border, 
		int r_value, 
		int g_value, 
		int b_value);
	static void paint_rect (BYTE *image, 
		ssi_video_params_t &params, 
		int left, 
		int top,
		int right,
		int bottom,
		int border, 
		int r_value, 
		int g_value, 
		int b_value);

	VideoChannel _video_channel;
	void setProvider (IProvider *provider);

	IProvider *_provider;
	BYTE *_buffer, *_org_buffer, *_rsz_buffer, *_flip_buffer;
	ssi_size_t _buffer_size, _org_buffer_size, _rsz_buffer_size;
	region_s _region;
	BITMAPINFO _binfo;
	ssi_video_params_t _params, _org_params, _rsz_params;
	Timer *_timer;
	IFilter *_rsz_filter;
	ssi_stream_t _rsz_stream_in;
	ssi_stream_t _rsz_stream_out;

	static char *ssi_log_name;
	int ssi_log_level;
	static int ssi_log_level_static;

};

}

#endif
