// CameraTools.cpp
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

#include "CameraTools.h"

#include <iostream>
using namespace std;

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

//#define FRANKYS_DEEP_DEBUG_FLAG

#ifndef CLSID_NullRenderer
DEFINE_GUID(CLSID_NullRenderer, 0xC1F400A4, 0x3F08, 0x11d3, 0x9F, 0x0B,
	    0x00, 0x60, 0x08, 0x03, 0x9E, 0x37);
#endif 

namespace ssi {

HRESULT CameraTools::InitCaptureGraphBuilder(IGraphBuilder **ppGraph, ICaptureGraphBuilder2 **ppBuild)
{
    if (!ppGraph || !ppBuild)
    {
        return E_POINTER;
    }
    IGraphBuilder *pGraph = NULL;
    ICaptureGraphBuilder2 *pBuild = NULL;

    // Create the Capture Graph Builder.
	SSI_DBG_STATIC (SSI_LOG_LEVEL_DEBUG, "Try to CoCreate CapureGraphBuilder...");
	
    HRESULT hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, 
        CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void**)&pBuild );
    if (SUCCEEDED(hr))
    {
		SSI_DBG_STATIC (SSI_LOG_LEVEL_VERBOSE, "SUCCEEDED!");
		
        // Create the Filter Graph Manager.
		SSI_DBG_STATIC (SSI_LOG_LEVEL_DEBUG, "Try to CoCreate FilterGraph...");
		
        hr = CoCreateInstance(CLSID_FilterGraph, 0, CLSCTX_INPROC_SERVER,
            IID_IGraphBuilder, (void**)&pGraph);
        if (SUCCEEDED(hr))
        {
            // Initialize the Capture Graph Builder.
			SSI_DBG_STATIC (SSI_LOG_LEVEL_VERBOSE, "SUCCEEDED!");
			
			SSI_DBG_STATIC (SSI_LOG_LEVEL_DEBUG, "Try to Initialize Graph with CaptureGraph...");
			
            hr = pBuild->SetFiltergraph(pGraph);
			if(SUCCEEDED(hr))
			{
				SSI_DBG_STATIC (SSI_LOG_LEVEL_VERBOSE, "SUCCEEDED!");
				
				// Return both interface pointers to the caller.
				*ppBuild = pBuild;
				*ppGraph = pGraph; // The caller must release both interfaces.
				return S_OK;
			}
			else
			{
				SSI_DBG_STATIC (SSI_LOG_LEVEL_DEBUG, "FAILED!");
				
				SafeReleaseFJ(pBuild);
				SafeReleaseFJ(pGraph);
			}
		}
        else
        {
			SSI_DBG_STATIC (SSI_LOG_LEVEL_DEBUG, "FAILED!");

            SafeReleaseFJ(pBuild);
        }
	}
	
    return hr; // Failed
}

// if 'nameOfCam' is NULL the camera device with index 'indexOfCam' is taken
HRESULT CameraTools::SelectCaptureDevice(ssi_char_t *nameOfCam, ssi_size_t indexOfCam, IBaseFilter **ppCapFilter, IGraphBuilder *pGraph)
{
	IBaseFilter *pCap = NULL;
	ICreateDevEnum *pDevEnum = NULL;
	IEnumMoniker *pEnum = NULL;
	bool selectByIndex = false;
	bool cameraFound = false;
	
	// Create the System Device Enumerator.
	SSI_DBG_STATIC (SSI_LOG_LEVEL_DEBUG, "Try to CoCreate DeviceEnumerator...");
	
	HRESULT returnCode = S_OK;
	HRESULT hr2;
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,
    CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, reinterpret_cast<void**>(&pDevEnum));
	if (SUCCEEDED(hr))
	{
		// SSI_DBG_STATIC (SSI_LOG_LEVEL_DEBUG, "SUCCEEDED!");
		// Create an enumerator for the video capture category.
		SSI_DBG_STATIC (SSI_LOG_LEVEL_DEBUG, "Try to CoCreate ClassEnumerator...");
		
		hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnum, 0);
		if (hr != S_OK) 
		{
			SSI_DBG_STATIC (SSI_LOG_LEVEL_DEBUG, "FAILED!");
			
			SafeReleaseFJ(pDevEnum);
			return E_UNEXPECTED;
		}

		// SSI_DBG_STATIC (SSI_LOG_LEVEL_DEBUG, "SUCCEEDED!");
		
		int count = 0;
		IMoniker *pMoniker = NULL;
		if(nameOfCam == NULL)
		{
			ssi_msg_static (SSI_LOG_LEVEL_DETAIL, "No specific camera wanted, selecting camera with index %u...", indexOfCam);
			selectByIndex = true;
		}
		else
		{
			ssi_msg_static (SSI_LOG_LEVEL_BASIC, "Trying to select camera...\n%s", nameOfCam);
		}
		SSI_DBG_STATIC (SSI_LOG_LEVEL_DEBUG, "Accessing available cameras...");
		
		while (pEnum->Next(1, &pMoniker, NULL) == S_OK)
		{
			if (cameraFound)
				break;

			count++;
			SSI_DBG_STATIC (SSI_LOG_LEVEL_DEBUG, "Access camera %d ...", count);
			
			IPropertyBag *pPropBag;
			hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, 
				(void**)(&pPropBag));
			if (FAILED(hr))
			{
				SSI_DBG_STATIC (SSI_LOG_LEVEL_DEBUG, "FAILED/EMPTY!");

				pMoniker->Release();
				continue;  // Skip this one, maybe the next one will work.
			} 
			// Find the description or friendly name.
			VARIANT varName;
			VariantInit(&varName);
			VARIANT varName2;
			VariantInit(&varName2);
			hr = pPropBag->Read(L"DevicePath", &varName, 0);
			hr2 = pPropBag->Read(L"FriendlyName", &varName2, 0);
			if (SUCCEEDED(hr) || SUCCEEDED(hr2))
			{
				char* lpszText = _com_util::ConvertBSTRToString(varName.bstrVal);
				char* lpszText2 = _com_util::ConvertBSTRToString(varName2.bstrVal);
				
				SSI_DBG_STATIC (SSI_LOG_LEVEL_DEBUG, "camera %d:\nfriendly name: %s\n%s", count, lpszText2, lpszText);
				
				if(cameraFound == false)
				{
					if(selectByIndex == false)
					{
						bool strComp1 = false;
						bool strComp2 = false;
						if (SUCCEEDED(hr))
						{
							strComp1 = (strcmp(lpszText, nameOfCam)==0);
						}
						if (SUCCEEDED(hr))
						{
							strComp2 = (strcmp(lpszText2, nameOfCam)==0);
						}
						if(strComp2 || strComp1)
						{
							cameraFound = true;

							ssi_msg_static (SSI_LOG_LEVEL_DETAIL, "Selecting camera with index %d:\nfriendly name: %s\n%s", count-1, lpszText2, lpszText);

							SSI_DBG_STATIC (SSI_LOG_LEVEL_DEBUG, "\n		******Cam Found...\n		******Try to attach to FilterGraph...\n		******Binding to Object...");
							
							hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&pCap);
							if (SUCCEEDED(hr))
							{
								SSI_DBG_STATIC (SSI_LOG_LEVEL_DEBUG, "		******Adding Filter to Graph...");
								
								hr = pGraph->AddFilter(pCap, L"Capture Filter");
								if (SUCCEEDED(hr))
								{
									// SSI_DBG_STATIC (SSI_LOG_LEVEL_DEBUG, "SUCCEEDED!");
									
									*ppCapFilter = pCap;
								}
								else
								{
									SSI_DBG_STATIC (SSI_LOG_LEVEL_DEBUG, "FAILED!");
									
									returnCode = E_UNEXPECTED;
								}

							}
							else
							{
								SSI_DBG_STATIC (SSI_LOG_LEVEL_DEBUG, "FAILED!");
							}
						}//StrCmp
						else
						{
							;
						}//else StrCompare
					}//selectByIndex
					else if (indexOfCam + 1 == count)
					{
						SSI_DBG_STATIC (SSI_LOG_LEVEL_DEBUG, "\n		******Cam Found...\n		******Try to attach to FilterGraph...\n		******Binding to Object...");
						
						hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&pCap);
						if (SUCCEEDED(hr))
						{
							SSI_DBG_STATIC (SSI_LOG_LEVEL_DEBUG, "\n		******Adding Filter to Graph...");
							
							hr = pGraph->AddFilter(pCap, L"Capture Filter");
							if (SUCCEEDED(hr))
							{
								// SSI_DBG_STATIC (SSI_LOG_LEVEL_DEBUG, "SUCCEEDED!");
								
								*ppCapFilter = pCap;
								cameraFound = true;

								ssi_msg_static (SSI_LOG_LEVEL_DETAIL, "Selecting camera with index %d:\nfriendly name: %s\n%s", count-1, lpszText2, lpszText);
							}
							else
							{
								SSI_DBG_STATIC (SSI_LOG_LEVEL_DEBUG, "FAILED!");
								
								returnCode = E_UNEXPECTED;
							}

						}
						else
						{
							SSI_DBG_STATIC (SSI_LOG_LEVEL_DEBUG, "FAILED!");
						}
					}//else SelectFirst
				}//CameraFound
				delete[] lpszText;
				delete[] lpszText2;
				VariantClear(&varName);
				VariantClear(&varName2);
			}
			else
			{
				SSI_DBG_STATIC (SSI_LOG_LEVEL_DEBUG, "READ FAILED!");
			}
			pPropBag->Release();
			pMoniker->Release();
		}
	
	}
	else
	{
		SSI_DBG_STATIC (SSI_LOG_LEVEL_DEBUG, "FAILED!");
		return hr;
	}
	
	SafeReleaseFJ(pEnum);
	SafeReleaseFJ(pDevEnum);
	return returnCode;

}

HRESULT CameraTools::QueryInterfaces(IGraphBuilder *pGraph, IMediaControl **ppControl, IMediaEvent **ppEvent, IVideoWindow **ppVidWin, IMediaSeeking **ppMediaSeek, IBasicVideo **ppBasicVideo)
{
	IVideoWindow *pVidWin = NULL;
	IMediaControl *pControl = NULL;
	IMediaEvent *pEvent = NULL;
	IMediaSeeking *pMediaSeek = NULL;
	IBasicVideo *pBasicVideo = NULL;

	HRESULT hr = S_OK;
	if(ppControl != NULL)
	{
		#ifdef FRANKYS_DEEP_DEBUG_FLAG
		cout << "	Try to get MediaControl Interface...";
		#endif
		hr = pGraph->QueryInterface(IID_IMediaControl,(LPVOID *) &pControl);
		if(SUCCEEDED(hr))
		{
			#ifdef FRANKYS_DEEP_DEBUG_FLAG
			cout << "...SUCCEEDED!" << endl;
			#endif
		}
		else
		{
			#ifdef FRANKYS_DEEP_DEBUG_FLAG
			cout << "...FAILED!" << endl;
			#endif
			return hr;
		}
	}

	if(ppVidWin != NULL)
	{
		#ifdef FRANKYS_DEEP_DEBUG_FLAG
		cout << "	Try to get VideoWindow Interface...";
		#endif
		hr = pGraph->QueryInterface(IID_IVideoWindow, (LPVOID *) &pVidWin);
		if(SUCCEEDED(hr))
		{
			#ifdef FRANKYS_DEEP_DEBUG_FLAG
			cout << "...SUCCEEDED!" << endl;
			#endif
		}
		else
		{
			#ifdef FRANKYS_DEEP_DEBUG_FLAG
			cout << "...FAILED!" << endl;
			#endif
			SafeReleaseFJ(pControl);
			return hr;
		}
	}
    
	if(ppEvent != NULL)
	{
		#ifdef FRANKYS_DEEP_DEBUG_FLAG
		cout << "	Try to get MediaEvent Interface...";
		#endif
		hr = pGraph->QueryInterface(IID_IMediaEvent, (LPVOID *) &pEvent);
		if(SUCCEEDED(hr))
		{
			#ifdef FRANKYS_DEEP_DEBUG_FLAG
			cout << "...SUCCEEDED!" << endl;
			#endif
		}
		else
		{
			#ifdef FRANKYS_DEEP_DEBUG_FLAG
			cout << "...FAILED!" << endl;
			#endif
			SafeReleaseFJ(pControl);
			SafeReleaseFJ(pVidWin);
			return hr;
		}
	}

	if(ppMediaSeek != NULL)
	{
		#ifdef FRANKYS_DEEP_DEBUG_FLAG
		cout << "	Try to get MediaSeek Interface...";
		#endif
		hr = pGraph->QueryInterface(IID_IMediaSeeking, (LPVOID *) &pMediaSeek);
		if(SUCCEEDED(hr))
		{
			#ifdef FRANKYS_DEEP_DEBUG_FLAG
			cout << "...SUCCEEDED!" << endl;
			#endif
		}
		else
		{
			#ifdef FRANKYS_DEEP_DEBUG_FLAG
			cout << "...FAILED!" << endl;
			#endif
			SafeReleaseFJ(pControl);
			SafeReleaseFJ(pVidWin);
			SafeReleaseFJ(pEvent);
			return hr;
		}
	}
	
	if(ppBasicVideo != NULL)
	{
		#ifdef FRANKYS_DEEP_DEBUG_FLAG
		cout << "	Try to get BasicVideo Interface...";
		#endif
		hr = pGraph->QueryInterface(IID_IBasicVideo, (LPVOID *) &pBasicVideo);
		if(SUCCEEDED(hr))
		{
			#ifdef FRANKYS_DEEP_DEBUG_FLAG
			cout << "...SUCCEEDED!" << endl;
			#endif
		}
		else
		{
			#ifdef FRANKYS_DEEP_DEBUG_FLAG
			cout << "...FAILED!" << endl;
			#endif
			SafeReleaseFJ(pControl);
			SafeReleaseFJ(pVidWin);
			SafeReleaseFJ(pEvent);
			SafeReleaseFJ(pMediaSeek);
			return hr;
		}
	}

	if(ppEvent != NULL)
		*ppEvent = pEvent;
	if(ppControl != NULL)
		*ppControl = pControl;
	if(ppVidWin != NULL)
		*ppVidWin = pVidWin;
	if(ppMediaSeek != NULL)
		*ppMediaSeek = pMediaSeek;
	if(ppBasicVideo != NULL)
		*ppBasicVideo = pBasicVideo;

	return S_OK;
}

HRESULT CameraTools::SelectMediaTypeOfCam(IBaseFilter *pCapFilter, ICaptureGraphBuilder2 *pBuild, int desiredFPS, GUID desiredMediaSubType, LONG desiredWidth, LONG desiredHeight)
{
	IAMStreamConfig *pConfig = NULL;
	#ifdef FRANKYS_DEEP_DEBUG_FLAG
	cout << "	Try to find Interface for Camera Stream Configuration...";
	#endif
	HRESULT hr = pBuild->FindInterface(&PIN_CATEGORY_CAPTURE, // Preview pin.
								0,    // Any media type.
								pCapFilter, // Pointer to the capture filter.
								IID_IAMStreamConfig, (void**)&pConfig);
	if(SUCCEEDED(hr))
	{
		#ifdef FRANKYS_DEEP_DEBUG_FLAG
		cout <<"SUCCEEDED!" <<endl;
		#endif
	}
	else
	{
		#ifdef FRANKYS_DEEP_DEBUG_FLAG
		cout << "FAILED!" << endl;
		#endif
		return hr;
	}

	int iCount = 0, iSize = 0;
	#ifdef FRANKYS_DEEP_DEBUG_FLAG
	cout << "	Determining number of Capabilities...";
	#endif
	hr = pConfig->GetNumberOfCapabilities(&iCount, &iSize);
	if(SUCCEEDED(hr))
	{
		#ifdef FRANKYS_DEEP_DEBUG_FLAG
		cout <<"SUCCEEDED!" <<endl;
		#endif
	}
	else
	{
		#ifdef FRANKYS_DEEP_DEBUG_FLAG
		cout << "FAILED!" << endl;
		#endif
		SafeReleaseFJ(pConfig);
		return hr;
	}

	// Check the size to make sure we pass in the correct structure.
	//	Audio uses same Interface...
	if (iSize == sizeof(VIDEO_STREAM_CONFIG_CAPS))
	{
		// Use the video capabilities structure.

		for (int iFormat = 0; iFormat < iCount; iFormat++)
		{
			VIDEO_STREAM_CONFIG_CAPS scc;
			AM_MEDIA_TYPE *pmtConfig;
			hr = pConfig->GetStreamCaps(iFormat, &pmtConfig, (BYTE*)&scc);
			if (SUCCEEDED(hr))
			{
				//*
				if ((pmtConfig->majortype == MEDIATYPE_Video) &&
					(pmtConfig->subtype == desiredMediaSubType) &&
					(pmtConfig->formattype == FORMAT_VideoInfo) &&
					(pmtConfig->cbFormat >= sizeof (VIDEOINFOHEADER)) &&
					(pmtConfig->pbFormat != NULL))
				{

					if(desiredMediaSubType == MEDIASUBTYPE_RGB24)
					{
						#ifdef FRANKYS_DEEP_DEBUG_FLAG
						cout << "\n		...Displaying info for Video RGB24" <<endl;
						#endif
					}
					if(desiredMediaSubType == MEDIASUBTYPE_RGB32)
					{
						#ifdef FRANKYS_DEEP_DEBUG_FLAG
						cout << "\n		...Displaying info for Video RGB32" <<endl;
						#endif
					}
					VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*)pmtConfig->pbFormat;
					//SCC
					#ifdef FRANKYS_DEEP_DEBUG_FLAG
					cout << "Input Size X: " << scc.InputSize.cx << " Y: "<< scc.InputSize.cy << endl;
					cout << "MinCrop Size X: " << scc.MinCroppingSize.cx << " Y: "<< scc.MinCroppingSize.cy << endl;
					cout << "MaxCrop Size X: " << scc.MaxCroppingSize.cx << " Y: "<< scc.MaxCroppingSize.cy << endl;
					cout << "CropGranul X: " << scc.CropGranularityX << " Y: "<< scc.CropGranularityY << endl;
					cout << "Crop Align X: " << scc.CropAlignX << " Y: "<< scc.CropAlignY << endl;
					cout << "Min Output Size X: " << scc.MinOutputSize.cx << " Y: "<< scc.MinOutputSize.cy << endl;
					cout << "Max Output Size X: " << scc.MaxOutputSize.cx << " Y: "<< scc.MaxOutputSize.cy << endl;
					cout << "Output Granul X: " << scc.OutputGranularityX << " Y: "<< scc.OutputGranularityY << endl;
					cout << "Shrink Taps X: " << scc.ShrinkTapsX << " Y: "<< scc.ShrinkTapsY << endl;
					cout << "Stretch Taps X: " << scc.StretchTapsX << " Y: "<< scc.StretchTapsY << endl;
					cout << "Frame Int Min: " << 10000000 / scc.MinFrameInterval << " Max: "<< 10000000 / scc.MaxFrameInterval << endl;
					cout << "bPS Min: " << scc.MinBitsPerSecond << " Max: "<< scc.MaxBitsPerSecond << endl;
					cout << "\n";
					//VIH
					cout << "rcSource L: " << pVih->rcSource.left << " O: " << pVih->rcSource.top<< " R: " << pVih->rcSource.right << " U: " << pVih->rcSource.bottom << endl;
					cout << "rcTarget L: " << pVih->rcTarget.left << " O: " << pVih->rcTarget.top<< " R: " << pVih->rcTarget.right << " U: " << pVih->rcTarget.bottom << endl;
					cout << "BitRate: " << pVih->dwBitRate << endl;
					cout << "BitErrorRate: " << pVih->dwBitErrorRate << endl;
					cout << "FPS: " << 10000000 / pVih->AvgTimePerFrame << endl;
					cout << "\n";
					//BIH
					cout << "Width in Pixels: " << pVih->bmiHeader.biWidth << endl;
					cout << "Height in Pixels: " << pVih->bmiHeader.biHeight << endl;
					cout << "BitsPerPixel: " << pVih->bmiHeader.biBitCount << endl;
					cout << "Size of Image in Bytes: " << pVih->bmiHeader.biSizeImage << endl;
					#endif

					LONG lWidth = pVih->bmiHeader.biWidth;
					LONG lHeight = pVih->bmiHeader.biHeight;
					if(lWidth == desiredWidth && lHeight == desiredHeight)
					{
						#ifdef FRANKYS_DEEP_DEBUG_FLAG
						cout << "\n*******Found Desired Format, try to set it...";
						#endif
						((VIDEOINFOHEADER*)(pmtConfig->pbFormat))->AvgTimePerFrame = (REFERENCE_TIME)((REFERENCE_TIME)10000000 / (REFERENCE_TIME)desiredFPS);
						hr = pConfig->SetFormat(pmtConfig);
						if(SUCCEEDED(hr))
						{
							#ifdef FRANKYS_DEEP_DEBUG_FLAG
							cout <<"SUCCEEDED!" <<endl;
							cout <<"Try to get the new set Format..." << endl;
							#endif
							DeleteMediaType(pmtConfig);
							pmtConfig = NULL;
							hr = pConfig->GetFormat(&pmtConfig);
							if(SUCCEEDED(hr))
							{
								#ifdef FRANKYS_DEEP_DEBUG_FLAG
								cout <<"SUCCEEDED!" <<endl;
								cout <<"Display the new Format: " << endl;
								#endif
								pVih = (VIDEOINFOHEADER*)pmtConfig->pbFormat;
								//SCC
								#ifdef FRANKYS_DEEP_DEBUG_FLAG
								cout << "Input Size X: " << scc.InputSize.cx << " Y: "<< scc.InputSize.cy << endl;
								cout << "MinCrop Size X: " << scc.MinCroppingSize.cx << " Y: "<< scc.MinCroppingSize.cy << endl;
								cout << "MaxCrop Size X: " << scc.MaxCroppingSize.cx << " Y: "<< scc.MaxCroppingSize.cy << endl;
								cout << "CropGranul X: " << scc.CropGranularityX << " Y: "<< scc.CropGranularityY << endl;
								cout << "Crop Align X: " << scc.CropAlignX << " Y: "<< scc.CropAlignY << endl;
								cout << "Min Output Size X: " << scc.MinOutputSize.cx << " Y: "<< scc.MinOutputSize.cy << endl;
								cout << "Max Output Size X: " << scc.MaxOutputSize.cx << " Y: "<< scc.MaxOutputSize.cy << endl;
								cout << "Output Granul X: " << scc.OutputGranularityX << " Y: "<< scc.OutputGranularityY << endl;
								cout << "Shrink Taps X: " << scc.ShrinkTapsX << " Y: "<< scc.ShrinkTapsY << endl;
								cout << "Stretch Taps X: " << scc.StretchTapsX << " Y: "<< scc.StretchTapsY << endl;
								cout << "Frame Int Min: " << 10000000 / scc.MinFrameInterval << " Max: "<< 10000000 / scc.MaxFrameInterval << endl;
								cout << "bPS Min: " << scc.MinBitsPerSecond << " Max: "<< scc.MaxBitsPerSecond << endl;
								cout << "\n";
								//VIH
								cout << "rcSource L: " << pVih->rcSource.left << " O: " << pVih->rcSource.top<< " R: " << pVih->rcSource.right << " U: " << pVih->rcSource.bottom << endl;
								cout << "rcTarget L: " << pVih->rcTarget.left << " O: " << pVih->rcTarget.top<< " R: " << pVih->rcTarget.right << " U: " << pVih->rcTarget.bottom << endl;
								cout << "BitRate: " << pVih->dwBitRate << endl;
								cout << "BitErrorRate: " << pVih->dwBitErrorRate << endl;
								cout << "FPS: " << 10000000 / pVih->AvgTimePerFrame << endl;
								cout << "\n";
								//BIH
								cout << "Width in Pixels: " << pVih->bmiHeader.biWidth << endl;
								cout << "Height in Pixels: " << pVih->bmiHeader.biHeight << endl;
								cout << "BitsPerPixel: " << pVih->bmiHeader.biBitCount << endl;
								cout << "Size of Image in Bytes: " << pVih->bmiHeader.biSizeImage << endl;
								#endif
							}
							else
							{
								#ifdef FRANKYS_DEEP_DEBUG_FLAG
								cout << "FAILED!" << endl;
								#endif
							}
						}
						else
						{
							#ifdef FRANKYS_DEEP_DEBUG_FLAG
							cout << "FAILED!" << endl;
							#endif
						}
					}
				}
				
				if(pmtConfig != NULL)
					DeleteMediaType(pmtConfig);
			}
		}

	SafeReleaseFJ(pConfig);
	}
	return S_OK;
}

IPin* CameraTools::GetFirstPin(IBaseFilter *pFilter, PIN_DIRECTION PinDir)

{

    BOOL       bFound = FALSE;
    IEnumPins  *pEnum = NULL;
    IPin       *pPin = NULL;

	if(pFilter == NULL)
		return NULL;

	#ifdef FRANKYS_DEEP_DEBUG_FLAG
	cout << "	Try to enumerate Pins...";
	#endif
	HRESULT hr = pFilter->EnumPins(&pEnum);
	if(SUCCEEDED(hr))
	{
		#ifdef FRANKYS_DEEP_DEBUG_FLAG
		cout << "...SUCCEEDED!" << endl;
		#endif
	}
	else
	{
		#ifdef FRANKYS_DEEP_DEBUG_FLAG
		cout << "...FAILED!" << endl;
		#endif
		return NULL;
	}

	#ifdef FRANKYS_DEEP_DEBUG_FLAG
	cout << "	Going through Pins...";
	#endif
    while(pEnum->Next(1, &pPin, 0) == S_OK)
    {
        PIN_DIRECTION PinDirThis;
        pPin->QueryDirection(&PinDirThis);
        if (PinDir == PinDirThis)
		{
			bFound = true;
            break;
		}
        SafeReleaseFJ(pPin);
    }

    SafeReleaseFJ(pEnum);

	if(bFound)
	{
		#ifdef FRANKYS_DEEP_DEBUG_FLAG
		cout << "...FOUND ONE -> SUCCEEDED" << endl;
		#endif
	}
	else
	{
		#ifdef FRANKYS_DEEP_DEBUG_FLAG
		cout << "...FOUND NONE -> FAILED" << endl;
		#endif
	}
    return (bFound ? pPin : NULL); 

}

HRESULT CameraTools::AddAndConnectFilterByCLSIDToGivenOutputPin(const GUID& clsidOfTheFilter, LPCWSTR wszName, IGraphBuilder *pGraph, IPin *outputPin)
{

	IBaseFilter* baseFilter = NULL;

	//char tmp[100];

	#ifdef FRANKYS_DEEP_DEBUG_FLAG
	cout << "Calling AddFilterToGraphByCLSID..." << endl;
	#endif
	HRESULT hr = AddFilterToGraphByCLSID(pGraph, clsidOfTheFilter, wszName, &baseFilter);
	if(SUCCEEDED(hr))
	{
		#ifdef FRANKYS_DEEP_DEBUG_FLAG
		cout << "SUCCEEDED!" << endl;
		#endif
	}
	else
	{
		#ifdef FRANKYS_DEEP_DEBUG_FLAG
		cout << "FAILED!" << endl;
		#endif
		return hr;
	}

	#ifdef FRANKYS_DEEP_DEBUG_FLAG
	cout << "Calling GetPin..." << endl;
	#endif
	IPin* inputPin = GetFirstPin(baseFilter, PINDIR_INPUT);

	if (!inputPin)
	{
		#ifdef FRANKYS_DEEP_DEBUG_FLAG
		cout << "FAILED!" << endl;
		#endif
		SafeReleaseFJ(baseFilter);
		return E_UNEXPECTED;
	}
	else
	{
		#ifdef FRANKYS_DEEP_DEBUG_FLAG
		cout << "SUCCEEDED!" << endl;
		#endif
	}

	#ifdef FRANKYS_DEEP_DEBUG_FLAG
	cout << "Trying to Connect the Pins...";
	#endif
	hr = pGraph->Connect(outputPin, inputPin);
	if(SUCCEEDED(hr))
	{
		#ifdef FRANKYS_DEEP_DEBUG_FLAG
		cout << "SUCCEEDED!" << endl;
		#endif
	}
	else
	{
		#ifdef FRANKYS_DEEP_DEBUG_FLAG
		cout << "FAILED!" << endl;
		#endif
		SafeReleaseFJ(baseFilter);
		SafeReleaseFJ(inputPin);
		return hr;
	}

	SafeReleaseFJ(baseFilter);
	SafeReleaseFJ(inputPin);

	return S_OK;

}

HRESULT CameraTools::AddFilterToGraphByCLSID(IGraphBuilder *pGraph, const GUID& clsidOfTheFilter, LPCWSTR wszName, IBaseFilter **ppFilterToBeAddedtoTheGraph)
{
    if (!pGraph || ! ppFilterToBeAddedtoTheGraph)
		return E_POINTER;

	HRESULT hr = 0;
    *ppFilterToBeAddedtoTheGraph = NULL;
    IBaseFilter *pF = 0;
	CUAProxyForceGrabber *pmyFilter = 0;
	CFakeCamPushSource *pmyFilter2 = 0;
	CFakeAudioPushSource *pmyFilter3 = 0;
	#ifdef FRANKYS_DEEP_DEBUG_FLAG
	cout << "Try to CreatInstance of the Filter...";
	#endif
	//*
	if(clsidOfTheFilter == CLSID_UAProxyForceGrabber)
	{
		pmyFilter = new CUAProxyForceGrabber(NAME("UAProxyForceGrabber"),0,&hr);
		if(SUCCEEDED(hr))
		{
			hr = pmyFilter->QueryInterface(IID_IBaseFilter, reinterpret_cast<void**>(&pF));
		}
	}
	else if(clsidOfTheFilter == CLSID_FakeCamPushSource)
	{
		pmyFilter2 = new CFakeCamPushSource(0,&hr);
		if(SUCCEEDED(hr))
		{
			hr = pmyFilter2->QueryInterface(IID_IBaseFilter, reinterpret_cast<void**>(&pF));
		}
	}
	else if(clsidOfTheFilter == CLSID_FakeAudioPushSource)
	{
		pmyFilter3 = new CFakeAudioPushSource(0,&hr);
		if(SUCCEEDED(hr))
		{
			hr = pmyFilter3->QueryInterface(IID_IBaseFilter, reinterpret_cast<void**>(&pF));
		}
	}
	else
	{
		hr = CoCreateInstance(clsidOfTheFilter, 0, CLSCTX_INPROC_SERVER,
			IID_IBaseFilter, reinterpret_cast<void**>(&pF));
	}
    if (SUCCEEDED(hr))
    {
		#ifdef FRANKYS_DEEP_DEBUG_FLAG
		cout << "SUCCEEDED!" << endl;
		cout << "Try to Add filter to Graph...";
		#endif
        hr = pGraph->AddFilter(pF, wszName);
        if (SUCCEEDED(hr))
		{
			#ifdef FRANKYS_DEEP_DEBUG_FLAG
			cout << "SUCCEEDED!" << endl;
			#endif
            *ppFilterToBeAddedtoTheGraph = pF;
		}
        else
		{
			#ifdef FRANKYS_DEEP_DEBUG_FLAG
			cout << "FAILED!" << endl;
			#endif
            SafeReleaseFJ(pF);
		}
    }
	else
	{
		#ifdef FRANKYS_DEEP_DEBUG_FLAG
		cout << "FAILED!" << endl;
		#endif
	}
    return hr;
}


HRESULT CameraTools::ConnectToNullRenderer(IBaseFilter *pFilterToConnectToNullRenderer, IGraphBuilder *pGraph)
{
	#ifdef FRANKYS_DEEP_DEBUG_FLAG
	cout << "Calling GetPin..." << endl;
	#endif
	IPin* outputPin = GetFirstPin(pFilterToConnectToNullRenderer, PINDIR_OUTPUT);

	if (!outputPin)
	{
		#ifdef FRANKYS_DEEP_DEBUG_FLAG
		cout << "FAILED!" << endl;
		#endif
		return E_UNEXPECTED;
	}
	else
	{
		#ifdef FRANKYS_DEEP_DEBUG_FLAG
		cout << "SUCCEEDED!" << endl;
		#endif
	}


	#ifdef FRANKYS_DEEP_DEBUG_FLAG
	cout << "Calling ConnectToNullRenderer with Pin..." << endl;
	#endif
	HRESULT hr = ConnectToNullRenderer(outputPin, pGraph);
	SafeReleaseFJ(outputPin);
	if (SUCCEEDED(hr))
	{
		#ifdef FRANKYS_DEEP_DEBUG_FLAG
		cout << "SUCCEEDED!" << endl;
		#endif
		
	}
	else
	{
		#ifdef FRANKYS_DEEP_DEBUG_FLAG
		cout << "FAILED!" << endl;
		#endif
		return hr;
	}

	return S_OK;
}

HRESULT CameraTools::ConnectToNullRenderer(IPin *pPinToConnectToNullRenderer, IGraphBuilder *pGraph)
{
	WCHAR name[] = L"NullRenderer";

	#ifdef FRANKYS_DEEP_DEBUG_FLAG
	cout << "Calling AddAndConnectFilterByCLSIDToGivenOutputPin..." << endl;
	#endif
	HRESULT hr = AddAndConnectFilterByCLSIDToGivenOutputPin(CLSID_NullRenderer, &name[0] , pGraph, pPinToConnectToNullRenderer);
	if(SUCCEEDED(hr))
	{
		#ifdef FRANKYS_DEEP_DEBUG_FLAG
		cout << "SUCCEEDED!" << endl;
		#endif
	}
	else
	{
		#ifdef FRANKYS_DEEP_DEBUG_FLAG
		cout << "FAILED!" << endl;
		#endif
		return hr;
	}

	return S_OK;
}

HRESULT CameraTools::RenderMyGraph(IGraphBuilder *pGraph, IPin *pOutputPintoRender)
{
	#ifdef FRANKYS_DEEP_DEBUG_FLAG
	cout << "Try to render Graph from Pin using GraphBuilder-Render method...";
	#endif
	HRESULT hr = pGraph->Render(pOutputPintoRender);
	if(hr == S_OK)
	{
		#ifdef FRANKYS_DEEP_DEBUG_FLAG
		cout << "...SUCCEEDED!" << endl;
		#endif
	}
	else 
	{
		if(SUCCEEDED(hr))
		{
			#ifdef FRANKYS_DEEP_DEBUG_FLAG
			cout << "...PARTIALLY SUCCEEDED with Code " << hr << endl;
			#endif
		}
		else
		{
			#ifdef FRANKYS_DEEP_DEBUG_FLAG
			cout << "...FAILED!" << endl;
			#endif
			return hr;
		}
	}
	return hr;
	
}


bool CameraTools::BuildAndDestroyGraphToDetermineCamSettings(int *widthInPix, int *heightInPix, int *frameRate, GUID *mediaSubType, int *widthInBytes, ssi_char_t *nameOfCam, ssi_size_t indexOfCam)
{
	IGraphBuilder *pGraph = NULL;
	ICaptureGraphBuilder2 *pBuild = NULL;
	IBaseFilter	*pCapDevice = NULL;
    IMediaControl *pControl = NULL;
    //IMediaEvent   *pEvent = NULL;
	//IVideoWindow *pVidWin = NULL;
	IBaseFilter *pGrabber = NULL;
	IUAProxyForceGrabber *pGrabInterface = NULL;

	bool isComInitialized = false;


	ssi_msg_static (SSI_LOG_LEVEL_BASIC, "connect device");

	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr))
    {
		if(hr == RPC_E_CHANGED_MODE)
		{
			#ifdef FRANKYS_DEEP_DEBUG_FLAG
			cout << "Tried to reinitialize COM with different threading model!" << endl;
			#endif 
		}
		else
		{
			cerr << "Could not initialize COM library in BuildAndDestroyGraph" <<endl;
			SafeReleaseFJ(pGrabInterface);
			SafeReleaseFJ(pGrabber);
			//SafeReleaseFJ(pPin);
			SafeReleaseFJ(pControl);
			//SafeReleaseFJ(pEvent);
			//SafeReleaseFJ(pVidWin);
			SafeReleaseFJ(pCapDevice);
			SafeReleaseFJ(pBuild);
			SafeReleaseFJ(pGraph);

			if(isComInitialized)
			{
				CoUninitialize();
			}
			return false;
		}
    }
	else 
	{
		if(hr == S_FALSE)
		{
			#ifdef FRANKYS_DEEP_DEBUG_FLAG
			cout << "COM was already initialized for this thread!" << endl;
			#endif 
		}
		isComInitialized = true;
 
	}
	
	#ifdef FRANKYS_DEEP_DEBUG_FLAG
	cout << "\nCalling InitCaptureGraphBuilder..." << endl;
	#endif
	hr = InitCaptureGraphBuilder(&pGraph, &pBuild);
	if(SUCCEEDED(hr))
	{
		#ifdef FRANKYS_DEEP_DEBUG_FLAG
		cout << "...SUCCEEDED!" << endl;
		#endif
	}
	else
	{
		cerr << "InitCapturegraphBuilder" << endl;
		SafeReleaseFJ(pGrabInterface);
		SafeReleaseFJ(pGrabber);
		//SafeReleaseFJ(pPin);
		SafeReleaseFJ(pControl);
		//SafeReleaseFJ(pEvent);
		//SafeReleaseFJ(pVidWin);
		SafeReleaseFJ(pCapDevice);
		SafeReleaseFJ(pBuild);
		SafeReleaseFJ(pGraph);

		if(isComInitialized)
		{
			CoUninitialize();
		}
		return false;
	}

	#ifdef FRANKYS_DEEP_DEBUG_FLAG
	cout << "\nCalling SelectCaptureDevice..." << endl;
	#endif
	hr = SelectCaptureDevice(nameOfCam, indexOfCam, &pCapDevice, pGraph);
	if(SUCCEEDED(hr))
	{
		#ifdef FRANKYS_DEEP_DEBUG_FLAG
		cout << "...SUCCEEDED!" << endl;
		#endif
	}
	else
	{
		cerr << "SelectCaptureDevice" << endl;
		SafeReleaseFJ(pGrabInterface);
		SafeReleaseFJ(pGrabber);
		//SafeReleaseFJ(pPin);
		SafeReleaseFJ(pControl);
		//SafeReleaseFJ(pEvent);
		//SafeReleaseFJ(pVidWin);
		SafeReleaseFJ(pCapDevice);
		SafeReleaseFJ(pBuild);
		SafeReleaseFJ(pGraph);

		if(isComInitialized)
		{
			CoUninitialize();
		}
		return false;
	}

	#ifdef FRANKYS_DEEP_DEBUG_FLAG
	cout << "\nCalling QueryInterfaces..." << endl;
	#endif
	hr = QueryInterfaces(pGraph, &pControl);
	if(SUCCEEDED(hr))
	{
		#ifdef FRANKYS_DEEP_DEBUG_FLAG
		cout << "...SUCCEEDED!" << endl;
		#endif
	}
	else
	{
		cerr << "QueryInterfaces" << endl;
		SafeReleaseFJ(pGrabInterface);
		SafeReleaseFJ(pGrabber);
		//SafeReleaseFJ(pPin);
		SafeReleaseFJ(pControl);
		//SafeReleaseFJ(pEvent);
		//SafeReleaseFJ(pVidWin);
		SafeReleaseFJ(pCapDevice);
		SafeReleaseFJ(pBuild);
		SafeReleaseFJ(pGraph);

		if(isComInitialized)
		{
			CoUninitialize();
		}
		return false;
	}

	#ifdef FRANKYS_DEEP_DEBUG_FLAG
	cout << "\nCalling SelectMediaTypeOfCam..." << endl;
	#endif
	//hr = SelectMediaTypeOfCam(_pCapDevice, _pBuild, _video_params.framerate, MEDIASUBTYPE_RGB24, _video_params.width, _video_params.height );
	hr = SelectMediaTypeOfCam(pCapDevice, pBuild, (*frameRate), *mediaSubType, *widthInPix, *heightInPix);
	if(SUCCEEDED(hr))
	{
		#ifdef FRANKYS_DEEP_DEBUG_FLAG
		cout << "...SUCCEEDED!" << endl;
		#endif
	}
	else
	{
		cerr << "SelectMediaTypeOfCam" << endl;
		SafeReleaseFJ(pGrabInterface);
		SafeReleaseFJ(pGrabber);
		//SafeReleaseFJ(pPin);
		SafeReleaseFJ(pControl);
		//SafeReleaseFJ(pEvent);
		//SafeReleaseFJ(pVidWin);
		SafeReleaseFJ(pCapDevice);
		SafeReleaseFJ(pBuild);
		SafeReleaseFJ(pGraph);

		if(isComInitialized)
		{
			CoUninitialize();
		}
		return false;
	}

	#ifdef FRANKYS_DEEP_DEBUG_FLAG
	cout << "\nCalling Get Pin with CaptureDevice..." << endl;
	#endif
	IPin *pPin = GetFirstPin(pCapDevice, PINDIR_OUTPUT);
	if(pPin != NULL)
	{
		#ifdef FRANKYS_DEEP_DEBUG_FLAG
		cout << "...SUCCEEDED!" << endl;
		#endif
	}
	else
	{
		cerr << "Get Output Pin of Capture Device" << endl;
		SafeReleaseFJ(pGrabInterface);
		SafeReleaseFJ(pGrabber);
		//SafeReleaseFJ(pPin);
		SafeReleaseFJ(pControl);
		//SafeReleaseFJ(pEvent);
		//SafeReleaseFJ(pVidWin);
		SafeReleaseFJ(pCapDevice);
		SafeReleaseFJ(pBuild);
		SafeReleaseFJ(pGraph);

		if(isComInitialized)
		{
			CoUninitialize();
		}
		return false;
	}

	#ifdef FRANKYS_DEEP_DEBUG_FLAG
	cout << "\nCalling AddAndConnectFilterByCLSIDToGivenOutputPin with Capture and Grabber..." << endl;
	#endif
	hr = AddAndConnectFilterByCLSIDToGivenOutputPin(CLSID_UAProxyForceGrabber, L"Grabber", pGraph, pPin);
	if(SUCCEEDED(hr))
	{
		#ifdef FRANKYS_DEEP_DEBUG_FLAG
		cout << "...SUCCEEDED!" << endl;
		#endif
	}
	else
	{
		cerr << "AddAndConnectFilterByCLSIDToGivenOutputPin with UAProxyForceGrabber" << endl;
		SafeReleaseFJ(pPin);
		SafeReleaseFJ(pGrabInterface);
		SafeReleaseFJ(pGrabber);
		//SafeReleaseFJ(pPin);
		SafeReleaseFJ(pControl);
		//SafeReleaseFJ(pEvent);
		//SafeReleaseFJ(pVidWin);
		SafeReleaseFJ(pCapDevice);
		SafeReleaseFJ(pBuild);
		SafeReleaseFJ(pGraph);

		if(isComInitialized)
		{
			CoUninitialize();
		}
		return false;
	}

	SafeReleaseFJ(pPin);

	#ifdef FRANKYS_DEEP_DEBUG_FLAG
	cout << "\nTrying to find Grabber by Name in Graph...";
	#endif
	hr = pGraph->FindFilterByName(L"Grabber", &pGrabber);
	if(SUCCEEDED(hr))
	{
		#ifdef FRANKYS_DEEP_DEBUG_FLAG
		cout << "...SUCCEEDED!" << endl;
		#endif
	}
	else
	{
		cerr << "FindFilterByName with UAProxyForceGrabber" << endl;
		SafeReleaseFJ(pGrabInterface);
		SafeReleaseFJ(pGrabber);
		//SafeReleaseFJ(pPin);
		SafeReleaseFJ(pControl);
		//SafeReleaseFJ(pEvent);
		//SafeReleaseFJ(pVidWin);
		SafeReleaseFJ(pCapDevice);
		SafeReleaseFJ(pBuild);
		SafeReleaseFJ(pGraph);

		if(isComInitialized)
		{
			CoUninitialize();
		}
		return false;
	}

	#ifdef FRANKYS_DEEP_DEBUG_FLAG
	cout << "\nCalling Get Pin with Grabber..." << endl;
	#endif
	pPin = GetFirstPin(pGrabber, PINDIR_OUTPUT);
	if(pPin != NULL)
	{
		#ifdef FRANKYS_DEEP_DEBUG_FLAG
		cout << "...SUCCEEDED!" << endl;
		#endif
	}
	else
	{
		cerr << "Get Output Pin of Grabber Device" << endl;
		SafeReleaseFJ(pGrabInterface);
		SafeReleaseFJ(pGrabber);
		//SafeReleaseFJ(pPin);
		SafeReleaseFJ(pControl);
		//SafeReleaseFJ(pEvent);
		//SafeReleaseFJ(pVidWin);
		SafeReleaseFJ(pCapDevice);
		SafeReleaseFJ(pBuild);
		SafeReleaseFJ(pGraph);

		if(isComInitialized)
		{
			CoUninitialize();
		}
		return false;
	}

	#ifdef FRANKYS_DEEP_DEBUG_FLAG
	cout << "\nTry to Grabber-Interface..."; 
	#endif
	hr = pGrabber->QueryInterface(IID_IUAProxyForceGrabber, (LPVOID *)&pGrabInterface);
	if(SUCCEEDED(hr))
	{
		#ifdef FRANKYS_DEEP_DEBUG_FLAG
		cout << "...SUCCEEDED!" << endl;
		#endif
	}
	else
	{
		cerr << "Query Grabber Interface" << endl;
		SafeReleaseFJ(pPin);
		SafeReleaseFJ(pGrabInterface);
		SafeReleaseFJ(pGrabber);
		//SafeReleaseFJ(pPin);
		SafeReleaseFJ(pControl);
		//SafeReleaseFJ(pEvent);
		//SafeReleaseFJ(pVidWin);
		SafeReleaseFJ(pCapDevice);
		SafeReleaseFJ(pBuild);
		SafeReleaseFJ(pGraph);

		if(isComInitialized)
		{
			CoUninitialize();
		}
		return false;
	}

	/*
	#ifdef RENDER_MY_STREAM_INSTEAD_OF_NULLRENDERER
		#ifdef FRANKYS_DEEP_DEBUG_FLAG
		cout << "\nCalling RenderMyGraph..." << endl;
		#endif
		hr = RenderMyGraph(pGraph, pPin);
		if(SUCCEEDED(hr))
		{
			#ifdef FRANKYS_DEEP_DEBUG_FLAG
			cout << "...SUCCEEDED!" << endl;
			#endif
		}
		else
		{
			cerr << "Could not Render Graph to OutputWindow" << endl;
			SafeReleaseFJ(pPin);
			SafeReleaseFJ(pGrabInterface);
			SafeReleaseFJ(pGrabber);
			//SafeReleaseFJ(pPin);
			SafeReleaseFJ(pControl);
			//SafeReleaseFJ(pEvent);
			//SafeReleaseFJ(pVidWin);
			SafeReleaseFJ(pCapDevice);
			SafeReleaseFJ(pBuild);
			SafeReleaseFJ(pGraph);

			if(isComInititialized)
			{
				CoUninitialize();
			}
			return false;
		}
	#else
	//*/
		#ifdef FRANKYS_DEEP_DEBUG_FLAG
		cout << "\nCalling ConnectToNullRenderer..." << endl;
		#endif
		hr = ConnectToNullRenderer(pPin, pGraph);
		if(SUCCEEDED(hr))
		{
			#ifdef FRANKYS_DEEP_DEBUG_FLAG
			cout << "...SUCCEEDED!" << endl;
			#endif
		}
		else
		{
			cerr << "Could not Render Graph to NullRenderer" << endl;
			SafeReleaseFJ(pPin);
			SafeReleaseFJ(pGrabInterface);
			SafeReleaseFJ(pGrabber);
			//SafeReleaseFJ(pPin);
			SafeReleaseFJ(pControl);
			//SafeReleaseFJ(pEvent);
			//SafeReleaseFJ(pVidWin);
			SafeReleaseFJ(pCapDevice);
			SafeReleaseFJ(pBuild);
			SafeReleaseFJ(pGraph);

			if(isComInitialized)
			{
				CoUninitialize();
			}
			return false;
		}
	/*
	#endif
	//*/

	SafeReleaseFJ(pPin);

	#ifdef FRANKYS_DEEP_DEBUG_FLAG
	cout << "\nActivating Framebuffering in Filter...";
	#endif
	hr = pGrabInterface->ToggleCallFrameBufferingForGrabbing(true);
	if(SUCCEEDED(hr))
	{
		#ifdef FRANKYS_DEEP_DEBUG_FLAG
		cout << "...SUCCEEDED!" << endl;
	#endif
	}
	else
	{
		cerr << "Activation of FrameBuffering for Grabbing" << endl;
		SafeReleaseFJ(pGrabInterface);
		SafeReleaseFJ(pGrabber);
		//SafeReleaseFJ(pPin);
		SafeReleaseFJ(pControl);
		//SafeReleaseFJ(pEvent);
		//SafeReleaseFJ(pVidWin);
		SafeReleaseFJ(pCapDevice);
		SafeReleaseFJ(pBuild);
		SafeReleaseFJ(pGraph);

		if(isComInitialized)
		{
			CoUninitialize();
		}
		return false;
	}

	#ifdef FRANKYS_DEEP_DEBUG_FLAG
	cout << "\nRunning Graph...";
	#endif
	hr = pControl->Run();
	if(SUCCEEDED(hr))
	{
		#ifdef FRANKYS_DEEP_DEBUG_FLAG
		cout << "...SUCCEEDED!" << endl;
		#endif
	}
	else
	{
		cerr << "Running Graph" << endl;
		SafeReleaseFJ(pGrabInterface);
		SafeReleaseFJ(pGrabber);
		//SafeReleaseFJ(pPin);
		SafeReleaseFJ(pControl);
		//SafeReleaseFJ(pEvent);
		//SafeReleaseFJ(pVidWin);
		SafeReleaseFJ(pCapDevice);
		SafeReleaseFJ(pBuild);
		SafeReleaseFJ(pGraph);

		if(isComInitialized)
		{
			CoUninitialize();
		}
		return false;
	}

	AM_MEDIA_TYPE *mediaType = CameraTools::GetCurrentMediaTypeOfCam(pCapDevice, pBuild);

	hr = pControl->Stop();

	if(mediaType == NULL)
	{
		SafeReleaseFJ(pGrabInterface);
		SafeReleaseFJ(pGrabber);
		//SafeReleaseFJ(pPin);
		SafeReleaseFJ(pControl);
		//SafeReleaseFJ(pEvent);
		//SafeReleaseFJ(pVidWin);
		SafeReleaseFJ(pCapDevice);
		SafeReleaseFJ(pBuild);
		SafeReleaseFJ(pGraph);

		if(isComInitialized)
		{
			CoUninitialize();
		}

		return false;
	}

	
	VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*)mediaType->pbFormat;

	*widthInPix		= pVih->bmiHeader.biWidth;
    *heightInPix    = abs(pVih->bmiHeader.biHeight);
	*widthInBytes   = ((*widthInPix) * ((pVih->bmiHeader.biBitCount) >> 3) + 3) & ~3;
	*frameRate		= static_cast<int> (10000000 / pVih->AvgTimePerFrame);

	DeleteMediaType(mediaType);

	SafeReleaseFJ(pGrabInterface);
	SafeReleaseFJ(pGrabber);
	//SafeReleaseFJ(pPin);
	SafeReleaseFJ(pControl);
	//SafeReleaseFJ(pEvent);
	//SafeReleaseFJ(pVidWin);
	SafeReleaseFJ(pCapDevice);
	SafeReleaseFJ(pBuild);
	SafeReleaseFJ(pGraph);

	if(isComInitialized)
	{
		CoUninitialize();
	}

	return true;
}

AM_MEDIA_TYPE* CameraTools::GetCurrentMediaTypeOfCam(IBaseFilter *pCapFilter, ICaptureGraphBuilder2 *pBuild)
{
	IAMStreamConfig *pConfig = NULL;
	#ifdef FRANKYS_DEEP_DEBUG_FLAG
	cout << "	Try to find Interface for Camera Stream Configuration...";
	#endif
	HRESULT hr = pBuild->FindInterface(&PIN_CATEGORY_CAPTURE, // Preview pin.
								0,    // Any media type.
								pCapFilter, // Pointer to the capture filter.
								IID_IAMStreamConfig, (void**)&pConfig);

	AM_MEDIA_TYPE *pmtConfig = NULL;

	if(SUCCEEDED(hr))
	{
		#ifdef FRANKYS_DEEP_DEBUG_FLAG
		cout <<"SUCCEEDED!" <<endl;
		#endif
	}
	else
	{
		#ifdef FRANKYS_DEEP_DEBUG_FLAG
		cout << "FAILED!" << endl;
		#endif
		return NULL;
	}

	hr = pConfig->GetFormat(&pmtConfig);
	if(SUCCEEDED(hr))
	{
		#ifdef FRANKYS_DEEP_DEBUG_FLAG
		cout <<"SUCCEEDED!" <<endl;
		cout <<"Display the new Format: " << endl;
		#endif
		VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*)pmtConfig->pbFormat;
		//SCC
		#ifdef FRANKYS_DEEP_DEBUG_FLAG
		/*
		cout << "Input Size X: " << scc.InputSize.cx << " Y: "<< scc.InputSize.cy << endl;
		cout << "MinCrop Size X: " << scc.MinCroppingSize.cx << " Y: "<< scc.MinCroppingSize.cy << endl;
		cout << "MaxCrop Size X: " << scc.MaxCroppingSize.cx << " Y: "<< scc.MaxCroppingSize.cy << endl;
		cout << "CropGranul X: " << scc.CropGranularityX << " Y: "<< scc.CropGranularityY << endl;
		cout << "Crop Align X: " << scc.CropAlignX << " Y: "<< scc.CropAlignY << endl;
		cout << "Min Output Size X: " << scc.MinOutputSize.cx << " Y: "<< scc.MinOutputSize.cy << endl;
		cout << "Max Output Size X: " << scc.MaxOutputSize.cx << " Y: "<< scc.MaxOutputSize.cy << endl;
		cout << "Output Granul X: " << scc.OutputGranularityX << " Y: "<< scc.OutputGranularityY << endl;
		cout << "Shrink Taps X: " << scc.ShrinkTapsX << " Y: "<< scc.ShrinkTapsY << endl;
		cout << "Stretch Taps X: " << scc.StretchTapsX << " Y: "<< scc.StretchTapsY << endl;
		cout << "Frame Int Min: " << 10000000 / scc.MinFrameInterval << " Max: "<< 10000000 / scc.MaxFrameInterval << endl;
		cout << "bPS Min: " << scc.MinBitsPerSecond << " Max: "<< scc.MaxBitsPerSecond << endl;
		cout << "\n";
		//*/
		//VIH
		cout << "rcSource L: " << pVih->rcSource.left << " O: " << pVih->rcSource.top<< " R: " << pVih->rcSource.right << " U: " << pVih->rcSource.bottom << endl;
		cout << "rcTarget L: " << pVih->rcTarget.left << " O: " << pVih->rcTarget.top<< " R: " << pVih->rcTarget.right << " U: " << pVih->rcTarget.bottom << endl;
		cout << "BitRate: " << pVih->dwBitRate << endl;
		cout << "BitErrorRate: " << pVih->dwBitErrorRate << endl;
		cout << "FPS: " << 10000000 / pVih->AvgTimePerFrame << endl;
		cout << "\n";
		//BIH
		cout << "Width in Pixels: " << pVih->bmiHeader.biWidth << endl;
		cout << "Height in Pixels: " << pVih->bmiHeader.biHeight << endl;
		cout << "BitsPerPixel: " << pVih->bmiHeader.biBitCount << endl;
		cout << "Size of Image in Bytes: " << pVih->bmiHeader.biSizeImage << endl;
		#endif
	}
	else
	{
		#ifdef FRANKYS_DEEP_DEBUG_FLAG
		cout << "FAILED!" << endl;
		#endif
	}

	SafeReleaseFJ(pConfig);
	return pmtConfig;
}

bool CameraTools::BuildAndDestroyGraphToDetermineFileVideoParams(const ssi_char_t *fileName, ssi_video_params_t *videoParams)
{
	USES_CONVERSION;
	bool					isComInitialized = false;
	int						comInitCount = 0;
	IGraphBuilder			*pGraph = NULL;
    IMediaControl			*pControl = NULL;
	IBasicVideo				*pBasicVideo = NULL;
	IBaseFilter				*pFileLoadFilter = NULL;
	IBaseFilter				*pGrabber = NULL;
	IUAProxyForceGrabber	*pGrabInterface = NULL;
	IPin					*pPin = NULL;
	IFileSourceFilter		*pFileSourceFilt = NULL;

	SSI_DBG_STATIC (SSI_LOG_LEVEL_DEBUG, "Initializing COM");
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    
	if (FAILED(hr))
    {
		if(hr == RPC_E_CHANGED_MODE)
		{
			ssi_msg_static (SSI_LOG_LEVEL_DETAIL, "Tried to reinitialize COM with different threading model!");
		}
		else
		{
			ssi_wrn_static ("Could not initialize COM library in BuildAndDestroyGraphToDetermineFileFPS()");
			return false;
		}
    }
	else 
	{
		if(hr == S_FALSE)
		{
			ssi_msg_static (SSI_LOG_LEVEL_DETAIL, "COM was already initialized for this thread!");
		}
		isComInitialized = true;
		++comInitCount;
	}

	ssi_msg_static (SSI_LOG_LEVEL_DETAIL, "Calling InitFilterGraphManager...");

	hr = CameraTools::InitFilterGraphManager(&pGraph);
	if(SUCCEEDED(hr))
	{
		// ssi_msg_static (SSI_LOG_LEVEL_DETAIL, "...SUCCEEDED!");
	}
	else 
	{
		if(isComInitialized == true)
		{
			CoUninitialize();
			--comInitCount;
		}
		ssi_wrn_static ("InitFilterGraphManager");
		return false;
	}

	ssi_msg_static (SSI_LOG_LEVEL_DETAIL, "Calling QueryInterfaces...");
	hr = CameraTools::QueryInterfaces(pGraph, &pControl, NULL, NULL, NULL, &pBasicVideo);
	if(SUCCEEDED(hr))
	{
		// ssi_msg_static (SSI_LOG_LEVEL_DETAIL, "...SUCCEEDED!");
	}
	else 
	{
		SafeReleaseFJ(pGraph);
		if(isComInitialized == true)
		{
			
			CoUninitialize();
			--comInitCount;
		}
		ssi_wrn_static ("QueryInterfaces");
		return false;
	}

	ssi_msg_static (SSI_LOG_LEVEL_DETAIL, "Calling AddFilterToGraphByCLSID with CLSID_AsyncReader...");
	hr = AddFilterToGraphByCLSID(pGraph, CLSID_AsyncReader, L"FileReader", &pFileLoadFilter);
	if(SUCCEEDED(hr))
	{
		// ssi_msg_static (SSI_LOG_LEVEL_DETAIL, "...SUCCEEDED!");
	}
	else 
	{
		SafeReleaseFJ(pBasicVideo);
		SafeReleaseFJ(pControl);
		SafeReleaseFJ(pGraph);
		if(isComInitialized == true)
		{
			
			CoUninitialize();
			--comInitCount;
		}
		ssi_wrn_static ("AddFilterToGraphByCLSID");
		return false;
	}

	ssi_msg_static (SSI_LOG_LEVEL_DETAIL, "\nCalling QueryInterface with IID_IFileSourceFilter...");
	hr = pFileLoadFilter->QueryInterface(IID_IFileSourceFilter, (LPVOID *) &pFileSourceFilt);
	if(SUCCEEDED(hr))
	{
		//ssi_msg_static (SSI_LOG_LEVEL_DETAIL, "SUCCEEDED!");
	}
	else
	{
		SafeReleaseFJ(pFileLoadFilter);
		SafeReleaseFJ(pBasicVideo);
		SafeReleaseFJ(pControl);
		SafeReleaseFJ(pGraph);
		if(isComInitialized == true)
		{
			
			CoUninitialize();
			--comInitCount;
		}
		ssi_wrn_static ("FAILED!");
		return false;
	}

	ssi_msg_static (SSI_LOG_LEVEL_DETAIL, "Calling IFileSourceFilter::Load...");
	wchar_t fileNameW[MAX_PATH*2];
	if(!MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, fileName, -1, fileNameW, MAX_PATH*2))
	{
		SafeReleaseFJ(pFileLoadFilter);
		SafeReleaseFJ(pFileSourceFilt);
		SafeReleaseFJ(pBasicVideo);
		SafeReleaseFJ(pControl);
		SafeReleaseFJ(pGraph);
		if(isComInitialized == true)
		{
			
			CoUninitialize();
			--comInitCount;
		}
		ssi_wrn_static("Character conversion to Unicode failed in BuildAndDestroyGraphToDetermineFileFPS");
		return false;
	}

	hr = pFileSourceFilt->Load(fileNameW, NULL);	
	if(SUCCEEDED(hr))
	{
		//ssi_msg_static (SSI_LOG_LEVEL_DETAIL, "SUCCEEDED!" );
	}
	else
	{
		SafeReleaseFJ(pFileLoadFilter);
		SafeReleaseFJ(pFileSourceFilt);
		SafeReleaseFJ(pBasicVideo);
		SafeReleaseFJ(pControl);
		SafeReleaseFJ(pGraph);
		if(isComInitialized == true)
		{
			
			CoUninitialize();
			--comInitCount;
		}
		ssi_wrn_static ("FAILED!");
		return false;
	}

	
	ssi_msg_static (SSI_LOG_LEVEL_DETAIL, "Calling GetPin with AsyncFileLoader...");
	pPin = CameraTools::GetFirstPin(pFileLoadFilter, PINDIR_OUTPUT);
	if(pPin)
	{
		//ssi_msg_static (SSI_LOG_LEVEL_DETAIL, "SUCCEEDED!" );
	}
	else
	{
		SafeReleaseFJ(pFileLoadFilter);
		SafeReleaseFJ(pFileSourceFilt);
		SafeReleaseFJ(pBasicVideo);
		SafeReleaseFJ(pControl);
		SafeReleaseFJ(pGraph);
		if(isComInitialized == true)
		{
			
			CoUninitialize();
			--comInitCount;
		}
		ssi_wrn_static ("FAILED!");
		return false;
	}

	#ifdef FRANKYS_DEEP_DEBUG_FLAG
	cout << "\nCalling AddAndConnectFilterByCLSIDToGivenOutputPin with Capture and Grabber..." << endl;
	#endif
	hr = AddAndConnectFilterByCLSIDToGivenOutputPin(CLSID_UAProxyForceGrabber, L"Grabber", pGraph, pPin);
	if(SUCCEEDED(hr))
	{
		//ssi_msg_static (SSI_LOG_LEVEL_DETAIL, "SUCCEEDED!" );
	}
	else
	{
		SafeReleaseFJ(pPin);
		SafeReleaseFJ(pFileLoadFilter);
		SafeReleaseFJ(pFileSourceFilt);
		SafeReleaseFJ(pBasicVideo);
		SafeReleaseFJ(pControl);
		SafeReleaseFJ(pGraph);

		if(isComInitialized)
		{
			CoUninitialize();
		}
		ssi_wrn_static ("FAILED");
		return false;
	}

	SafeReleaseFJ(pPin);

	#ifdef FRANKYS_DEEP_DEBUG_FLAG
	cout << "\nTrying to find Grabber by Name in Graph...";
	#endif
	hr = pGraph->FindFilterByName(L"Grabber", &pGrabber);
	if(SUCCEEDED(hr))
	{
		#ifdef FRANKYS_DEEP_DEBUG_FLAG
		cout << "...SUCCEEDED!" << endl;
		#endif
	}
	else
	{
		
		SafeReleaseFJ(pFileLoadFilter);
		SafeReleaseFJ(pFileSourceFilt);
		SafeReleaseFJ(pBasicVideo);
		SafeReleaseFJ(pControl);
		SafeReleaseFJ(pGraph);

		if(isComInitialized)
		{
			CoUninitialize();
		}
		ssi_wrn_static ("FindFilterByName with UAProxyForceGrabber");
		return false;
	}

	#ifdef FRANKYS_DEEP_DEBUG_FLAG
	cout << "\nCalling Get Pin with Grabber..." << endl;
	#endif
	pPin = GetFirstPin(pGrabber, PINDIR_OUTPUT);
	if(pPin != NULL)
	{
		#ifdef FRANKYS_DEEP_DEBUG_FLAG
		cout << "...SUCCEEDED!" << endl;
		#endif
	}
	else
	{
		
		SafeReleaseFJ(pGrabber);
		SafeReleaseFJ(pFileLoadFilter);
		SafeReleaseFJ(pFileSourceFilt);
		SafeReleaseFJ(pBasicVideo);
		SafeReleaseFJ(pControl);
		SafeReleaseFJ(pGraph);

		if(isComInitialized)
		{
			CoUninitialize();
		}
		ssi_wrn_static ("Get Output Pin of Grabber Device" );
		return false;
	}

	#ifdef FRANKYS_DEEP_DEBUG_FLAG
	cout << "\nTry to Grabber-Interface..."; 
	#endif
	hr = pGrabber->QueryInterface(IID_IUAProxyForceGrabber, (LPVOID *)&pGrabInterface);
	if(SUCCEEDED(hr))
	{
		#ifdef FRANKYS_DEEP_DEBUG_FLAG
		cout << "...SUCCEEDED!" << endl;
		#endif
	}
	else
	{
		SafeReleaseFJ(pGrabber);
		SafeReleaseFJ(pFileLoadFilter);
		SafeReleaseFJ(pFileSourceFilt);
		SafeReleaseFJ(pBasicVideo);
		SafeReleaseFJ(pControl);
		SafeReleaseFJ(pGraph);
		if(isComInitialized == true)
		{
			
			CoUninitialize();
			--comInitCount;
		}
		ssi_wrn_static ("FAILED!");
		return false;
	}

	ssi_msg_static (SSI_LOG_LEVEL_DETAIL, "Calling ConnectToNullRenderer...");
	hr = CameraTools::ConnectToNullRenderer(pPin, pGraph);
	if(SUCCEEDED(hr))
	{
		//ssi_msg_static (SSI_LOG_LEVEL_DETAIL, "SUCCEEDED!" );
	}
	else
	{
		SafeReleaseFJ(pGrabInterface);
		SafeReleaseFJ(pGrabber);
		SafeReleaseFJ(pFileLoadFilter);
		SafeReleaseFJ(pFileSourceFilt);
		SafeReleaseFJ(pBasicVideo);
		SafeReleaseFJ(pControl);
		SafeReleaseFJ(pGraph);
		if(isComInitialized == true)
		{
			
			CoUninitialize();
			--comInitCount;
		}
		ssi_wrn_static ("FAILED!");
		return false;
	}

	SafeReleaseFJ(pPin);

	//[TODO] Find out if graph has to run to determine FPS
	ssi_msg_static (SSI_LOG_LEVEL_DETAIL, "Running Graph...");
	hr = S_FALSE;
	//while(hr != S_OK)
	//{
		hr = pControl->Run();
		if(SUCCEEDED(hr))
		{
			//ssi_msg_static (SSI_LOG_LEVEL_DETAIL, "SUCCEEDED!" );
		}
		else
		{
			SafeReleaseFJ(pGrabInterface);
			SafeReleaseFJ(pGrabber);
			SafeReleaseFJ(pFileLoadFilter);
			SafeReleaseFJ(pFileSourceFilt);
			SafeReleaseFJ(pBasicVideo);
			SafeReleaseFJ(pControl);
			SafeReleaseFJ(pGraph);
			if(isComInitialized == true)
			{
				CoUninitialize();
				--comInitCount;
			}
			ssi_wrn_static ("FAILED!");
		}
		//Sleep(500);
	//}


	AM_MEDIA_TYPE *mt = NULL;
	REFTIME timePerFrame = 0.0;

	REFERENCE_TIME timePerFrameIn100NanoSeconds = 0;

	while(	pGrabInterface->GetConnectedMediaType(&mt) != NOERROR)
	{
		Sleep(500);
	}

	if(mt->subtype == MEDIASUBTYPE_RGB24)
	{
		videoParams->numOfChannels = 3;
		videoParams->depthInBitsPerChannel = SSI_VIDEO_DEPTH_8U;
		videoParams->outputSubTypeOfCaptureDevice = MEDIASUBTYPE_RGB24;
	}
	else if(mt->subtype == MEDIASUBTYPE_RGB32)
	{
		videoParams->numOfChannels = 4;
		videoParams->depthInBitsPerChannel = SSI_VIDEO_DEPTH_8U;
		videoParams->outputSubTypeOfCaptureDevice = MEDIASUBTYPE_RGB32;
	}
	else
	{
		DeleteMediaType(mt);
		pControl->Stop();

		SafeReleaseFJ(pGrabInterface);
		SafeReleaseFJ(pGrabber);
		SafeReleaseFJ(pFileLoadFilter);
		SafeReleaseFJ(pFileSourceFilt);
		SafeReleaseFJ(pBasicVideo);
		SafeReleaseFJ(pControl);
		SafeReleaseFJ(pGraph);
		ssi_wrn_static("Unknown Stream Structure!");
		return false;
	}
	if((IsEqualGUID(mt->formattype, FORMAT_VideoInfo)) && (mt->cbFormat >= sizeof(VIDEOINFOHEADER)))
	{
		VIDEOINFOHEADER *pVIH = reinterpret_cast<VIDEOINFOHEADER*>(mt->pbFormat);
		timePerFrame = (REFTIME)pVIH->AvgTimePerFrame / 10000000.0;
		videoParams->widthInPixels = pVIH->bmiHeader.biWidth;
		//For uncompressed RGB bitmaps, if biHeight is positive, the bitmap is a bottom-up DIB 
		//with the origin at the lower left corner. If biHeight is negative, 
		//the bitmap is a top-down DIB with the origin at the upper left corner.
		videoParams->heightInPixels = pVIH->bmiHeader.biHeight;	
	}
	//else if((IsEqualGUID(mt->formattype, FORMAT_VideoInfo2)) && (mt->cbFormat >= sizeof(VIDEOINFOHEADER2)))
	//{
	//	VIDEOINFOHEADER *pVIH = reinterpret_cast<VIDEOINFOHEADER2*>(mt->pbFormat);
	//	timePerFrame = (REFTIME)pVIH->AvgTimePerFrame / 10000000.0;
	//	videoParams->width = pVIH->bmiHeader.biWidth;
	//	//For uncompressed RGB bitmaps, if biHeight is positive, the bitmap is a bottom-up DIB 
	//	//with the origin at the lower left corner. If biHeight is negative, 
	//	//the bitmap is a top-down DIB with the origin at the upper left corner.
	//	videoParams->height = pVIH->bmiHeader.biHeight;

	//}
	else
	{
		DeleteMediaType(mt);
		pControl->Stop();

		SafeReleaseFJ(pGrabInterface);
		SafeReleaseFJ(pGrabber);
		SafeReleaseFJ(pFileLoadFilter);
		SafeReleaseFJ(pFileSourceFilt);
		SafeReleaseFJ(pBasicVideo);
		SafeReleaseFJ(pControl);
		SafeReleaseFJ(pGraph);
		ssi_wrn_static("Unknown Stream Structure!");
		return false;
	}

	videoParams->framesPerSecond = 1.0 / timePerFrame;
	
	DeleteMediaType(mt);

	pControl->Stop();

	SafeReleaseFJ(pGrabInterface);
	SafeReleaseFJ(pGrabber);
	SafeReleaseFJ(pFileLoadFilter);
	SafeReleaseFJ(pFileSourceFilt);
	SafeReleaseFJ(pBasicVideo);
	SafeReleaseFJ(pControl);
	SafeReleaseFJ(pGraph);

	if(isComInitialized)
	{
		--comInitCount;
		CoUninitialize();
	}

	//*fps = 1.0 / timePerFrame;

	return true;

}


HRESULT CameraTools::InitFilterGraphManager(IGraphBuilder **ppGraph)
{
    if (!ppGraph)
    {
        return E_POINTER;
    }
    

	IGraphBuilder *pGraph;

	// Create the Filter Graph Manager.
	#ifdef FRANKYS_DEEP_DEBUG_FLAG
	cout << "	Try to CoCreate FilterGraphManager...";
	#endif
	HRESULT hr = CoCreateInstance(CLSID_FilterGraph, NULL, 
		CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&pGraph);
	if (SUCCEEDED(hr))
    {
		#ifdef FRANKYS_DEEP_DEBUG_FLAG
		cout << "SUCCEEDED!" << endl;
		#endif
        *ppGraph = pGraph;
		return S_OK;
	}
	else
    {
		#ifdef FRANKYS_DEEP_DEBUG_FLAG
		cout << "FAILED!" << endl;
		#endif
        SafeReleaseFJ(pGraph);
    }
   
    return hr; // Failed
}

HRESULT CameraTools::FindAndBindToIBaseFilter(IBaseFilter **ppBaseFilter, CameraDeviceName *pDeviceName)
{
	if(ppBaseFilter == NULL || pDeviceName == NULL)
	{
		return E_POINTER;
	}

	ICreateDevEnum *pSysDevEnum = NULL;
	IEnumMoniker *pEnumMoniker = NULL;
	IMoniker *pMoniker = NULL;
	IPropertyBag *pPropertyBag = NULL;
	IBaseFilter *pBaseFilter = NULL;

	HRESULT hr;

	SSI_DBG_STATIC(SSI_LOG_LEVEL_DEBUG, "In HRESULT CameraTools::FindAndBindToIBaseFilter(IBaseFilter **ppBaseFilter, CameraDeviceName *pDeviceName) try to CoCreateInstance with CLSID_SystemDeviceEnum...");
	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, reinterpret_cast<void**>(&pSysDevEnum));
	if(SUCCEEDED(hr))
	{
		SSI_DBG_STATIC(SSI_LOG_LEVEL_VERBOSE, "\t...SUCCEEDED!");
	}
	else
	{
		ssi_wrn_static("In HRESULT CameraTools::FindAndBindToIBaseFilter(IBaseFilter **ppBaseFilter, CameraDeviceName *pDeviceName) try to CoCreateInstance with CLSID_SystemDeviceEnum failed with %ld", hr);
		return hr;
	}
	
	hr = pSysDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumMoniker, 0);
	if(hr != S_OK)
	{
		if(hr == S_FALSE)
		{
			SSI_DBG_STATIC(SSI_LOG_LEVEL_DETAIL, "In HRESULT CameraTools::FindAndBindToIBaseFilter(IBaseFilter **ppBaseFilter, CameraDeviceName *pDeviceName) try to CreateClassEnumerator with CLSID_VideoInputDeviceCategory failed because Category is Empty or does not exist");
			return hr;
		}
		SafeReleaseFJ(pSysDevEnum);
		ssi_wrn_static("In HRESULT CameraTools::FindAndBindToIBaseFilter(IBaseFilter **ppBaseFilter, CameraDeviceName *pDeviceName) try to CreateClassEnumerator with CLSID_VideoInputDeviceCategory failed with %ld", hr);
		
		return hr;
	}

	bool cameraFound = false;
	while(pEnumMoniker->Next(1, &pMoniker, NULL) == S_OK)
	{
		IPropertyBag *pPropertyBag = NULL;
		hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)(&pPropertyBag));
		if(FAILED(hr))
		{
			SafeReleaseFJ(pMoniker);
			continue;
		}
		VARIANT varFriendlyName;
		VARIANT varDescription;
		VARIANT varDevicePath;
		VariantInit(&varFriendlyName);
		VariantInit(&varDescription);
		VariantInit(&varDevicePath);

		char *pFriendlyName = NULL;
		char *pDescription = NULL;
		char *pDevicePath = NULL;

		hr = pPropertyBag->Read(L"FriendlyName", &varFriendlyName, 0);
		if(SUCCEEDED(hr))
		{
			pFriendlyName = _com_util::ConvertBSTRToString(varFriendlyName.bstrVal);
			VariantClear(&varFriendlyName);
		}
		hr = pPropertyBag->Read(L"Description", &varDescription, 0);
		if(SUCCEEDED(hr))
		{
			pDescription = _com_util::ConvertBSTRToString(varDescription.bstrVal);
			VariantClear(&varDescription);
		}
		hr = pPropertyBag->Read(L"DevicePath", &varDevicePath, 0);
		{
			pDevicePath = _com_util::ConvertBSTRToString(varDevicePath.bstrVal);
			VariantClear(&varDevicePath);
		}

		//if(!(pFriendlyName == NULL && pDescription == NULL && pDevicePath == NULL))
		{
			CameraDeviceName curCamDevice(pFriendlyName, pDescription, pDevicePath);
			if(curCamDevice == (*pDeviceName))
			{
				hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&pBaseFilter);
				if(SUCCEEDED(hr))
				{
					*ppBaseFilter = pBaseFilter;
				}
				else
				{
					delete[] pFriendlyName;
					pFriendlyName = NULL;
					delete[] pDescription;
					pDescription = NULL;
					delete[] pDevicePath;
					pDevicePath = NULL;
					SafeReleaseFJ(pPropertyBag);
					SafeReleaseFJ(pMoniker);
					SafeReleaseFJ(pEnumMoniker);
					SafeReleaseFJ(pSysDevEnum);
					return hr;
				}
				cameraFound = true;
			}

			delete[] pFriendlyName;
			pFriendlyName = NULL;
			delete[] pDescription;
			pDescription = NULL;
			delete[] pDevicePath;
			pDevicePath = NULL;

			SafeReleaseFJ(pPropertyBag);
			SafeReleaseFJ(pMoniker);
		}

		hr = E_FAIL;
		if(cameraFound)
		{
			hr = S_OK;
			break;
		}
	}

	SafeReleaseFJ(pEnumMoniker);
	SafeReleaseFJ(pSysDevEnum);

	return hr;
}
/*
GUID CameraTools::StringToGUID (const char *guidString) {
	
	if (strcmp (guidString, "GUID_NULL")	== 0)	return	GUID_NULL	;
	else if (strcmp (guidString, "CLSID_FilterMapper2")	== 0)	return	CLSID_FilterMapper2	;
	else if (strcmp (guidString, "CLSID_MOVReader")	== 0)	return	CLSID_MOVReader	;
	else if (strcmp (guidString, "CLSID_DirectDrawProperties")	== 0)	return	CLSID_DirectDrawProperties	;
	else if (strcmp (guidString, "IID_IDirectDrawSurface3")	== 0)	return	IID_IDirectDrawSurface3	;
	else if (strcmp (guidString, "CLSID_DVVideoCodec")	== 0)	return	CLSID_DVVideoCodec	;
	else if (strcmp (guidString, "CLSID_Dither")	== 0)	return	CLSID_Dither	;
	else if (strcmp (guidString, "CLSID_MediaPropertyBag")	== 0)	return	CLSID_MediaPropertyBag	;
	else if (strcmp (guidString, "CLSID_VBISurfaces")	== 0)	return	CLSID_VBISurfaces	;
	else if (strcmp (guidString, "IID_IVPVBIConfig")	== 0)	return	IID_IVPVBIConfig	;
	else if (strcmp (guidString, "AM_KSCATEGORY_TVTUNER")	== 0)	return	AM_KSCATEGORY_TVTUNER	;
	else if (strcmp (guidString, "AM_KSCATEGORY_DATACOMPRESSOR")	== 0)	return	AM_KSCATEGORY_DATACOMPRESSOR	;
	else if (strcmp (guidString, "MEDIASUBTYPE_PCM")	== 0)	return	MEDIASUBTYPE_PCM	;
	else if (strcmp (guidString, "CLSID_VPVBIObject")	== 0)	return	CLSID_VPVBIObject	;
	else if (strcmp (guidString, "IID_IVPVBINotify")	== 0)	return	IID_IVPVBINotify	;
	else if (strcmp (guidString, "AM_KSCATEGORY_CROSSBAR")	== 0)	return	AM_KSCATEGORY_CROSSBAR	;
	else if (strcmp (guidString, "AMPROPSETID_Pin")	== 0)	return	AMPROPSETID_Pin	;
	else if (strcmp (guidString, "IID_IVPVBIObject")	== 0)	return	IID_IVPVBIObject	;
	else if (strcmp (guidString, "AM_KSCATEGORY_TVAUDIO")	== 0)	return	AM_KSCATEGORY_TVAUDIO	;
	else if (strcmp (guidString, "IID_IKsDataTypeHandler")	== 0)	return	IID_IKsDataTypeHandler	;
	else if (strcmp (guidString, "DSATTRIB_PicSampleSeq")	== 0)	return	DSATTRIB_PicSampleSeq	;
	else if (strcmp (guidString, "CLSID_StreamBufferSource")	== 0)	return	CLSID_StreamBufferSource	;
	else if (strcmp (guidString, "MEDIASUBTYPE_IEEE_FLOAT")	== 0)	return	MEDIASUBTYPE_IEEE_FLOAT	;
	else if (strcmp (guidString, "AM_KSCATEGORY_AUDIO")	== 0)	return	AM_KSCATEGORY_AUDIO	;
	else if (strcmp (guidString, "AM_KSCATEGORY_VIDEO")	== 0)	return	AM_KSCATEGORY_VIDEO	;
	else if (strcmp (guidString, "MEDIASUBTYPE_DRM_Audio")	== 0)	return	MEDIASUBTYPE_DRM_Audio	;
	else if (strcmp (guidString, "AM_KSPROPSETID_DVD_RateChange")	== 0)	return	AM_KSPROPSETID_DVD_RateChange	;
	else if (strcmp (guidString, "CLSID_WMAsfWriter")	== 0)	return	CLSID_WMAsfWriter	;
	else if (strcmp (guidString, "CLSID_SystemDeviceEnum")	== 0)	return	CLSID_SystemDeviceEnum	;
	else if (strcmp (guidString, "IID_IFullScreenVideo")	== 0)	return	IID_IFullScreenVideo	;
	else if (strcmp (guidString, "CLSID_CVidCapClassManager")	== 0)	return	CLSID_CVidCapClassManager	;
	else if (strcmp (guidString, "CLSID_AviMuxProptyPage1")	== 0)	return	CLSID_AviMuxProptyPage1	;
	else if (strcmp (guidString, "MEDIATYPE_MPEG2_PACK")	== 0)	return	MEDIATYPE_MPEG2_PACK	;
	else if (strcmp (guidString, "CLSID_ATSCNetworkPropertyPage")	== 0)	return	CLSID_ATSCNetworkPropertyPage	;
	else if (strcmp (guidString, "CLSID_AVICo")	== 0)	return	CLSID_AVICo	;
	else if (strcmp (guidString, "CLSID_DVDNavigator")	== 0)	return	CLSID_DVDNavigator	;
	else if (strcmp (guidString, "CLSID_Line21Decoder")	== 0)	return	CLSID_Line21Decoder	;
	else if (strcmp (guidString, "CLSID_AviSplitter")	== 0)	return	CLSID_AviSplitter	;
	else if (strcmp (guidString, "CLSID_DSoundRender")	== 0)	return	CLSID_DSoundRender	;
	else if (strcmp (guidString, "CLSID_MMSPLITTER")	== 0)	return	CLSID_MMSPLITTER	;
	else if (strcmp (guidString, "MEDIATYPE_MPEG2_PES")	== 0)	return	MEDIATYPE_MPEG2_PES	;
	else if (strcmp (guidString, "CLSID_DVDHWDecodersCategory")	== 0)	return	CLSID_DVDHWDecodersCategory	;
	else if (strcmp (guidString, "IID_IDirectDrawKernel")	== 0)	return	IID_IDirectDrawKernel	;
	else if (strcmp (guidString, "AM_KSPROPSETID_AC3")	== 0)	return	AM_KSPROPSETID_AC3	;
	else if (strcmp (guidString, "CLSID_MediaEncoderCategory")	== 0)	return	CLSID_MediaEncoderCategory	;
	else if (strcmp (guidString, "MEDIASUBTYPE_VPS")	== 0)	return	MEDIASUBTYPE_VPS	;
	else if (strcmp (guidString, "IID_IAMLine21Decoder")	== 0)	return	IID_IAMLine21Decoder	;
	else if (strcmp (guidString, "CLSID_AviReader")	== 0)	return	CLSID_AviReader	;
	else if (strcmp (guidString, "IID_IAMWstDecoder")	== 0)	return	IID_IAMWstDecoder	;
	else if (strcmp (guidString, "MEDIASUBTYPE_Line21_BytePair")	== 0)	return	MEDIASUBTYPE_Line21_BytePair	;
	else if (strcmp (guidString, "CLSID_VfwCapture")	== 0)	return	CLSID_VfwCapture	;
	else if (strcmp (guidString, "CLSID_CaptureProperties")	== 0)	return	CLSID_CaptureProperties	;
	else if (strcmp (guidString, "MEDIASUBTYPE_MPEG2_PROGRAM")	== 0)	return	MEDIASUBTYPE_MPEG2_PROGRAM	;
	else if (strcmp (guidString, "MEDIASUBTYPE_Line21_GOPPacket")	== 0)	return	MEDIASUBTYPE_Line21_GOPPacket	;
	else if (strcmp (guidString, "MEDIASUBTYPE_MPEG2_TRANSPORT")	== 0)	return	MEDIASUBTYPE_MPEG2_TRANSPORT	;
	else if (strcmp (guidString, "MEDIASUBTYPE_Line21_VBIRawData")	== 0)	return	MEDIASUBTYPE_Line21_VBIRawData	;
	else if (strcmp (guidString, "MEDIASUBTYPE_MPEG2_VIDEO")	== 0)	return	MEDIASUBTYPE_MPEG2_VIDEO	;
	else if (strcmp (guidString, "CODECAPI_AVDecMmcssClass")	== 0)	return	CODECAPI_AVDecMmcssClass	;
	else if (strcmp (guidString, "MEDIASUBTYPE_MPEG2_AUDIO")	== 0)	return	MEDIASUBTYPE_MPEG2_AUDIO	;
	else if (strcmp (guidString, "MEDIASUBTYPE_DOLBY_AC3")	== 0)	return	MEDIASUBTYPE_DOLBY_AC3	;
	else if (strcmp (guidString, "MEDIASUBTYPE_DVD_SUBPICTURE")	== 0)	return	MEDIASUBTYPE_DVD_SUBPICTURE	;
	else if (strcmp (guidString, "MEDIATYPE_DVD_NAVIGATION")	== 0)	return	MEDIATYPE_DVD_NAVIGATION	;
	else if (strcmp (guidString, "MEDIASUBTYPE_DVD_NAVIGATION_PCI")	== 0)	return	MEDIASUBTYPE_DVD_NAVIGATION_PCI	;
	else if (strcmp (guidString, "MEDIASUBTYPE_DVD_NAVIGATION_DSI")	== 0)	return	MEDIASUBTYPE_DVD_NAVIGATION_DSI	;
	else if (strcmp (guidString, "IID_IDirectDrawSurface4")	== 0)	return	IID_IDirectDrawSurface4	;
	else if (strcmp (guidString, "CLSID_ModexProperties")	== 0)	return	CLSID_ModexProperties	;
	else if (strcmp (guidString, "MEDIASUBTYPE_DVD_NAVIGATION_PROVIDER")	== 0)	return	MEDIASUBTYPE_DVD_NAVIGATION_PROVIDER	;
	else if (strcmp (guidString, "CLSID_Line21Decoder2")	== 0)	return	CLSID_Line21Decoder2	;
	else if (strcmp (guidString, "MEDIASUBTYPE_DVD_LPCM_AUDIO")	== 0)	return	MEDIASUBTYPE_DVD_LPCM_AUDIO	;
	else if (strcmp (guidString, "MEDIASUBTYPE_DTS")	== 0)	return	MEDIASUBTYPE_DTS	;
	else if (strcmp (guidString, "MEDIASUBTYPE_SDDS")	== 0)	return	MEDIASUBTYPE_SDDS	;
	else if (strcmp (guidString, "MEDIASUBTYPE_ARGB1555_D3D_DX7_RT")	== 0)	return	MEDIASUBTYPE_ARGB1555_D3D_DX7_RT	;
	else if (strcmp (guidString, "MEDIASUBTYPE_ARGB4444_D3D_DX7_RT")	== 0)	return	MEDIASUBTYPE_ARGB4444_D3D_DX7_RT	;
	else if (strcmp (guidString, "MEDIASUBTYPE_ARGB32_D3D_DX7_RT")	== 0)	return	MEDIASUBTYPE_ARGB32_D3D_DX7_RT	;
	else if (strcmp (guidString, "MEDIASUBTYPE_RGB16_D3D_DX7_RT")	== 0)	return	MEDIASUBTYPE_RGB16_D3D_DX7_RT	;
	else if (strcmp (guidString, "MEDIASUBTYPE_RGB32_D3D_DX7_RT")	== 0)	return	MEDIASUBTYPE_RGB32_D3D_DX7_RT	;
	else if (strcmp (guidString, "CLSID_CDeviceMoniker")	== 0)	return	CLSID_CDeviceMoniker	;
	else if (strcmp (guidString, "MEDIASUBTYPE_ARGB1555_D3D_DX9_RT")	== 0)	return	MEDIASUBTYPE_ARGB1555_D3D_DX9_RT	;
	else if (strcmp (guidString, "MEDIASUBTYPE_ARGB4444_D3D_DX9_RT")	== 0)	return	MEDIASUBTYPE_ARGB4444_D3D_DX9_RT	;
	else if (strcmp (guidString, "MEDIASUBTYPE_ARGB32_D3D_DX9_RT")	== 0)	return	MEDIASUBTYPE_ARGB32_D3D_DX9_RT	;
	else if (strcmp (guidString, "MEDIASUBTYPE_RGB16_D3D_DX9_RT")	== 0)	return	MEDIASUBTYPE_RGB16_D3D_DX9_RT	;
	else if (strcmp (guidString, "MEDIASUBTYPE_RGB32_D3D_DX9_RT")	== 0)	return	MEDIASUBTYPE_RGB32_D3D_DX9_RT	;
	else if (strcmp (guidString, "AM_KSCATEGORY_CAPTURE")	== 0)	return	AM_KSCATEGORY_CAPTURE	;
	else if (strcmp (guidString, "AM_KSCATEGORY_RENDER")	== 0)	return	AM_KSCATEGORY_RENDER	;
	else if (strcmp (guidString, "CODECAPI_AUDIO_ENCODER")	== 0)	return	CODECAPI_AUDIO_ENCODER	;
	else if (strcmp (guidString, "MEDIASUBTYPE_RAW_SPORT")	== 0)	return	MEDIASUBTYPE_RAW_SPORT	;
	else if (strcmp (guidString, "CLSID_CMpegVideoCodec")	== 0)	return	CLSID_CMpegVideoCodec	;
	else if (strcmp (guidString, "AM_KSPROPSETID_CopyProt")	== 0)	return	AM_KSPROPSETID_CopyProt	;
	else if (strcmp (guidString, "CLSID_MPEG1PacketPlayer")	== 0)	return	CLSID_MPEG1PacketPlayer	;
	else if (strcmp (guidString, "MEDIASUBTYPE_VPVideo")	== 0)	return	MEDIASUBTYPE_VPVideo	;
	else if (strcmp (guidString, "CLSID_DVMux")	== 0)	return	CLSID_DVMux	;
	else if (strcmp (guidString, "CLSID_InfTee")	== 0)	return	CLSID_InfTee	;
	else if (strcmp (guidString, "IID_IBaseVideoMixer")	== 0)	return	IID_IBaseVideoMixer	;
	else if (strcmp (guidString, "MEDIASUBTYPE_SPDIF_TAG_241h")	== 0)	return	MEDIASUBTYPE_SPDIF_TAG_241h	;
	else if (strcmp (guidString, "MEDIASUBTYPE_AI44")	== 0)	return	MEDIASUBTYPE_AI44	;
	else if (strcmp (guidString, "MEDIASUBTYPE_AYUV")	== 0)	return	MEDIASUBTYPE_AYUV	;
	else if (strcmp (guidString, "MEDIASUBTYPE_VPVBI")	== 0)	return	MEDIASUBTYPE_VPVBI	;
	else if (strcmp (guidString, "CLSID_FilterGraphPrivateThread")	== 0)	return	CLSID_FilterGraphPrivateThread	;
	else if (strcmp (guidString, "CLSID_TVTunerFilterPropertyPage")	== 0)	return	CLSID_TVTunerFilterPropertyPage	;
	else if (strcmp (guidString, "CLSID_TransmitCategory")	== 0)	return	CLSID_TransmitCategory	;
	else if (strcmp (guidString, "MEDIASUBTYPE_CFCC")	== 0)	return	MEDIASUBTYPE_CFCC	;
	else if (strcmp (guidString, "MEDIASUBTYPE_CLJR")	== 0)	return	MEDIASUBTYPE_CLJR	;
	else if (strcmp (guidString, "MEDIASUBTYPE_CLPL")	== 0)	return	MEDIASUBTYPE_CLPL	;
	else if (strcmp (guidString, "ENCAPIPARAM_BITRATE")	== 0)	return	ENCAPIPARAM_BITRATE	;
	else if (strcmp (guidString, "MEDIASUBTYPE_CPLA")	== 0)	return	MEDIASUBTYPE_CPLA	;
	else if (strcmp (guidString, "MEDIASUBTYPE_DVCS")	== 0)	return	MEDIASUBTYPE_DVCS	;
	else if (strcmp (guidString, "MEDIASUBTYPE_DVSD")	== 0)	return	MEDIASUBTYPE_DVSD	;
	else if (strcmp (guidString, "CLSID_DeviceControlCategory")	== 0)	return	CLSID_DeviceControlCategory	;
	else if (strcmp (guidString, "MEDIASUBTYPE_H264")	== 0)	return	MEDIASUBTYPE_H264	;
	else if (strcmp (guidString, "MEDIASUBTYPE_IA44")	== 0)	return	MEDIASUBTYPE_IA44	;
	else if (strcmp (guidString, "MEDIASUBTYPE_IF09")	== 0)	return	MEDIASUBTYPE_IF09	;
	else if (strcmp (guidString, "MEDIASUBTYPE_IJPG")	== 0)	return	MEDIASUBTYPE_IJPG	;
	else if (strcmp (guidString, "MEDIASUBTYPE_IMC1")	== 0)	return	MEDIASUBTYPE_IMC1	;
	else if (strcmp (guidString, "MEDIASUBTYPE_IMC2")	== 0)	return	MEDIASUBTYPE_IMC2	;
	else if (strcmp (guidString, "MEDIASUBTYPE_IMC3")	== 0)	return	MEDIASUBTYPE_IMC3	;
	else if (strcmp (guidString, "MEDIASUBTYPE_IMC4")	== 0)	return	MEDIASUBTYPE_IMC4	;
	else if (strcmp (guidString, "MEDIASUBTYPE_IYUV")	== 0)	return	MEDIASUBTYPE_IYUV	;
	else if (strcmp (guidString, "MEDIASUBTYPE_MDVF")	== 0)	return	MEDIASUBTYPE_MDVF	;
	else if (strcmp (guidString, "MEDIASUBTYPE_MJPG")	== 0)	return	MEDIASUBTYPE_MJPG	;
	else if (strcmp (guidString, "FORMAT_525WSS")	== 0)	return	FORMAT_525WSS	;
	else if (strcmp (guidString, "MEDIASUBTYPE_NV12")	== 0)	return	MEDIASUBTYPE_NV12	;
	else if (strcmp (guidString, "MEDIASUBTYPE_NV24")	== 0)	return	MEDIASUBTYPE_NV24	;
	else if (strcmp (guidString, "MEDIASUBTYPE_MPEG1AudioPayload")	== 0)	return	MEDIASUBTYPE_MPEG1AudioPayload	;

	if (strcmp (guidString, "CLSID_DVVideoEnc")	== 0)	return	CLSID_DVVideoEnc	;
	else if (strcmp (guidString, "MEDIASUBTYPE_Plum")	== 0)	return	MEDIASUBTYPE_Plum	;
	else if (strcmp (guidString, "CLSID_DVEncPropertiesPage")	== 0)	return	CLSID_DVEncPropertiesPage	;
	else if (strcmp (guidString, "CLSID_CMidiOutClassManager")	== 0)	return	CLSID_CMidiOutClassManager	;
	else if (strcmp (guidString, "CLSID_AudioInputMixerProperties")	== 0)	return	CLSID_AudioInputMixerProperties	;
	else if (strcmp (guidString, "MEDIATYPE_DTVCCData")	== 0)	return	MEDIATYPE_DTVCCData	;
	else if (strcmp (guidString, "MEDIASUBTYPE_S340")	== 0)	return	MEDIASUBTYPE_S340	;
	else if (strcmp (guidString, "MEDIASUBTYPE_S342")	== 0)	return	MEDIASUBTYPE_S342	;
	else if (strcmp (guidString, "MEDIASUBTYPE_TVMJ")	== 0)	return	MEDIASUBTYPE_TVMJ	;
	else if (strcmp (guidString, "MEDIASUBTYPE_UYVY")	== 0)	return	MEDIASUBTYPE_UYVY	;
	else if (strcmp (guidString, "MEDIASUBTYPE_WAKE")	== 0)	return	MEDIASUBTYPE_WAKE	;
	else if (strcmp (guidString, "MEDIASUBTYPE_Y211")	== 0)	return	MEDIASUBTYPE_Y211	;
	else if (strcmp (guidString, "MEDIASUBTYPE_Y411")	== 0)	return	MEDIASUBTYPE_Y411	;
	else if (strcmp (guidString, "MEDIASUBTYPE_Y41P")	== 0)	return	MEDIASUBTYPE_Y41P	;
	else if (strcmp (guidString, "MEDIASUBTYPE_YUY2")	== 0)	return	MEDIASUBTYPE_YUY2	;
	else if (strcmp (guidString, "MEDIASUBTYPE_YUYV")	== 0)	return	MEDIASUBTYPE_YUYV	;
	else if (strcmp (guidString, "MEDIASUBTYPE_YV12")	== 0)	return	MEDIASUBTYPE_YV12	;
	else if (strcmp (guidString, "MEDIASUBTYPE_YVU9")	== 0)	return	MEDIASUBTYPE_YVU9	;
	else if (strcmp (guidString, "MEDIASUBTYPE_YVYU")	== 0)	return	MEDIASUBTYPE_YVYU	;
	else if (strcmp (guidString, "CLSID_MediaMultiplexerCategory")	== 0)	return	CLSID_MediaMultiplexerCategory	;
	else if (strcmp (guidString, "CLSID_DVBSNetworkProvider")	== 0)	return	CLSID_DVBSNetworkProvider	;
	else if (strcmp (guidString, "CLSID_MFVideoMixer9")	== 0)	return	CLSID_MFVideoMixer9	;
	else if (strcmp (guidString, "MEDIASUBTYPE_MPEG2DATA")	== 0)	return	MEDIASUBTYPE_MPEG2DATA	;
	else if (strcmp (guidString, "ENCAPIPARAM_BITRATE_MODE")	== 0)	return	ENCAPIPARAM_BITRATE_MODE	;
	else if (strcmp (guidString, "AM_KSPROPSETID_DvdSubPic")	== 0)	return	AM_KSPROPSETID_DvdSubPic	;
	else if (strcmp (guidString, "IID_IDDVideoPortContainer")	== 0)	return	IID_IDDVideoPortContainer	;
	else if (strcmp (guidString, "IID_IAMDirectSound")	== 0)	return	IID_IAMDirectSound	;
	else if (strcmp (guidString, "CLSID_AVIMIDIRender")	== 0)	return	CLSID_AVIMIDIRender	;
	else if (strcmp (guidString, "IID_IVPConfig")	== 0)	return	IID_IVPConfig	;
	else if (strcmp (guidString, "CLSID_CIcmCoClassManager")	== 0)	return	CLSID_CIcmCoClassManager	;
	else if (strcmp (guidString, "CLSID_MPEG1Doc")	== 0)	return	CLSID_MPEG1Doc	;
	else if (strcmp (guidString, "AM_KSCATEGORY_VBICODEC")	== 0)	return	AM_KSCATEGORY_VBICODEC	;
	else if (strcmp (guidString, "CLSID_AVIDraw")	== 0)	return	CLSID_AVIDraw	;
	else if (strcmp (guidString, "CLSID_VPObject")	== 0)	return	CLSID_VPObject	;
	else if (strcmp (guidString, "CLSID_MFVideoPresenter9")	== 0)	return	CLSID_MFVideoPresenter9	;
	else if (strcmp (guidString, "CLSID_CrossbarFilterPropertyPage")	== 0)	return	CLSID_CrossbarFilterPropertyPage	;
	else if (strcmp (guidString, "MEDIATYPE_Audio")	== 0)	return	MEDIATYPE_Audio	;
	else if (strcmp (guidString, "CLSID_CAcmCoClassManager")	== 0)	return	CLSID_CAcmCoClassManager	;
	else if (strcmp (guidString, "CLSID_Mpeg2VideoStreamAnalyzer")	== 0)	return	CLSID_Mpeg2VideoStreamAnalyzer	;
	else if (strcmp (guidString, "IID_IVPObject")	== 0)	return	IID_IVPObject	;
	else if (strcmp (guidString, "CLSID_CWaveinClassManager")	== 0)	return	CLSID_CWaveinClassManager	;
	else if (strcmp (guidString, "CLSID_AllocPresenter")	== 0)	return	CLSID_AllocPresenter	;
	else if (strcmp (guidString, "CLSID_TVAudioFilterPropertyPage")	== 0)	return	CLSID_TVAudioFilterPropertyPage	;
	else if (strcmp (guidString, "CLSID_VideoProcAmpPropertyPage")	== 0)	return	CLSID_VideoProcAmpPropertyPage	;
	else if (strcmp (guidString, "MEDIATYPE_ScriptCommand")	== 0)	return	MEDIATYPE_ScriptCommand	;
	else if (strcmp (guidString, "MEDIASUBTYPE_dv25")	== 0)	return	MEDIASUBTYPE_dv25	;
	else if (strcmp (guidString, "MEDIASUBTYPE_dv50")	== 0)	return	MEDIASUBTYPE_dv50	;
	else if (strcmp (guidString, "MEDIASUBTYPE_dvh1")	== 0)	return	MEDIASUBTYPE_dvh1	;
	else if (strcmp (guidString, "MEDIASUBTYPE_dvhd")	== 0)	return	MEDIASUBTYPE_dvhd	;
	else if (strcmp (guidString, "MEDIASUBTYPE_dvsd")	== 0)	return	MEDIASUBTYPE_dvsd	;
	else if (strcmp (guidString, "MEDIASUBTYPE_dvsl")	== 0)	return	MEDIASUBTYPE_dvsl	;
	else if (strcmp (guidString, "CLSID_CameraControlPropertyPage")	== 0)	return	CLSID_CameraControlPropertyPage	;
	else if (strcmp (guidString, "CLSID_ModexRenderer")	== 0)	return	CLSID_ModexRenderer	;
	else if (strcmp (guidString, "CLSID_AnalogVideoDecoderPropertyPage")	== 0)	return	CLSID_AnalogVideoDecoderPropertyPage	;
	else if (strcmp (guidString, "MEDIATYPE_File")	== 0)	return	MEDIATYPE_File	;
	else if (strcmp (guidString, "CLSID_VideoStreamConfigPropertyPage")	== 0)	return	CLSID_VideoStreamConfigPropertyPage	;
	else if (strcmp (guidString, "MEDIATYPE_Interleaved")	== 0)	return	MEDIATYPE_Interleaved	;
	else if (strcmp (guidString, "MEDIASUBTYPE_QTJpeg")	== 0)	return	MEDIASUBTYPE_QTJpeg	;
	else if (strcmp (guidString, "MEDIATYPE_DVD_ENCRYPTED_PACK")	== 0)	return	MEDIATYPE_DVD_ENCRYPTED_PACK	;
	else if (strcmp (guidString, "IID_IKsPinFactory")	== 0)	return	IID_IKsPinFactory	;
	else if (strcmp (guidString, "MEDIATYPE_MPEG2_SECTIONS")	== 0)	return	MEDIATYPE_MPEG2_SECTIONS	;
	else if (strcmp (guidString, "CLSID_EnhancedVideoRenderer")	== 0)	return	CLSID_EnhancedVideoRenderer	;
	else if (strcmp (guidString, "CLSID_SeekingPassThru")	== 0)	return	CLSID_SeekingPassThru	;
	else if (strcmp (guidString, "MEDIATYPE_Midi")	== 0)	return	MEDIATYPE_Midi	;
	else if (strcmp (guidString, "MEDIATYPE_LMRT")	== 0)	return	MEDIATYPE_LMRT	;
	else if (strcmp (guidString, "MEDIASUBTYPE_A2R10G10B10")	== 0)	return	MEDIASUBTYPE_A2R10G10B10	;
	else if (strcmp (guidString, "CLSID_AviDest")	== 0)	return	CLSID_AviDest	;
	else if (strcmp (guidString, "CLSID_DVSplitter")	== 0)	return	CLSID_DVSplitter	;
	else if (strcmp (guidString, "TIME_FORMAT_FRAME")	== 0)	return	TIME_FORMAT_FRAME	;
	else if (strcmp (guidString, "IID_IFullScreenVideoEx")	== 0)	return	IID_IFullScreenVideoEx	;
	else if (strcmp (guidString, "IID_IMpegAudioDecoder")	== 0)	return	IID_IMpegAudioDecoder	;
	else if (strcmp (guidString, "CLSID_QualityProperties")	== 0)	return	CLSID_QualityProperties	;
	else if (strcmp (guidString, "TIME_FORMAT_BYTE")	== 0)	return	TIME_FORMAT_BYTE	;
	else if (strcmp (guidString, "TIME_FORMAT_SAMPLE")	== 0)	return	TIME_FORMAT_SAMPLE	;
	else if (strcmp (guidString, "MEDIASUBTYPE_QTRle")	== 0)	return	MEDIASUBTYPE_QTRle	;
	else if (strcmp (guidString, "MEDIASUBTYPE_QTRpza")	== 0)	return	MEDIASUBTYPE_QTRpza	;
	else if (strcmp (guidString, "TIME_FORMAT_FIELD")	== 0)	return	TIME_FORMAT_FIELD	;
	else if (strcmp (guidString, "MEDIASUBTYPE_QTSmc")	== 0)	return	MEDIASUBTYPE_QTSmc	;
	else if (strcmp (guidString, "TIME_FORMAT_MEDIA_TIME")	== 0)	return	TIME_FORMAT_MEDIA_TIME	;
	else if (strcmp (guidString, "MEDIATYPE_Text")	== 0)	return	MEDIATYPE_Text	;
	else if (strcmp (guidString, "MEDIATYPE_URL_STREAM")	== 0)	return	MEDIATYPE_URL_STREAM	;
	else if (strcmp (guidString, "MEDIATYPE_Video")	== 0)	return	MEDIATYPE_Video	;
	else if (strcmp (guidString, "MEDIASUBTYPE_WSS")	== 0)	return	MEDIASUBTYPE_WSS	;
	else if (strcmp (guidString, "AM_KSCATEGORY_VBICODEC_MI")	== 0)	return	AM_KSCATEGORY_VBICODEC_MI	;
	else if (strcmp (guidString, "MEDIASUBTYPE_RGB1")	== 0)	return	MEDIASUBTYPE_RGB1	;
	else if (strcmp (guidString, "MEDIASUBTYPE_RGB4")	== 0)	return	MEDIASUBTYPE_RGB4	;
	else if (strcmp (guidString, "CLSID_ICodecAPIProxy")	== 0)	return	CLSID_ICodecAPIProxy	;
	else if (strcmp (guidString, "MEDIASUBTYPE_RGB8")	== 0)	return	MEDIASUBTYPE_RGB8	;
	else if (strcmp (guidString, "CLSID_EVRTearlessWindowPresenter9")	== 0)	return	CLSID_EVRTearlessWindowPresenter9	;
	else if (strcmp (guidString, "MEDIASUBTYPE_RGB565")	== 0)	return	MEDIASUBTYPE_RGB565	;
	else if (strcmp (guidString, "CLSID_VideoMixingRenderer")	== 0)	return	CLSID_VideoMixingRenderer	;
	else if (strcmp (guidString, "MEDIASUBTYPE_ATSC_SI")	== 0)	return	MEDIASUBTYPE_ATSC_SI	;
	else if (strcmp (guidString, "CODECAPI_SETALLDEFAULTS")	== 0)	return	CODECAPI_SETALLDEFAULTS	;
	else if (strcmp (guidString, "CLSID_NetworkProvider")	== 0)	return	CLSID_NetworkProvider	;
	else if (strcmp (guidString, "MEDIASUBTYPE_RGB555")	== 0)	return	MEDIASUBTYPE_RGB555	;
	else if (strcmp (guidString, "MEDIASUBTYPE_RGB24")	== 0)	return	MEDIASUBTYPE_RGB24	;
	else if (strcmp (guidString, "MEDIASUBTYPE_RGB32")	== 0)	return	MEDIASUBTYPE_RGB32	;
	else if (strcmp (guidString, "MEDIASUBTYPE_Overlay")	== 0)	return	MEDIASUBTYPE_Overlay	;
	else if (strcmp (guidString, "CLSID_DShowTVEFilter")	== 0)	return	CLSID_DShowTVEFilter	;
	else if (strcmp (guidString, "IID_IDirectDrawSurface7")	== 0)	return	IID_IDirectDrawSurface7	;
	else if (strcmp (guidString, "CLSID_PerformanceProperties")	== 0)	return	CLSID_PerformanceProperties	;
	else if (strcmp (guidString, "CLSID_WstDecoderPropertyPage")	== 0)	return	CLSID_WstDecoderPropertyPage	;
	else if (strcmp (guidString, "FORMAT_VideoInfo")	== 0)	return	FORMAT_VideoInfo	;
	else if (strcmp (guidString, "CLSID_MPEG2Demultiplexer")	== 0)	return	CLSID_MPEG2Demultiplexer	;
	else if (strcmp (guidString, "CLSID_ACMWrapper")	== 0)	return	CLSID_ACMWrapper	;
	else if (strcmp (guidString, "IID_IDirectDraw")	== 0)	return	IID_IDirectDraw	;
	else if (strcmp (guidString, "CLSID_Colour")	== 0)	return	CLSID_Colour	;
	else if (strcmp (guidString, "CLSID_SmartTee")	== 0)	return	CLSID_SmartTee	;
	else if (strcmp (guidString, "MEDIATYPE_AUXLine21Data")	== 0)	return	MEDIATYPE_AUXLine21Data	;
	else if (strcmp (guidString, "MEDIASUBTYPE_MPEG1Packet")	== 0)	return	MEDIASUBTYPE_MPEG1Packet	;
	else if (strcmp (guidString, "CLSID_TVEFilterTuneProperties")	== 0)	return	CLSID_TVEFilterTuneProperties	;
	else if (strcmp (guidString, "PIN_CATEGORY_CAPTURE")	== 0)	return	PIN_CATEGORY_CAPTURE	;
	else if (strcmp (guidString, "MEDIASUBTYPE_DssVideo")	== 0)	return	MEDIASUBTYPE_DssVideo	;
	else if (strcmp (guidString, "CLSID_QTDec")	== 0)	return	CLSID_QTDec	;
	else if (strcmp (guidString, "FORMAT_WaveFormatEx")	== 0)	return	FORMAT_WaveFormatEx	;
	else if (strcmp (guidString, "IID_IDirectDrawSurface")	== 0)	return	IID_IDirectDrawSurface	;
	else if (strcmp (guidString, "MEDIASUBTYPE_MPEG1Payload")	== 0)	return	MEDIASUBTYPE_MPEG1Payload	;
	else if (strcmp (guidString, "CLSID_TVEFilterCCProperties")	== 0)	return	CLSID_TVEFilterCCProperties	;
	else if (strcmp (guidString, "PIN_CATEGORY_PREVIEW")	== 0)	return	PIN_CATEGORY_PREVIEW	;
	else if (strcmp (guidString, "MEDIASUBTYPE_DssAudio")	== 0)	return	MEDIASUBTYPE_DssAudio	;
	else if (strcmp (guidString, "IID_IMixerPinConfig2")	== 0)	return	IID_IMixerPinConfig2	;
	else if (strcmp (guidString, "FORMAT_MPEGVideo")	== 0)	return	FORMAT_MPEGVideo	;
	else if (strcmp (guidString, "MEDIATYPE_MPEG1SystemStream")	== 0)	return	MEDIATYPE_MPEG1SystemStream	;
	
	if (strcmp (guidString, "CLSID_TVEFilterStatsProperties")	== 0)	return	CLSID_TVEFilterStatsProperties	;
	else if (strcmp (guidString, "PIN_CATEGORY_ANALOGVIDEOIN")	== 0)	return	PIN_CATEGORY_ANALOGVIDEOIN	;
	else if (strcmp (guidString, "CODECAPI_CURRENTCHANGELIST")	== 0)	return	CODECAPI_CURRENTCHANGELIST	;
	else if (strcmp (guidString, "IID_IVPNotify2")	== 0)	return	IID_IVPNotify2	;
	else if (strcmp (guidString, "FORMAT_MPEGStreams")	== 0)	return	FORMAT_MPEGStreams	;
	else if (strcmp (guidString, "MEDIATYPE_Stream")	== 0)	return	MEDIATYPE_Stream	;
	else if (strcmp (guidString, "PIN_CATEGORY_VBI")	== 0)	return	PIN_CATEGORY_VBI	;
	else if (strcmp (guidString, "FORMAT_DvInfo")	== 0)	return	FORMAT_DvInfo	;
	else if (strcmp (guidString, "IID_IDirectDrawPalette")	== 0)	return	IID_IDirectDrawPalette	;
	else if (strcmp (guidString, "MEDIASUBTYPE_MPEG1System")	== 0)	return	MEDIASUBTYPE_MPEG1System	;
	else if (strcmp (guidString, "PIN_CATEGORY_VIDEOPORT")	== 0)	return	PIN_CATEGORY_VIDEOPORT	;
	else if (strcmp (guidString, "IID_IKsControl")	== 0)	return	IID_IKsControl	;
	else if (strcmp (guidString, "IID_IDirectDrawSurface2")	== 0)	return	IID_IDirectDrawSurface2	;
	else if (strcmp (guidString, "IID_IDirectDrawClipper")	== 0)	return	IID_IDirectDrawClipper	;
	else if (strcmp (guidString, "MEDIASUBTYPE_MPEG1VideoCD")	== 0)	return	MEDIASUBTYPE_MPEG1VideoCD	;
	else if (strcmp (guidString, "PIN_CATEGORY_NABTS")	== 0)	return	PIN_CATEGORY_NABTS	;
	else if (strcmp (guidString, "MEDIASUBTYPE_MPEG1Video")	== 0)	return	MEDIASUBTYPE_MPEG1Video	;
	else if (strcmp (guidString, "PIN_CATEGORY_EDS")	== 0)	return	PIN_CATEGORY_EDS	;
	else if (strcmp (guidString, "MEDIASUBTYPE_MPEG1Audio")	== 0)	return	MEDIASUBTYPE_MPEG1Audio	;
	else if (strcmp (guidString, "PIN_CATEGORY_TELETEXT")	== 0)	return	PIN_CATEGORY_TELETEXT	;
	else if (strcmp (guidString, "MEDIASUBTYPE_Avi")	== 0)	return	MEDIASUBTYPE_Avi	;
	else if (strcmp (guidString, "PIN_CATEGORY_CC")	== 0)	return	PIN_CATEGORY_CC	;
	else if (strcmp (guidString, "MEDIATYPE_MSTVCaption")	== 0)	return	MEDIATYPE_MSTVCaption	;
	else if (strcmp (guidString, "MEDIASUBTYPE_QTMovie")	== 0)	return	MEDIASUBTYPE_QTMovie	;
	else if (strcmp (guidString, "PIN_CATEGORY_STILL")	== 0)	return	PIN_CATEGORY_STILL	;
	else if (strcmp (guidString, "MEDIASUBTYPE_PCMAudio_Obsolete")	== 0)	return	MEDIASUBTYPE_PCMAudio_Obsolete	;
	else if (strcmp (guidString, "PIN_CATEGORY_TIMECODE")	== 0)	return	PIN_CATEGORY_TIMECODE	;
	else if (strcmp (guidString, "MEDIASUBTYPE_WAVE")	== 0)	return	MEDIASUBTYPE_WAVE	;
	else if (strcmp (guidString, "PIN_CATEGORY_VIDEOPORT_VBI")	== 0)	return	PIN_CATEGORY_VIDEOPORT_VBI	;
	else if (strcmp (guidString, "MEDIASUBTYPE_AU")	== 0)	return	MEDIASUBTYPE_AU	;
	else if (strcmp (guidString, "MEDIASUBTYPE_AIFF")	== 0)	return	MEDIASUBTYPE_AIFF	;
	else if (strcmp (guidString, "MEDIASUBTYPE_None")	== 0)	return	MEDIASUBTYPE_None	;
	else if (strcmp (guidString, "MEDIASUBTYPE_Asf")	== 0)	return	MEDIASUBTYPE_Asf	;
	else if (strcmp (guidString, "MEDIASUBTYPE_DOLBY_AC3_SPDIF")	== 0)	return	MEDIASUBTYPE_DOLBY_AC3_SPDIF	;
	else if (strcmp (guidString, "CLSID_AudioRendererAdvancedProperties")	== 0)	return	CLSID_AudioRendererAdvancedProperties	;
	else if (strcmp (guidString, "CODECAPI_ALLSETTINGS")	== 0)	return	CODECAPI_ALLSETTINGS	;
	else if (strcmp (guidString, "MEDIASUBTYPE_A2B10G10R10")	== 0)	return	MEDIASUBTYPE_A2B10G10R10	;
	else if (strcmp (guidString, "CODECAPI_SUPPORTSEVENTS")	== 0)	return	CODECAPI_SUPPORTSEVENTS	;
	else if (strcmp (guidString, "CLSID_AllocPresenterDDXclMode")	== 0)	return	CLSID_AllocPresenterDDXclMode	;
	else if (strcmp (guidString, "CLSID_DirectDrawClipper")	== 0)	return	CLSID_DirectDrawClipper	;
	else if (strcmp (guidString, "CLSID_ActiveMovieCategories")	== 0)	return	CLSID_ActiveMovieCategories	;
	else if (strcmp (guidString, "AM_KSCATEGORY_SPLITTER")	== 0)	return	AM_KSCATEGORY_SPLITTER	;
	else if (strcmp (guidString, "IID_IDirectDrawSurfaceKernel")	== 0)	return	IID_IDirectDrawSurfaceKernel	;
	else if (strcmp (guidString, "CLSID_WMAsfReader")	== 0)	return	CLSID_WMAsfReader	;
	else if (strcmp (guidString, "AM_INTERFACESETID_Standard")	== 0)	return	AM_INTERFACESETID_Standard	;
	else if (strcmp (guidString, "CLSID_DtvCcFilter")	== 0)	return	CLSID_DtvCcFilter	;
	else if (strcmp (guidString, "FORMAT_VIDEOINFO2")	== 0)	return	FORMAT_VIDEOINFO2	;
	else if (strcmp (guidString, "CLSID_MJPGEnc")	== 0)	return	CLSID_MJPGEnc	;
	else if (strcmp (guidString, "CLSID_QuickTimeParser")	== 0)	return	CLSID_QuickTimeParser	;
	else if (strcmp (guidString, "CLSID_OverlayMixer")	== 0)	return	CLSID_OverlayMixer	;
	else if (strcmp (guidString, "IID_IVPNotify")	== 0)	return	IID_IVPNotify	;
	else if (strcmp (guidString, "MEDIASUBTYPE_DVB_SI")	== 0)	return	MEDIASUBTYPE_DVB_SI	;
	else if (strcmp (guidString, "MEDIASUBTYPE_MPEG2_TRANSPORT_STRIDE")	== 0)	return	MEDIASUBTYPE_MPEG2_TRANSPORT_STRIDE	;
	else if (strcmp (guidString, "ENCAPIPARAM_PEAK_BITRATE")	== 0)	return	ENCAPIPARAM_PEAK_BITRATE	;
	else if (strcmp (guidString, "MEDIASUBTYPE_DtvCcData")	== 0)	return	MEDIASUBTYPE_DtvCcData	;
	else if (strcmp (guidString, "CLSID_StreamBufferRecordingAttributes")	== 0)	return	CLSID_StreamBufferRecordingAttributes	;
	else if (strcmp (guidString, "AM_KSPROPSETID_DvdKaraoke")	== 0)	return	AM_KSPROPSETID_DvdKaraoke	;
	else if (strcmp (guidString, "MEDIASUBTYPE_ARGB1555")	== 0)	return	MEDIASUBTYPE_ARGB1555	;
	else if (strcmp (guidString, "CLSID_AudioProperties")	== 0)	return	CLSID_AudioProperties	;
	else if (strcmp (guidString, "CLSID_VideoRenderer")	== 0)	return	CLSID_VideoRenderer	;
	else if (strcmp (guidString, "MEDIASUBTYPE_MPEG2_VERSIONED_TABLES")	== 0)	return	MEDIASUBTYPE_MPEG2_VERSIONED_TABLES	;
	else if (strcmp (guidString, "CLSID_AVIDoc")	== 0)	return	CLSID_AVIDoc	;
	else if (strcmp (guidString, "IID_IDirectDrawVideo")	== 0)	return	IID_IDirectDrawVideo	;
	else if (strcmp (guidString, "CLSID_ProtoFilterGraph")	== 0)	return	CLSID_ProtoFilterGraph	;
	else if (strcmp (guidString, "IID_IQualProp")	== 0)	return	IID_IQualProp	;
	else if (strcmp (guidString, "CLSID_SystemClock")	== 0)	return	CLSID_SystemClock	;
	else if (strcmp (guidString, "MEDIASUBTYPE_TIF_SI")	== 0)	return	MEDIASUBTYPE_TIF_SI	;
	else if (strcmp (guidString, "CLSID_StreamBufferConfig")	== 0)	return	CLSID_StreamBufferConfig	;
	else if (strcmp (guidString, "CLSID_FilterMapper")	== 0)	return	CLSID_FilterMapper	;
	else if (strcmp (guidString, "CLSID_FilterGraph")	== 0)	return	CLSID_FilterGraph	;
	else if (strcmp (guidString, "CLSID_FGControl")	== 0)	return	CLSID_FGControl	;
	else if (strcmp (guidString, "CLSID_AsyncReader")	== 0)	return	CLSID_AsyncReader	;
	else if (strcmp (guidString, "CLSID_MSTVCaptionFilter")	== 0)	return	CLSID_MSTVCaptionFilter	;
	else if (strcmp (guidString, "CLSID_URLReader")	== 0)	return	CLSID_URLReader	;
	else if (strcmp (guidString, "CLSID_DvdGraphBuilder")	== 0)	return	CLSID_DvdGraphBuilder	;
	else if (strcmp (guidString, "CLSID_PersistMonikerPID")	== 0)	return	CLSID_PersistMonikerPID	;
	else if (strcmp (guidString, "CLSID_FilterGraphNoThread")	== 0)	return	CLSID_FilterGraphNoThread	;
	else if (strcmp (guidString, "CLSID_StreamBufferComposeRecording")	== 0)	return	CLSID_StreamBufferComposeRecording	;
	else if (strcmp (guidString, "MEDIASUBTYPE_708_608Data")	== 0)	return	MEDIASUBTYPE_708_608Data	;
	else if (strcmp (guidString, "AM_KSPROPSETID_FrameStep")	== 0)	return	AM_KSPROPSETID_FrameStep	;
	else if (strcmp (guidString, "CLSID_MemoryAllocator")	== 0)	return	CLSID_MemoryAllocator	;
	else if (strcmp (guidString, "CLSID_DVDecPropertiesPage")	== 0)	return	CLSID_DVDecPropertiesPage	;
	else if (strcmp (guidString, "MEDIASUBTYPE_ARGB32")	== 0)	return	MEDIASUBTYPE_ARGB32	;
	else if (strcmp (guidString, "CLSID_AviMuxProptyPage")	== 0)	return	CLSID_AviMuxProptyPage	;
	else if (strcmp (guidString, "AM_KSPROPSETID_TSRateChange")	== 0)	return	AM_KSPROPSETID_TSRateChange	;
	else if (strcmp (guidString, "IID_IVPControl")	== 0)	return	IID_IVPControl	;
	else if (strcmp (guidString, "DSATTRIB_UDCRTag")	== 0)	return	DSATTRIB_UDCRTag	;
	else if (strcmp (guidString, "CLSID_VideoPortManager")	== 0)	return	CLSID_VideoPortManager	;
	else if (strcmp (guidString, "CODECAPI_CHANGELISTS")	== 0)	return	CODECAPI_CHANGELISTS	;
	else if (strcmp (guidString, "CLSID_DVDState")	== 0)	return	CLSID_DVDState	;
	else if (strcmp (guidString, "CLSID_MjpegDec")	== 0)	return	CLSID_MjpegDec	;
	else if (strcmp (guidString, "CLSID_MPEG1Splitter")	== 0)	return	CLSID_MPEG1Splitter	;
	else if (strcmp (guidString, "CLSID_AudioRender")	== 0)	return	CLSID_AudioRender	;
	else if (strcmp (guidString, "IID_IKsPin")	== 0)	return	IID_IKsPin	;
	else if (strcmp (guidString, "CLSID_AudioRecord")	== 0)	return	CLSID_AudioRecord	;
	else if (strcmp (guidString, "CLSID_TextRender")	== 0)	return	CLSID_TextRender	;
	else if (strcmp (guidString, "FORMAT_None")	== 0)	return	FORMAT_None	;
	else if (strcmp (guidString, "CLSID_IVideoEncoderCodecAPIProxy")	== 0)	return	CLSID_IVideoEncoderCodecAPIProxy	;
	else if (strcmp (guidString, "ENCAPIPARAM_SAP_MODE")	== 0)	return	ENCAPIPARAM_SAP_MODE	;
	else if (strcmp (guidString, "CLSID_ATSCNetworkProvider")	== 0)	return	CLSID_ATSCNetworkProvider	;
	else if (strcmp (guidString, "CLSID_DVBTNetworkProvider")	== 0)	return	CLSID_DVBTNetworkProvider	;
	else if (strcmp (guidString, "CLSID_WSTDecoder")	== 0)	return	CLSID_WSTDecoder	;
	else if (strcmp (guidString, "IID_IDirectDrawColorControl")	== 0)	return	IID_IDirectDrawColorControl	;
	else if (strcmp (guidString, "CLSID_DirectDraw")	== 0)	return	CLSID_DirectDraw	;
	else if (strcmp (guidString, "CLSID_FileSource")	== 0)	return	CLSID_FileSource	;
	else if (strcmp (guidString, "CLSID_DVMuxPropertyPage")	== 0)	return	CLSID_DVMuxPropertyPage	;
	else if (strcmp (guidString, "CLSID_CMpegAudioCodec")	== 0)	return	CLSID_CMpegAudioCodec	;
	else if (strcmp (guidString, "LOOK_UPSTREAM_ONLY")	== 0)	return	LOOK_UPSTREAM_ONLY	;
	else if (strcmp (guidString, "CLSID_CaptureGraphBuilder")	== 0)	return	CLSID_CaptureGraphBuilder	;
	else if (strcmp (guidString, "IID_IKsInterfaceHandler")	== 0)	return	IID_IKsInterfaceHandler	;
	else if (strcmp (guidString, "CLSID_AVIDec")	== 0)	return	CLSID_AVIDec	;
	else if (strcmp (guidString, "FORMAT_AnalogVideo")	== 0)	return	FORMAT_AnalogVideo	;
	else if (strcmp (guidString, "IID_IDirectDraw2")	== 0)	return	IID_IDirectDraw2	;
	else if (strcmp (guidString, "CLSID_CWaveOutClassManager")	== 0)	return	CLSID_CWaveOutClassManager	;
	else if (strcmp (guidString, "MEDIATYPE_VBI")	== 0)	return	MEDIATYPE_VBI	;
	else if (strcmp (guidString, "LOOK_DOWNSTREAM_ONLY")	== 0)	return	LOOK_DOWNSTREAM_ONLY	;
	else if (strcmp (guidString, "CLSID_CaptureGraphBuilder2")	== 0)	return	CLSID_CaptureGraphBuilder2	;
	else if (strcmp (guidString, "IID_IMixerPinConfig")	== 0)	return	IID_IMixerPinConfig	;
	else if (strcmp (guidString, "MEDIATYPE_AnalogVideo")	== 0)	return	MEDIATYPE_AnalogVideo	;
	else if (strcmp (guidString, "MEDIATYPE_AnalogAudio")	== 0)	return	MEDIATYPE_AnalogAudio	;
	else if (strcmp (guidString, "CODECAPI_VIDEO_ENCODER")	== 0)	return	CODECAPI_VIDEO_ENCODER	;
	else if (strcmp (guidString, "MEDIASUBTYPE_AnalogVideo_NTSC_M")	== 0)	return	MEDIASUBTYPE_AnalogVideo_NTSC_M	;
	else if (strcmp (guidString, "MEDIASUBTYPE_TELETEXT")	== 0)	return	MEDIASUBTYPE_TELETEXT	;
	
	if (strcmp (guidString, "FORMAT_MPEG2Video")	== 0)	return	FORMAT_MPEG2Video	;
	else if (strcmp (guidString, "MEDIATYPE_Timecode")	== 0)	return	MEDIATYPE_Timecode	;
	else if (strcmp (guidString, "FORMAT_DolbyAC3")	== 0)	return	FORMAT_DolbyAC3	;
	else if (strcmp (guidString, "CLSID_StreamBufferSink")	== 0)	return	CLSID_StreamBufferSink	;
	else if (strcmp (guidString, "FORMAT_MPEG2Audio")	== 0)	return	FORMAT_MPEG2Audio	;
	else if (strcmp (guidString, "MEDIASUBTYPE_AnalogVideo_PAL_B")	== 0)	return	MEDIASUBTYPE_AnalogVideo_PAL_B	;
	else if (strcmp (guidString, "MEDIASUBTYPE_ARGB4444")	== 0)	return	MEDIASUBTYPE_ARGB4444	;
	else if (strcmp (guidString, "FORMAT_DVD_LPCMAudio")	== 0)	return	FORMAT_DVD_LPCMAudio	;
	else if (strcmp (guidString, "MEDIASUBTYPE_AnalogVideo_PAL_D")	== 0)	return	MEDIASUBTYPE_AnalogVideo_PAL_D	;
	else if (strcmp (guidString, "CLSID_DVBCNetworkProvider")	== 0)	return	CLSID_DVBCNetworkProvider	;
	else if (strcmp (guidString, "MEDIASUBTYPE_AnalogVideo_PAL_G")	== 0)	return	MEDIASUBTYPE_AnalogVideo_PAL_G	;
	else if (strcmp (guidString, "MEDIASUBTYPE_AnalogVideo_PAL_H")	== 0)	return	MEDIASUBTYPE_AnalogVideo_PAL_H	;
	else if (strcmp (guidString, "MEDIASUBTYPE_AnalogVideo_PAL_I")	== 0)	return	MEDIASUBTYPE_AnalogVideo_PAL_I	;
	else if (strcmp (guidString, "MEDIASUBTYPE_MPEG2_UDCR_TRANSPORT")	== 0)	return	MEDIASUBTYPE_MPEG2_UDCR_TRANSPORT	;
	else if (strcmp (guidString, "MEDIASUBTYPE_AnalogVideo_PAL_M")	== 0)	return	MEDIASUBTYPE_AnalogVideo_PAL_M	;
	else if (strcmp (guidString, "MEDIASUBTYPE_AnalogVideo_PAL_N")	== 0)	return	MEDIASUBTYPE_AnalogVideo_PAL_N	;
	else if (strcmp (guidString, "CLSID_IVideoEncoderProxy")	== 0)	return	CLSID_IVideoEncoderProxy	;
	else if (strcmp (guidString, "MEDIASUBTYPE_AnalogVideo_PAL_N_COMBO")	== 0)	return	MEDIASUBTYPE_AnalogVideo_PAL_N_COMBO	;
	else if (strcmp (guidString, "MEDIASUBTYPE_AnalogVideo_SECAM_B")	== 0)	return	MEDIASUBTYPE_AnalogVideo_SECAM_B	;
	else if (strcmp (guidString, "CLSID_FileWriter")	== 0)	return	CLSID_FileWriter	;
	else if (strcmp (guidString, "CLSID_CQzFilterClassManager")	== 0)	return	CLSID_CQzFilterClassManager	;
	else if (strcmp (guidString, "MEDIASUBTYPE_AnalogVideo_SECAM_D")	== 0)	return	MEDIASUBTYPE_AnalogVideo_SECAM_D	;
	else if (strcmp (guidString, "MEDIASUBTYPE_AnalogVideo_SECAM_G")	== 0)	return	MEDIASUBTYPE_AnalogVideo_SECAM_G	;
	else if (strcmp (guidString, "CLSID_VideoMixingRenderer9")	== 0)	return	CLSID_VideoMixingRenderer9	;
	else if (strcmp (guidString, "MEDIASUBTYPE_AnalogVideo_SECAM_H")	== 0)	return	MEDIASUBTYPE_AnalogVideo_SECAM_H	;
	else if (strcmp (guidString, "MEDIASUBTYPE_AnalogVideo_SECAM_K")	== 0)	return	MEDIASUBTYPE_AnalogVideo_SECAM_K	;
	else if (strcmp (guidString, "MEDIASUBTYPE_AnalogVideo_SECAM_K1")	== 0)	return	MEDIASUBTYPE_AnalogVideo_SECAM_K1	;
	else if (strcmp (guidString, "MEDIASUBTYPE_AnalogVideo_SECAM_L")	== 0)	return	MEDIASUBTYPE_AnalogVideo_SECAM_L	;
	else if (strcmp (guidString, "CLSID_VideoRendererDefault")	== 0)	return	CLSID_VideoRendererDefault	;

	return SSI_GUID_NULL;
}*/


bool CameraTools::StringToGUID (const char *guidString, GUID &guid) {

	USES_CONVERSION;
	LPOLESTR str = T2OLE (ssi_ccast (char *, guidString));

	if (::CLSIDFromString (str, &guid) != NOERROR) {
		return false;
	}

	return true;
}

bool CameraTools::GUIDToString (GUID guid, char *guidString, ssi_size_t len) {

	LPOLESTR str = new OLECHAR[len];
	
	if (::StringFromGUID2 (guid, str, len) == 0) {
		delete[] str;
		return false;
	}

	sprintf (guidString, "%ls", str); 

	delete[] str;
	return true;
}


void CameraTools::FlipImage (BYTE *dst, const BYTE *src, ssi_video_params_t &params) {

	BYTE *dstPtr = NULL;
	const BYTE *srcPtr = NULL;
	int copyLength = 0;
	int widthStepInBytes = ssi_video_stride (params);
	int biBitCount = params.depthInBitsPerChannel * params.numOfChannels;
	dstPtr = dst + ((params.heightInPixels - 1) * widthStepInBytes);
	srcPtr = src;
	switch(biBitCount)
	{
	case 24:
		copyLength = params.widthInPixels * 3;				
		break;
	case 32:
		copyLength = params.widthInPixels * 4;
		break;
	default:
		ssi_err ("biBitCount = %d not supported", biBitCount);
		break;
	}
	for(int j = 0; j < params.heightInPixels; ++j)
	{
		memcpy(dstPtr, srcPtr, copyLength);
		dstPtr -= widthStepInBytes;
		srcPtr += widthStepInBytes;
	}

}

}
