// Rawinput.cpp
// author: Andreas Seiderer <seiderer@hcm-lab.de>
// created: 2015/08/10
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

#include "Rawinput.h"
#include "base/Factory.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
#include <iostream>
static char THIS_FILE[] = __FILE__;
	#endif
#endif




namespace ssi {

static char ssi_log_name[] = "rawinput__";


Rawinput* ssi::Rawinput::inst = nullptr;

int x = 0, y = 0;




#define MAX_BUTTONS		128
#define CHECK(exp)		{ if(!(exp)) goto Error; }
#define SAFE_FREE(p)	{ if(p) { HeapFree(hHeap, 0, p); (p) = NULL; } }

BOOL bButtonStates[MAX_BUTTONS];
/*LONG lAxisX;
LONG lAxisY;
LONG lAxisZ;
LONG lAxisRz;
LONG lHat;*/
INT  g_NumberOfButtons;



std::string axis_values;

// Raw Input API sample showing joystick support
//http://www.codeproject.com/Articles/185522/Using-the-Raw-Input-API-to-Process-Joystick-Input
void ParseRawInput(RAWINPUT *pRawInput, bool isXBOXOneController)
{
	PHIDP_PREPARSED_DATA pPreparsedData;
	HIDP_CAPS            Caps;
	PHIDP_BUTTON_CAPS    pButtonCaps;
	PHIDP_VALUE_CAPS     pValueCaps;
	USHORT               capsLength;
	UINT                 bufferSize;
	HANDLE               hHeap;
	USAGE                usage[MAX_BUTTONS];
	ULONG                i, usageLength, value;

	pPreparsedData = NULL;
	pButtonCaps = NULL;
	pValueCaps = NULL;
	hHeap = GetProcessHeap();

	std::stringstream strstr;

	//
	// Get the preparsed data block
	//

	CHECK(GetRawInputDeviceInfo(pRawInput->header.hDevice, RIDI_PREPARSEDDATA, NULL, &bufferSize) == 0);
	CHECK(pPreparsedData = (PHIDP_PREPARSED_DATA)HeapAlloc(hHeap, 0, bufferSize));
	CHECK((int)GetRawInputDeviceInfo(pRawInput->header.hDevice, RIDI_PREPARSEDDATA, pPreparsedData, &bufferSize) >= 0);

	//
	// Get the joystick's capabilities
	//

	// Button caps
	CHECK(HidP_GetCaps(pPreparsedData, &Caps) == HIDP_STATUS_SUCCESS)
		CHECK(pButtonCaps = (PHIDP_BUTTON_CAPS)HeapAlloc(hHeap, 0, sizeof(HIDP_BUTTON_CAPS) * Caps.NumberInputButtonCaps));

	capsLength = Caps.NumberInputButtonCaps;
	CHECK(HidP_GetButtonCaps(HidP_Input, pButtonCaps, &capsLength, pPreparsedData) == HIDP_STATUS_SUCCESS)
		g_NumberOfButtons = pButtonCaps->Range.UsageMax - pButtonCaps->Range.UsageMin + 1;

	// Value caps
	CHECK(pValueCaps = (PHIDP_VALUE_CAPS)HeapAlloc(hHeap, 0, sizeof(HIDP_VALUE_CAPS) * Caps.NumberInputValueCaps));
	capsLength = Caps.NumberInputValueCaps;
	CHECK(HidP_GetValueCaps(HidP_Input, pValueCaps, &capsLength, pPreparsedData) == HIDP_STATUS_SUCCESS)

		//
		// Get the pressed buttons
		//

		usageLength = g_NumberOfButtons;
	CHECK(
		HidP_GetUsages(
		HidP_Input, pButtonCaps->UsagePage, 0, usage, &usageLength, pPreparsedData,
		(PCHAR)pRawInput->data.hid.bRawData, pRawInput->data.hid.dwSizeHid
		) == HIDP_STATUS_SUCCESS);

	ZeroMemory(bButtonStates, sizeof(bButtonStates));
	for (i = 0; i < usageLength; i++)
		bButtonStates[usage[i] - pButtonCaps->Range.UsageMin] = TRUE;

	//
	// Get the state of discrete-valued-controls
	//

	for (i = 0; i < Caps.NumberInputValueCaps; i++)
	{
		CHECK(
			HidP_GetUsageValue(
			HidP_Input, pValueCaps[i].UsagePage, 0, pValueCaps[i].Range.UsageMin, &value, pPreparsedData,
			(PCHAR)pRawInput->data.hid.bRawData, pRawInput->data.hid.dwSizeHid
			) == HIDP_STATUS_SUCCESS);


		strstr << value;

		if (i < Caps.NumberInputValueCaps - 1)
			strstr << ",";

		
		/*switch (pValueCaps[i].Range.UsageMin)
		{
		case 0x30:	// X-axis
			lAxisX = (LONG)value - 128;
			break;

		case 0x31:	// Y-axis
			lAxisY = (LONG)value - 128;
			break;

		case 0x32: // Z-axis
			lAxisZ = (LONG)value - 128;
			break;

		case 0x35: // Rotate-Z
			lAxisRz = (LONG)value - 128;
			break;

		case 0x39:	// Hat Switch
			lHat = value;
			break;
		}*/
	}


	axis_values = strstr.str();


	//
	// Clean up
	//

Error:
	SAFE_FREE(pPreparsedData);
	SAFE_FREE(pButtonCaps);
	SAFE_FREE(pValueCaps);
}








LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	LPBYTE lpb;
	UINT dwSize;
	RAWINPUT *raw;
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
		break;

	case WM_INPUT:
	{
		UINT dwSize;

		GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
		LPBYTE lpb = new BYTE[dwSize];
		if (lpb == NULL)
		{
			return 0;
		}

		if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
			OutputDebugString(TEXT("GetRawInputData does not return correct size !\n"));

		RAWINPUT* raw = (RAWINPUT*)lpb;


		if (raw->header.dwType == RIM_TYPEKEYBOARD)
		{
			Rawinput::inst->sendKeyboardEvent(raw->data.keyboard.Flags, raw->data.keyboard.VKey, raw->data.keyboard.Message);
		}
		else if (raw->header.dwType == RIM_TYPEMOUSE)
		{
			Rawinput::inst->sendMouseEvent(raw->data.mouse.usFlags, raw->data.mouse.usButtonFlags, raw->data.mouse.usButtonData, raw->data.mouse.ulRawButtons, raw->data.mouse.lLastX, raw->data.mouse.lLastY, raw->data.mouse.ulExtraInformation);
		}
		else if (raw->header.dwType == RIM_TYPEHID)
		{
			
			UINT size = 1024;
			TCHAR tBuffer[1024] = { 0 };

			UINT nResult = GetRawInputDeviceInfo(raw->header.hDevice, RIDI_DEVICENAME, tBuffer, &size);
			if (nResult < 0)
			{
				ssi_msg(SSI_LOG_LEVEL_DEFAULT, "ERR: Unable to get device name.");
			}

			bool isXBOXOneController = false;
			
			RID_DEVICE_INFO device_info;

			UINT info_size(sizeof(RID_DEVICE_INFO));
			if (GetRawInputDeviceInfo(raw->header.hDevice, RIDI_DEVICEINFO, (LPVOID)&device_info, &info_size) == info_size)
			{
				if (device_info.hid.dwVendorId == 0x045E && device_info.hid.dwProductId == 0x02D1)
					isXBOXOneController = true;
			}
			

			ParseRawInput(raw, isXBOXOneController);

			const char* axis_values_c_str = axis_values.c_str();

			Rawinput::inst->sendJoystickEvent(bButtonStates, g_NumberOfButtons, axis_values_c_str, tBuffer);
		}

		delete[] lpb;
		return 0;
	}
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}




//https://msdn.microsoft.com/en-us/library/windows/desktop/ee418864%28v=vs.85%29.aspx
//https://www.tutorials.de/threads/wm_mousehwheel-wird-nicht-richtig-empfangen.399383/
//https://github.com/abort/Rinput-Library/blob/master/rawinput.h
//https://github.com/abort/Rinput-Library/blob/master/rawinput.cpp
//http://stackoverflow.com/questions/4081334/using-createwindowex-to-make-a-message-only-window
//https://msdn.microsoft.com/en-us/library/windows/desktop/ms645546%28v=vs.85%29.aspx

HWND _hWnd = NULL;



Rawinput::Rawinput(const ssi_char_t *file)
	: _elistener(0),
	_file (0),
	stop_loop(false),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

	if (Rawinput::inst == nullptr)
		Rawinput::inst = this;


	_single_execution = true;

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}

	ssi_event_init(_event, SSI_ETYPE_STRING);
	ssi_event_init(_event_key, SSI_ETYPE_STRING);
	ssi_event_init(_event_joy, SSI_ETYPE_STRING);
}

Rawinput::~Rawinput() {

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}

	ssi_event_destroy(_event);
	ssi_event_destroy(_event_key);
	ssi_event_destroy(_event_joy);
}





bool Rawinput::setEventListener(IEventListener *listener) {

	_elistener = listener;
	_event.sender_id = Factory::AddString(_options.sname);
	if (_event.sender_id == SSI_FACTORY_STRINGS_INVALID_ID) {
		return false;
	}
	_event.event_id = Factory::AddString("raw_mouse");
	if (_event.event_id == SSI_FACTORY_STRINGS_INVALID_ID) {
		return false;
	}

	_event_key.sender_id = Factory::AddString(_options.sname);
	if (_event_key.sender_id == SSI_FACTORY_STRINGS_INVALID_ID) {
		return false;
	}
	_event_key.event_id = Factory::AddString("key");
	if (_event_key.event_id == SSI_FACTORY_STRINGS_INVALID_ID) {
		return false;
	}

	_event_joy.sender_id = Factory::AddString(_options.sname);
	if (_event_joy.sender_id == SSI_FACTORY_STRINGS_INVALID_ID) {
		return false;
	}
	_event_joy.event_id = Factory::AddString("joystick");
	if (_event_joy.event_id == SSI_FACTORY_STRINGS_INVALID_ID) {
		return false;
	}


	_eaddress.setSender(_options.sname);

	std::stringstream strstr;
	strstr << "raw_mouse" << "," << "key" << "," << "joystick";

	_eaddress.setEvents(strstr.str().c_str());

	return true;
}


	void Rawinput::sendMouseEvent(unsigned short usFlags, unsigned short usButtonFlags, unsigned short usButtonData, unsigned long ulRawButtons, long lLastX, long lLastY, unsigned long ulExtraInformation)
	{
		if (_elistener) {

			std::stringstream strstr;

			strstr << usFlags << "," << usButtonFlags << "," << usButtonData << "," << ulRawButtons << "," << lLastX << "," << lLastY << "," << ulExtraInformation;

			std::string str = strstr.str();
			ssi_event_adjust(_event, str.length() + 1);
			memcpy(_event.ptr, str.c_str(), str.length() + 1);

			_event.time = Factory::GetFramework()->GetElapsedTimeMs();
			_elistener->update(_event);
		}
	}

	void Rawinput::sendKeyboardEvent(unsigned short flags, unsigned short vkey, unsigned message)
{
	if (_elistener) {

		std::stringstream strstr;

		strstr << flags << "," << vkey << "," << message;

		std::string str = strstr.str();
		ssi_event_adjust(_event_key, str.length() + 1);
		memcpy(_event_key.ptr, str.c_str(), str.length() + 1);

		_event_key.time = Factory::GetFramework()->GetElapsedTimeMs();
		_elistener->update(_event_key);
	}
}

	void Rawinput::sendJoystickEvent(int* buttonStates, unsigned int buttoncount, const char* axes, char* id)
	{
		if (_elistener) {

			std::stringstream strstr;

			strstr << buttoncount << ",";

			for (auto i = 0; i < buttoncount; i++) {
				strstr << buttonStates[i] << ",";
			}

			strstr << axes;

			if (_options.sendId)
				strstr << "," << id;


			std::string str = strstr.str();
			ssi_event_adjust(_event_joy, str.length() + 1);
			memcpy(_event_joy.ptr, str.c_str(), str.length() + 1);

			_event_joy.time = Factory::GetFramework()->GetElapsedTimeMs();
			_elistener->update(_event_joy);
		}
	}


	void Rawinput::enter()
{

	const char* class_name = "DUMMY_CLASS";
	WNDCLASSEX wx = {};

	wx.cbSize = sizeof(WNDCLASSEX);
	wx.lpfnWndProc = WndProc;        // function which will handle messages
	//wx.hInstance = GetCurrentModule();
	//wx.hInstance = GetModuleHandle(NULL);
	wx.lpszClassName = class_name;
	if (RegisterClassEx(&wx)) {
		//WS_EX_TOOLWINDOW remove taskbar entry
		_hWnd = CreateWindowEx(WS_EX_TOOLWINDOW, class_name, class_name, WS_VISIBLE | WS_POPUP, 640, 480, 0, 0, NULL, NULL, NULL, NULL);

		if (!_hWnd)
			ssi_msg(SSI_LOG_LEVEL_DEFAULT, "Input window creation failed!");
	}


	


	//http://zfx.info/viewtopic.php?f=11&t=2977

	ssi_msg(SSI_LOG_LEVEL_DEFAULT, "Found HID devices (excludes mouse / keyboard): ");

	// get the number of HID input devices
	UINT input_device_count(0);
	if (GetRawInputDeviceList(NULL, &input_device_count, sizeof(RAWINPUTDEVICELIST)) != 0)
	{
	}
	// allocate memory for their structures
	std::vector<RAWINPUTDEVICELIST> input_devices;
	input_devices.resize(input_device_count);
	// then populate the device list
	if (GetRawInputDeviceList(&input_devices[0], &input_device_count,
		sizeof(RAWINPUTDEVICELIST)) == (UINT)-1)
	{
	}

	RID_DEVICE_INFO device_info;
	for (std::vector<RAWINPUTDEVICELIST>::iterator device_iterator(input_devices.begin());
		device_iterator != input_devices.end();
		++device_iterator)
	{
		UINT info_size(sizeof(RID_DEVICE_INFO));
		if (GetRawInputDeviceInfo(device_iterator->hDevice,
			RIDI_DEVICEINFO, (LPVOID)&device_info, &info_size) == info_size)
		{
			// non-keyboard, non-mouse HID device?
			if (device_info.dwType == RIM_TYPEHID)
			{
				ssi_msg(SSI_LOG_LEVEL_DEFAULT, "HID device: vendorId: %i | productId: %i | usage: %i | usagePage: %i", device_info.hid.dwVendorId, device_info.hid.dwProductId, device_info.hid.usUsage, device_info.hid.usUsagePage);
			}
		}
	}


	RAWINPUTDEVICE Rid[4];

	int count = 0;

	if (_options.activedevices[0]) {
		// adds HID mouse
		Rid[count].usUsagePage = 0x01;
		Rid[count].usUsage = 0x02;
		Rid[count].dwFlags = RIDEV_INPUTSINK;
		Rid[count].hwndTarget = _hWnd;

		count++;
	}

	if (_options.activedevices[1]) {
		// adds HID keyboard
		Rid[count].usUsagePage = 0x01;
		Rid[count].usUsage = 0x06;
		Rid[count].dwFlags = RIDEV_INPUTSINK;
		Rid[count].hwndTarget = _hWnd;

		count++;
	}

	if (_options.activedevices[2]) {
		// adds HID Joystick
		Rid[count].usUsagePage = 0x01;
		Rid[count].usUsage = 0x04;				//joystick 04
		Rid[count].dwFlags = RIDEV_INPUTSINK;
		Rid[count].hwndTarget = _hWnd;

		count++;
	}

	if (_options.activedevices[2]) {
		// adds HID Joystick
		Rid[count].usUsagePage = 0x01;
		Rid[count].usUsage = 0x05;				//gamepad 05
		Rid[count].dwFlags = RIDEV_INPUTSINK;
		Rid[count].hwndTarget = _hWnd;

		count++;
	}

	if (RegisterRawInputDevices(Rid, count, sizeof(Rid[0])) == false)
	{
		ssi_msg(SSI_LOG_LEVEL_DEFAULT, "Registration failed!");
	}



	Thread::setName(getName());
}

void Rawinput::run()
{
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0) != 0 && !stop_loop)
	{
		DispatchMessage(&msg);
	}

	ssi_msg(SSI_LOG_LEVEL_DEFAULT, "Message loop stopped!");

}

	bool Rawinput::stop()
	{
		stop_loop = true;

		return true;
	}
}
