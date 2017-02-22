/***
 * @file lightstone_win32.c
 * @brief Win32 functions for lightstone communication
 * @author Kyle Machulis (kyle@nonpolynomial.com)
 * @copyright (c) 2006-2011 Nonpolynomial Labs/Kyle Machulis
 * @license BSD License
 *
 * Project info at http://liblightstone.nonpolynomial.com/
 *
 */

#include "lightstone/lightstone.h"
#include <setupapi.h>
#include <hidsdi.h>
#include <stdlib.h>

//Application global variables 
DWORD								ActualBytesRead;
DWORD								BytesRead;
DWORD								cbBytesRead;
PSP_DEVICE_INTERFACE_DETAIL_DATA	detailData;
DWORD								dwError;
char								FeatureReport[256];
HANDLE								hEventObject;
HANDLE								hDevInfo;
GUID								HidGuid;
OVERLAPPED							HIDOverlapped;
char								InputReport[256];
ULONG								Length;
LPOVERLAPPED						lpOverLap;
BOOL								MyDeviceDetected = FALSE; 
TCHAR*								MyDevicePathName;
DWORD								NumberOfBytesRead;
char								OutputReport[256];
DWORD								ReportType;
ULONG								Required;
TCHAR*								ValueToDisplay;
HIDP_CAPS							Capabilities;

void GetDeviceCapabilities(HANDLE DeviceHandle)
{
	//Get the Capabilities structure for the device.

	PHIDP_PREPARSED_DATA	PreparsedData;

	/*
	API function: HidD_GetPreparsedData
	Returns: a pointer to a buffer containing the information about the device's capabilities.
	Requires: A handle returned by CreateFile.
	There's no need to access the buffer directly,
	but HidP_GetCaps and other API functions require a pointer to the buffer.
	*/

	HidD_GetPreparsedData 
		(DeviceHandle, 
		&PreparsedData);

	/*
	API function: HidP_GetCaps
	Learn the device's capabilities.
	For standard devices such as joysticks, you can find out the specific
	capabilities of the device.
	For a custom device, the software will probably know what the device is capable of,
	and the call only verifies the information.
	Requires: the pointer to the buffer returned by HidD_GetPreparsedData.
	Returns: a Capabilities structure containing the information.
	*/
	
	HidP_GetCaps 
		(PreparsedData, 
		&Capabilities);

	HidD_FreePreparsedData(PreparsedData);
}

int lightstone_open_func(lightstone* dev, unsigned int device_index, int get_count)
{	
	//Use a series of API calls to find a HID with a specified Vendor IF and Product ID.

	HIDD_ATTRIBUTES						Attributes;
	SP_DEVICE_INTERFACE_DATA			devInfoData;
	BOOL								LastDevice = FALSE;
	int									MemberIndex = 0;
	LONG								Result;	
	int									device_count = 0;	
	int i = 0;
	Length = 0;
	detailData = NULL;
	dev->_dev = NULL;

	/*
	API function: HidD_GetHidGuid
	Get the GUID for all system HIDs.
	Returns: the GUID in HidGuid.
	*/

	HidD_GetHidGuid(&HidGuid);	
	
	/*
	API function: SetupDiGetClassDevs
	Returns: a handle to a device information set for all installed devices.
	Requires: the GUID returned by GetHidGuid.
	*/
	
	hDevInfo=SetupDiGetClassDevs 
		(&HidGuid, 
		NULL, 
		NULL, 
		DIGCF_PRESENT|DIGCF_INTERFACEDEVICE);
		
	devInfoData.cbSize = sizeof(devInfoData);

	//Step through the available devices looking for the one we want. 
	//Quit on detecting the desired device or checking all available devices without success.

	MemberIndex = 0;
	LastDevice = FALSE;

	do
	{
		/*
		API function: SetupDiEnumDeviceInterfaces
		On return, MyDeviceInterfaceData contains the handle to a
		SP_DEVICE_INTERFACE_DATA structure for a detected device.
		Requires:
		The DeviceInfoSet returned in SetupDiGetClassDevs.
		The HidGuid returned in GetHidGuid.
		An index to specify a device.
		*/

		Result=SetupDiEnumDeviceInterfaces 
			(hDevInfo, 
			0, 
			&HidGuid, 
			MemberIndex, 
			&devInfoData);

		if (Result != 0)
		{
			//A device has been detected, so get more information about it.

			/*
			API function: SetupDiGetDeviceInterfaceDetail
			Returns: an SP_DEVICE_INTERFACE_DETAIL_DATA structure
			containing information about a device.
			To retrieve the information, call this function twice.
			The first time returns the size of the structure in Length.
			The second time returns a pointer to the data in DeviceInfoSet.
			Requires:
			A DeviceInfoSet returned by SetupDiGetClassDevs
			The SP_DEVICE_INTERFACE_DATA structure returned by SetupDiEnumDeviceInterfaces.
			
			The final parameter is an optional pointer to an SP_DEV_INFO_DATA structure.
			This application doesn't retrieve or use the structure.			
			If retrieving the structure, set 
			MyDeviceInfoData.cbSize = length of MyDeviceInfoData.
			and pass the structure's address.
			*/
			
			//Get the Length value.
			//The call will return with a "buffer too small" error which can be ignored.

			Result = SetupDiGetDeviceInterfaceDetail 
				(hDevInfo, 
				&devInfoData, 
				NULL, 
				0, 
				&Length, 
				NULL);

			//Allocate memory for the hDevInfo structure, using the returned Length.

			detailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(Length);
			
			//Set cbSize in the detailData structure.

			detailData -> cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

			//Call the function again, this time passing it the returned buffer size.

			Result = SetupDiGetDeviceInterfaceDetail 
				(hDevInfo, 
				&devInfoData, 
				detailData, 
				Length, 
				&Required, 
				NULL);

			// Open a handle to the device.
			// To enable retrieving information about a system mouse or keyboard,
			// don't request Read or Write access for this handle.

			/*
			API function: CreateFile
			Returns: a handle that enables reading and writing to the device.
			Requires:
			The DevicePath in the detailData structure
			returned by SetupDiGetDeviceInterfaceDetail.
			*/


			dev->_dev =CreateFile 
				(detailData->DevicePath, 
				GENERIC_READ | GENERIC_WRITE,  
				FILE_SHARE_READ|FILE_SHARE_WRITE, 
				(LPSECURITY_ATTRIBUTES)NULL,
				OPEN_EXISTING, 
				0, 
				NULL);

			if(dev->_dev == INVALID_HANDLE_VALUE)
			{
				MemberIndex = MemberIndex + 1;
				continue;
			}
			/*
			API function: HidD_GetAttributes
			Requests information from the device.
			Requires: the handle returned by CreateFile.
			Returns: a HIDD_ATTRIBUTES structure containing
			the Vendor ID, Product ID, and Product Version Number.
			Use this information to decide if the detected device is
			the one we're looking for.
			*/

			//Set the Size to the number of bytes in the structure.

			Attributes.Size = sizeof(Attributes);

			Result = HidD_GetAttributes( 
				dev->_dev,
				&Attributes);
			
			//Is it the desired device?

			MyDeviceDetected = FALSE;

			for(i = 0; i < LIGHTSTONE_VID_PID_PAIRS_COUNT; ++i)
			{
				if (Attributes.VendorID == lightstone_vid_pid_pairs[i][0] && Attributes.ProductID == lightstone_vid_pid_pairs[i][1])
				{
					if(get_count)
					{
						++device_count;
						CloseHandle(dev->_dev);
						break;
					}
					else if(device_count == device_index)
					{
						MyDeviceDetected = TRUE;
						MyDevicePathName = detailData->DevicePath;
						GetDeviceCapabilities(dev->_dev);
						break;
					}
					++device_count;
				}
			}  //if (Result != 0)
			if(!MyDeviceDetected) free(detailData);
		}
		else
		{
			LastDevice=TRUE;
		}
		if(MyDeviceDetected) break;
		//If we haven't found the device yet, and haven't tried every available device,
		//try the next one.
		MemberIndex = MemberIndex + 1;
	}
	while (!LastDevice);
	SetupDiDestroyDeviceInfoList(hDevInfo);
	if(get_count) return device_count;
	if(MyDeviceDetected)
	{
		dev->_is_open = 1;
		return 0;
	}
	return -1;
}

LIGHTSTONE_DECLSPEC int lightstone_get_count(lightstone* dev)
{
	return lightstone_open_func(dev, 0, 1);
}

LIGHTSTONE_DECLSPEC lightstone* lightstone_create()
{
	lightstone* s = (lightstone*)malloc(sizeof(lightstone));
	s->_is_open = 0;
	s->_is_inited = 0;
	return s;   
}

LIGHTSTONE_DECLSPEC void lightstone_delete(lightstone* dev)
{
	free(dev);
}

LIGHTSTONE_DECLSPEC int lightstone_open(lightstone* dev, unsigned int device_index)
{
	return lightstone_open_func(dev, device_index, 0);
}

LIGHTSTONE_DECLSPEC int lightstone_close(lightstone* dev)
{
	if(!dev->_is_open) return E_LIGHTSTONE_NOT_OPENED;
	dev->_is_open = 0;
	CloseHandle(dev->_dev);
	return 0;
}

int lightstone_read(lightstone* dev, unsigned char* report)
{
	DWORD transferred;
	int t;
	t = ReadFile 
		(dev->_dev,
		 report,
		 Capabilities.InputReportByteLength,
		 &transferred,
		 NULL);
	if(t <= 0)
	{
		return t;
	}
	//There's a padding byte at the beginning
	//Copy over that so it looks like what libusb gives us.
	memcpy(report, report + 1, 0x8);
	return transferred - 1;
}
