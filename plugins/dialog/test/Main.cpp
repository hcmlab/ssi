// Main.cpp
// author: Frank Jung <frank.jung@informatik.uni-augsburg.de>
// created: 2009/03/01
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

#include "ssi.h"
using namespace ssi;

typedef int (*GetNewDialogHandle_ptr)(char*);
typedef int (*RunSpecificDialog_ptr)(int);
typedef bool (*RemoveDialog_ptr)(int);
typedef int (*AddItem_ptr)(int, char*, int, char*);
typedef int (*RemoveItem_ptr)(int, char*, int);
typedef char* (*RetrieveString_ptr)(int, const char*);
typedef double (*RetrieveDouble_ptr)(int, const char*);
typedef float (*RetrieveFloat_ptr)(int, const char*);
typedef int (*RetrieveInt_ptr)(int, const char*);

void init (HMODULE hDLL);

bool ex_selection (void *arg); 
bool ex_camdevice(void *arg);
bool ex_pinandmedia(void *arg);

GetNewDialogHandle_ptr getNewHandle;
RunSpecificDialog_ptr runSpecificDialog;
RemoveDialog_ptr removeDialog;
AddItem_ptr addItem;
RemoveItem_ptr removeItem;
RetrieveString_ptr retrieveString;
RetrieveDouble_ptr retrieveDouble;
RetrieveFloat_ptr retrieveFloat;
RetrieveInt_ptr retrieveInt;

int main () {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	HMODULE hDLL = NULL;
#ifdef _DEBUG
	hDLL = LoadLibrary("ssidialogd.dll");
#else
	hDLL = LoadLibrary("ssidialog.dll");
#endif
	init (hDLL);

	Exsemble ex;
	ex.add(&ex_selection, 0, "SELECTION", "How to create a selection dialog.");
	ex.add(&ex_camdevice, 0, "CAMDEVICE", "How to use camera device selection dialog.");
	ex.add(&ex_pinandmedia, 0, "CAMMEDIA", "How to use camera media selection dialog.");
	ex.show();

	FreeLibrary (hDLL);

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif
	
	return 0;
}

void init(HMODULE hDLL) {

	getNewHandle = (GetNewDialogHandle_ptr)GetProcAddress(hDLL, "GetNewDialogHandle");
	runSpecificDialog = (RunSpecificDialog_ptr)GetProcAddress(hDLL, "RunSpecificDialog");
	removeDialog = (RemoveDialog_ptr)GetProcAddress(hDLL, "RemoveDialog");
	addItem = (AddItem_ptr)GetProcAddress(hDLL, "AddItem");
	removeItem = (RemoveItem_ptr)GetProcAddress(hDLL, "RemoveItem");
	retrieveString = (RetrieveString_ptr)GetProcAddress(hDLL, "RetrieveString");
	retrieveDouble = (RetrieveDouble_ptr)GetProcAddress(hDLL, "RetrieveDouble");
	retrieveFloat = (RetrieveFloat_ptr)GetProcAddress(hDLL, "RetrieveFloat");
	retrieveInt = (RetrieveInt_ptr)GetProcAddress(hDLL, "RetrieveInt");
}

bool ex_selection(void *arg) {

	int nDlgHandle = getNewHandle("SimpleSelectionDialog");
	int tmp = addItem (nDlgHandle, "Caption", -1, "Dialog For Selection");
	
	tmp = addItem(nDlgHandle, "Item", -1, "Select1");			
	addItem(nDlgHandle, "Description", tmp, "Description to Select1");

	addItem(nDlgHandle, "Item", -1, "Select2");
	addItem(nDlgHandle, "Item", -1, "Select3");
		
	ssi_print ("Selected Index: %d\n", runSpecificDialog(nDlgHandle));

	return true;
}

bool ex_camdevice(void *arg) {

	int nDlgHandle = getNewHandle("CamSelectionDialog");
	int tmp = addItem(nDlgHandle, "Caption", -1, "Dialog For Cam Selection");
	
	tmp = addItem(nDlgHandle, "FriendlyName", -1, "Camera1");
	addItem(nDlgHandle, "Description", tmp, "Manufacturer 1");
	addItem(nDlgHandle, "DevicePath", tmp, "DevicePath1sdfdslfdhfsdfs//ycyxcxyöcxycm?yxfcdyjfjdfdfjdfdjfkdfkldjfdsklvnvvncxvnxdlvvdsvvsdlvvlknvdlkvnxmyvmxlvnsdlkfnsldfsdkldj");
	
	tmp = addItem(nDlgHandle, "FriendlyName", -1, "Camera2");
	addItem(nDlgHandle, "Description", tmp, "Manufacturer 2");
	addItem(nDlgHandle, "DevicePath", tmp, "DevicePath1sdfdslfdhfsdfs//reeoittoirtutrioretureioturtuirtreotireutreiotuioe13135413541xc,vbcxvnbcvxcnvcvxcvnxvbvxvbvxcvnbvxcmv");

	ssi_print ("Selected Index: %d\n", runSpecificDialog(nDlgHandle));

	return true;
}

bool ex_pinandmedia(void *arg) {

	int nDlgHandle = getNewHandle("PinAndMediaSelectionDialog");
	int tmp = addItem(nDlgHandle, "Caption", -1, "Dialog For Pin and Media Selection");

	tmp = addItem(nDlgHandle, "majortype", 0000, "{E06D80E3-DB46-11CF-B4D1-00805F6CBBEA}");
	tmp = addItem(nDlgHandle, "subtype", 0000, "{6E6415E6-5C24-425F-93CD-80102B3D1CCA}");
	tmp = addItem(nDlgHandle, "bFixedSizeSamples", 0000, "True");
	tmp = addItem(nDlgHandle, "InputSize", 0000, "100_50");
	tmp = addItem(nDlgHandle, "rcSource", 0000, "10_50_30_80");

	tmp = addItem(nDlgHandle, "majortype", 101, "{593CDDE1-0759-11D1-9E69-00C04FD7C15B}");
	tmp = addItem(nDlgHandle, "bFixedSizeSamples", 101, "False");
	tmp = addItem(nDlgHandle, "InputSize", 101, "20_40");
	tmp = addItem(nDlgHandle, "rcSource", 101, "10_45_30_65");
	tmp = addItem(nDlgHandle, "InputSize", 101, "NULL_10");
	tmp = addItem(nDlgHandle, "rcSource", 101, "5_NULL_15_NULL");

	ssi_print ("Selected Index: %d\n", runSpecificDialog(nDlgHandle));
	ssi_print ("  PinIndex: %d\n  MediaTypeIndex: %d\n", retrieveInt(nDlgHandle, "PinIndex"), retrieveInt(nDlgHandle, "MediaTypeIndex"));

	return true;
}
