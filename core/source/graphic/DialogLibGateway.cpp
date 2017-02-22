// DialogLibGateway.cpp
// author: Frank Jung <frank.jung@informatik.uni-augsburg.de>
// created: 2009/04/21
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute itand/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#include "SSI_Define.h"

#ifndef SSI_USE_SDL

#include "graphic/DialogLibGateway.h"
#include <float.h>
#include <cstring>
#ifdef __MINGW32__
#include <limits.h>
#endif // __MINGW32__

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi
{

int DialogLibGateway::ssi_log_level_static = SSI_LOG_LEVEL_DEFAULT;
char DialogLibGateway::ssi_log_name_static[] = "dialoggtws";

void DialogLibGateway::setLogLevel(int level)
{
	ssi_log_level = level;
}

void DialogLibGateway::SetLogLevelStatic(int level)
{
	ssi_log_level_static = level;
}

DialogLibGateway::DialogLibGateway()
{
	ssi_log_name = new char[11];
	ssi_sprint (ssi_log_name, "dialoggtw_");

	_dialogHandle = -1;
	_initValid = false;

	hDLL = NULL;
	getNewHandle = NULL;
	runSpecificDialog = NULL;
	removeDialog = NULL;
	addItem = NULL;
	removeItem = NULL;
	retrieveString = NULL;
	retrieveDouble = NULL;
	retrieveFloat = NULL;
	retrieveInt = NULL;

#ifdef _DEBUG
	hDLL = LoadLibrary("ssidialogd.dll");
#else
	hDLL = LoadLibrary("ssidialog.dll");
#endif

	if(hDLL)
	{
		getNewHandle = (GetNewDialogHandle_ptr)GetProcAddress(hDLL, "GetNewDialogHandle");
		runSpecificDialog = (RunSpecificDialog_ptr)GetProcAddress(hDLL, "RunSpecificDialog");
		removeDialog = (RemoveDialog_ptr)GetProcAddress(hDLL, "RemoveDialog");
		addItem = (AddItem_ptr)GetProcAddress(hDLL, "AddItem");
		removeItem = (RemoveItem_ptr)GetProcAddress(hDLL, "RemoveItem");
		retrieveString = (RetrieveString_ptr)GetProcAddress(hDLL, "RetrieveString");
		retrieveDouble = (RetrieveDouble_ptr)GetProcAddress(hDLL, "RetrieveDouble");
		retrieveFloat = (RetrieveFloat_ptr)GetProcAddress(hDLL, "RetrieveFloat");
		retrieveInt = (RetrieveInt_ptr)GetProcAddress(hDLL, "RetrieveInt");



		if(getNewHandle && runSpecificDialog &&
			removeDialog && addItem && removeItem &&
			retrieveString && retrieveDouble &&
			retrieveFloat && retrieveInt)
		{
			_initValid = true;
		}
		else
		{
			getNewHandle = NULL;
			runSpecificDialog = NULL;
			removeDialog = NULL;
			addItem = NULL;
			removeItem = NULL;
			retrieveString = NULL;
			retrieveDouble = NULL;
			retrieveFloat = NULL;
			retrieveInt = NULL;
			if(!FreeLibrary(hDLL))
			{
				DWORD freeLibraryError = GetLastError();
				ssi_msg(SSI_LOG_LEVEL_ERROR, "Error while getting the function Addresses in ssidialog[d].dll: %u", freeLibraryError);
			}
			hDLL = NULL;
		}
	}
	else
	{
		DWORD loadLibraryError = GetLastError();
		ssi_msg(SSI_LOG_LEVEL_ERROR, "Error while Loading ssidialog[d].dll: %u", loadLibraryError);
	}
}

DialogLibGateway::~DialogLibGateway()
{
	delete[] ssi_log_name;

	if(_dialogHandle >= 0)
	{
		removeDialog(_dialogHandle);
		_dialogHandle = -1;
	}

	_initValid = false;
	getNewHandle = NULL;
	runSpecificDialog = NULL;
	removeDialog = NULL;
	addItem = NULL;
	removeItem = NULL;
	retrieveString = NULL;
	retrieveDouble = NULL;
	retrieveFloat = NULL;
	retrieveInt = NULL;
	if(hDLL)
	{
		if(!FreeLibrary(hDLL))
		{
			DWORD freeLibraryError = GetLastError();
			ssi_msg(SSI_LOG_LEVEL_ERROR, "Error while getting the function Addresses in ssidialog[d].dll: %u", freeLibraryError);
		}
	}
	hDLL = NULL;
}

bool DialogLibGateway::SetNewDialogType(const char *dialogType)
{
	if(!_initValid)
	{
		ssi_msg(SSI_LOG_LEVEL_ERROR, "ssidialog[d].dll Library was not successfully initialized and therefore can't be used. Subsequent calls will also fail with this message!");
		return false;
	}

	int tmpDialogHandle = getNewHandle(dialogType);
	if(tmpDialogHandle >= 0)
	{
		_dialogHandle = tmpDialogHandle;
		return true;
	}

	ssi_msg(SSI_LOG_LEVEL_ERROR, "The requested type of Dialog (%s) was not found, check for typos", dialogType);
	return false;
}

int DialogLibGateway::RunDialog()
{
	if(!_initValid)
	{
		ssi_msg(SSI_LOG_LEVEL_ERROR, "ssidialog[d].dll Library was not successfully initialized and therefore can't be used. Subsequent calls will also fail with this message!");
		return -4;
	}
	if(_dialogHandle < 0)
	{
		ssi_msg(SSI_LOG_LEVEL_ERROR, "Looks like SetNewDialogType was not called yet successfully.");
		return -3;
	}

	int retVal = runSpecificDialog(_dialogHandle);

	switch(retVal)
	{
	case -2:
		ssi_msg(SSI_LOG_LEVEL_ERROR, "Dialog could not be found in List... severe Error");
		break;
	case -1:
		SSI_DBG(SSI_LOG_LEVEL_DEBUG, "User chose to end dialog with Cancel!");
		break;
	default:
		break;
	}

	return retVal;
}

int DialogLibGateway::AppendItem(const char *itemCategory, const char *itemValue)
{
	if(!_initValid)
	{
		ssi_msg(SSI_LOG_LEVEL_ERROR, "ssidialog[d].dll Library was not successfully initialized and therefore can't be used. Subsequent calls will also fail with this message!");
		return -4;
	}
	if(_dialogHandle < 0)
	{
		ssi_msg(SSI_LOG_LEVEL_ERROR, "Looks like SetNewDialogType was not called yet successfully.");
		return -3;
	}

	int retVal = this->AlterExistingItem(itemCategory, -1, itemValue);

	return retVal;
}

int DialogLibGateway::AlterExistingItem(const char *itemCategory, int ordinalValueOfItem, const char *itemValue)
{
	if(!_initValid)
	{
		ssi_msg(SSI_LOG_LEVEL_ERROR, "ssidialog[d].dll Library was not successfully initialized and therefore can't be used. Subsequent calls will also fail with this message!");
		return -4;
	}
	if(_dialogHandle < 0)
	{
		ssi_msg(SSI_LOG_LEVEL_ERROR, "Looks like SetNewDialogType was not called yet successfully.");
		return -3;
	}

	int retVal = addItem(_dialogHandle, itemCategory, ordinalValueOfItem, itemValue);

	return retVal;
}

bool DialogLibGateway::RemoveItems(const char *itemCategory)
{
	if(!_initValid)
	{
		ssi_msg(SSI_LOG_LEVEL_ERROR, "ssidialog[d].dll Library was not successfully initialized and therefore can't be used. Subsequent calls will also fail with this message!");
		return false;
	}
	if(_dialogHandle < 0)
	{
		ssi_msg(SSI_LOG_LEVEL_ERROR, "Looks like SetNewDialogType was not called yet successfully.");
		return false;
	}

	return this->RemoveItem(itemCategory, -1);
}

bool DialogLibGateway::RemoveItem(const char *itemCategory, int ordinalValueOfItem)
{
	if(!_initValid)
	{
		ssi_msg(SSI_LOG_LEVEL_ERROR, "ssidialog[d].dll Library was not successfully initialized and therefore can't be used. Subsequent calls will also fail with this message!");
		return false;
	}
	if(_dialogHandle < 0)
	{
		ssi_msg(SSI_LOG_LEVEL_ERROR, "Looks like SetNewDialogType was not called yet successfully.");
		return false;
	}

	int retVal = removeItem(_dialogHandle, itemCategory, ordinalValueOfItem);

	if(retVal < 0)
		return false;

	return true;
}

bool DialogLibGateway::didInitWork()
{
	return _initValid;
}

bool DialogLibGateway::RetrieveString(const char *itemID, char **retrievedString)
{
	if(!_initValid)
	{
		ssi_msg(SSI_LOG_LEVEL_ERROR, "ssidialog[d].dll Library was not successfully initialized and therefore can't be used. Subsequent calls will also fail with this message!");
		return false;
	}
	if(_dialogHandle < 0)
	{
		ssi_msg(SSI_LOG_LEVEL_ERROR, "Looks like SetNewDialogType was not called yet successfully.");
		return false;
	}

	if(retrievedString == NULL)
	{
		ssi_msg(SSI_LOG_LEVEL_ERROR, "RetrieveString: The adress of the pointer to be filled may not be NULL");
		return false;
	}

	char *tmpString = retrieveString(_dialogHandle, itemID);
	if(tmpString == NULL)
	{
		return false;
	}

	int strLength = ssi_cast (int, strlen(tmpString));
	if(strLength >= 1023)
	{
		delete[] tmpString;
		return false;
	}
	char *retString = new char[strLength+2];
	strncpy(retString, tmpString, strLength+2);

	delete[] tmpString;
	*retrievedString = retString;
	return true;
}

bool DialogLibGateway::RetrieveDouble(const char *itemID, double *retrievedDouble)
{
	if(!_initValid)
	{
		ssi_msg(SSI_LOG_LEVEL_ERROR, "ssidialog[d].dll Library was not successfully initialized and therefore can't be used. Subsequent calls will also fail with this message!");
		return false;
	}
	if(_dialogHandle < 0)
	{
		ssi_msg(SSI_LOG_LEVEL_ERROR, "Looks like SetNewDialogType was not called yet successfully.");
		return false;
	}

	if(retrievedDouble == NULL)
	{
		ssi_msg(SSI_LOG_LEVEL_ERROR, "RetrieveDouble: The adress of the double to be filled may not be NULL");
		return false;
	}

	double tmpDouble = retrieveDouble(_dialogHandle, itemID);

#if __MINGW32__
    if((tmpDouble!=tmpDouble) != 0)
#else
    if(_isnan(tmpDouble) != 0)
#endif // __MINGW32__

	{
		return false;
	}

	*retrievedDouble = tmpDouble;

	return true;
}

bool DialogLibGateway::RetrieveFloat(const char *itemID, float *retrievedFloat)
{
	if(!_initValid)
	{
		ssi_msg(SSI_LOG_LEVEL_ERROR, "ssidialog[d].dll Library was not successfully initialized and therefore can't be used. Subsequent calls will also fail with this message!");
		return false;
	}
	if(_dialogHandle < 0)
	{
		ssi_msg(SSI_LOG_LEVEL_ERROR, "Looks like SetNewDialogType was not called yet successfully.");
		return false;
	}

	if(retrievedFloat == NULL)
	{
		ssi_msg(SSI_LOG_LEVEL_ERROR, "RetrieveFloat: The adress of the float to be filled may not be NULL");
		return false;
	}

	float tmpFloat = retrieveFloat(_dialogHandle, itemID);
#if __MINGW32__
    if((tmpFloat!=tmpFloat) != 0)
#else
	if(_isnan(tmpFloat) != 0)
#endif // __MINGW32__

	{
		return false;
	}

	*retrievedFloat = tmpFloat;

	return true;
}

bool DialogLibGateway::RetrieveInt(const char *itemID, int *retrievedInt)
{
	if(!_initValid)
	{
		ssi_msg(SSI_LOG_LEVEL_ERROR, "ssidialog[d].dll Library was not successfully initialized and therefore can't be used. Subsequent calls will also fail with this message!");
		return false;
	}
	if(_dialogHandle < 0)
	{
		ssi_msg(SSI_LOG_LEVEL_ERROR, "Looks like SetNewDialogType was not called yet successfully.");
		return false;
	}

	if(retrievedInt == NULL)
	{
		ssi_msg(SSI_LOG_LEVEL_ERROR, "RetrieveInt: The adress of the int to be filled may not be NULL");
		return false;
	}

	int tmpInt = retrieveInt(_dialogHandle, itemID);

	if(tmpInt == INT_MIN)
	{
		return false;
	}

	*retrievedInt = tmpInt;

	return true;
}

}

#endif