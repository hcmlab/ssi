// VideoTypeInfoList.h
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

#ifndef SSI_SENSOR_VIDEOTYPEINFOLIST_H
#define	SSI_SENSOR_VIDEOTYPEINFOLIST_H

#include <comutil.h>
#include <string.h>
#include <vector>
#include "SSI_Cons.h"
#include "CameraCons.h"
#include <DShow.h>
#include <streams.h>
#include <iostream>
#include <fstream>

#include "VideoTypeInfo.h"

#ifndef SSI_VIDEOTYPEINFOLIST_CHARSTRINGLENGTH
#define SSI_VIDEOTYPEINFOLIST_CHARSTRINGLENGTH 1024
#endif

namespace ssi
{
	class VideoTypeInfoList
	{
	public:
		//TODO
		VideoTypeInfoList(IBaseFilter *pBaseFilter = NULL);
		~VideoTypeInfoList();

		HRESULT setBaseFilter(IBaseFilter *pBaseFilter);
		bool isEmpty();
		int howManyPinsPresent(PIN_DIRECTION pinDirection);
		int howManyMediaInfosPresentForSpecificPin(PIN_DIRECTION pinDirection, int indexOfDesiredPin);
		VideoTypeInfo getCopyOfVideoTypeInfo(PIN_DIRECTION pinDirection, int indexOfDesiredPin, int indexOfDesiredFormat);
		VideoTypeInfo& getReferenceOfVideoTypeInfo(PIN_DIRECTION pinDirection, int indexOfDesiredPin, int indexOfDesiredFormat) throw(std::out_of_range);
		bool isPresentInList(const VideoTypeInfo &videoTypeInfo, PIN_DIRECTION pinDirection, int &indexOfDesiredPin, int &indexOfFoundFilterInCertainPin) throw(std::out_of_range);

		static int LetUserSelectMediaType(VideoTypeInfoList *availableMediaTypesToSelectFrom, int &indexOfSelectedOutputPin, int &indexOfSelectedMediaType, bool fallBackToConsole = true);

		void setLogLevel (int level);
		static void SetLogLevelStatic (int level);

	protected:

		HRESULT extractVideoTypeInfosFromBaseFilter(IBaseFilter *pBaseFilter);

		char			*ssi_log_name;
		int				ssi_log_level;
		static int		ssi_log_level_static;

		int				_comInitCountConstructor;

		std::vector<std::vector<VideoTypeInfo> > _inputPins;
		std::vector<std::vector<VideoTypeInfo> > _outputPins;
	};
}

#endif
