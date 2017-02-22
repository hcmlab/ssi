// CameraCons.h
// author: Frank Jung <frank.jung@informatik.uni-augsburg.de>
// created: 2008/04/01
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

#ifndef SSI_SENSOR_CAMERACONS_H
#define	SSI_SENSOR_CAMERACONS_H

#include <uuids.h>

//// link libraries
//#ifdef _MSC_VER 
//#pragma comment (lib, "strmiids.lib")
//#pragma comment (lib, "quartz.lib")
//#pragma comment (lib, "ole32.lib")
//#pragma comment (lib, "comsuppw.lib")
//#pragma comment (lib, "vfw32.lib")
//#pragma comment (lib, "winmm.lib")
//#ifdef _DEBUG
//#pragma comment (lib, "msvcrtd.lib")
////#pragma comment (lib, "strmbasd.lib")
//#else
//#pragma comment (lib, "msvcrt.lib")
////#pragma comment (lib, "strmbase.lib")
//#endif
//#endif

namespace ssi {

//#define RENDER_MY_STREAM_INSTEAD_OF_NULLRENDERER
#define SSI_CAMERA_SAMPLES_PER_STEP_TO_BUFFER 1
#define SSI_CAMERA_COM_THREADS	0x2
#define SSI_DV_PAL_NTSC_MASK 0x200000
#define SSI_CAMERA_USE_NO_COMPRESSION  "None"
#define SSI_CAMERA_PROVIDER_NAME       "video"
#define SSI_CAMERAREADER_PROVIDER_NAME "video"
#define SSI_CAMERASCREEN_PROVIDER_NAME "video"

//#pragma pack()
typedef struct CALLBACKBMPINFO_{
	BITMAPINFOHEADER	*pInfoHeader;
	BYTE				*pDataOfBMP;
	int					widthStepInBytes;
	int					widthInPixels;
	int					heightInPixels;
	GUID				mediaSubType;
} CALLBACKBMPINFO;

}

#endif
