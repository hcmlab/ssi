// Keyboard.cpp
// author: Ionut Damian <damian@hcm-lab.de>
// created: 2015/10/29
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

#include "Keyboard.h"
#include "base/Factory.h"

#include <dinput.h>
#include <stdarg.h>

#include <sstream>
#include <iomanip>

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ssi_char_t Keyboard::ssi_log_name[] = "keyboard__";

Keyboard::Keyboard (const ssi_char_t *file) 
	: _elistener(0),
	_file (0),
	_new_word(false),
	_n_word(0),
	_n_word_buffer(0),
	_word_buffer(0),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}

	ssi_event_init(_event, SSI_ETYPE_STRING);

	// set thread name
	Thread::setName(getName());
}

Keyboard::~Keyboard () {

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
	ssi_event_destroy(_event);
}

void Keyboard::enter () {

	if (s_hookStarted) {
		ssi_wrn("Keyboard hook already started. Only one keyboard component per pipe supported.");
		return;
	}

	ssi_wrn("This component can be used to do evil! Please make sure nobody types in any sensitive information while the pipe is running!");

	s_stopThread = false;
	s_hThread = CreateThread(NULL, 0, MessageLoopThread, NULL, 0, NULL);
	s_hookStarted = true;

	_new_word = true;
	_n_word = 0;
	_n_word_buffer = _options.maxBuffer;
	_word_buffer = new ssi_char_t[_n_word_buffer];	
}

void Keyboard::run () {
	
	if (s_newKeyEvent)
	{
		sendEvent(s_keyEvent.scanCode, s_keyEvent.vkCode, s_keyEvent.isUp);
		s_newKeyEvent = false;
	}
}

void Keyboard::flush () {

	if (!s_hookStarted) {
		return;
	}

	s_stopThread = true;

	if (s_hThread != NULL) {
		DWORD waitResult = WaitForSingleObject(s_hThread, 3000);
		if (waitResult != WAIT_OBJECT_0) {
			TerminateThread(s_hThread, 0);
		}

		CloseHandle(s_hThread);
		s_hThread = NULL;
	}

	s_stopThread = false;
	s_hookStarted = false;

	_new_word = true;
	_n_word = 0;
	_n_word_buffer = 0;
	delete[] _word_buffer; _word_buffer = 0;
}

bool Keyboard::setEventListener(IEventListener *listener) {

	_elistener = listener;
	_event.sender_id = Factory::AddString(_options.sname);
	if (_event.sender_id == SSI_FACTORY_STRINGS_INVALID_ID) {
		return false;
	}
	_event.event_id = Factory::AddString(_options.ename);
	if (_event.event_id == SSI_FACTORY_STRINGS_INVALID_ID) {
		return false;
	}

	_eaddress.setSender(_options.sname);
	_eaddress.setEvents(_options.ename);

	return true;
}

void Keyboard::sendEvent(int scanCode, int vkCode, bool isUp) {

	if (_elistener) {

		std::string name;
		switch (_options.keymap)
		{
		case KeyMap::VirtualKey:
			if (s_virtualKeyMap.find(vkCode) != s_virtualKeyMap.end())
				name = s_virtualKeyMap[vkCode];
			else
				name = getHex(vkCode);
			break;
		case KeyMap::VirtualKey_DE:
			if (s_virtualKeyMap_DE.find(vkCode) != s_virtualKeyMap_DE.end())
				name = s_virtualKeyMap_DE[vkCode];
			else
				name = getHex(vkCode);
			break;
		case KeyMap::ScanCode:
			if (s_scanCodeKeyMap.find(scanCode) != s_scanCodeKeyMap.end())
				name = s_scanCodeKeyMap[scanCode];
			else
				name = getHex(scanCode);
			break;
		case KeyMap::DirectInput:
			if (s_directInputKeyMap.find(scanCode) != s_directInputKeyMap.end())
				name = s_directInputKeyMap[scanCode];
			else
				name = getHex(scanCode);
			break;
		}

		if (_options.waitForEnter) {

			if (isUp) {

				bool is_enter = ssi_strcmp(name.c_str(), "enter");

				if (!is_enter) {

					if (_new_word) {
						_event.time = Factory::GetFramework()->GetElapsedTimeMs();
						_n_word = 0;
						_new_word = false;
					}

					if (_n_word < _n_word_buffer) {

						ssi_size_t n_chars = ssi_cast(ssi_size_t, name.length());
						memcpy(_word_buffer + _n_word, name.c_str(), n_chars);
						_n_word += n_chars;

					} else {
						ssi_wrn("word buffer too small");
					}

				} else if (_n_word > 0) {

					_word_buffer[_n_word] = '\0';
					ssi_event_adjust(_event, _n_word + 1);
					memcpy(_event.ptr, _word_buffer, _n_word + 1);
					_event.dur = Factory::GetFramework()->GetElapsedTimeMs() - _event.time;
					_event.state = SSI_ESTATE_COMPLETED;

					_elistener->update(_event);

					_event.glue_id++;
					_new_word = true;
					_n_word = 0;
				}
			}

		} else {

			_event.time = Factory::GetFramework()->GetElapsedTimeMs();
			_event.dur = 0;
			ssi_event_adjust(_event, ssi_cast(ssi_size_t, name.length()) + 1);
			strcpy(_event.ptr, name.c_str());
			_event.state = isUp ? SSI_ESTATE_COMPLETED : SSI_ESTATE_CONTINUED;

			if (isUp || _options.eager) {
				_elistener->update(_event);
			}

			if (isUp) {
				_event.glue_id++;
			}
		}
	}
}

DWORD WINAPI Keyboard::MessageLoopThread(LPVOID lpThreadParameter) {

	HINSTANCE appInstance = GetModuleHandle(NULL);
	s_hHookKeyboard = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)LowLevelKeyboardProc, appInstance, 0);
	s_newKeyEvent = false;

	const unsigned int timerId = 0xAABB1337;
	SetTimer(NULL, timerId, 1000, NULL);
	MSG msg;
	while (!s_stopThread && GetMessage(&msg, NULL, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	KillTimer(NULL, timerId);

	if (s_hHookKeyboard != NULL) {
		UnhookWindowsHookEx(s_hHookKeyboard);
	}

	return 0;
}

LRESULT CALLBACK Keyboard::LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode == HC_ACTION)
	{
		KBDLLHOOKSTRUCT *keyEventData = (KBDLLHOOKSTRUCT*)lParam;

		if (keyEventData->flags & LLKHF_INJECTED) {
			return CallNextHookEx(s_hHookKeyboard, nCode, wParam, lParam);
		}

		bool isUp = false;

		switch (wParam)
		{
		case WM_KEYDOWN:
			isUp = false;
			break;

		case WM_KEYUP:
			isUp = true;
			break;

		default:
			return CallNextHookEx(s_hHookKeyboard, nCode, wParam, lParam);
		}

		s_keyEvent.scanCode = keyEventData->scanCode;
		s_keyEvent.vkCode = keyEventData->vkCode;
		s_keyEvent.isUp = isUp;

		s_newKeyEvent = true;
	}

	return CallNextHookEx(s_hHookKeyboard, nCode, wParam, lParam);
}

std::string Keyboard::getHex(int x)
{
	using namespace std;

	stringstream val;

	val << showbase // show the 0x prefix
		<< internal // fill between the prefix and the number
		<< setfill('0'); // fill with 0s

	val << hex << setw(4) << x << endl;

	return val.str();
}

/*
 * Based on InputMapper plugin by Philipp Harzig, Stephan Brehm, Niclas Geiger, Michael Heerklotz, Sebastian Witthus
 */
HHOOK Keyboard::s_hHookKeyboard;
bool Keyboard::s_hookStarted;
HANDLE Keyboard::s_hThread;
bool Keyboard::s_stopThread;
bool Keyboard::s_newKeyEvent;
Keyboard::KeyEvent Keyboard::s_keyEvent;

typedef std::map<int, std::string> inputKeyMapType;
const inputKeyMapType::value_type  directInputKeyMapData[] = {
	inputKeyMapType::value_type(DIK_ESCAPE,"esc"),
	inputKeyMapType::value_type(DIK_1,"1"),
	inputKeyMapType::value_type(DIK_2,"2"),
	inputKeyMapType::value_type(DIK_3,"3"),
	inputKeyMapType::value_type(DIK_4,"4"),
	inputKeyMapType::value_type(DIK_5,"5"),
	inputKeyMapType::value_type(DIK_6,"6"),
	inputKeyMapType::value_type(DIK_7,"7"),
	inputKeyMapType::value_type(DIK_8,"8"),
	inputKeyMapType::value_type(DIK_9,"9"),
	inputKeyMapType::value_type(DIK_0,"0"),
	inputKeyMapType::value_type(DIK_MINUS,"-"),
	inputKeyMapType::value_type(DIK_EQUALS,"="),
	inputKeyMapType::value_type(DIK_BACK,"back"),
	inputKeyMapType::value_type(DIK_TAB,"tab"),
	inputKeyMapType::value_type(DIK_Q,"q"),
	inputKeyMapType::value_type(DIK_W,"w"),
	inputKeyMapType::value_type(DIK_E,"e"),
	inputKeyMapType::value_type(DIK_R,"r"),
	inputKeyMapType::value_type(DIK_T,"t"),
	inputKeyMapType::value_type(DIK_Y,"y"),
	inputKeyMapType::value_type(DIK_U,"u"),
	inputKeyMapType::value_type(DIK_I,"i"),
	inputKeyMapType::value_type(DIK_O,"o"),
	inputKeyMapType::value_type(DIK_P,"p"),
	inputKeyMapType::value_type(DIK_LBRACKET,"lbracket"),
	inputKeyMapType::value_type(DIK_RBRACKET,"bracket"),
	inputKeyMapType::value_type(DIK_RETURN,"enter"),
	inputKeyMapType::value_type(DIK_LCONTROL,"lctrl"),
	inputKeyMapType::value_type(DIK_A,"a"),
	inputKeyMapType::value_type(DIK_S,"s"),
	inputKeyMapType::value_type(DIK_D,"d"),
	inputKeyMapType::value_type(DIK_F,"f"),
	inputKeyMapType::value_type(DIK_G,"g"),
	inputKeyMapType::value_type(DIK_H,"h"),
	inputKeyMapType::value_type(DIK_J,"j"),
	inputKeyMapType::value_type(DIK_K,"k"),
	inputKeyMapType::value_type(DIK_L,"l"),
	inputKeyMapType::value_type(DIK_SEMICOLON,";"),
	inputKeyMapType::value_type(DIK_APOSTROPHE,"'"),
	inputKeyMapType::value_type(DIK_GRAVE,"`"),
	inputKeyMapType::value_type(DIK_LSHIFT,"lshift"),
	inputKeyMapType::value_type(DIK_BACKSLASH,"\\"),
	inputKeyMapType::value_type(DIK_Z,"z"),
	inputKeyMapType::value_type(DIK_X,"x"),
	inputKeyMapType::value_type(DIK_C,"c"),
	inputKeyMapType::value_type(DIK_V,"v"),
	inputKeyMapType::value_type(DIK_B,"b"),
	inputKeyMapType::value_type(DIK_N,"n"),
	inputKeyMapType::value_type(DIK_M,"m"),
	inputKeyMapType::value_type(DIK_COMMA,","),
	inputKeyMapType::value_type(DIK_PERIOD,"."),
	inputKeyMapType::value_type(DIK_SLASH,"/"),
	inputKeyMapType::value_type(DIK_RSHIFT,"rshift"),
	inputKeyMapType::value_type(DIK_MULTIPLY,"numpad*"),
	inputKeyMapType::value_type(DIK_LMENU,"lalt"),
	inputKeyMapType::value_type(DIK_SPACE," "),
	inputKeyMapType::value_type(DIK_CAPITAL,"caps"),
	inputKeyMapType::value_type(DIK_F1,"f1"),
	inputKeyMapType::value_type(DIK_F2,"f2"),
	inputKeyMapType::value_type(DIK_F3,"f3"),
	inputKeyMapType::value_type(DIK_F4,"f4"),
	inputKeyMapType::value_type(DIK_F5,"f5"),
	inputKeyMapType::value_type(DIK_F6,"f6"),
	inputKeyMapType::value_type(DIK_F7,"f7"),
	inputKeyMapType::value_type(DIK_F8,"f8"),
	inputKeyMapType::value_type(DIK_F9,"f9"),
	inputKeyMapType::value_type(DIK_F10,"f10"),
	inputKeyMapType::value_type(DIK_NUMLOCK,"numlock"),
	inputKeyMapType::value_type(DIK_SCROLL,"scrolllock"),
	inputKeyMapType::value_type(DIK_NUMPAD7,"numpad7"),
	inputKeyMapType::value_type(DIK_NUMPAD8,"numpad8"),
	inputKeyMapType::value_type(DIK_NUMPAD9,"numpad9"),
	inputKeyMapType::value_type(DIK_SUBTRACT,"numpad-"),
	inputKeyMapType::value_type(DIK_NUMPAD4,"numpad4"),
	inputKeyMapType::value_type(DIK_NUMPAD5,"numpad5"),
	inputKeyMapType::value_type(DIK_NUMPAD6,"numpad6"),
	inputKeyMapType::value_type(DIK_ADD,"numpad+"),
	inputKeyMapType::value_type(DIK_NUMPAD1,"numpad1"),
	inputKeyMapType::value_type(DIK_NUMPAD2,"numpad2"),
	inputKeyMapType::value_type(DIK_NUMPAD3,"numpad3"),
	inputKeyMapType::value_type(DIK_NUMPAD0,"numpad0"),
	inputKeyMapType::value_type(DIK_DECIMAL,"numpad."),
	inputKeyMapType::value_type(DIK_OEM_102,"oem102"),
	inputKeyMapType::value_type(DIK_F11,"f11"),
	inputKeyMapType::value_type(DIK_F12,"f12"),
	inputKeyMapType::value_type(DIK_F13,"f13"),
	inputKeyMapType::value_type(DIK_F14,"f14"),
	inputKeyMapType::value_type(DIK_F15,"f15"),
	inputKeyMapType::value_type(DIK_KANA,"kana"),
	inputKeyMapType::value_type(DIK_ABNT_C1,"brazilian?"),
	inputKeyMapType::value_type(DIK_CONVERT,"convert"),
	inputKeyMapType::value_type(DIK_NOCONVERT,"noconvert"),
	inputKeyMapType::value_type(DIK_YEN,"yen"),
	inputKeyMapType::value_type(DIK_ABNT_C2,"brazilian."),
	inputKeyMapType::value_type(DIK_NUMPADEQUALS,"numpad="),
	inputKeyMapType::value_type(DIK_PREVTRACK,"prevtrack"),
	inputKeyMapType::value_type(DIK_AT,"at"),
	inputKeyMapType::value_type(DIK_COLON,"japanese,"),
	inputKeyMapType::value_type(DIK_UNDERLINE,"_"),
	inputKeyMapType::value_type(DIK_KANJI,"kanji"),
	inputKeyMapType::value_type(DIK_STOP,"stop"),
	inputKeyMapType::value_type(DIK_AX,"ax"),
	inputKeyMapType::value_type(DIK_UNLABELED,"unlabeled"),
	inputKeyMapType::value_type(DIK_NEXTTRACK,"nexttrack"),
	inputKeyMapType::value_type(DIK_NUMPADENTER,"numpadenter"),
	inputKeyMapType::value_type(DIK_RCONTROL,"rctrl"),
	inputKeyMapType::value_type(DIK_MUTE,"mute"),
	inputKeyMapType::value_type(DIK_CALCULATOR,"calculator"),
	inputKeyMapType::value_type(DIK_PLAYPAUSE,"playpause"),
	inputKeyMapType::value_type(DIK_MEDIASTOP,"mediastop"),
	inputKeyMapType::value_type(DIK_VOLUMEDOWN,"volumedown"),
	inputKeyMapType::value_type(DIK_VOLUMEUP,"volumeup"),
	inputKeyMapType::value_type(DIK_WEBHOME,"webhome"),
	inputKeyMapType::value_type(DIK_NUMPADCOMMA,"numpad,"),
	inputKeyMapType::value_type(DIK_DIVIDE,"numpad/"),
	inputKeyMapType::value_type(DIK_SYSRQ,"print"),
	inputKeyMapType::value_type(DIK_RMENU,"ralt_"),
	inputKeyMapType::value_type(DIK_PAUSE,"pause"),
	inputKeyMapType::value_type(DIK_HOME,"home"),
	inputKeyMapType::value_type(DIK_UP,"up"),
	inputKeyMapType::value_type(DIK_PRIOR,"pageup"),
	inputKeyMapType::value_type(DIK_LEFT,"left"),
	inputKeyMapType::value_type(DIK_RIGHT,"right"),
	inputKeyMapType::value_type(DIK_END,"end"),
	inputKeyMapType::value_type(DIK_DOWN,"down"),
	inputKeyMapType::value_type(DIK_NEXT,"pagedown"),
	inputKeyMapType::value_type(DIK_INSERT,"insert"),
	inputKeyMapType::value_type(DIK_DELETE,"delete"),
	inputKeyMapType::value_type(DIK_LWIN,"lwin"),
	inputKeyMapType::value_type(DIK_RWIN,"rwin"),
	inputKeyMapType::value_type(DIK_APPS,"apps"),
	inputKeyMapType::value_type(DIK_POWER,"power"),
	inputKeyMapType::value_type(DIK_SLEEP,"sleep"),
	inputKeyMapType::value_type(DIK_WAKE,"wake"),
	inputKeyMapType::value_type(DIK_WEBSEARCH,"websearch"),
	inputKeyMapType::value_type(DIK_WEBFAVORITES,"webfavorites"),
	inputKeyMapType::value_type(DIK_WEBREFRESH,"webrefresh"),
	inputKeyMapType::value_type(DIK_WEBSTOP,"webstop"),
	inputKeyMapType::value_type(DIK_WEBFORWARD,"webforward"),
	inputKeyMapType::value_type(DIK_WEBBACK,"webback"),
	inputKeyMapType::value_type(DIK_MYCOMPUTER,"mycomputer"),
	inputKeyMapType::value_type(DIK_MAIL,"mail"),
	inputKeyMapType::value_type(DIK_MEDIASELECT,"mediaselect"),
};
inputKeyMapType Keyboard::s_directInputKeyMap(directInputKeyMapData, directInputKeyMapData + (sizeof directInputKeyMapData / sizeof directInputKeyMapData[0]));

const inputKeyMapType::value_type virtualKeyMapData[] = {
	inputKeyMapType::value_type(VK_ESCAPE,"esc"),
	inputKeyMapType::value_type(0x30,"0"),
	inputKeyMapType::value_type(0x31,"1"),
	inputKeyMapType::value_type(0x32,"2"),
	inputKeyMapType::value_type(0x33,"3"),
	inputKeyMapType::value_type(0x34,"4"),
	inputKeyMapType::value_type(0x35,"5"),
	inputKeyMapType::value_type(0x36,"6"),
	inputKeyMapType::value_type(0x37,"7"),
	inputKeyMapType::value_type(0x38,"9"),
	inputKeyMapType::value_type(0x39,"9"),
	inputKeyMapType::value_type(VK_OEM_MINUS,"-"),
	inputKeyMapType::value_type(VK_OEM_PLUS,"="),
	//inputKeyMapType::value_type("=", ),
	inputKeyMapType::value_type(VK_BACK,"back"),
	inputKeyMapType::value_type(VK_TAB,"tab"),
	inputKeyMapType::value_type(0x51,"q"),
	inputKeyMapType::value_type(0x57,"w"),
	inputKeyMapType::value_type(0x45,"e"),
	inputKeyMapType::value_type(0x52,"r"),
	inputKeyMapType::value_type(0x54,"t"),
	inputKeyMapType::value_type(0x59,"y"),
	inputKeyMapType::value_type(0x55,"u"),
	inputKeyMapType::value_type(0x49,"i"),
	inputKeyMapType::value_type(0x4F,"o"),
	inputKeyMapType::value_type(0x50,"p"),
	inputKeyMapType::value_type(VK_OEM_4,"lbracket"),
	inputKeyMapType::value_type(VK_OEM_6,"bracket"),
	inputKeyMapType::value_type(VK_RETURN,"enter"),
	inputKeyMapType::value_type(VK_CONTROL,"lctrl"),
	inputKeyMapType::value_type(0x41,"a"),
	inputKeyMapType::value_type(0x53,"s"),
	inputKeyMapType::value_type(0x44,"d"),
	inputKeyMapType::value_type(0x46,"f"),
	inputKeyMapType::value_type(0x47,"g"),
	inputKeyMapType::value_type(0x48,"h"),
	inputKeyMapType::value_type(0x4A,"j"),
	inputKeyMapType::value_type(0x4B,"k"),
	inputKeyMapType::value_type(0x4C,"l"),
	inputKeyMapType::value_type(VK_OEM_1,";"),
	inputKeyMapType::value_type(VK_OEM_7,"'"),
	inputKeyMapType::value_type(VK_OEM_3,"`"),
	inputKeyMapType::value_type(VK_SHIFT,"lshift"),
	inputKeyMapType::value_type(VK_OEM_5,"\\"),
	inputKeyMapType::value_type(0x5A,"z"),
	inputKeyMapType::value_type(0x58,"x"),
	inputKeyMapType::value_type(0x43,"c"),
	inputKeyMapType::value_type(0x56,"v"),
	inputKeyMapType::value_type(0x42,"b"),
	inputKeyMapType::value_type(0x4E,"n"),
	inputKeyMapType::value_type(0x4D,"m"),
	inputKeyMapType::value_type(VK_OEM_COMMA,","),
	inputKeyMapType::value_type(VK_OEM_PERIOD,"."),
	inputKeyMapType::value_type(VK_OEM_2,"/"),
	inputKeyMapType::value_type(VK_SHIFT,"rshift"),
	inputKeyMapType::value_type(VK_MULTIPLY,"numpad*"),
	inputKeyMapType::value_type(VK_MENU,"lalt"),
	inputKeyMapType::value_type(VK_SPACE," "),
	inputKeyMapType::value_type(VK_CAPITAL,"caps"),
	inputKeyMapType::value_type(VK_F1,"f1"),
	inputKeyMapType::value_type(VK_F2,"f2"),
	inputKeyMapType::value_type(VK_F3,"f3"),
	inputKeyMapType::value_type(VK_F4,"f4"),
	inputKeyMapType::value_type(VK_F5,"f5"),
	inputKeyMapType::value_type(VK_F6,"f6"),
	inputKeyMapType::value_type(VK_F7,"f7"),
	inputKeyMapType::value_type(VK_F8,"f8"),
	inputKeyMapType::value_type(VK_F9,"f9"),
	inputKeyMapType::value_type(VK_F10,"f10"),
	inputKeyMapType::value_type(VK_NUMLOCK,"numlock"),
	inputKeyMapType::value_type(VK_SCROLL,"scrolllock"),
	inputKeyMapType::value_type(VK_NUMPAD7,"numpad7"),
	inputKeyMapType::value_type(VK_NUMPAD8,"numpad8"),
	inputKeyMapType::value_type(VK_NUMPAD9,"numpad9"),
	inputKeyMapType::value_type(VK_SUBTRACT,"numpad-"),
	inputKeyMapType::value_type(VK_NUMPAD4,"numpad4"),
	inputKeyMapType::value_type(VK_NUMPAD5,"numpad5"),
	inputKeyMapType::value_type(VK_NUMPAD6,"numpad6"),
	inputKeyMapType::value_type(VK_ADD,"numpad+"),
	inputKeyMapType::value_type(VK_NUMPAD1,"numpad1"),
	inputKeyMapType::value_type(VK_NUMPAD2,"numpad2"),
	inputKeyMapType::value_type(VK_NUMPAD3,"numpad3"),
	inputKeyMapType::value_type(VK_NUMPAD0,"numpad0"),
	inputKeyMapType::value_type(VK_DECIMAL,"numpad."),
	inputKeyMapType::value_type(VK_OEM_102,"\\"),
	inputKeyMapType::value_type(VK_F11,"f11"),
	inputKeyMapType::value_type(VK_F12,"f12"),
	inputKeyMapType::value_type(VK_F13,"f13"),
	inputKeyMapType::value_type(VK_F14,"f14"),
	inputKeyMapType::value_type(VK_F15,"f15"),
	inputKeyMapType::value_type(VK_KANA,"kana"),
	//inputKeyMapType::value_type("brazilian?", ),
	inputKeyMapType::value_type(VK_CONVERT,"convert"),
	inputKeyMapType::value_type(VK_NONCONVERT,"noconvert"),
	//inputKeyMapType::value_type("yen", ),
	//inputKeyMapType::value_type("brazilian.", ),
	inputKeyMapType::value_type(VK_OEM_NEC_EQUAL,"numpad="),
	inputKeyMapType::value_type(VK_MEDIA_PREV_TRACK,"prevtrack"),
	//inputKeyMapType::value_type("at", ),
	//inputKeyMapType::value_type("japanese,", ),
	//inputKeyMapType::value_type("_", ),
	inputKeyMapType::value_type(VK_KANJI,"kanji"),
	inputKeyMapType::value_type(VK_MEDIA_STOP,"stop"),
	inputKeyMapType::value_type(VK_OEM_AX,"ax"),
	//inputKeyMapType::value_type("unlabeled", ),
	inputKeyMapType::value_type(VK_MEDIA_NEXT_TRACK,"nexttrack"),
	//inputKeyMapType::value_type("numpadenter", ),
	inputKeyMapType::value_type(VK_CONTROL,"rctrl"),
	inputKeyMapType::value_type(VK_VOLUME_MUTE,"mute"),
	inputKeyMapType::value_type(VK_LAUNCH_APP1,"calculator"),
	inputKeyMapType::value_type(VK_MEDIA_PLAY_PAUSE,"playpause"),
	inputKeyMapType::value_type(VK_MEDIA_STOP,"mediastop"),
	inputKeyMapType::value_type(VK_VOLUME_DOWN,"volumedown"),
	inputKeyMapType::value_type(VK_VOLUME_UP,"volumeup"),
	inputKeyMapType::value_type(VK_BROWSER_HOME,"webhome"),
	inputKeyMapType::value_type(VK_OEM_COMMA,"numpad,"),
	inputKeyMapType::value_type(VK_DIVIDE,"numpad/"),
	inputKeyMapType::value_type(VK_PRINT,"print"),
	inputKeyMapType::value_type(VK_MENU,"ralt_"),
	inputKeyMapType::value_type(VK_PAUSE,"pause"),
	inputKeyMapType::value_type(VK_HOME,"home"),
	inputKeyMapType::value_type(VK_UP,"up"),
	inputKeyMapType::value_type(VK_PRIOR,"pageup"),
	inputKeyMapType::value_type(VK_LEFT,"left"),
	inputKeyMapType::value_type(VK_RIGHT,"right"),
	inputKeyMapType::value_type(VK_END,"end"),
	inputKeyMapType::value_type(VK_DOWN,"down"),
	inputKeyMapType::value_type(VK_NEXT,"pagedown"),
	inputKeyMapType::value_type(VK_INSERT,"insert"),
	inputKeyMapType::value_type(VK_DELETE,"delete"),
	inputKeyMapType::value_type(VK_LWIN,"lwin"),
	inputKeyMapType::value_type(VK_RWIN,"rwin"),
	inputKeyMapType::value_type(VK_APPS,"apps"),
	//inputKeyMapType::value_type("power", ),
	inputKeyMapType::value_type(VK_SLEEP,"sleep"),
	//inputKeyMapType::value_type("wake", ),
	inputKeyMapType::value_type(VK_BROWSER_SEARCH,"websearch"),
	inputKeyMapType::value_type(VK_BROWSER_FAVORITES,"webfavorites"),
	inputKeyMapType::value_type(VK_BROWSER_REFRESH,"webrefresh"),
	inputKeyMapType::value_type(VK_BROWSER_STOP,"webstop"),
	inputKeyMapType::value_type(VK_BROWSER_FORWARD,"webforward"),
	inputKeyMapType::value_type(VK_BROWSER_BACK,"webback"),
	//inputKeyMapType::value_type("mycomputer", ),
	inputKeyMapType::value_type(VK_LAUNCH_MAIL,"mail"),
	inputKeyMapType::value_type(VK_LAUNCH_MEDIA_SELECT,"mediaselect"),
};
inputKeyMapType Keyboard::s_virtualKeyMap(virtualKeyMapData, virtualKeyMapData + (sizeof virtualKeyMapData / sizeof virtualKeyMapData[0]));

const inputKeyMapType::value_type virtualKeyMapData_DE[] = {
	inputKeyMapType::value_type(VK_ESCAPE,"esc"),
	inputKeyMapType::value_type(0x30,"0"),
	inputKeyMapType::value_type(0x31,"1"),
	inputKeyMapType::value_type(0x32,"2"),
	inputKeyMapType::value_type(0x33,"3"),
	inputKeyMapType::value_type(0x34,"4"),
	inputKeyMapType::value_type(0x35,"5"),
	inputKeyMapType::value_type(0x36,"6"),
	inputKeyMapType::value_type(0x37,"7"),
	inputKeyMapType::value_type(0x38,"8"),
	inputKeyMapType::value_type(0x39,"9"),
	inputKeyMapType::value_type(VK_OEM_MINUS,"-"),
	inputKeyMapType::value_type(VK_OEM_PLUS,"+"),
	//inputKeyMapType::value_type("=", ),
	inputKeyMapType::value_type(VK_BACK,"back"),
	inputKeyMapType::value_type(VK_TAB,"tab"),
	inputKeyMapType::value_type(0x51,"q"),
	inputKeyMapType::value_type(0x57,"w"),
	inputKeyMapType::value_type(0x45,"e"),
	inputKeyMapType::value_type(0x52,"r"),
	inputKeyMapType::value_type(0x54,"t"),
	inputKeyMapType::value_type(0x59,"y"),
	inputKeyMapType::value_type(0x55,"u"),
	inputKeyMapType::value_type(0x49,"i"),
	inputKeyMapType::value_type(0x4F,"o"),
	inputKeyMapType::value_type(0x50,"p"),
	inputKeyMapType::value_type(VK_OEM_4,"ß"),
	inputKeyMapType::value_type(VK_OEM_6,"´"),
	inputKeyMapType::value_type(VK_RETURN,"enter"),
	inputKeyMapType::value_type(VK_CONTROL,"lctrl"),
	inputKeyMapType::value_type(0x41,"a"),
	inputKeyMapType::value_type(0x53,"s"),
	inputKeyMapType::value_type(0x44,"d"),
	inputKeyMapType::value_type(0x46,"f"),
	inputKeyMapType::value_type(0x47,"g"),
	inputKeyMapType::value_type(0x48,"h"),
	inputKeyMapType::value_type(0x4A,"j"),
	inputKeyMapType::value_type(0x4B,"k"),
	inputKeyMapType::value_type(0x4C,"l"),
	inputKeyMapType::value_type(VK_OEM_1,"ü"),
	inputKeyMapType::value_type(VK_OEM_7,"ä"),
	inputKeyMapType::value_type(VK_OEM_3,"ö"),
	inputKeyMapType::value_type(VK_SHIFT,"lshift"),
	inputKeyMapType::value_type(VK_OEM_5,"^"),
	inputKeyMapType::value_type(0x5A,"z"),
	inputKeyMapType::value_type(0x58,"x"),
	inputKeyMapType::value_type(0x43,"c"),
	inputKeyMapType::value_type(0x56,"v"),
	inputKeyMapType::value_type(0x42,"b"),
	inputKeyMapType::value_type(0x4E,"n"),
	inputKeyMapType::value_type(0x4D,"m"),
	inputKeyMapType::value_type(VK_OEM_COMMA,","),
	inputKeyMapType::value_type(VK_OEM_PERIOD,"."),
	inputKeyMapType::value_type(VK_OEM_2,"#"),
	inputKeyMapType::value_type(VK_SHIFT,"rshift"),
	inputKeyMapType::value_type(VK_MULTIPLY,"numpad*"),
	inputKeyMapType::value_type(VK_MENU,"lalt"),
	inputKeyMapType::value_type(VK_SPACE," "),
	inputKeyMapType::value_type(VK_CAPITAL,"caps"),
	inputKeyMapType::value_type(VK_F1,"f1"),
	inputKeyMapType::value_type(VK_F2,"f2"),
	inputKeyMapType::value_type(VK_F3,"f3"),
	inputKeyMapType::value_type(VK_F4,"f4"),
	inputKeyMapType::value_type(VK_F5,"f5"),
	inputKeyMapType::value_type(VK_F6,"f6"),
	inputKeyMapType::value_type(VK_F7,"f7"),
	inputKeyMapType::value_type(VK_F8,"f8"),
	inputKeyMapType::value_type(VK_F9,"f9"),
	inputKeyMapType::value_type(VK_F10,"f10"),
	inputKeyMapType::value_type(VK_NUMLOCK,"numlock"),
	inputKeyMapType::value_type(VK_SCROLL,"scrolllock"),
	inputKeyMapType::value_type(VK_NUMPAD7,"numpad7"),
	inputKeyMapType::value_type(VK_NUMPAD8,"numpad8"),
	inputKeyMapType::value_type(VK_NUMPAD9,"numpad9"),
	inputKeyMapType::value_type(VK_SUBTRACT,"numpad-"),
	inputKeyMapType::value_type(VK_NUMPAD4,"numpad4"),
	inputKeyMapType::value_type(VK_NUMPAD5,"numpad5"),
	inputKeyMapType::value_type(VK_NUMPAD6,"numpad6"),
	inputKeyMapType::value_type(VK_ADD,"numpad+"),
	inputKeyMapType::value_type(VK_NUMPAD1,"numpad1"),
	inputKeyMapType::value_type(VK_NUMPAD2,"numpad2"),
	inputKeyMapType::value_type(VK_NUMPAD3,"numpad3"),
	inputKeyMapType::value_type(VK_NUMPAD0,"numpad0"),
	inputKeyMapType::value_type(VK_DECIMAL,"numpad."),
	inputKeyMapType::value_type(VK_OEM_102,"oem102"),
	inputKeyMapType::value_type(VK_F11,"f11"),
	inputKeyMapType::value_type(VK_F12,"f12"),
	inputKeyMapType::value_type(VK_F13,"f13"),
	inputKeyMapType::value_type(VK_F14,"f14"),
	inputKeyMapType::value_type(VK_F15,"f15"),
	inputKeyMapType::value_type(VK_KANA,"kana"),
	//inputKeyMapType::value_type("brazilian?", ),
	inputKeyMapType::value_type(VK_CONVERT,"convert"),
	inputKeyMapType::value_type(VK_NONCONVERT,"noconvert"),
	//inputKeyMapType::value_type("yen", ),
	//inputKeyMapType::value_type("brazilian.", ),
	inputKeyMapType::value_type(VK_OEM_NEC_EQUAL,"numpad="),
	inputKeyMapType::value_type(VK_MEDIA_PREV_TRACK,"prevtrack"),
	//inputKeyMapType::value_type("at", ),
	//inputKeyMapType::value_type("japanese,", ),
	//inputKeyMapType::value_type("_", ),
	inputKeyMapType::value_type(VK_KANJI,"kanji"),
	inputKeyMapType::value_type(VK_MEDIA_STOP,"stop"),
	inputKeyMapType::value_type(VK_OEM_AX,"ax"),
	//inputKeyMapType::value_type("unlabeled", ),
	inputKeyMapType::value_type(VK_MEDIA_NEXT_TRACK,"nexttrack"),
	//inputKeyMapType::value_type("numpadenter", ),
	inputKeyMapType::value_type(VK_CONTROL,"rctrl"),
	inputKeyMapType::value_type(VK_VOLUME_MUTE,"mute"),
	inputKeyMapType::value_type(VK_LAUNCH_APP1,"calculator"),
	inputKeyMapType::value_type(VK_MEDIA_PLAY_PAUSE,"playpause"),
	inputKeyMapType::value_type(VK_MEDIA_STOP,"mediastop"),
	inputKeyMapType::value_type(VK_VOLUME_DOWN,"volumedown"),
	inputKeyMapType::value_type(VK_VOLUME_UP,"volumeup"),
	inputKeyMapType::value_type(VK_BROWSER_HOME,"webhome"),
	inputKeyMapType::value_type(VK_OEM_COMMA,"numpad,"),
	inputKeyMapType::value_type(VK_DIVIDE,"numpad/"),
	inputKeyMapType::value_type(VK_PRINT,"print"),
	inputKeyMapType::value_type(VK_MENU,"ralt_"),
	inputKeyMapType::value_type(VK_PAUSE,"pause"),
	inputKeyMapType::value_type(VK_HOME,"home"),
	inputKeyMapType::value_type(VK_UP,"up"),
	inputKeyMapType::value_type(VK_PRIOR,"pageup"),
	inputKeyMapType::value_type(VK_LEFT,"left"),
	inputKeyMapType::value_type(VK_RIGHT,"right"),
	inputKeyMapType::value_type(VK_END,"end"),
	inputKeyMapType::value_type(VK_DOWN,"down"),
	inputKeyMapType::value_type(VK_NEXT,"pagedown"),
	inputKeyMapType::value_type(VK_INSERT,"insert"),
	inputKeyMapType::value_type(VK_DELETE,"delete"),
	inputKeyMapType::value_type(VK_LWIN,"lwin"),
	inputKeyMapType::value_type(VK_RWIN,"rwin"),
	inputKeyMapType::value_type(VK_APPS,"apps"),
	//inputKeyMapType::value_type("power", ),
	inputKeyMapType::value_type(VK_SLEEP,"sleep"),
	//inputKeyMapType::value_type("wake", ),
	inputKeyMapType::value_type(VK_BROWSER_SEARCH,"websearch"),
	inputKeyMapType::value_type(VK_BROWSER_FAVORITES,"webfavorites"),
	inputKeyMapType::value_type(VK_BROWSER_REFRESH,"webrefresh"),
	inputKeyMapType::value_type(VK_BROWSER_STOP,"webstop"),
	inputKeyMapType::value_type(VK_BROWSER_FORWARD,"webforward"),
	inputKeyMapType::value_type(VK_BROWSER_BACK,"webback"),
	//inputKeyMapType::value_type("mycomputer", ),
	inputKeyMapType::value_type(VK_LAUNCH_MAIL,"mail"),
	inputKeyMapType::value_type(VK_LAUNCH_MEDIA_SELECT,"mediaselect"),
};
inputKeyMapType Keyboard::s_virtualKeyMap_DE(virtualKeyMapData_DE, virtualKeyMapData_DE + (sizeof virtualKeyMapData_DE / sizeof virtualKeyMapData[0]));

const inputKeyMapType::value_type scanCodeKeyMapData[] = {

	inputKeyMapType::value_type(MapVirtualKey(VK_ESCAPE, MAPVK_VK_TO_VSC),"esc"),
	inputKeyMapType::value_type(MapVirtualKey(0x30, MAPVK_VK_TO_VSC),"1"),
	inputKeyMapType::value_type(MapVirtualKey(0x31, MAPVK_VK_TO_VSC),"2"),
	inputKeyMapType::value_type(MapVirtualKey(0x32, MAPVK_VK_TO_VSC),"3"),
	inputKeyMapType::value_type(MapVirtualKey(0x33, MAPVK_VK_TO_VSC),"4"),
	inputKeyMapType::value_type(MapVirtualKey(0x34, MAPVK_VK_TO_VSC),"5"),
	inputKeyMapType::value_type(MapVirtualKey(0x35, MAPVK_VK_TO_VSC),"6"),
	inputKeyMapType::value_type(MapVirtualKey(0x36, MAPVK_VK_TO_VSC),"7"),
	inputKeyMapType::value_type(MapVirtualKey(0x37, MAPVK_VK_TO_VSC),"8"),
	inputKeyMapType::value_type(MapVirtualKey(0x38, MAPVK_VK_TO_VSC),"9"),
	inputKeyMapType::value_type(MapVirtualKey(0x39, MAPVK_VK_TO_VSC),"0"),
	inputKeyMapType::value_type(MapVirtualKey(VK_OEM_MINUS, MAPVK_VK_TO_VSC),"-"),
	//inputKeyMapType::value_type("=", ),
	inputKeyMapType::value_type(MapVirtualKey(VK_BACK, MAPVK_VK_TO_VSC),"back"),
	inputKeyMapType::value_type(MapVirtualKey(VK_TAB, MAPVK_VK_TO_VSC),"tab"),
	inputKeyMapType::value_type(MapVirtualKey(0x51, MAPVK_VK_TO_VSC),"q"),
	inputKeyMapType::value_type(MapVirtualKey(0x57, MAPVK_VK_TO_VSC),"w"),
	inputKeyMapType::value_type(MapVirtualKey(0x45, MAPVK_VK_TO_VSC),"e"),
	inputKeyMapType::value_type(MapVirtualKey(0x52, MAPVK_VK_TO_VSC),"r"),
	inputKeyMapType::value_type(MapVirtualKey(0x54, MAPVK_VK_TO_VSC),"t"),
	inputKeyMapType::value_type(MapVirtualKey(0x59, MAPVK_VK_TO_VSC),"y"),
	inputKeyMapType::value_type(MapVirtualKey(0x55, MAPVK_VK_TO_VSC),"u"),
	inputKeyMapType::value_type(MapVirtualKey(0x49, MAPVK_VK_TO_VSC),"i"),
	inputKeyMapType::value_type(MapVirtualKey(0x4F, MAPVK_VK_TO_VSC),"o"),
	inputKeyMapType::value_type(MapVirtualKey(0x50, MAPVK_VK_TO_VSC),"p"),
	inputKeyMapType::value_type(MapVirtualKey(VK_OEM_4, MAPVK_VK_TO_VSC),"lbracket"),
	inputKeyMapType::value_type(MapVirtualKey(VK_OEM_6, MAPVK_VK_TO_VSC),"bracket"),
	inputKeyMapType::value_type(MapVirtualKey(VK_RETURN, MAPVK_VK_TO_VSC),"enter"),
	inputKeyMapType::value_type(MapVirtualKey(VK_LCONTROL, MAPVK_VK_TO_VSC),"lctrl"),
	inputKeyMapType::value_type(MapVirtualKey(0x41, MAPVK_VK_TO_VSC),"a"),
	inputKeyMapType::value_type(MapVirtualKey(0x53, MAPVK_VK_TO_VSC),"s"),
	inputKeyMapType::value_type(MapVirtualKey(0x44, MAPVK_VK_TO_VSC),"d"),
	inputKeyMapType::value_type(MapVirtualKey(0x46, MAPVK_VK_TO_VSC),"f"),
	inputKeyMapType::value_type(MapVirtualKey(0x47, MAPVK_VK_TO_VSC),"g"),
	inputKeyMapType::value_type(MapVirtualKey(0x48, MAPVK_VK_TO_VSC),"h"),
	inputKeyMapType::value_type(MapVirtualKey(0x4A, MAPVK_VK_TO_VSC),"j"),
	inputKeyMapType::value_type(MapVirtualKey(0x4B, MAPVK_VK_TO_VSC),"k"),
	inputKeyMapType::value_type(MapVirtualKey(0x4C, MAPVK_VK_TO_VSC),"l"),
	inputKeyMapType::value_type(MapVirtualKey(VK_OEM_1, MAPVK_VK_TO_VSC),";"),
	inputKeyMapType::value_type(MapVirtualKey(VK_OEM_7, MAPVK_VK_TO_VSC),"'"),
	inputKeyMapType::value_type(MapVirtualKey(VK_OEM_3, MAPVK_VK_TO_VSC),"`"),
	inputKeyMapType::value_type(MapVirtualKey(VK_LSHIFT, MAPVK_VK_TO_VSC),"lshift"),
	inputKeyMapType::value_type(MapVirtualKey(VK_OEM_5, MAPVK_VK_TO_VSC),"\\"),
	inputKeyMapType::value_type(MapVirtualKey(0x5A, MAPVK_VK_TO_VSC),"z"),
	inputKeyMapType::value_type(MapVirtualKey(0x58, MAPVK_VK_TO_VSC),"x"),
	inputKeyMapType::value_type(MapVirtualKey(0x43, MAPVK_VK_TO_VSC),"c"),
	inputKeyMapType::value_type(MapVirtualKey(0x56, MAPVK_VK_TO_VSC),"v"),
	inputKeyMapType::value_type(MapVirtualKey(0x42, MAPVK_VK_TO_VSC),"b"),
	inputKeyMapType::value_type(MapVirtualKey(0x4E, MAPVK_VK_TO_VSC),"n"),
	inputKeyMapType::value_type(MapVirtualKey(0x4D, MAPVK_VK_TO_VSC),"m"),
	inputKeyMapType::value_type(MapVirtualKey(VK_OEM_COMMA, MAPVK_VK_TO_VSC),","),
	inputKeyMapType::value_type(MapVirtualKey(VK_OEM_PERIOD, MAPVK_VK_TO_VSC),"."),
	inputKeyMapType::value_type(MapVirtualKey(VK_OEM_2, MAPVK_VK_TO_VSC),"/"),
	inputKeyMapType::value_type(MapVirtualKey(VK_RSHIFT, MAPVK_VK_TO_VSC),"rshift"),
	inputKeyMapType::value_type(MapVirtualKey(VK_MULTIPLY, MAPVK_VK_TO_VSC),"numpad*"),
	inputKeyMapType::value_type(MapVirtualKey(VK_LMENU, MAPVK_VK_TO_VSC),"lalt"),
	inputKeyMapType::value_type(MapVirtualKey(VK_SPACE, MAPVK_VK_TO_VSC)," "),
	inputKeyMapType::value_type(MapVirtualKey(VK_CAPITAL, MAPVK_VK_TO_VSC),"caps"),
	inputKeyMapType::value_type(MapVirtualKey(VK_F1, MAPVK_VK_TO_VSC),"f1"),
	inputKeyMapType::value_type(MapVirtualKey(VK_F2, MAPVK_VK_TO_VSC),"f2"),
	inputKeyMapType::value_type(MapVirtualKey(VK_F3, MAPVK_VK_TO_VSC),"f3"),
	inputKeyMapType::value_type(MapVirtualKey(VK_F4, MAPVK_VK_TO_VSC),"f4"),
	inputKeyMapType::value_type(MapVirtualKey(VK_F5, MAPVK_VK_TO_VSC),"f5"),
	inputKeyMapType::value_type(MapVirtualKey(VK_F6, MAPVK_VK_TO_VSC),"f6"),
	inputKeyMapType::value_type(MapVirtualKey(VK_F7, MAPVK_VK_TO_VSC),"f7"),
	inputKeyMapType::value_type(MapVirtualKey(VK_F8, MAPVK_VK_TO_VSC),"f8"),
	inputKeyMapType::value_type(MapVirtualKey(VK_F9, MAPVK_VK_TO_VSC),"f9"),
	inputKeyMapType::value_type(MapVirtualKey(VK_F10, MAPVK_VK_TO_VSC),"f10"),
	inputKeyMapType::value_type(MapVirtualKey(VK_NUMLOCK, MAPVK_VK_TO_VSC),"numlock"),
	inputKeyMapType::value_type(MapVirtualKey(VK_SCROLL, MAPVK_VK_TO_VSC),"scrolllock"),
	inputKeyMapType::value_type(MapVirtualKey(VK_NUMPAD7, MAPVK_VK_TO_VSC),"numpad7"),
	inputKeyMapType::value_type(MapVirtualKey(VK_NUMPAD8, MAPVK_VK_TO_VSC),"numpad8"),
	inputKeyMapType::value_type(MapVirtualKey(VK_NUMPAD9, MAPVK_VK_TO_VSC),"numpad9"),
	inputKeyMapType::value_type(MapVirtualKey(VK_SUBTRACT, MAPVK_VK_TO_VSC),"numpad-"),
	inputKeyMapType::value_type(MapVirtualKey(VK_NUMPAD4, MAPVK_VK_TO_VSC),"numpad4"),
	inputKeyMapType::value_type(MapVirtualKey(VK_NUMPAD5, MAPVK_VK_TO_VSC),"numpad5"),
	inputKeyMapType::value_type(MapVirtualKey(VK_NUMPAD6, MAPVK_VK_TO_VSC),"numpad6"),
	inputKeyMapType::value_type(MapVirtualKey(VK_ADD, MAPVK_VK_TO_VSC),"numpad+"),
	inputKeyMapType::value_type(MapVirtualKey(VK_NUMPAD1, MAPVK_VK_TO_VSC),"numpad1"),
	inputKeyMapType::value_type(MapVirtualKey(VK_NUMPAD2, MAPVK_VK_TO_VSC),"numpad2"),
	inputKeyMapType::value_type(MapVirtualKey(VK_NUMPAD3, MAPVK_VK_TO_VSC),"numpad3"),
	inputKeyMapType::value_type(MapVirtualKey(VK_NUMPAD0, MAPVK_VK_TO_VSC),"numpad0"),
	inputKeyMapType::value_type(MapVirtualKey(VK_DECIMAL, MAPVK_VK_TO_VSC),"numpad."),
	inputKeyMapType::value_type(MapVirtualKey(VK_OEM_102, MAPVK_VK_TO_VSC),"oem102"),
	inputKeyMapType::value_type(MapVirtualKey(VK_F11, MAPVK_VK_TO_VSC),"f11"),
	inputKeyMapType::value_type(MapVirtualKey(VK_F12, MAPVK_VK_TO_VSC),"f12"),
	inputKeyMapType::value_type(MapVirtualKey(VK_F13, MAPVK_VK_TO_VSC),"f13"),
	inputKeyMapType::value_type(MapVirtualKey(VK_F14, MAPVK_VK_TO_VSC),"f14"),
	inputKeyMapType::value_type(MapVirtualKey(VK_F15, MAPVK_VK_TO_VSC),"f15"),
	inputKeyMapType::value_type(MapVirtualKey(VK_KANA, MAPVK_VK_TO_VSC),"kana"),
	//inputKeyMapType::value_type("brazilian?", ),
	inputKeyMapType::value_type(MapVirtualKey(VK_CONVERT, MAPVK_VK_TO_VSC),"convert"),
	inputKeyMapType::value_type(MapVirtualKey(VK_NONCONVERT, MAPVK_VK_TO_VSC),"noconvert"),
	//inputKeyMapType::value_type("yen", ),
	//inputKeyMapType::value_type("brazilian.", ),
	inputKeyMapType::value_type(MapVirtualKey(VK_OEM_NEC_EQUAL, MAPVK_VK_TO_VSC),"numpad="),
	inputKeyMapType::value_type(MapVirtualKey(VK_MEDIA_PREV_TRACK, MAPVK_VK_TO_VSC),"prevtrack"),
	//inputKeyMapType::value_type("at", ),
	//inputKeyMapType::value_type("japanese,", ),
	//inputKeyMapType::value_type("_", ),
	inputKeyMapType::value_type(MapVirtualKey(VK_KANJI, MAPVK_VK_TO_VSC),"kanji"),
	inputKeyMapType::value_type(MapVirtualKey(VK_MEDIA_STOP, MAPVK_VK_TO_VSC),"stop"),
	inputKeyMapType::value_type(MapVirtualKey(VK_OEM_AX, MAPVK_VK_TO_VSC),"ax"),
	//inputKeyMapType::value_type("unlabeled", ),
	inputKeyMapType::value_type(MapVirtualKey(VK_MEDIA_NEXT_TRACK, MAPVK_VK_TO_VSC),"nexttrack"),
	//inputKeyMapType::value_type("numpadenter", ),
	inputKeyMapType::value_type(MapVirtualKey(VK_RCONTROL, MAPVK_VK_TO_VSC),"rctrl"),
	inputKeyMapType::value_type(MapVirtualKey(VK_VOLUME_MUTE, MAPVK_VK_TO_VSC),"mute"),
	inputKeyMapType::value_type(MapVirtualKey(VK_LAUNCH_APP1, MAPVK_VK_TO_VSC),"calculator"),
	inputKeyMapType::value_type(MapVirtualKey(VK_MEDIA_PLAY_PAUSE, MAPVK_VK_TO_VSC),"playpause"),
	inputKeyMapType::value_type(MapVirtualKey(VK_MEDIA_STOP, MAPVK_VK_TO_VSC),"mediastop"),
	inputKeyMapType::value_type(MapVirtualKey(VK_VOLUME_DOWN, MAPVK_VK_TO_VSC),"volumedown"),
	inputKeyMapType::value_type(MapVirtualKey(VK_VOLUME_UP, MAPVK_VK_TO_VSC),"volumeup"),
	inputKeyMapType::value_type(MapVirtualKey(VK_BROWSER_HOME, MAPVK_VK_TO_VSC),"webhome"),
	inputKeyMapType::value_type(MapVirtualKey(VK_OEM_COMMA, MAPVK_VK_TO_VSC),"numpad,"),
	inputKeyMapType::value_type(MapVirtualKey(VK_DIVIDE, MAPVK_VK_TO_VSC),"numpad/"),
	inputKeyMapType::value_type(MapVirtualKey(VK_PRINT, MAPVK_VK_TO_VSC),"print"),
	inputKeyMapType::value_type(MapVirtualKey(VK_RMENU, MAPVK_VK_TO_VSC),"ralt_"),
	inputKeyMapType::value_type(MapVirtualKey(VK_PAUSE, MAPVK_VK_TO_VSC),"pause"),
	inputKeyMapType::value_type(MapVirtualKey(VK_HOME, MAPVK_VK_TO_VSC),"home"),
	inputKeyMapType::value_type(MapVirtualKey(VK_UP, MAPVK_VK_TO_VSC),"up"),
	inputKeyMapType::value_type(MapVirtualKey(VK_PRIOR, MAPVK_VK_TO_VSC),"pageup"),
	inputKeyMapType::value_type(MapVirtualKey(VK_LEFT, MAPVK_VK_TO_VSC),"left"),
	inputKeyMapType::value_type(MapVirtualKey(VK_RIGHT, MAPVK_VK_TO_VSC),"right"),
	inputKeyMapType::value_type(MapVirtualKey(VK_END, MAPVK_VK_TO_VSC),"end"),
	inputKeyMapType::value_type(MapVirtualKey(VK_DOWN, MAPVK_VK_TO_VSC),"down"),
	inputKeyMapType::value_type(MapVirtualKey(VK_NEXT, MAPVK_VK_TO_VSC),"pagedown"),
	inputKeyMapType::value_type(MapVirtualKey(VK_INSERT, MAPVK_VK_TO_VSC),"insert"),
	inputKeyMapType::value_type(MapVirtualKey(VK_DELETE, MAPVK_VK_TO_VSC),"delete"),
	inputKeyMapType::value_type(MapVirtualKey(VK_LWIN, MAPVK_VK_TO_VSC),"lwin"),
	inputKeyMapType::value_type(MapVirtualKey(VK_RWIN, MAPVK_VK_TO_VSC),"rwin"),
	inputKeyMapType::value_type(MapVirtualKey(VK_APPS, MAPVK_VK_TO_VSC),"apps"),
	//inputKeyMapType::value_type("power", ),
	inputKeyMapType::value_type(MapVirtualKey(VK_SLEEP, MAPVK_VK_TO_VSC),"sleep"),
	//inputKeyMapType::value_type("wake", ),
	inputKeyMapType::value_type(MapVirtualKey(VK_BROWSER_SEARCH, MAPVK_VK_TO_VSC),"websearch"),
	inputKeyMapType::value_type(MapVirtualKey(VK_BROWSER_FAVORITES, MAPVK_VK_TO_VSC),"webfavorites"),
	inputKeyMapType::value_type(MapVirtualKey(VK_BROWSER_REFRESH, MAPVK_VK_TO_VSC),"webrefresh"),
	inputKeyMapType::value_type(MapVirtualKey(VK_BROWSER_STOP, MAPVK_VK_TO_VSC),"webstop"),
	inputKeyMapType::value_type(MapVirtualKey(VK_BROWSER_FORWARD, MAPVK_VK_TO_VSC),"webforward"),
	inputKeyMapType::value_type(MapVirtualKey(VK_BROWSER_BACK, MAPVK_VK_TO_VSC),"webback"),
	//inputKeyMapType::value_type("mycomputer", ),
	inputKeyMapType::value_type(MapVirtualKey(VK_LAUNCH_MAIL, MAPVK_VK_TO_VSC),"mail"),
	inputKeyMapType::value_type(MapVirtualKey(VK_LAUNCH_MEDIA_SELECT, MAPVK_VK_TO_VSC),"mediaselect"),
};
inputKeyMapType Keyboard::s_scanCodeKeyMap(scanCodeKeyMapData, scanCodeKeyMapData + (sizeof scanCodeKeyMapData / sizeof scanCodeKeyMapData[0]));


}
