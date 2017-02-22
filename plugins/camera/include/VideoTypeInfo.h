// VideoTypeInfo.h
// author: Frank Jung <frank.jung@informatik.uni-augsburg.de>
// created: 2009/05/07
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

#ifndef SSI_SENSOR_VIDEOTYPEINFO_H
#define	SSI_SENSOR_VIDEOTYPEINFO_H

#include <comutil.h>
#include <string.h>
#include <vector>
#include "SSI_Cons.h"
#include "CameraCons.h"
#include <DShow.h>
#include <streams.h>
#include <iostream>
#include <fstream>

#ifndef SSI_VIDEOTYPEINFO_CHARSTRINGLENGTH
#define SSI_VIDEOTYPEINFO_CHARSTRINGLENGTH 1024
#endif

namespace ssi
{
	//Be aware: Needs COM but doesn't initialize it!!!!!
	class VideoTypeInfo
	{
	public:
		VideoTypeInfo(const VideoTypeInfo &rhs);
		VideoTypeInfo(const AM_MEDIA_TYPE *mediaType = NULL, const VIDEO_STREAM_CONFIG_CAPS *streamConfigCaps = NULL);
		~VideoTypeInfo();

		bool								isEmpty();
		AM_MEDIA_TYPE*						getMediaType();
		VIDEO_STREAM_CONFIG_CAPS*			getStreamConfigCaps();
		GUID								getFormatType();	

		bool operator==(const VideoTypeInfo &rhs);
		VideoTypeInfo& operator=(const VideoTypeInfo &rhs);

		void setLogLevel (int level);
		static void SetLogLevelStatic (int level);

	protected:

		AM_MEDIA_TYPE				*_underlyingMediaType;
		VIDEO_STREAM_CONFIG_CAPS	_streamConfigCaps;
		GUID						_formatType;

		char *ssi_log_name;
		int ssi_log_level;
		static int ssi_log_level_static;
	};
}

#endif
