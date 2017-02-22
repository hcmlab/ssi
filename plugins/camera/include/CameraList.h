// CameraList.h
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

#pragma once

#ifndef SSI_SENSOR_CAMLIST_H
#define	SSI_SENSOR_CAMLIST_H

#include <comutil.h>
#include <string.h>
#include <vector>
#include "SSI_Cons.h"
#include "CameraCons.h"
#include <DShow.h>
#include <iostream>
#include <fstream>

#ifndef SSI_CAMERADEVICE_CHARSTRINGLENGTH
#define SSI_CAMERADEVICE_CHARSTRINGLENGTH 1024
#endif

namespace ssi
{
class CameraDeviceName
{
public:
	CameraDeviceName();
	CameraDeviceName(const CameraDeviceName &rhs);
	CameraDeviceName(char *friendlyName, char *description, char *devicePath);
	CameraDeviceName(BSTR *friendlyName, BSTR *description, BSTR *devicePath);
	~CameraDeviceName();

	const char* getFriendlyName() const;
	const char* getDescription() const ;
	const char* getDevicePath() const;

	bool setFriendlyName(char *newFriendlyName);
	bool setDescription(char *newDescription);
	bool setDevicePath(char *newDevicePath);

	void setFriendlyName(BSTR newFriendlyName);
	void setDescription(BSTR newDescription);
	void setDevicePath(BSTR newDevicePath);

	bool operator==(const CameraDeviceName &rhs);
	bool operator==(const char *rhs);
	CameraDeviceName& operator=(const CameraDeviceName &rhs);

protected:
	char	*_friendlyName;
	char	*_description;
	char	*_devicePath;

	char *ssi_log_name;
	int ssi_log_level;
	static int ssi_log_level_static;
};

class CameraList
{
public:
	CameraList();
	~CameraList();

	CameraDeviceName getCopyOfCameraDeviceName(int index);
	CameraDeviceName& getReferenceToCameraDeviceName(int index) throw(std::out_of_range);
	ssi_size_t getListSize();

	CameraList& operator+=(const CameraList &rhs);
	const CameraList operator+(const CameraList &rhs);
	CameraList& operator+=(const CameraDeviceName &rhs);
	CameraList& operator-=(const CameraDeviceName &rhs);

	static CameraList* LoadCameraDeviceNameList(char *fileName);
	static bool SaveCameraDeviceNameList(CameraList *camDeviceNameListToSave, char *fileName);
	static CameraList* FindAllCamsInSystem(CameraList *optionalListToThatShouldBeAdded);
	static int LetUserSelectDesiredCam(CameraList *availableCamsToSelectFrom, bool fallBackToConsole = true);
	static int LetUserSelectDesiredCamOnConsole(CameraList *availableCamsToSelectFrom);

	void setLogLevel (int level);
	static void SetLogLevelStatic (int level);

protected:

	int isThisElementAlreadyInList(const CameraDeviceName &elementToTest);

	std::vector<CameraDeviceName>				_listOfCameraDevices;
	std::vector<CameraDeviceName>::iterator	_listOfCameraDevicesIterator;

	char *ssi_log_name;
	int ssi_log_level;
	static int ssi_log_level_static;

};
}

#endif
