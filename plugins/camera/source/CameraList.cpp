// CameraList.cpp
// author: Frank Jung <frank.jung@informatik.uni-augsburg.de>
// created: 2009/04/07
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

#include "CameraList.h"
#include "graphic/DialogLibGateway.h"

#ifndef _WIN32_DCOM
#define _WIN32_DCOM 
#endif

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

#ifndef SafeReleaseFJ
#define SafeReleaseFJ(p) { if( (p) != 0 ) { (p)->Release(); (p)= 0; } }
#endif

namespace ssi
{

int CameraDeviceName::ssi_log_level_static = SSI_LOG_LEVEL_DEFAULT;

CameraDeviceName::CameraDeviceName() :
	_friendlyName(NULL),
	_description(NULL),
	_devicePath(NULL)
{
	ssi_log_name = new char[SSI_MAX_CHAR];
	sprintf (ssi_log_name, "camdevicename"); 
}

CameraDeviceName::CameraDeviceName(const CameraDeviceName &rhs) 
	
{
	ssi_log_name = new char[SSI_MAX_CHAR];
	sprintf (ssi_log_name, "camdevicename"); 

	_friendlyName = NULL;
	_description = NULL;
	_devicePath = NULL;

	if(rhs._friendlyName != NULL)
	{
		_friendlyName = new char[SSI_CAMERADEVICE_CHARSTRINGLENGTH];
		if(strcpy_s(_friendlyName, SSI_CAMERADEVICE_CHARSTRINGLENGTH, rhs._friendlyName) != 0)
		{
			ssi_err("Error in CameraDeviceName(const CameraDeviceName&) Constructor with friendlyName copying");
		}
	}
	if(rhs._description != NULL)
	{
		_description = new char[SSI_CAMERADEVICE_CHARSTRINGLENGTH];
		if(strcpy_s(_description, SSI_CAMERADEVICE_CHARSTRINGLENGTH, rhs._description) != 0)
		{
			ssi_err("Error in CameraDeviceName(const CameraDeviceName&) Constructor with description copying");
		}
	}
	if(rhs._devicePath != NULL)
	{
		_devicePath = new char[SSI_CAMERADEVICE_CHARSTRINGLENGTH];
		if(strcpy_s(_devicePath, SSI_CAMERADEVICE_CHARSTRINGLENGTH, rhs._devicePath) != 0)
		{
			ssi_err("Error in CameraDeviceName(const CameraDeviceName&) Constructor with devicePath copying");
		}
	}
}

CameraDeviceName::CameraDeviceName(char *friendlyName, char *description, char *devicePath)
{
	ssi_log_name = new char[SSI_MAX_CHAR];
	sprintf (ssi_log_name, "camdevicename"); 

	_friendlyName = NULL;
	_description = NULL;
	_devicePath = NULL;

	if(friendlyName != NULL)
	{
		_friendlyName = new char[SSI_CAMERADEVICE_CHARSTRINGLENGTH];
		if(strcpy_s(_friendlyName, SSI_CAMERADEVICE_CHARSTRINGLENGTH, friendlyName) != 0)
		{
			ssi_err("Error in CameraDeviceName(char*, char*, char*) Constructor with friendlyName copying");
		}
	}
	if(description != NULL)
	{
		_description = new char[SSI_CAMERADEVICE_CHARSTRINGLENGTH];
		if(strcpy_s(_description, SSI_CAMERADEVICE_CHARSTRINGLENGTH, description) != 0)
		{
			ssi_err("Error in CameraDeviceName(char*, char*, char*) Constructor with description copying");
		}
	}
	if(devicePath != NULL)
	{
		_devicePath = new char[SSI_CAMERADEVICE_CHARSTRINGLENGTH];
		if(strcpy_s(_devicePath, SSI_CAMERADEVICE_CHARSTRINGLENGTH, devicePath) != 0)
		{
			ssi_err("Error in CameraDeviceName(char*, char*, char*) Constructor with devicePath copying");
		}
	}
}

CameraDeviceName::CameraDeviceName(BSTR *friendlyName, BSTR *description, BSTR *devicePath)
{
	ssi_log_name = new char[SSI_MAX_CHAR];
	sprintf (ssi_log_name, "camdevicename"); 

	_friendlyName = NULL;
	_description = NULL;
	_devicePath = NULL;

	if(friendlyName != NULL)
	{
		_friendlyName = _com_util::ConvertBSTRToString(*friendlyName);
	}
	if(description != NULL)
	{
		_description = _com_util::ConvertBSTRToString(*description);
	}
	if(devicePath != NULL)
	{
		_devicePath = _com_util::ConvertBSTRToString(*devicePath);
	}
}

const char* CameraDeviceName::getFriendlyName() const
{
	return _friendlyName;
}

const char* CameraDeviceName::getDescription() const
{
	return _description;
}

const char* CameraDeviceName::getDevicePath() const
{
	return _devicePath;
}

bool CameraDeviceName::setFriendlyName(char *newFriendlyName)
{
	if(newFriendlyName == NULL)
	{
		return false;
	}
	delete[] _friendlyName;
	_friendlyName = new char[SSI_CAMERADEVICE_CHARSTRINGLENGTH];
	if(strcpy_s(_friendlyName, SSI_CAMERADEVICE_CHARSTRINGLENGTH, newFriendlyName) != 0)
	{
		delete[] _friendlyName;
		return false;
	}
	return true;
}

void CameraDeviceName::setFriendlyName(BSTR newFriendlyName)
{
	delete[] _friendlyName;
	_friendlyName = _com_util::ConvertBSTRToString(newFriendlyName);
}

bool CameraDeviceName::setDescription(char *newDescription)
{
	if(newDescription == NULL)
	{
		return false;
	}
	delete[] _description;
	_description = new char[SSI_CAMERADEVICE_CHARSTRINGLENGTH];
	if(strcpy_s(_description, SSI_CAMERADEVICE_CHARSTRINGLENGTH, newDescription) != 0)
	{
		delete[] _description;
		return false;
	}
	return true;
}

void CameraDeviceName::setDescription(BSTR newDescription)
{
	delete[] _description;
	_description = _com_util::ConvertBSTRToString(newDescription);
}

bool CameraDeviceName::setDevicePath(char *newDevicePath)
{
	if(newDevicePath == NULL)
	{
		return false;
	}
	delete[] _devicePath;
	_devicePath = new char[SSI_CAMERADEVICE_CHARSTRINGLENGTH];
	if(strcpy_s(_devicePath, SSI_CAMERADEVICE_CHARSTRINGLENGTH, newDevicePath) != 0)
	{
		delete[] _devicePath;
		return false;
	}
	return true;
}

void CameraDeviceName::setDevicePath(BSTR newDevicePath)
{
	delete[] _devicePath;
	_devicePath = _com_util::ConvertBSTRToString(newDevicePath);
}

bool CameraDeviceName::operator ==(const CameraDeviceName &rhs)
{
	const char *rhsDevPath = rhs._devicePath;
	const char *rhsFrName = rhs._friendlyName;
	const char *rhsDesc = rhs._description;

	if(rhsDevPath != NULL && this->_devicePath != NULL)
	{
		if(strcmp(rhsDevPath, this->_devicePath) == 0)
		{
			return true;
		}
		return false;
	}
	else if(rhsDevPath != NULL || this->_devicePath != NULL)
	{
		return false;
	}

	if(rhsFrName != NULL && this->_friendlyName != NULL)
	{
		if(strcmp(rhsFrName, this->_friendlyName) == 0)
		{
			return true;
		}
		return false;
	}
	else if(rhsFrName != NULL || this->_friendlyName != NULL)
	{
		return false;
	}

	if(rhsDesc != NULL && this->_description != NULL)
	{
		if(strcmp(rhsDesc, this->_description) == 0)
		{
			return true;
		}
		return false;
	}
	else if(rhsDesc != NULL || this->_description != NULL)
	{
		return false;
	}
	return true;
}

bool CameraDeviceName::operator ==(const char *rhs)
{
	if(rhs == NULL)
	{
		if(this->_friendlyName == NULL && this->_description == NULL && this->_devicePath == NULL)
		{
			return true;
		}
		return false;
	}
	if(this->_devicePath != NULL)
	{
		if(strcmp(this->_devicePath, rhs) == 0)
		{
			return true;
		}
	}
	if(this->_friendlyName != NULL)
	{
		if(strcmp(this->_friendlyName, rhs) == 0)
		{
			return true;
		}
	}
	if(this->_description != NULL)
	{
		if(strcmp(this->_description, rhs) == 0)
		{
			return true;
		}
	}
	return false;
}

CameraDeviceName& CameraDeviceName::operator =(const CameraDeviceName &rhs)
{
	if(this == &rhs)
		return *this;

	delete[] _friendlyName;
	_friendlyName = NULL;
	if(rhs._friendlyName != NULL)
	{
		_friendlyName = new char[SSI_CAMERADEVICE_CHARSTRINGLENGTH];
		if(strcpy_s(_friendlyName, SSI_CAMERADEVICE_CHARSTRINGLENGTH, rhs._friendlyName) != 0)
		{
			ssi_err("Error in CameraDeviceName(const CameraDeviceName&) operator= with friendlyName copying");
		}
	}
	delete[] _description;
	_description = NULL;
	if(rhs._description != NULL)
	{
		_description = new char[SSI_CAMERADEVICE_CHARSTRINGLENGTH];
		if(strcpy_s(_description, SSI_CAMERADEVICE_CHARSTRINGLENGTH, rhs._description) != 0)
		{
			ssi_err("Error in CameraDeviceName(const CameraDeviceName&) operator= with description copying");
		}
	}
	delete[] _devicePath;
	_devicePath = NULL;
	if(rhs._devicePath != NULL)
	{
		_devicePath = new char[SSI_CAMERADEVICE_CHARSTRINGLENGTH];
		if(strcpy_s(_devicePath, SSI_CAMERADEVICE_CHARSTRINGLENGTH, rhs._devicePath) != 0)
		{
			ssi_err("Error in CameraDeviceName(const CameraDeviceName&) operator= with devicePath copying");
		}
	}
	return *this;
}

CameraDeviceName::~CameraDeviceName()
{
	delete[] ssi_log_name;
	delete[] _friendlyName;
	delete[] _description;
	delete[] _devicePath;
}

//****************************************************************************************

int CameraList::ssi_log_level_static = SSI_LOG_LEVEL_DEFAULT;
static char ssi_log_name_static[] = "camlist__s";

void CameraList::setLogLevel (int level)
{
	ssi_log_level = level;
}

void CameraList::SetLogLevelStatic (int level)
{
	ssi_log_level_static = level;
}

CameraList::CameraList()
{
	_listOfCameraDevices.clear();
	_listOfCameraDevicesIterator = _listOfCameraDevices.begin();

	static int device_counter = 1;
	ssi_log_name = new char[SSI_MAX_CHAR];
	sprintf (ssi_log_name, "camlist__%s%d", device_counter > 9 ? "" : "_", device_counter); 
	++device_counter;
}

CameraList::~CameraList()
{
	_listOfCameraDevices.clear();
	_listOfCameraDevicesIterator = _listOfCameraDevices.begin();
	delete[] ssi_log_name;
}

int CameraList::isThisElementAlreadyInList(const CameraDeviceName &elementToTest)
{
	int retVal = -1;

	if(_listOfCameraDevices.empty())
	{
		return retVal;
	}

	std::vector<CameraDeviceName>::iterator elemIterator;
	for(elemIterator = _listOfCameraDevices.begin(); elemIterator != _listOfCameraDevices.end(); ++elemIterator)
	{
		++retVal;
		if((*elemIterator) == elementToTest)
		{
			return retVal;
		}
	}

	return -1;
}

CameraList& CameraList::operator +=(const CameraDeviceName &rhs)
{
	if(this->isThisElementAlreadyInList(rhs) == -1)
	{
		_listOfCameraDevices.push_back(rhs);
	}

	return *this;
}

CameraList& CameraList::operator -=(const CameraDeviceName &rhs)
{
	if(_listOfCameraDevices.empty())
	{
		return *this;
	}

	bool elemErased = false;
	std::vector<CameraDeviceName>::iterator elemIterator;
	for(elemIterator = _listOfCameraDevices.begin(); elemIterator != _listOfCameraDevices.end(); elemErased ? elemIterator : ++elemIterator)
	{
		elemErased = false;
		if((*elemIterator) == rhs)
		{
			elemIterator = _listOfCameraDevices.erase(elemIterator);
			elemErased = true;
			if(_listOfCameraDevices.empty())
			{
				break;
			}
		}
	}

	return *this;
}

CameraList& CameraList::operator +=(const CameraList &rhs)
{
	if(rhs._listOfCameraDevices.empty())
	{
		return *this;
	}

	std::vector<CameraDeviceName>::const_iterator elemIterator;
	for(elemIterator = rhs._listOfCameraDevices.begin(); elemIterator != rhs._listOfCameraDevices.end(); ++elemIterator)
	{
		(*this) += (*elemIterator);
	}

	return *this;
}

const CameraList CameraList::operator +(const CameraList &rhs)
{
	CameraList result = *this;
	result += rhs;
	return result;
}

CameraDeviceName CameraList::getCopyOfCameraDeviceName(int index)
{
	CameraDeviceName retVal;

	if(_listOfCameraDevices.empty() || index < 0 || index >= ssi_cast (int, _listOfCameraDevices.size()))
		return retVal;

	retVal = _listOfCameraDevices[index];

	return retVal;
}

CameraDeviceName& CameraList::getReferenceToCameraDeviceName(int index)
{
	static CameraDeviceName retVal;

	if(_listOfCameraDevices.empty() || index < 0 || index >= ssi_cast (int, _listOfCameraDevices.size()))
	{
		ssi_wrn("index out of range in getReferenceToCameraDeviceName!");
		throw std::out_of_range("index out of range in getReferenceToCameraDeviceName!");
	}

	return _listOfCameraDevices[index];
}

ssi_size_t CameraList::getListSize()
{
	return ssi_cast (ssi_size_t, _listOfCameraDevices.size());
}

CameraList *CameraList::LoadCameraDeviceNameList(char *fileName)
{
	ssi_err_static("Not Implemented yet: CameraList* CameraList::LoadCameraDeviceNameList(char *fileName)");
	return NULL;
}

bool CameraList::SaveCameraDeviceNameList(CameraList *camDeviceNameListToSave, char *fileName)
{
	ssi_err_static("Not Implemented yet: bool CameraList::SaveCameraDeviceNameList(CameraList *camDeviceNameListToSave, char *fileName)");

	/*std::ofstream configFile;
	configFile.open(fileName, std::ios::out | std::ios::trunc);
	if(!configFile.is_open())
	{
		return false;
	}

	configFile << camDeviceNameListToSave->_listOfCameraDevices.size() << endl;

	std::vector<CameraDeviceName>::const_iterator elemIterator;
	for(elemIterator = camDeviceNameListToSave->_listOfCameraDevices.begin(); elemIterator != camDeviceNameListToSave->_listOfCameraDevices.end(); ++elemIterator)
	{
		
	}*/


	return false;
}

CameraList *CameraList::FindAllCamsInSystem(CameraList *optionalListToThatShouldBeAdded)
{
	CameraList *retList;
	if( optionalListToThatShouldBeAdded != NULL )
	{
		retList = optionalListToThatShouldBeAdded;
	}
	else
	{
		retList = new CameraList();
	}

	int comInitCount = 0;
	SSI_DBG_STATIC(SSI_LOG_LEVEL_DEBUG, "In CameraList *CameraList::FindAllCamsInSystem(CameraList *optionalListToThatShouldBeAdded) try to CoInitializeEx with COINIT_MULTITHREADED...");
	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if(SUCCEEDED(hr))
	{
		SSI_DBG_STATIC(SSI_LOG_LEVEL_VERBOSE, "\t...SUCCEEDED!");
		++comInitCount;
	}
	else if(hr == RPC_E_CHANGED_MODE)
	{
		ssi_wrn_static("In CameraList *CameraList::FindAllCamsInSystem(CameraList *optionalListToThatShouldBeAdded) try to CoInitializeEx with COINIT_MULTITHREADED failed with RPC_E_CHANGED_MODE. This might cause trouble!");
	}
	else
	{
		ssi_err_static("In CameraList *CameraList::FindAllCamsInSystem(CameraList *optionalListToThatShouldBeAdded) try to CoInitializeEx with COINIT_MULTITHREADED failed with %ld", hr);
		return retList;
	}

	ICreateDevEnum *pSysDevEnum = NULL;
	IEnumMoniker *pEnumMoniker = NULL;
	IMoniker *pMoniker = NULL;
	IPropertyBag *pPropertyBag = NULL;

	SSI_DBG_STATIC(SSI_LOG_LEVEL_DEBUG, "In CameraList *CameraList::FindAllCamsInSystem(CameraList *optionalListToThatShouldBeAdded) try to CoCreateInstance with CLSID_SystemDeviceEnum...");
	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, reinterpret_cast<void**>(&pSysDevEnum));
	if(SUCCEEDED(hr))
	{
		SSI_DBG_STATIC(SSI_LOG_LEVEL_VERBOSE, "\t...SUCCEEDED!");
	}
	else
	{
		ssi_err_static("In CameraList *CameraList::FindAllCamsInSystem(CameraList *optionalListToThatShouldBeAdded) try to CoCreateInstance with CLSID_SystemDeviceEnum failed with %ld", hr);
		if(comInitCount > 0)
		{
			CoUninitialize();
			--comInitCount;
		}
		return retList;
	}
	
	hr = pSysDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumMoniker, 0);
	if(hr != S_OK)
	{
		if(hr == S_FALSE)
		{
			SafeReleaseFJ(pSysDevEnum);
			if(comInitCount > 0)
			{
				CoUninitialize();
				--comInitCount;
			}
			ssi_msg_static(SSI_LOG_LEVEL_DETAIL, "In CameraList *CameraList::FindAllCamsInSystem(CameraList *optionalListToThatShouldBeAdded) try to CreateClassEnumerator with CLSID_VideoInputDeviceCategory failed because Category is Empty or does not exist");
			return retList;
		}
		SafeReleaseFJ(pSysDevEnum);
		ssi_err_static("In CameraList *CameraList::FindAllCamsInSystem(CameraList *optionalListToThatShouldBeAdded) try to CreateClassEnumerator with CLSID_VideoInputDeviceCategory failed with %ld", hr);
		if(comInitCount > 0)
		{
			CoUninitialize();
			--comInitCount;
		}
		return retList;
	}

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
		if(SUCCEEDED(hr))
		{
			pDevicePath = _com_util::ConvertBSTRToString(varDevicePath.bstrVal);
			VariantClear(&varDevicePath);
		}

		//if(pFriendlyName != NULL && pDescription != NULL && pDevicePath != NULL)
		{
			CameraDeviceName curCamDevice(pFriendlyName, pDescription, pDevicePath);
			(*retList) += curCamDevice;

			delete[] pFriendlyName;
			pFriendlyName = NULL;
			delete[] pDescription;
			pDescription = NULL;
			delete[] pDevicePath;
			pDevicePath = NULL;

			SafeReleaseFJ(pPropertyBag);
			SafeReleaseFJ(pMoniker);
		}

	}

	SafeReleaseFJ(pEnumMoniker);
	SafeReleaseFJ(pSysDevEnum);
	if(comInitCount > 0)
	{
		CoUninitialize();
		--comInitCount;
	}
	return retList;
}

int CameraList::LetUserSelectDesiredCam(CameraList *availableCamsToSelectFrom, bool fallBackToConsole)
{
	if(availableCamsToSelectFrom == NULL)
	{
		ssi_err_static("In int CameraList::LetUserSelectDesiredCam -> availableCamsToSelectFrom == NULL not allowed");
		return -1;
	}

	DialogLibGateway dialogGateway;

	if(!dialogGateway.didInitWork())
	{
		if(fallBackToConsole)
		{
			return LetUserSelectDesiredCamOnConsole(availableCamsToSelectFrom);
		}
		ssi_err_static("In int CameraList::LetUserSelectDesiredCam -> DialogLibGateWay initialisation failed and no fallback available");
	}

	if(!dialogGateway.SetNewDialogType("CamSelectionDialog"))
	{
		if(fallBackToConsole)
		{
			return LetUserSelectDesiredCamOnConsole(availableCamsToSelectFrom);
		}
		ssi_err_static("In int CameraList::LetUserSelectDesiredCam -> Could not set CamSelectionDialog and no fallback available");
	}

	int intHandle = dialogGateway.AlterExistingItem("Caption", -1, "Select a video camera");

	std::vector<CameraDeviceName>::const_iterator elemIterator;
	for(elemIterator = availableCamsToSelectFrom->_listOfCameraDevices.begin(); elemIterator != availableCamsToSelectFrom->_listOfCameraDevices.end(); ++elemIterator)
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

	int retVal = dialogGateway.RunDialog();

	return (retVal - 1);
}

int CameraList::LetUserSelectDesiredCamOnConsole(CameraList *availableCamsToSelectFrom)
{
	if(availableCamsToSelectFrom == NULL)
	{
		ssi_err_static("In int CameraList::LetUserSelectDesiredCamOnConsole -> availableCamsToSelectFrom == NULL not allowed");
		return -1;
	}

	ssi_print("Select a video camera:\n");
	ssi_print("Camera 0:\n");
	ssi_print("\tFriendly Name:\n");
	ssi_print("\t\tNO CAMERA\n");
	ssi_print("\tDescription:\n");
	ssi_print("\t\tNO CAMERA\n");
	ssi_print("\tDevice Path:\n");
	ssi_print("\t\tNO CAMERA\n");

	std::vector<CameraDeviceName>::const_iterator elemIterator;
	int numOfCams = 0;
	for(elemIterator = availableCamsToSelectFrom->_listOfCameraDevices.begin(); elemIterator != availableCamsToSelectFrom->_listOfCameraDevices.end(); ++elemIterator)
	{
		++numOfCams;
		ssi_print("Camera %d:", numOfCams);
		ssi_print("\tFriendly Name:\n");
		if(elemIterator->getFriendlyName()) {
			ssi_print("\t\t%s\n", elemIterator->getFriendlyName());
		} else {
			ssi_print("\n");
		}
		ssi_print("\tDescription:\n");
		if(elemIterator->getDescription()) {
			ssi_print("\t\t%s\n", elemIterator->getDescription());
		} else {
			ssi_print("\n");
		}
		ssi_print("\tDevice Path:\n");
		if(elemIterator->getDevicePath()) {
			ssi_print("\t\t%s\n", elemIterator->getDevicePath());
		} else {
			ssi_print("\n");
		}
	}

	int selection = -1;
	ssi_print("Your selection: ");
	scanf("%d", &selection);
	if(selection == EOF || selection < 0 || selection > numOfCams)
		return -1;
	
	return (selection - 1);
}

}
