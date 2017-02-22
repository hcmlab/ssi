// VideoTypeInfoList.cpp
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

#include "VideoTypeInfoList.h"
#include "graphic/DialogLibGateway.h"

#ifndef SafeReleaseFJ
#define SafeReleaseFJ(p) { if( (p) != 0 ) { (p)->Release(); (p)= 0; } }
#endif

namespace ssi
{
int VideoTypeInfoList::ssi_log_level_static = SSI_LOG_LEVEL_DEFAULT;
static char ssi_log_name_static[] = "vidinfol_s";

void VideoTypeInfoList::setLogLevel(int level)
{
	ssi_log_level = level;
}

void VideoTypeInfoList::SetLogLevelStatic(int level)
{
	ssi_log_level_static = level;
}

VideoTypeInfoList::VideoTypeInfoList(IBaseFilter *pBaseFilter) :
	_comInitCountConstructor(0),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT)
{
	//Set up logging facility
	static int device_counter = 1;
	ssi_log_name = new char[SSI_MAX_CHAR];
	sprintf (ssi_log_name, "vidinfol%s%d", device_counter > 9 ? "" : "_", device_counter); 
	++device_counter;

	_inputPins.clear();
	_outputPins.clear();

	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr))
    {
		if(hr == RPC_E_CHANGED_MODE)
		{
			ssi_wrn("Tried to reinitialize COM with different threading model! This might cause trouble! Code %ld", hr);
		}
		else
		{
			ssi_err ("Could not initialize COM library in constructor(). Failed with %ld", hr);
		}
    }
	else 
	{
		if(hr == S_FALSE)
		{
			ssi_msg (SSI_LOG_LEVEL_DETAIL, "COM was already initialized for this thread!  Code %ld", hr);
		}
		++_comInitCountConstructor;
	}

	if(pBaseFilter == NULL)
	{
		return;
	}

	extractVideoTypeInfosFromBaseFilter(pBaseFilter);
	
}

VideoTypeInfoList::~VideoTypeInfoList()
{
	_inputPins.clear();
	_outputPins.clear();

	delete[] ssi_log_name;
	ssi_log_name = NULL;

	if(_comInitCountConstructor > 0)
	{
		CoUninitialize();
		--_comInitCountConstructor;
	}
}

HRESULT VideoTypeInfoList::setBaseFilter(IBaseFilter *pBaseFilter)
{
	_inputPins.clear();
	_outputPins.clear();

	if(pBaseFilter == NULL)
	{
		return S_FALSE;
	}

	return extractVideoTypeInfosFromBaseFilter(pBaseFilter);

}

bool VideoTypeInfoList::isEmpty()
{
	if(_inputPins.empty() && _outputPins.empty())
	{
		return true;
	}

	if(_outputPins.empty() == false)
	{
		for(int i = 0; i < ssi_cast (int, _outputPins.size()); ++i)
		{
			if((_outputPins[i]).empty() == false)
			{
				return false;
			}
		}
	}

	if(_inputPins.empty() == false)
	{
		for(int i = 0; i < ssi_cast (int, _inputPins.size()); ++i)
		{
			if((_inputPins[i]).empty() == false)
			{
				return false;
			}
		}
	}

	return true;

}

int VideoTypeInfoList::howManyPinsPresent(PIN_DIRECTION pinDirection)
{
	int retVal = -1;

	switch(pinDirection)
	{
	case PINDIR_INPUT:
		retVal = ssi_cast (int, _inputPins.size());
		break;
	case PINDIR_OUTPUT:
		retVal = ssi_cast (int, _outputPins.size());
		break;
	default:
		break;
	}

	return retVal;
}

int VideoTypeInfoList::howManyMediaInfosPresentForSpecificPin(PIN_DIRECTION pinDirection, int indexOfDesiredPin)
{
	int retVal = -1;

	switch(pinDirection)
	{
	case PINDIR_INPUT:
		if(indexOfDesiredPin >= ssi_cast (int, _inputPins.size()) || indexOfDesiredPin < 0)
			break;
		retVal = ssi_cast (int, (_inputPins[indexOfDesiredPin]).size());
		break;
	case PINDIR_OUTPUT:
		if(indexOfDesiredPin >= ssi_cast (int, _outputPins.size()) || indexOfDesiredPin < 0)
			break;
		retVal = ssi_cast (int, (_outputPins[indexOfDesiredPin]).size());
		break;
	default:
		break;
	}

	return retVal;
}

VideoTypeInfo VideoTypeInfoList::getCopyOfVideoTypeInfo(PIN_DIRECTION pinDirection, int indexOfDesiredPin, int indexOfDesiredFormat)
{
	VideoTypeInfo retVal;

	switch(pinDirection)
	{
	case PINDIR_INPUT:
		if(indexOfDesiredPin >= ssi_cast (int, _inputPins.size()) || indexOfDesiredPin < 0)
			break;
		if(indexOfDesiredFormat >= ssi_cast (int, (_inputPins[indexOfDesiredPin]).size()) || indexOfDesiredFormat < 0)
			break;
		retVal = (_inputPins[indexOfDesiredPin])[indexOfDesiredFormat];
		break;
	case PINDIR_OUTPUT:
		if(indexOfDesiredPin >= ssi_cast (int, _outputPins.size()) || indexOfDesiredPin < 0)
			break;
		if(indexOfDesiredFormat >= ssi_cast (int, (_outputPins[indexOfDesiredPin]).size()) || indexOfDesiredFormat < 0)
			break;
		retVal = (_outputPins[indexOfDesiredPin])[indexOfDesiredFormat];
		break;
	default:
		break;
	}

	return retVal;
}

VideoTypeInfo& VideoTypeInfoList::getReferenceOfVideoTypeInfo(PIN_DIRECTION pinDirection, int indexOfDesiredPin, int indexOfDesiredFormat)
{
	VideoTypeInfo retVal;

	switch(pinDirection)
	{
	case PINDIR_INPUT:
		if(indexOfDesiredPin >= ssi_cast (int, _inputPins.size()) || indexOfDesiredPin < 0)
		{
			ssi_wrn("indexOfDesiredPin is out of range in getReferenceOfVideoTypeInfo!");
			throw std::out_of_range("indexOfDesiredPin is out of range in getReferenceOfVideoTypeInfo!");
		}
		if(indexOfDesiredFormat >= ssi_cast (int, (_inputPins[indexOfDesiredPin]).size()) || indexOfDesiredFormat < 0)
		{
			ssi_wrn("indexOfDesiredPin is out of range in getReferenceOfVideoTypeInfo!");
			throw std::out_of_range("indexOfDesiredFormat is out of range in getReferenceOfVideoTypeInfo!");
		}
		return (_inputPins[indexOfDesiredPin])[indexOfDesiredFormat];
		break;
	case PINDIR_OUTPUT:
		if(indexOfDesiredPin >= ssi_cast (int, _outputPins.size()) || indexOfDesiredPin < 0)
		{
			ssi_wrn("indexOfDesiredPin is out of range in getReferenceOfVideoTypeInfo!");
			throw std::out_of_range("indexOfDesiredPin is out of range in getReferenceOfVideoTypeInfo!");
		}
		if(indexOfDesiredFormat >= ssi_cast (int, (_outputPins[indexOfDesiredPin]).size()) || indexOfDesiredFormat < 0)
		{
			ssi_wrn("indexOfDesiredPin is out of range in getReferenceOfVideoTypeInfo!");
			throw std::out_of_range("indexOfDesiredFormat is out of range in getReferenceOfVideoTypeInfo!");
		}
		return (_outputPins[indexOfDesiredPin])[indexOfDesiredFormat];
		break;
	default:
			ssi_wrn("PIN_DIRECTION unknown in getReferenceOfVideoTypeInfo!");
			throw std::out_of_range("PIN_DIRECTION unknown in getReferenceOfVideoTypeInfo!");
		break;
	}

	return retVal;
}

bool VideoTypeInfoList::isPresentInList(const VideoTypeInfo &videoTypeInfo, PIN_DIRECTION pinDirection, int &indexOfDesiredANDORfoundPin, int &indexOfFoundFilterInCertainPin)
{
	switch(pinDirection)
	{
	case PINDIR_INPUT:
		if(indexOfDesiredANDORfoundPin >= ssi_cast (int, _inputPins.size()))
		{
			//indexOfDesiredPin = -1;
			//indexOfFoundFilterInCertainPin = -1;
			ssi_wrn("indexOfDesiredPin is out of range in isPresentInList!");
			throw std::out_of_range("indexOfDesiredPin is out of range in isPresentInList!");
			return false;
		}
		if(indexOfDesiredANDORfoundPin < 0)
		{
			for(int i = 0; i < ssi_cast (int, _inputPins.size()); ++i)
			{
				for(int j = 0; j < ssi_cast (int, _inputPins[i].size()); ++j)
				{
					if(_inputPins[i][j] == videoTypeInfo)
					{
						indexOfDesiredANDORfoundPin = i;
						indexOfFoundFilterInCertainPin = j;
						return true;
					}
				}
			}
		}
		else
		{
			int i = indexOfDesiredANDORfoundPin;
			for(int j = 0; j < ssi_cast (int, _inputPins[i].size()); ++j)
			{
				if(_inputPins[i][j] == videoTypeInfo)
				{
					indexOfFoundFilterInCertainPin = j;
					return true;
				}
			}
		}
		break;
	case PINDIR_OUTPUT:
		if(indexOfDesiredANDORfoundPin >= ssi_cast (int, _outputPins.size()))
		{
			ssi_wrn("indexOfDesiredPin is out of range in isPresentInList!");
			throw std::out_of_range("indexOfDesiredPin is out of range in isPresentInList!");
			return false;
		}
		if(indexOfDesiredANDORfoundPin < 0)
		{
			for(int i = 0; i < ssi_cast (int, _outputPins.size()); ++i)
			{
				for(int j = 0; j < ssi_cast (int, _outputPins[i].size()); ++j)
				{
					if(_outputPins[i][j] == videoTypeInfo)
					{
						indexOfDesiredANDORfoundPin = i;
						indexOfFoundFilterInCertainPin = j;
						return true;
					}
				}
			}
		}
		else
		{
			int i = indexOfDesiredANDORfoundPin;
			for(int j = 0; j < ssi_cast (int, _outputPins[i].size()); ++j)
			{
				if(_outputPins[i][j] == videoTypeInfo)
				{
					indexOfFoundFilterInCertainPin = j;
					return true;
				}
			}
		}
	default:
		ssi_wrn("PIN_DIRECTION unknown in getReferenceOfVideoTypeInfo!");
		throw std::out_of_range("PIN_DIRECTION unknown in getReferenceOfVideoTypeInfo!");
		break;
	}

	//indexOfFoundFilterInCertainPin = -1;
	return false;
}

int VideoTypeInfoList::LetUserSelectMediaType(VideoTypeInfoList *availableMediaTypesToSelectFrom, int &indexOfSelectedOutputPin, int &indexOfSelectedMediaType, bool fallBackToConsole)
{
	if(availableMediaTypesToSelectFrom == NULL)
	{
		ssi_err_static("In int VideoTypeInfoList::LetUserSelectMediaType -> availableMediaTypesToSelectFrom == NULL not allowed");
		return -1;
	}

	DialogLibGateway dialogGateway;

	if(!dialogGateway.didInitWork())
	{
		if(fallBackToConsole)
		{
			//TODO
			//return LetUserSelectDesiredCamOnConsole(availableCamsToSelectFrom);
		}
		ssi_err_static("In int VideoTypeInfoList::LetUserSelectMediaType -> DialogLibGateWay initialisation failed and no fallback available");
	}

	if(!dialogGateway.SetNewDialogType("PinAndMediaSelectionDialog"))
	{
		if(fallBackToConsole)
		{
			//TODO
			//return LetUserSelectDesiredCamOnConsole(availableCamsToSelectFrom);
		}
		ssi_err_static("In int VideoTypeInfoList::LetUserSelectMediaType -> -> Could not set PinAndMediaSelectionDialog and no fallback available");
	}

	//int intHandle = dialogGateway.AlterExistingItem("Caption", -1, "Select a video camera");

	//std::vector<CameraDeviceName>::const_iterator elemIterator;
	/*for(elemIterator = availableCamsToSelectFrom->_listOfCameraDevices.begin(); elemIterator != availableCamsToSelectFrom->_listOfCameraDevices.end(); ++elemIterator)
	{
		intHandle = dialogGateway.AppendItem("FriendlyName", elemIterator->getFriendlyName());
		if(intHandle < 0)
		{
			if(fallBackToConsole)
			{
				return LetUserSelectDesiredCamOnConsole(availableCamsToSelectFrom);
			}

			ssi_err_static("In int CameraList::LetUserSelectDesiredCam -> Could not set FriendlyName and no fallback available");
			return -1;
		}
		if(dialogGateway.AlterExistingItem("Description", intHandle, elemIterator->getDescription()) != intHandle)
		{
			if(fallBackToConsole)
			{
				return LetUserSelectDesiredCamOnConsole(availableCamsToSelectFrom);
			}

			ssi_err_static("In int CameraList::LetUserSelectDesiredCam -> Could not set Description and no fallback available");
			return -1;
		}
		if(dialogGateway.AlterExistingItem("DevicePath", intHandle, elemIterator->getDevicePath()) != intHandle)
		{
			if(fallBackToConsole)
			{
				return LetUserSelectDesiredCamOnConsole(availableCamsToSelectFrom);
			}

			ssi_err_static("In int CameraList::LetUserSelectDesiredCam -> Could not set DevicePath and no fallback available");
			return -1;
		}
	}

	dialogGateway.AlterExistingItem("Help", -1, "No further help available");

	 * */
	int retVal = dialogGateway.RunDialog();

	dialogGateway.AlterExistingItem("IntRetrieval", -1, "Resolution X");

	return (retVal - 1);
}

HRESULT VideoTypeInfoList::extractVideoTypeInfosFromBaseFilter(IBaseFilter *pBaseFilter)
{
	if(pBaseFilter == NULL)
	{
		return S_FALSE;
	}

	IEnumPins		*pEnumPins = NULL;
	IPin			*pPin = NULL;
	IAMStreamConfig	*pConfig = NULL;
	IKsPropertySet	*pKsPropertySet = NULL;
	GUID			pinCategory = GUID_NULL;

	HRESULT hr = pBaseFilter->EnumPins(&pEnumPins);
	if(FAILED(hr))
	{
		ssi_err ("Could enumerate pins in extractVideoTypeInfosFromBaseFilter(). Failed with %ld", hr);
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
			ssi_wrn ("The pin does not support IKsPropertySet in extractVideoTypeInfosFromBaseFilter. Failed with %ld", hr);
			continue;
		}

		DWORD cbReturned;
		hr = pKsPropertySet->Get(AMPROPSETID_Pin, AMPROPERTY_PIN_CATEGORY, NULL, 0, &pinCategory, sizeof(GUID), &cbReturned);
		if(FAILED(hr))
		{
			SafeReleaseFJ(pKsPropertySet);
			SafeReleaseFJ(pPin);
			ssi_wrn ("The pin does not support IKsPropertySet AMPROPERTY_PIN_CATEGORY in extractVideoTypeInfosFromBaseFilter. Failed with %ld", hr);
			continue;
		}
		if(IsEqualGUID(pinCategory, PIN_CATEGORY_CAPTURE) == FALSE)
		{
			SafeReleaseFJ(pKsPropertySet);
			SafeReleaseFJ(pPin);
			SSI_DBG (SSI_LOG_LEVEL_DEBUG, "Was not a capture pin");
			continue;
		}

		hr = pPin->QueryInterface(IID_IAMStreamConfig, (void**)&pConfig);
		if(FAILED(hr))
		{
			SafeReleaseFJ(pKsPropertySet);
			SafeReleaseFJ(pPin);
			ssi_err ("The pin does not support IAMStreamConfig in extractVideoTypeInfosFromBaseFilter. Failed with %ld", hr);
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
			ssi_err ("Failed to get number of capabilities in extractVideoTypeInfosFromBaseFilter. Failed with %ld", hr);
			continue;
		}

		if(iSize == sizeof(VIDEO_STREAM_CONFIG_CAPS))
		{
			_outputPins.push_back(std::vector<VideoTypeInfo>());
			int newElId = ssi_cast (int, _outputPins.size());
			--newElId;

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
					ssi_err ("Failed retrieving StreamCaps in extractVideoTypeInfosFromBaseFilter. Failed with %ld", hr);
					continue;
				}
				//if(pmtConfig->majortype == MEDIATYPE_Video)
				//{
					VideoTypeInfo vTypeInfo(pmtConfig, &streamConfigCaps);
					(_outputPins[newElId]).push_back(vTypeInfo);
				//}
				DeleteMediaType(pmtConfig);
			}
		}

	
		pinCategory = GUID_NULL;
		SafeReleaseFJ(pConfig);
		SafeReleaseFJ(pKsPropertySet);
        SafeReleaseFJ(pPin);
    }

    SafeReleaseFJ(pEnumPins);

	return S_OK;
}

}
