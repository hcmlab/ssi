// Camera.cpp
// author: Frank Jung <frank.jung@informatik.uni-augsburg.de>
// created: 2009/04/06
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

#include "Camera.h"

#include <AtlBase.h>
#include <AtlConv.h>
#include <dshow.h>
#include <Ocidl.h>
#include <OAIDL.H>
#include <comutil.h>
#include <streams.h>
//#include <Qedit.h>
#include <initguid.h>
#include <list>

#include "CameraQualityProps.h"
#include "CameraTools.h"
#include "CameraList.h"
#include "VideoTypeInfoList.h"
#include "UAProxyForceGrabber.h"
#include "iUAProxyForceGrabber.h"

#include "graphic/DialogLibGateway.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif


#ifndef CLSID_NullRenderer
DEFINE_GUID(CLSID_NullRenderer, 0xC1F400A4, 0x3F08, 0x11d3, 0x9F, 0x0B,
	    0x00, 0x60, 0x08, 0x03, 0x9E, 0x37);
#endif

namespace ssi
{

int Camera::ssi_log_level_static = SSI_LOG_LEVEL_DEFAULT;
static char ssi_log_name_static[] = "camera____";
char *Camera::ssi_log_name = "camera____";

CameraDeviceName& Camera::getCameraDeviceName () { return _listOfVideoCameras->getReferenceToCameraDeviceName(_indexOfSelectedCamera); };

Camera::Camera(const ssi_char_t *file) :
	_provider(0),
	_comInitCountConstructor(0),
	_comInitCountConnectRunDisconnect(0),
	_useDV(false),
	_dvImageSizeDenominator(1),
	_selectedMediaTypePinIndex(-1),
	_selectedMediaTypeVideoInfoIndex(-1),
	_listOfVideoCameras(NULL),
	_listOfVideoTypesInTheSpecifiedFilter(NULL),
	_indexOfSelectedCamera(-1),
	_nameOfDesiredCam(NULL),
	_pGraph(NULL),
	_pBuild(NULL),
	_pCapDevice(NULL),
	_pGrabber(NULL),
	_pControl(NULL),
	_pGrabInterface(NULL),
	_timer(NULL),
	_picData(NULL),
	_picDataFlipBuffer(NULL),
	_sizeOfPicData(0),
	_file (0),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

	_cbIH.pInfoHeader = &_bmpIH;
	_cbIH.pDataOfBMP = NULL;

	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr)) {
		if(hr == RPC_E_CHANGED_MODE) {
			ssi_wrn("Tried to reinitialize COM with different threading model! This might cause trouble!");
		}
		else {
			ssi_err ("Could not initialize COM library in constructor()");
		}
    }
	else {
		if(hr == S_FALSE) {
			ssi_msg (SSI_LOG_LEVEL_DETAIL, "COM was already initialized for this thread!");
		}
		++_comInitCountConstructor;
	}

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
		_options.applySubTypeName ();
	}
}

Camera::~Camera() {

	if (_file) {
		if (_options.apply) {			
			_options.udpateSubTypeName ();
			_options.setDeviceName (getCameraDeviceName ().getDevicePath());
			//_options.params = 
		}
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}

	delete [] _picData;
	delete[] _picDataFlipBuffer;
	_picData = NULL;
	_picDataFlipBuffer = NULL;
	_sizeOfPicData = 0;

	delete _listOfVideoCameras;
	delete _listOfVideoTypesInTheSpecifiedFilter;
	delete[] _nameOfDesiredCam;

	if(_comInitCountConstructor > 0) {
		CoUninitialize();
		--_comInitCountConstructor;
	}
}

CameraList* Camera::GetVideoCaptureDevices()
{
	return CameraList::FindAllCamsInSystem(NULL);

}

bool Camera::setProvider (const ssi_char_t *name, IProvider *provider) {

	if (strcmp (name, SSI_CAMERA_PROVIDER_NAME) == 0) {
		setProvider (provider);		
		return true;
	}

	ssi_wrn ("unkown provider name '%s'", name);

	return false;
}

void Camera::setProvider(IProvider *provider) {

	if (provider) {

		// determine format

		_listOfVideoCameras = Camera::GetVideoCaptureDevices();

		if(_listOfVideoCameras->getListSize() < 1) {
			ssi_err("No Capture Device found!!!\n");
		}

		const ssi_char_t *nameOfDesiredCam = _options.getDeviceName ();
		if (nameOfDesiredCam) {
			size_t strLenVar = strnlen(nameOfDesiredCam, 1024);
			if(strLenVar == 1024) {
				_nameOfDesiredCam = NULL;
				ssi_err ("String that indicated the Camera-Name was not NULL-Terminated or longer than 1024 bytes");
			}
			_nameOfDesiredCam = new ssi_char_t[strLenVar + 1];
			strcpy_s(_nameOfDesiredCam, strLenVar+1, nameOfDesiredCam);
			bool desiredCamFound = false;
			int tmpIndex = 0;
			for(ssi_size_t i = 0; i < _listOfVideoCameras->getListSize(); ++i, ++tmpIndex) {
				CameraDeviceName curDev = _listOfVideoCameras->getReferenceToCameraDeviceName(i);
				if(curDev == (char*)nameOfDesiredCam) {
					desiredCamFound = true;
					break;
				}
			}
			if(desiredCamFound) {
				_indexOfSelectedCamera = tmpIndex;
			}
			else {
				ssi_wrn ("video capture device '%s' not found", nameOfDesiredCam);
				int tmpIndex = CameraList::LetUserSelectDesiredCam(_listOfVideoCameras);
				if(tmpIndex < 0) {
					ssi_err("invalid video capture device");
				}
				else {
					_indexOfSelectedCamera = tmpIndex;
				}
			}
		}
		else {
			int tmpIndex = CameraList::LetUserSelectDesiredCam(_listOfVideoCameras);
			if(tmpIndex < 0) {
				ssi_err("invalid video capture device");
			}
			else {
				_indexOfSelectedCamera = tmpIndex;
			}
		}

		IBaseFilter *pCap = NULL;
		HRESULT hr = CameraTools::FindAndBindToIBaseFilter(&pCap, &(_listOfVideoCameras->getReferenceToCameraDeviceName(_indexOfSelectedCamera)));
		if(FAILED(hr))
		{
			ssi_err("could not bind the cam to IBaseFilter. Failed with %ld", hr);
		}
		
		bool foundType = this->determineIfDesiredMediaTypeExists(pCap);

		if(foundType == false)
		{
			//TODO show dialog and maybe selection
			if(LetUserSelectMediaType())
			{
				if(!this->determineIfDesiredMediaTypeExists(NULL))
				{
					ssi_err("type Info not supported.");
				}
			}
			else
			{
				ssi_err("type Info not supported.");
			}
		}

		SafeReleaseFJ(pCap);

		_options.params.flipImage = _options.flip;
		provider->setMetaData (sizeof (_options.params), &_options.params);
		ssi_stream_init (_video_channel.stream, 0, 1, ssi_video_size (_options.params), SSI_IMAGE, _options.params.framesPerSecond);
		provider->init (&_video_channel);
	}

	Lock lock (_setProviderMutex); 
	
	{
		_provider = provider;
	}

	// set thread name
	ssi_char_t string[SSI_MAX_CHAR];
	ssi_sprint (string, "%s@%s", getName (), _options.getDeviceName ());
	Thread::setName (string);
}

bool Camera::setVideoOutputFormatAndConnectPin()
{
	if(_pCapDevice == NULL || _selectedMediaTypePinIndex < 0 || _selectedMediaTypeVideoInfoIndex < 0)
	{
		//TODO error output
		return false;
	}

	VideoTypeInfo selectedVideoTypeInfo = _listOfVideoTypesInTheSpecifiedFilter->getReferenceOfVideoTypeInfo(PINDIR_OUTPUT, _selectedMediaTypePinIndex, _selectedMediaTypeVideoInfoIndex);

	IEnumPins		*pEnumPins = NULL;
	IPin			*pPin = NULL;
	IAMStreamConfig	*pConfig = NULL;
	IKsPropertySet	*pKsPropertySet = NULL;
	GUID			pinCategory = GUID_NULL;

	HRESULT hr = _pCapDevice->EnumPins(&pEnumPins);
	if(FAILED(hr))
	{
		ssi_err ("could enumerate pins in setVideoOutputFormat(). Failed with %ld", hr);
		return false;
	}

	while(pEnumPins->Next(1, &pPin, 0) == S_OK)
    {
        PIN_DIRECTION currentPinDirection;
        pPin->QueryDirection(&currentPinDirection);
        if (currentPinDirection != PINDIR_OUTPUT)
		{
			SafeReleaseFJ(pPin);
			continue;
		}
		hr = pPin->QueryInterface(IID_IKsPropertySet, (void**)&pKsPropertySet);
		if(FAILED(hr))
		{
			SafeReleaseFJ(pPin);
			ssi_wrn ("the pin does not support IKsPropertySet in setVideoOutputFormat(). Failed with %ld", hr);
			continue;
		}

		DWORD cbReturned;
		hr = pKsPropertySet->Get(AMPROPSETID_Pin, AMPROPERTY_PIN_CATEGORY, NULL, 0, &pinCategory, sizeof(GUID), &cbReturned);
		if(FAILED(hr))
		{
			SafeReleaseFJ(pKsPropertySet);
			SafeReleaseFJ(pPin);
			ssi_wrn ("the pin does not support IKsPropertySet AMPROPERTY_PIN_CATEGORY in setVideoOutputFormat(). Failed with %ld", hr);
			continue;
		}
		if(IsEqualGUID(pinCategory, PIN_CATEGORY_CAPTURE) == FALSE)
		{
			SafeReleaseFJ(pKsPropertySet);
			SafeReleaseFJ(pPin);
			SSI_DBG (SSI_LOG_LEVEL_DEBUG, "was not a capture pin");
			continue;
		}

		hr = pPin->QueryInterface(IID_IAMStreamConfig, (void**)&pConfig);
		if(FAILED(hr))
		{
			SafeReleaseFJ(pKsPropertySet);
			SafeReleaseFJ(pPin);
			ssi_err ("the pin does not support IAMStreamConfig in setVideoOutputFormat(). Failed with %ld", hr);
			continue;
		}


		int iCount = 0, iSize= 0;
		hr = pConfig->GetNumberOfCapabilities(&iCount, &iSize);
		if(FAILED(hr))
		{
			pinCategory = GUID_NULL;
			SafeReleaseFJ(pConfig);
			SafeReleaseFJ(pKsPropertySet);
			SafeReleaseFJ(pPin);
			ssi_err ("failed to get number of capabilities in setVideoOutputFormat(). Failed with %ld", hr);
			continue;
		}

		if(iSize == sizeof(VIDEO_STREAM_CONFIG_CAPS))
		{
			for(int iFormat = 0; iFormat < iCount; ++iFormat)
			{
				VIDEO_STREAM_CONFIG_CAPS streamConfigCaps;
				AM_MEDIA_TYPE *pmtConfig;
				hr = pConfig->GetStreamCaps(iFormat, &pmtConfig, (BYTE*)&streamConfigCaps);
				if(FAILED(hr))
				{
					pinCategory = GUID_NULL;
					SafeReleaseFJ(pConfig);
					SafeReleaseFJ(pKsPropertySet);
					SafeReleaseFJ(pPin);
					ssi_err ("failed retrieving StreamCaps in setVideoOutputFormat(). Failed with %ld", hr);
					continue;
				}

				if((IsEqualGUID(pmtConfig->majortype, MEDIATYPE_Video) == TRUE) || ((_options.params.majorVideoType == 2) ? (IsEqualGUID(pmtConfig->majortype, MEDIATYPE_Interleaved) == TRUE) : (false)))
				{
					VideoTypeInfo vTypeInfo(pmtConfig, &streamConfigCaps);

					if(vTypeInfo == selectedVideoTypeInfo)
					{
						
						if(_useDV)
						{
							int outputFormatX = 720;
							int outputFormatY = 0;
							double maxFpsOfDVStream = 0.0;
							DVINFO *pDVI = (DVINFO*)pmtConfig->pbFormat;
							if(pDVI->dwDVVAuxSrc & SSI_DV_PAL_NTSC_MASK)
							{
								//PAL
								outputFormatY = 576;
								maxFpsOfDVStream = 25.0;
							}
							else
							{
								//NTSC
								outputFormatY = 480;
								maxFpsOfDVStream = 29.97;
							}
							if(outputFormatX == _options.params.widthInPixels && outputFormatY == _options.params.heightInPixels)
							{
								_dvImageSizeDenominator = DVRESOLUTION_FULL;
							}
							if((outputFormatX/2) == _options.params.widthInPixels && (outputFormatY/2) == _options.params.heightInPixels)
							{
								_dvImageSizeDenominator = DVRESOLUTION_HALF;
							}
							if((outputFormatX/4) == _options.params.widthInPixels && (outputFormatY/4) == _options.params.heightInPixels)
							{
								_dvImageSizeDenominator = DVRESOLUTION_QUARTER;
							}

							pConfig->SetFormat(pmtConfig);
							//TODO connect Pin

							hr = CameraTools::AddAndConnectFilterByCLSIDToGivenOutputPin(CLSID_DVVideoCodec, L"DVDecode", _pGraph, pPin);
							if(FAILED(hr))
							{
								DeleteMediaType(pmtConfig);
								SafeReleaseFJ(pConfig);
								SafeReleaseFJ(pKsPropertySet);
								SafeReleaseFJ(pPin);
								SafeReleaseFJ(pEnumPins);
								return false;
							}
							IBaseFilter	*pDVDecoder;
							hr = _pGraph->FindFilterByName(L"DVDecode", &pDVDecoder);
							if(FAILED(hr))
							{
								DeleteMediaType(pmtConfig);
								SafeReleaseFJ(pConfig);
								SafeReleaseFJ(pKsPropertySet);
								SafeReleaseFJ(pPin);
								SafeReleaseFJ(pEnumPins);
								return false;
							}
							IIPDVDec *pDVDecoderInterface = NULL;
							hr = pDVDecoder->QueryInterface(IID_IIPDVDec,(void**) &pDVDecoderInterface);
							if(FAILED(hr))
							{
								DeleteMediaType(pmtConfig);
								SafeReleaseFJ(pConfig);
								SafeReleaseFJ(pKsPropertySet);
								SafeReleaseFJ(pPin);
								SafeReleaseFJ(pEnumPins);
								return false;
							}

							hr = pDVDecoderInterface->put_IPDisplay(_dvImageSizeDenominator);
							SafeReleaseFJ(pDVDecoderInterface);
							if(FAILED(hr))
							{
								SafeReleaseFJ(pDVDecoder);
								DeleteMediaType(pmtConfig);
								SafeReleaseFJ(pConfig);
								SafeReleaseFJ(pKsPropertySet);
								SafeReleaseFJ(pPin);
								SafeReleaseFJ(pEnumPins);
								return false;
							}

							SafeReleaseFJ(pPin);
							pPin = CameraTools::GetFirstPin(pDVDecoder, PINDIR_OUTPUT);
							if(!pPin)
							{
								SafeReleaseFJ(pDVDecoder);
								DeleteMediaType(pmtConfig);
								SafeReleaseFJ(pConfig);
								SafeReleaseFJ(pKsPropertySet);
								SafeReleaseFJ(pPin);
								SafeReleaseFJ(pEnumPins);
								return false;
							}
							hr = CameraTools::AddAndConnectFilterByCLSIDToGivenOutputPin(CLSID_UAProxyForceGrabber, L"Grabber", _pGraph, pPin);

							SafeReleaseFJ(pDVDecoder);

							if(FAILED(hr))
							{
								DeleteMediaType(pmtConfig);
								SafeReleaseFJ(pConfig);
								SafeReleaseFJ(pKsPropertySet);
								SafeReleaseFJ(pPin);
								SafeReleaseFJ(pEnumPins);
								return false;
							}

						}
						else
						{
							VIDEOINFOHEADER *pVIH = (VIDEOINFOHEADER*)pmtConfig->pbFormat;
							BITMAPINFOHEADER *pBmIH = &(pVIH->bmiHeader);
							if(pBmIH->biHeight < 0)
							{
								pBmIH->biHeight = - std::abs(_options.params.heightInPixels);
							}
							else
							{
								pBmIH->biHeight = std::abs(_options.params.heightInPixels);
							}
							pBmIH->biWidth = _options.params.widthInPixels;
							

							REFERENCE_TIME desiredTimePerFrame = (REFERENCE_TIME)(10000000.0 / _options.params.framesPerSecond + 0.5);
							if(desiredTimePerFrame >= streamConfigCaps.MinFrameInterval && desiredTimePerFrame <= streamConfigCaps.MaxFrameInterval)
							{
								pVIH->AvgTimePerFrame = desiredTimePerFrame;
							}
							else
							{
								if(desiredTimePerFrame < streamConfigCaps.MinFrameInterval)
								{
									pVIH->AvgTimePerFrame = streamConfigCaps.MinFrameInterval;
								}
								if(desiredTimePerFrame > streamConfigCaps.MaxFrameInterval)
								{
									pVIH->AvgTimePerFrame = streamConfigCaps.MaxFrameInterval;
								}
							}
							pConfig->SetFormat(pmtConfig);

							hr = CameraTools::AddAndConnectFilterByCLSIDToGivenOutputPin(CLSID_UAProxyForceGrabber, L"Grabber", _pGraph, pPin);
						}

						//Clean Up
						DeleteMediaType(pmtConfig);
						SafeReleaseFJ(pConfig);
						SafeReleaseFJ(pKsPropertySet);
						SafeReleaseFJ(pPin);
						SafeReleaseFJ(pEnumPins);
						if(SUCCEEDED(hr))
						{
							return true;
						}
						else
						{
							return false;
						}
						
					}

					DeleteMediaType(pmtConfig);
				}
			}
		}


		pinCategory = GUID_NULL;
		SafeReleaseFJ(pConfig);
		SafeReleaseFJ(pKsPropertySet);
        SafeReleaseFJ(pPin);
    }

    SafeReleaseFJ(pEnumPins);

	return false;

}

bool Camera::determineIfDesiredMediaTypeExists(IBaseFilter *pCap)
{
	//TODO
	if(pCap == NULL && _listOfVideoTypesInTheSpecifiedFilter == NULL)
	{
		return false;
	}

	if(_listOfVideoTypesInTheSpecifiedFilter == NULL)
		_listOfVideoTypesInTheSpecifiedFilter = new VideoTypeInfoList(pCap);
	//SafeReleaseFJ(pCap);
	bool foundType = false;
	VideoTypeInfo vidInfo;
	int pinIndex = -1;
	int formatIndex = -1;
	for(int i = 0; i < _listOfVideoTypesInTheSpecifiedFilter->howManyPinsPresent(PINDIR_OUTPUT); ++i)
	{
		for(int j = 0; j < _listOfVideoTypesInTheSpecifiedFilter->howManyMediaInfosPresentForSpecificPin(PINDIR_OUTPUT, i); ++j)
		{
			vidInfo = _listOfVideoTypesInTheSpecifiedFilter->getReferenceOfVideoTypeInfo(PINDIR_OUTPUT, i, j);
			GUID formatType = vidInfo.getFormatType();
			AM_MEDIA_TYPE *pMediaType = vidInfo.getMediaType();
			VIDEO_STREAM_CONFIG_CAPS *pSCC = vidInfo.getStreamConfigCaps();
			bool foundHeightResolution = false;
			bool foundWidthResolution = false;
			bool foundFPS = false;
			if((IsEqualGUID(pMediaType->majortype, MEDIATYPE_Video) == TRUE) || ((_options.params.majorVideoType == 2) ? (IsEqualGUID(pMediaType->majortype, MEDIATYPE_Interleaved) == TRUE) : (false)))
			{
				if(IsEqualGUID(formatType, FORMAT_DvInfo) == TRUE)
				{
					if(IsEqualGUID(_options.params.outputSubTypeOfCaptureDevice, pMediaType->subtype) == FALSE)
					{
						continue;
					}
					_useDV = true;
					if(pMediaType->cbFormat >= sizeof(DVINFO))
					{
						int outputFormatX = 720;
						int outputFormatY = 0;
						double maxFpsOfDVStream = 0.0;
						DVINFO *pDVI = (DVINFO*)pMediaType->pbFormat;
						if(pDVI->dwDVVAuxSrc & SSI_DV_PAL_NTSC_MASK)
						{
							//PAL
							outputFormatY = 576;
							maxFpsOfDVStream = 25.0;
						}
						else
						{
							//NTSC
							outputFormatY = 480;
							maxFpsOfDVStream = 29.97;
						}
						if((outputFormatX == _options.params.widthInPixels && outputFormatY == _options.params.heightInPixels) ||
							((outputFormatX/2) == _options.params.widthInPixels && (outputFormatY/2) == _options.params.heightInPixels) ||
							((outputFormatX/4) == _options.params.widthInPixels && (outputFormatY/4) == _options.params.heightInPixels))
						{
							foundWidthResolution = true;
							foundHeightResolution = true;
						}
						if(maxFpsOfDVStream == _options.params.framesPerSecond)
						{
							foundFPS = true;
						}
						else if(foundHeightResolution && foundWidthResolution)
						{
							ssi_msg(SSI_LOG_LEVEL_DEBUG, "desired FPS %4.3lf is not supported framerate of %4.3lf but matches otherwise", _options.params.framesPerSecond, maxFpsOfDVStream);
							if(_options.params.useClosestFramerateForGraph)
							{
								foundFPS = true;
							}
						}
					}//END is DVINFO?
				}//END IF format is FORMAT_DvInfo?
				else if(IsEqualGUID(formatType, FORMAT_VideoInfo) == TRUE)
				{
					if(IsEqualGUID(_options.params.outputSubTypeOfCaptureDevice, pMediaType->subtype) == FALSE)
					{
						continue;
					}
					if(pMediaType->cbFormat >= sizeof(VIDEOINFOHEADER))
					{
						VIDEOINFOHEADER *pVIH = (VIDEOINFOHEADER*)pMediaType->pbFormat;
						BITMAPINFOHEADER *pBmIH = &(pVIH->bmiHeader);
						if(std::abs(pBmIH->biHeight) != _options.params.heightInPixels)
						{
							if(pSCC->MinOutputSize.cy <= _options.params.heightInPixels && pSCC->MaxOutputSize.cy >= _options.params.heightInPixels)
							{
								for(int yRunner = pSCC->MinOutputSize.cy; yRunner < pSCC->MaxOutputSize.cy; yRunner += pSCC->OutputGranularityY)
								{
									if(yRunner == _options.params.heightInPixels)
									{
										foundHeightResolution = true;
										//FOUND height format
									}//END yRunner == height
								}//END FOR yRunner in range
							}////END IF desired Format in range
						}
						else // FOUND HEIGHT FORMAT
						{
							foundHeightResolution = true;
						}

						if(pBmIH->biWidth != _options.params.widthInPixels)
						{
							if(pSCC->MinOutputSize.cx <= _options.params.widthInPixels && pSCC->MaxOutputSize.cx >= _options.params.widthInPixels)
							{
								for(int xRunner = pSCC->MinOutputSize.cx; xRunner < pSCC->MaxOutputSize.cx; xRunner += pSCC->OutputGranularityX)
								{
									if(xRunner == _options.params.widthInPixels)
									{
										foundWidthResolution = true;
										//FOUND WIDTH format
									}//END xRUnner == width
								}//END FOR xRunner in range
							}////END IF desired Format in range
						}
						else // FOUND WIDTH FORMAT
						{
							foundWidthResolution = true;
						}

						REFERENCE_TIME desiredTimePerFrame = (REFERENCE_TIME)(10000000.0 / _options.params.framesPerSecond);
						if(desiredTimePerFrame >= pSCC->MinFrameInterval && desiredTimePerFrame <= pSCC->MaxFrameInterval)
						{
							foundFPS = true;
						}
						else if(foundHeightResolution && foundWidthResolution)
						{
							ssi_msg(SSI_LOG_LEVEL_DEBUG, "desired FPS %u is outside of the supported range of %u and %u but matches otherwise", ssi_cast (unsigned int, desiredTimePerFrame), ssi_cast (unsigned int, pSCC->MinFrameInterval), ssi_cast (unsigned int, pSCC->MaxFrameInterval));
							if(_options.params.useClosestFramerateForGraph)
							{
								foundFPS = true;
							}
						}
					}//END is VideoInfoHeader?
				}//END is FORMAT_VideoInfo?

			}

			if(foundFPS && foundHeightResolution && foundWidthResolution)
			{
				foundType = true;
				pinIndex = i;
				formatIndex = j;
				break;
				//TODO
			}
		}//END FOR all formats in specific pin
		if(foundType)
		{
			break;
		}
	}//END FOR all output pins
	


	_selectedMediaTypePinIndex = pinIndex;
	_selectedMediaTypeVideoInfoIndex = formatIndex;
	return foundType;
}

bool Camera::connect()
{
	SSI_DBG (SSI_LOG_LEVEL_DETAIL, "try to connect sensor...");

	SSI_DBG (SSI_LOG_LEVEL_DEBUG, "Calling InitCaptureGraphBuilder...");
	
	HRESULT hr = CameraTools::InitCaptureGraphBuilder(&_pGraph, &_pBuild);
	if(SUCCEEDED(hr))
	{
		 SSI_DBG (SSI_LOG_LEVEL_VERBOSE, "...SUCCEEDED!");
	}
	else 
	{
		ssi_err ("InitCapturegraphBuilder failed");
		return false;
	}

	SSI_DBG (SSI_LOG_LEVEL_DEBUG, "Calling FindAndBindToIBaseFilter...");
	hr = CameraTools::FindAndBindToIBaseFilter(&_pCapDevice, &(_listOfVideoCameras->getReferenceToCameraDeviceName(_indexOfSelectedCamera)));
	if(FAILED(hr))
	{
		ssi_err("Could not bind the cam to IBaseFilter. Failed with %ld", hr);
	}

	SSI_DBG (SSI_LOG_LEVEL_DEBUG, "Adding Capture Filter to graph...");
	hr = _pGraph->AddFilter(_pCapDevice, L"Capture Filter");
	if(FAILED(hr))
	{
		ssi_err("FAILED! Could not add the cam to IBaseFilter. Failed with %ld", hr);
		return false;
	}
	else
	{
		SSI_DBG_STATIC (SSI_LOG_LEVEL_VERBOSE, "...SUCCEDED!");
	}
	
	SSI_DBG (SSI_LOG_LEVEL_DEBUG, "Calling QueryInterfaces...");

	hr = CameraTools::QueryInterfaces(_pGraph, &_pControl);
	if(SUCCEEDED(hr))
	{
		SSI_DBG (SSI_LOG_LEVEL_VERBOSE, "...SUCCEEDED!");
	}
	else
	{
		ssi_err ("QueryInterfaces");
		return false;
	}

	if(setVideoOutputFormatAndConnectPin() == false)
	{
		ssi_err("Could not set desired VideoOutputFormat!");
		return false;
	}
	
	hr = _pGraph->FindFilterByName(L"Grabber", &_pGrabber);
	if(SUCCEEDED(hr))
	{
		// ssi_msg (SSI_LOG_LEVEL_DETAIL, "...SUCCEEDED!");
	}
	else
	{
		ssi_err ("FindFilterByName with UAProxyForceGrabber");
		return false;
	}

	ssi_msg (SSI_LOG_LEVEL_DETAIL, "Calling GetFirstPin with Grabber...");

	IPin *pPin = NULL;

	pPin = CameraTools::GetFirstPin(_pGrabber, PINDIR_OUTPUT);
	if(pPin != NULL)
	{
		// ssi_msg (SSI_LOG_LEVEL_DETAIL, "...SUCCEEDED!");
	}
	else
	{
		ssi_err ("Get Output Pin of Grabber Device");
		return false;
	}

	SSI_DBG (SSI_LOG_LEVEL_DETAIL, "Try to Grabber-Interface..."); 
	
	hr = _pGrabber->QueryInterface(IID_IUAProxyForceGrabber, (LPVOID *)&_pGrabInterface);
	if(SUCCEEDED(hr))
	{
		// ssi_msg (SSI_LOG_LEVEL_DETAIL, "...SUCCEEDED!");
	}
	else
	{
		ssi_err ("Query Grabber Interface");
		SafeReleaseFJ(pPin);
		return false;
	}

	hr = CameraTools::ConnectToNullRenderer(pPin, _pGraph);

	if(SUCCEEDED(hr))
	{
		// ssi_msg (SSI_LOG_LEVEL_DETAIL, "...SUCCEEDED!");
	}
	else
	{
		SafeReleaseFJ(pPin);
		ssi_err ("Could not Render Graph to NullRenderer");
		
		return false;
	}

	SafeReleaseFJ(pPin);

	SSI_DBG (SSI_LOG_LEVEL_DEBUG, "Activating Framebuffering in Filter...");
	
	hr = _pGrabInterface->ToggleCallFrameBufferingForGrabbing(true);
	if(SUCCEEDED(hr))
	{
		// ssi_msg (SSI_LOG_LEVEL_DETAIL, "...SUCCEEDED!");
	}
	else
	{
		ssi_err ("Activation of FrameBuffering for Grabbing");
		return false;
	}

	hr = _pControl->Run();
	if(SUCCEEDED(hr))
	{
		FILTER_STATE oaf;
		while(hr != S_OK)
		{
			hr = _pControl->GetState(INFINITE, (OAFilterState*)&oaf);
			Sleep(100);

		}

		hr = E_FAIL;
		int bla = 0;
		while(hr == E_FAIL)
		{
			hr = _pGrabInterface->GrabFrame(NULL, &bla, NULL);
			Sleep(100);
		}
		// ssi_msg (SSI_LOG_LEVEL_DETAIL, "...SUCCEEDED!");
	}
	else
	{
		ssi_err ("Running Graph");
		return false;
	}

	_timer = 0;

	ssi_msg (SSI_LOG_LEVEL_BASIC, "connected");
	Lock lock (_setProviderMutex);
	{
		if (_provider && ssi_log_level >= SSI_LOG_LEVEL_DETAIL) {
				ssi_print ("\
             width\t= %d\n\
             height\t= %d\n\
             frame\t= %f\n\
             dim\t= %u\n\
             bytes\t= %u\n",
				 _options.params.widthInPixels,
				 _options.params.heightInPixels,
				 _options.params.framesPerSecond,
				 1, 
				 ssi_video_size (_options.params));
		}
	}

	return true;
}

void Camera::run()
{	
	if (!_timer)
	{
		_timer = new Timer (1.0 / _options.params.framesPerSecond);
	}

	int oldSizeOfPicData = _sizeOfPicData;
	
	HRESULT hr = _pGrabInterface->GrabFrame(_picData, &_sizeOfPicData,(void*) &_cbIH);
	switch(hr)
	{
		case NOERROR:
		{
			if(_options.params.flipImage)
			{
				CameraTools::FlipImage (_picDataFlipBuffer, _picData, _options.params);
			}
			Lock lock (_setProviderMutex);
			if (_provider)
			{
				_provider->provide(_options.params.flipImage ? reinterpret_cast<char *>(_picDataFlipBuffer) : reinterpret_cast<char *>(_picData), SSI_CAMERA_SAMPLES_PER_STEP_TO_BUFFER);
			}
			break;
		}
		case E_FAIL:

			break;

		case E_ABORT:
			if(oldSizeOfPicData != _sizeOfPicData)
			{
				if(_picData != NULL)
				{
					delete [] _picData;
					_picData = NULL;
				}
				if(_picDataFlipBuffer != NULL)
				{
					delete []_picDataFlipBuffer;
					_picDataFlipBuffer = NULL;
				}
				_picData = new BYTE[_sizeOfPicData];
				_picDataFlipBuffer = new BYTE[_sizeOfPicData];
			}
			break;

		case E_POINTER:
			if((_picData == NULL) && (_sizeOfPicData > 0))
			{
				if(_picDataFlipBuffer == NULL)
				{
					_picDataFlipBuffer = new BYTE[_sizeOfPicData];
				}
				_picData = new BYTE[_sizeOfPicData];
			}
			break;

		default:
			break;
	}

	_timer->wait ();
}

bool Camera::disconnect()
{
	//TODO forgot something?
	ssi_msg (SSI_LOG_LEVEL_BASIC, "try to disconnect sensor...");

	if(_pControl)
		_pControl->Stop();
	//SafeReleaseFJ(_pCamQualInterface);
	SafeReleaseFJ(_pGrabInterface);
	SafeReleaseFJ(_pGrabber);
	SafeReleaseFJ(_pControl);
	SafeReleaseFJ(_pCapDevice);
    SafeReleaseFJ(_pBuild);
	SafeReleaseFJ(_pGraph);

	//if(!(_stlListQualityProps.empty()))
	//{
	//	_stlListQualityProps.clear();
	//}

	if(_comInitCountConnectRunDisconnect > 0)
	{
		CoUninitialize();
		--_comInitCountConnectRunDisconnect;
	}
	
	delete _timer;	

	ssi_msg (SSI_LOG_LEVEL_BASIC, "sensor disconnected");

	return true;
}

HRESULT Camera::GetCurrentSampleInStream (BYTE *sampleData, int *sampleBufferSize, bool flip, bool mirror)
{
	HRESULT hr = E_ABORT;

	if(flip)
	{
		int oldSizeOfPicData = _sizeOfPicData;
		if(_sizeOfPicData < *sampleBufferSize)
		{
			if(_picData != NULL)
			{
				delete[] _picData;
				_picData = NULL;
			}
			_picData = new BYTE[*sampleBufferSize];
			_sizeOfPicData = *sampleBufferSize;
		}
		hr = _pGrabInterface->GrabFrame(_picData, &_sizeOfPicData,(void*) &_cbIH);
		BYTE *dstPtr = NULL;
		BYTE *srcPtr = NULL;
		int copyLength = 0;
		dstPtr = sampleData + ((_cbIH.heightInPixels - 1) * _cbIH.widthStepInBytes);
		srcPtr = _picData;
		if(SUCCEEDED(hr))
		{
			switch(_cbIH.pInfoHeader->biBitCount)
			{
			case 24:
				copyLength = _cbIH.widthInPixels * 3;
				
				break;
			case 32:
				copyLength = _cbIH.widthInPixels * 4;
				break;
			default:
				return E_UNEXPECTED;
				break;
			}
			for(int j = 0; j < _cbIH.heightInPixels; ++j)
			{
				memcpy(dstPtr, srcPtr, copyLength);
				dstPtr -= _cbIH.widthStepInBytes;
				srcPtr += _cbIH.widthStepInBytes;
			}
		}
	}
	else
	{
		hr = _pGrabInterface->GrabFrame(sampleData, sampleBufferSize,(void*) &_cbIH);
	}

	return hr;
}

bool Camera::LetUserSelectMediaType()
{
	DialogLibGateway dialogGateway;

	if(!dialogGateway.didInitWork())
	{
		ssi_err("In Camera::LetUserSelectMediaType -> DialogLibGateWay initialisation failed and no fallback available");
		return false;
	}

	if(!dialogGateway.SetNewDialogType("PinAndMediaSelectionDialog"))
	{
		ssi_err_static("In Camera::LetUserSelectMediaType -> Could not set PinAndMediaSelectionDialog and no fallback available");
	}

	int intHandle = dialogGateway.AlterExistingItem("Caption", -1, "Select a MediaType");

	char myValueString[1000];

	for(int i = 0; i < _listOfVideoTypesInTheSpecifiedFilter->howManyPinsPresent(PINDIR_OUTPUT); ++i)
	{
		for(int j = 0; j < _listOfVideoTypesInTheSpecifiedFilter->howManyMediaInfosPresentForSpecificPin(PINDIR_OUTPUT, i); ++j)
		{
			VideoTypeInfo vidInfo = _listOfVideoTypesInTheSpecifiedFilter->getReferenceOfVideoTypeInfo(PINDIR_OUTPUT, i, j);
			GUID formatType = vidInfo.getFormatType();
			AM_MEDIA_TYPE *pMediaType = vidInfo.getMediaType();
			VIDEO_STREAM_CONFIG_CAPS *pSCC = vidInfo.getStreamConfigCaps();

			int ordinalNumber = i * 100 + j;

			//******** AM_MEDIA_TYPE ***********
			//majortype
			LPOLESTR psz;
			StringFromCLSID(pMediaType->majortype, &psz);
			char * myTmpValueString = _com_util::ConvertBSTRToString(psz);
			CoTaskMemFree(psz);
			dialogGateway.AlterExistingItem("majortype", ordinalNumber, myTmpValueString);
			delete[] myTmpValueString;
			myTmpValueString = NULL;
			//subtype
			StringFromCLSID(pMediaType->subtype, &psz);
			myTmpValueString = _com_util::ConvertBSTRToString(psz);
			CoTaskMemFree(psz);
			dialogGateway.AlterExistingItem("subtype", ordinalNumber, myTmpValueString);
			delete[] myTmpValueString;
			myTmpValueString = NULL;
			//bFixedSizeSamples
			dialogGateway.AlterExistingItem("bFixedSizeSamples", ordinalNumber, (pMediaType->bFixedSizeSamples) ? "True" : "False");
			//bTemporalCompression
			dialogGateway.AlterExistingItem("bTemporalCompression", ordinalNumber, (pMediaType->bTemporalCompression) ? "True" : "False");
			//lSampleSize
			ssi_sprint(myValueString, "%lu", pMediaType->lSampleSize);
			dialogGateway.AlterExistingItem("lSampleSize", ordinalNumber, myValueString);
			//formattype
			StringFromCLSID(pMediaType->formattype, &psz);
			myTmpValueString = _com_util::ConvertBSTRToString(psz);
			CoTaskMemFree(psz);
			dialogGateway.AlterExistingItem("formattype", ordinalNumber, myTmpValueString);
			delete[] myTmpValueString;
			myTmpValueString = NULL;
			//cbFormat
			ssi_sprint(myValueString, "%lu", pMediaType->cbFormat);
			dialogGateway.AlterExistingItem("cbFormat", ordinalNumber, myValueString);
			//*************************************************************************

			//**********************VIDEO_STREAM_CONFIG_CAPS *********************
			//guid
			StringFromCLSID(pSCC->guid, &psz);
			myTmpValueString = _com_util::ConvertBSTRToString(psz);
			CoTaskMemFree(psz);
			dialogGateway.AlterExistingItem("guid", ordinalNumber, myTmpValueString);
			delete[] myTmpValueString;
			myTmpValueString = NULL;
			//VideoStandard
			ssi_sprint(myValueString, "%lu", pSCC->VideoStandard);
			dialogGateway.AlterExistingItem("VideoStandard", ordinalNumber, myValueString);
			//InputSize
			ssi_sprint(myValueString, "%i_%i", pSCC->InputSize.cx, pSCC->InputSize.cy);
			dialogGateway.AlterExistingItem("InputSize", ordinalNumber, myValueString);
			//MinCroppingSize
			ssi_sprint(myValueString, "%i_%i", pSCC->MinCroppingSize.cx, pSCC->MinCroppingSize.cy);
			dialogGateway.AlterExistingItem("MinCroppingSize", ordinalNumber, myValueString);
			//MaxCroppingSize
			ssi_sprint(myValueString, "%i_%i", pSCC->MaxCroppingSize.cx, pSCC->MaxCroppingSize.cy);
			dialogGateway.AlterExistingItem("MaxCroppingSize", ordinalNumber, myValueString);
			//CropGranularityX
			ssi_sprint(myValueString, "%i", pSCC->CropGranularityX);
			dialogGateway.AlterExistingItem("CropGranularityX", ordinalNumber, myValueString);
			//CropGranularityY
			ssi_sprint(myValueString, "%i", pSCC->CropGranularityY);
			dialogGateway.AlterExistingItem("CropGranularityY", ordinalNumber, myValueString);
			//CropAlignX
			ssi_sprint(myValueString, "%i", pSCC->CropAlignX);
			dialogGateway.AlterExistingItem("CropAlignX", ordinalNumber, myValueString);
			//CropAlignY
			ssi_sprint(myValueString, "%i", pSCC->CropAlignY);
			dialogGateway.AlterExistingItem("CropAlignY", ordinalNumber, myValueString);
			//MinOutputSize
			ssi_sprint(myValueString, "%i_%i", pSCC->MinOutputSize.cx, pSCC->MinOutputSize.cy);
			dialogGateway.AlterExistingItem("MinOutputSize", ordinalNumber, myValueString);
			//MaxOutputSize
			ssi_sprint(myValueString, "%i_%i", pSCC->MaxOutputSize.cx, pSCC->MaxOutputSize.cy);
			dialogGateway.AlterExistingItem("MaxOutputSize", ordinalNumber, myValueString);
			//OutputGranularityX
			ssi_sprint(myValueString, "%i", pSCC->OutputGranularityX);
			dialogGateway.AlterExistingItem("OutputGranularityX", ordinalNumber, myValueString);
			//OutputGranularityY
			ssi_sprint(myValueString, "%i", pSCC->OutputGranularityY);
			dialogGateway.AlterExistingItem("OutputGranularityY", ordinalNumber, myValueString);
			//StretchTapsX
			ssi_sprint(myValueString, "%i", pSCC->StretchTapsX);
			dialogGateway.AlterExistingItem("StretchTapsX", ordinalNumber, myValueString);
			//StretchTapsY
			ssi_sprint(myValueString, "%i", pSCC->StretchTapsY);
			dialogGateway.AlterExistingItem("StretchTapsY", ordinalNumber, myValueString);
			//ShrinkTapsX
			ssi_sprint(myValueString, "%i", pSCC->ShrinkTapsX);
			dialogGateway.AlterExistingItem("ShrinkTapsX", ordinalNumber, myValueString);
			//ShrinkTapsY
			ssi_sprint(myValueString, "%i", pSCC->ShrinkTapsY);
			dialogGateway.AlterExistingItem("ShrinkTapsY", ordinalNumber, myValueString);
			//MinFrameInterval
			ssi_sprint(myValueString, "%lli", pSCC->MinFrameInterval);
			dialogGateway.AlterExistingItem("MinFrameInterval", ordinalNumber, myValueString);
			//MaxFrameInterval
			ssi_sprint(myValueString, "%lli", pSCC->MaxFrameInterval);
			dialogGateway.AlterExistingItem("MaxFrameInterval", ordinalNumber, myValueString);
			//MinBitsPerSecond
			ssi_sprint(myValueString, "%li", pSCC->MinBitsPerSecond);
			dialogGateway.AlterExistingItem("MinBitsPerSecond", ordinalNumber, myValueString);
			//MaxBitsPerSecond
			ssi_sprint(myValueString, "%li", pSCC->MaxBitsPerSecond);
			dialogGateway.AlterExistingItem("MaxBitsPerSecond", ordinalNumber, myValueString);
			//*************************************************************************
			

			if((IsEqualGUID(pMediaType->majortype, MEDIATYPE_Video) == TRUE) || (IsEqualGUID(pMediaType->majortype, MEDIATYPE_Interleaved) == TRUE))
			{
				if(IsEqualGUID(formatType, FORMAT_DvInfo) == TRUE)
				{
					if(pMediaType->cbFormat >= sizeof(DVINFO))
					{
						DVINFO *pDVI = (DVINFO*)pMediaType->pbFormat;
						//**********************DVINFO *********************
						//dwDVAAuxSrc
						ssi_sprint(myValueString, "%lu", pDVI->dwDVAAuxSrc);
						dialogGateway.AlterExistingItem("dwDVAAuxSrc", ordinalNumber, myValueString);
						//dwDVAAuxCtl
						ssi_sprint(myValueString, "%lu", pDVI->dwDVAAuxCtl);
						dialogGateway.AlterExistingItem("dwDVAAuxCtl", ordinalNumber, myValueString);
						//dwDVAAuxSrc1
						ssi_sprint(myValueString, "%lu", pDVI->dwDVAAuxSrc1);
						dialogGateway.AlterExistingItem("dwDVAAuxSrc1", ordinalNumber, myValueString);
						//dwDVAAuxCtl1
						ssi_sprint(myValueString, "%lu", pDVI->dwDVAAuxCtl1);
						dialogGateway.AlterExistingItem("dwDVAAuxCtl1", ordinalNumber, myValueString);
						//dwDVVAuxSrc
						ssi_sprint(myValueString, "%lu", pDVI->dwDVVAuxSrc);
						dialogGateway.AlterExistingItem("dwDVVAuxSrc", ordinalNumber, myValueString);
						//dwDVVAuxCtl
						ssi_sprint(myValueString, "%lu", pDVI->dwDVVAuxCtl);
						dialogGateway.AlterExistingItem("dwDVVAuxCtl", ordinalNumber, myValueString);
						//******************************************************
					}
				}
				else if(IsEqualGUID(formatType, FORMAT_VideoInfo) == TRUE)
				{
					if(pMediaType->cbFormat >= sizeof(VIDEOINFOHEADER))
					{
						VIDEOINFOHEADER *pVIH = (VIDEOINFOHEADER*)pMediaType->pbFormat;
						BITMAPINFOHEADER *pBmIH = &(pVIH->bmiHeader);
						//**********************VIDEOINFOHEADER *********************
						//rcSource
						ssi_sprint(myValueString, "%i_%i_%i_%i", pVIH->rcSource.left, pVIH->rcSource.top, pVIH->rcSource.right, pVIH->rcSource.bottom);
						dialogGateway.AlterExistingItem("rcSource", ordinalNumber, myValueString);
						//rcTarget
						ssi_sprint(myValueString, "%i_%i_%i_%i", pVIH->rcTarget.left, pVIH->rcTarget.top, pVIH->rcTarget.right, pVIH->rcTarget.bottom);
						dialogGateway.AlterExistingItem("rcTarget", ordinalNumber, myValueString);
						//dwBitRate
						ssi_sprint(myValueString, "%lu", pVIH->dwBitRate);
						dialogGateway.AlterExistingItem("dwBitRate", ordinalNumber, myValueString);
						//dwBitErrorRate
						ssi_sprint(myValueString, "%lu", pVIH->dwBitErrorRate);
						dialogGateway.AlterExistingItem("dwBitErrorRate", ordinalNumber, myValueString);
						//AvgTimePerFrame
						ssi_sprint(myValueString, "%lli", pVIH->AvgTimePerFrame);
						dialogGateway.AlterExistingItem("AvgTimePerFrame", ordinalNumber, myValueString);
						//******************************************************
						//**********************BITMAPINFOHEADER *********************
						//biSize
						ssi_sprint(myValueString, "%lu", pBmIH->biSize);
						dialogGateway.AlterExistingItem("biSize", ordinalNumber, myValueString);
						//biWidth
						ssi_sprint(myValueString, "%li", pBmIH->biWidth);
						dialogGateway.AlterExistingItem("biWidth", ordinalNumber, myValueString);
						//biHeight
						ssi_sprint(myValueString, "%li", pBmIH->biHeight);
						dialogGateway.AlterExistingItem("biHeight", ordinalNumber, myValueString);
						//biPlanes
						ssi_sprint(myValueString, "%hu", pBmIH->biPlanes);
						dialogGateway.AlterExistingItem("biPlanes", ordinalNumber, myValueString);
						//biBitCount
						ssi_sprint(myValueString, "%hu", pBmIH->biBitCount);
						dialogGateway.AlterExistingItem("biBitCount", ordinalNumber, myValueString);
						//biCompression
						ssi_sprint(myValueString, "%lu", pBmIH->biCompression);
						dialogGateway.AlterExistingItem("biCompression", ordinalNumber, myValueString);
						//biSizeImage
						ssi_sprint(myValueString, "%lu", pBmIH->biSizeImage);
						dialogGateway.AlterExistingItem("biSizeImage", ordinalNumber, myValueString);
						//biXPelsPerMeter
						ssi_sprint(myValueString, "%li", pBmIH->biXPelsPerMeter);
						dialogGateway.AlterExistingItem("biXPelsPerMeter", ordinalNumber, myValueString);
						//biYPelsPerMeter
						ssi_sprint(myValueString, "%li", pBmIH->biYPelsPerMeter);
						dialogGateway.AlterExistingItem("biYPelsPerMeter", ordinalNumber, myValueString);
						//biClrUsed
						ssi_sprint(myValueString, "%lu", pBmIH->biClrUsed);
						dialogGateway.AlterExistingItem("biClrUsed", ordinalNumber, myValueString);
						//biClrImportant
						ssi_sprint(myValueString, "%lu", pBmIH->biClrImportant);
						dialogGateway.AlterExistingItem("biClrImportant", ordinalNumber, myValueString);
						//******************************************************
					}
				}
			}
		}
	}

	dialogGateway.AlterExistingItem("Help", -1, "No further help available");

	int retVal = dialogGateway.RunDialog();

	if(retVal > -1)
	{
		int width;
		if(!dialogGateway.RetrieveInt("WidthInPixels", &width))
		{
			ssi_err("Error retrieving WidthInPixels");
			return false;
		}
		int height;
		if(!dialogGateway.RetrieveInt("HeightInPixels", &height))
		{
			ssi_err("Error retrieving HeightInPixels");
			return false;
		}
		int depth;
		if(!dialogGateway.RetrieveInt("DepthInBitsPerChannel", &depth))
		{
			ssi_err("Error retrieving DepthInBitsPerChannel");
			return false;
		}
		int channels;
		if(!dialogGateway.RetrieveInt("NumOfChannels", &channels))
		{
			ssi_err("Error retrieving NumOfChannels");
			return false;
		}
		double fps;
		if(!dialogGateway.RetrieveDouble("FramesPerSecond", &fps))
		{
			ssi_err("Error retrieving FramesPerSecond");
			return false;
		}
		int tmp;
		
		if(!dialogGateway.RetrieveInt("UseClosestFramerateForGraph", &tmp))
		{
			ssi_err("Error retrieving UseClosestFramerateForGraph");
			return false;
		}
		bool closeFPS = (tmp > 0) ? true : false;

		if(!dialogGateway.RetrieveInt("FlipImage", &tmp))
		{
			ssi_err("Error retrieving FlipImage");
			return false;
		}
		bool flip = (tmp > 0) ? true : false;

		int majortype;
		if(!dialogGateway.RetrieveInt("MajorVideoType", &majortype))
		{
			ssi_err("Error retrieving MajorVideoType");
			return false;
		}

		char *subtype;
		if(!dialogGateway.RetrieveString("OutputSubTypeOfCaptureDevice", &subtype))
		{
			ssi_err("Error retrieving MajorVideoType");
			return false;
		}
		
		BSTR bstr = _com_util::ConvertStringToBSTR(subtype);

		GUID subtypeguid;
		IIDFromString(bstr, &subtypeguid);

		delete[] subtype;
		SysFreeString(bstr);

		ssi_video_params (_options.params, width, height, fps, depth, channels, subtypeguid, closeFPS, flip, true, majortype);
	}

	return true;
}

}
