// CameraTools.h
// author: Frank Jung <frank.jung@informatik.uni-augsburg.de>
// created: 2009/02/23
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

#ifndef SSI_SENSOR_CAMTOOLS_H
#define	SSI_SENSOR_CAMTOOLS_H

#include "SSI_Cons.h"
#include "CameraCons.h"
#include "CameraTools.h"
#include "ioput/file/StringList.h"
#include "graphic/DialogLibGateway.h"

#include <AtlBase.h>
#include <AtlConv.h>
#include <dshow.h>

#include <Ocidl.h>
#include <OAIDL.H>
#include <comutil.h>
#include <streams.h>
#include <dvdmedia.h>
//#include <Qedit.h>
#include <initguid.h>

#include <list>

#include "iUAProxyForceGrabber.h"
#include "UAProxyForceGrabber.h"
#include "FakeCamPushSource.h"
#include "FakeAudioPushSource.h"

#include "CameraList.h"

namespace ssi {

#ifndef SafeReleaseFJ
#define SafeReleaseFJ(p) { if( (p) != 0 ) { (p)->Release(); (p)= 0; } }
#endif

class CameraTools
{
public:
	static HRESULT			InitCaptureGraphBuilder(IGraphBuilder **ppGraph, ICaptureGraphBuilder2 **ppBuild);
	static HRESULT			SelectCaptureDevice(ssi_char_t *nameOfCam, ssi_size_t indexOfCam, IBaseFilter **ppCapFilter, IGraphBuilder *pGraph);
	static HRESULT			QueryInterfaces(IGraphBuilder *pGraph, IMediaControl **ppControl = NULL, IMediaEvent **ppEvent = NULL, IVideoWindow **ppVidWin = NULL, IMediaSeeking **ppMediaSeek = NULL, IBasicVideo **ppBasicVideo = NULL);
	static HRESULT			SelectMediaTypeOfCam(IBaseFilter *pCapFilter, ICaptureGraphBuilder2 *pBuild, int desiredFPS, GUID desiredMediaSubType = MEDIASUBTYPE_RGB24, LONG desiredWidth = 320, LONG desiredHeight = 240);
	static IPin*			GetFirstPin(IBaseFilter *pFilter, PIN_DIRECTION PinDir);
	static HRESULT			AddAndConnectFilterByCLSIDToGivenOutputPin(const GUID& clsidOfTheFilter, LPCWSTR wszName, IGraphBuilder *pGraph, IPin *outputPin);
	static HRESULT			AddFilterToGraphByCLSID(IGraphBuilder *pGraph, const GUID& clsidOfTheFilter, LPCWSTR wszName, IBaseFilter **ppFilterToBeAddedtoTheGraph);
	static HRESULT			ConnectToNullRenderer(IBaseFilter *pFilterToConnectToNullRenderer, IGraphBuilder *pGraph);
	static HRESULT			ConnectToNullRenderer(IPin *pPinToConnectToNullRenderer, IGraphBuilder *pGraph);
	static HRESULT			RenderMyGraph(IGraphBuilder *pGraph, IPin *pOutputPintoRender);
	static AM_MEDIA_TYPE*	GetCurrentMediaTypeOfCam(IBaseFilter *pCapFilter, ICaptureGraphBuilder2 *pBuild);
	static bool				BuildAndDestroyGraphToDetermineCamSettings(int *widthInPix, int *heightInPix, int *frameRate, GUID *mediaSubType, int *widthInBytes, ssi_char_t *nameOfCam = NULL, ssi_size_t indexOfCam = 0);
	static bool				BuildAndDestroyGraphToDetermineFileVideoParams(const ssi_char_t *fileName, ssi_video_params_t *videoParams);
	static HRESULT			InitFilterGraphManager(IGraphBuilder **ppGraph);
	static HRESULT			FindAndBindToIBaseFilter(IBaseFilter **ppBaseFilter, CameraDeviceName *pDeviceName);
	static bool				StringToGUID (const char *guidString, GUID &guid);
	static bool				GUIDToString (GUID guid, char *guidString, ssi_size_t len);	
	static void				FlipImage (BYTE *dst, const BYTE *src, ssi_video_params_t &params);
};

}


#endif
