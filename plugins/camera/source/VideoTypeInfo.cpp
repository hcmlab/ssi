// VideoTypeInfo.cpp
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

#include "VideoTypeInfo.h"

namespace ssi
{

int VideoTypeInfo::ssi_log_level_static = SSI_LOG_LEVEL_DEFAULT;
static char ssi_log_name_static[] = "vidinfo__s";

void VideoTypeInfo::setLogLevel(int level)
{
	ssi_log_level = level;
}

void VideoTypeInfo::SetLogLevelStatic(int level)
{
	ssi_log_level_static = level;
}

VideoTypeInfo::VideoTypeInfo(const VideoTypeInfo &rhs) :
	_underlyingMediaType(NULL),
	_formatType(GUID_NULL),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT)
{
	ssi_log_name = new char[SSI_MAX_CHAR];
	sprintf (ssi_log_name, "vidinfo___"); 


	if(rhs._underlyingMediaType != NULL)
	{
		_underlyingMediaType = CreateMediaType(rhs._underlyingMediaType);
	}

	memcpy_s(&_streamConfigCaps, sizeof(VIDEO_STREAM_CONFIG_CAPS), &(rhs._streamConfigCaps), sizeof(VIDEO_STREAM_CONFIG_CAPS));
	_formatType = rhs._formatType;
}

VideoTypeInfo::VideoTypeInfo(const AM_MEDIA_TYPE *mediaType, const VIDEO_STREAM_CONFIG_CAPS *streamConfigCaps) :
	_underlyingMediaType(NULL),
	_formatType(GUID_NULL),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT)
{
	ssi_log_name = new char[SSI_MAX_CHAR];
	sprintf (ssi_log_name, "vidinfo___"); 

	SecureZeroMemory(&_streamConfigCaps, sizeof(VIDEO_STREAM_CONFIG_CAPS));

	if(mediaType != NULL)
	{
		_underlyingMediaType = CreateMediaType(mediaType);
		_formatType = _underlyingMediaType->formattype;
	}

	if(streamConfigCaps != NULL)
	{
		_streamConfigCaps = (*streamConfigCaps);
	}

}

VideoTypeInfo::~VideoTypeInfo()
{
	SecureZeroMemory(&_streamConfigCaps, sizeof(VIDEO_STREAM_CONFIG_CAPS));
	_formatType = GUID_NULL;
	if(_underlyingMediaType != NULL)
	{
		DeleteMediaType(_underlyingMediaType);
	}
	delete[] ssi_log_name;
}

bool VideoTypeInfo::isEmpty()
{
	if(_underlyingMediaType == NULL)
	{
		return true;
	}

	return false;
}

AM_MEDIA_TYPE* VideoTypeInfo::getMediaType()
{
	return _underlyingMediaType;
}

VIDEO_STREAM_CONFIG_CAPS* VideoTypeInfo::getStreamConfigCaps()
{
	return &_streamConfigCaps;
}

GUID VideoTypeInfo::getFormatType()
{
	return _formatType;
}

bool VideoTypeInfo::operator==(const VideoTypeInfo &rhs)
{
	if(_underlyingMediaType == NULL || rhs._underlyingMediaType == NULL)
	{
		if(_underlyingMediaType == rhs._underlyingMediaType)
			return true;
		return false;
	}

	return ((IsEqualGUID(_underlyingMediaType->majortype, rhs._underlyingMediaType->majortype) == TRUE) &&
			(IsEqualGUID(_underlyingMediaType->subtype, rhs._underlyingMediaType->subtype) == TRUE) &&
			(IsEqualGUID(_underlyingMediaType->formattype, rhs._underlyingMediaType->formattype) == TRUE) &&
			(_underlyingMediaType->cbFormat == rhs._underlyingMediaType->cbFormat) &&
		  ( (_underlyingMediaType->cbFormat == 0) ||
			(memcmp(_underlyingMediaType->pbFormat, rhs._underlyingMediaType->pbFormat, _underlyingMediaType->cbFormat) == 0) ));

}

VideoTypeInfo& VideoTypeInfo::operator =(const VideoTypeInfo &rhs)
{
	if(this == &rhs)
		return *this;

	SecureZeroMemory(&_streamConfigCaps, sizeof(VIDEO_STREAM_CONFIG_CAPS));
	_formatType = GUID_NULL;

	if(_underlyingMediaType != NULL)
	{
		DeleteMediaType(_underlyingMediaType);
		_underlyingMediaType = NULL;
	}

	if(rhs._underlyingMediaType != NULL)
	{
		_underlyingMediaType = CreateMediaType(rhs._underlyingMediaType);
	}

	memcpy_s(&_streamConfigCaps, sizeof(VIDEO_STREAM_CONFIG_CAPS), &(rhs._streamConfigCaps), sizeof(VIDEO_STREAM_CONFIG_CAPS));
	_formatType = rhs._formatType;

	return *this;
}

}

