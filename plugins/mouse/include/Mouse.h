// Mouse.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/03/06
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

// Provides live recording from Alive Heart Rate Monitor via Bluetooth

#pragma once

#ifndef SSI_SENSOR_MOUSESENSOR_H
#define	SSI_SENSOR_MOUSESENSOR_H

#include "base/ISensor.h"
#include "base/IProvider.h"
#include "thread/Timer.h"
#include "thread/Thread.h"
#include "ioput/option/OptionList.h"
#include "event/EventAddress.h"

#if __gnu_linux__
	#include <X11/extensions/XInput2.h>

	#include<X11/Xlib.h>
	#include<X11/Xutil.h>
        #define VK_LBUTTON 2
        #define VK_MBUTTON 4
        #define VK_RBUTTON 8
	typedef struct point
					{
				double x;
				double y;
				}POINT;
	typedef Window X11Window;
	

static X11Window create_win2(Display *dpy);
	
	
#endif
namespace ssi {

#define SSI_MOUSE_CURSOR_SAMPLE_TYPE		ssi_real_t
#define SSI_MOUSE_CURSOR_PROVIDER_NAME		"cursor"
#define SSI_MOUSE_BUTTON_SAMPLE_TYPE		ssi_int_t
#define SSI_MOUSE_BUTTON_PROVIDER_NAME		"button"

#define	SSI_MOUSE_BUTTON_MASK				1

class Mouse : public ISensor, public Thread {

public:

	class CursorChannel : public IChannel {

		friend class Mouse;

		public:

			CursorChannel () {
				ssi_stream_init (stream, 0, 2, sizeof (SSI_MOUSE_CURSOR_SAMPLE_TYPE), SSI_REAL, 0);
			}
			~CursorChannel () {
				ssi_stream_destroy (stream);
			}

			const ssi_char_t *getName () { return SSI_MOUSE_CURSOR_PROVIDER_NAME; };
			const ssi_char_t *getInfo () { return "Reports mouse cursor as float values either as raw pixel positions or scaled in the interval [0..1]."; };
			ssi_stream_t getStream () { return stream; };

		protected:
			ssi_stream_t stream;
	};

	class ButtonChannel : public IChannel {

		friend class Mouse;

		public:

			ButtonChannel () {
				ssi_stream_init (stream, 0, 1, sizeof (SSI_MOUSE_BUTTON_SAMPLE_TYPE), SSI_INT, 0);
			}
			~ButtonChannel () {
				ssi_stream_destroy (stream);
			}

			const ssi_char_t *getName () { return SSI_MOUSE_BUTTON_PROVIDER_NAME; };
			const ssi_char_t *getInfo () { return "Reports button events according to the virtual-key codes used by the system."; };
			ssi_stream_t getStream () { return stream; };

		protected:
			ssi_stream_t stream;
	};

public:

	enum MASK {
		NONE		= 0,		
		LEFT		= VK_LBUTTON,
		MIDDLE		= VK_MBUTTON,
		RIGHT		= VK_RBUTTON,
	}; // for more see http://msdn.microsoft.com/en-us/library/windows/desktop/dd375731%28v=vs.85%29.aspx

	class Options : public OptionList {

	public:

		Options ()
			: sr (50.0), size (0.2), scale (true), flip (true), single (false), mask (NONE), sendEvent(false) {
			
			addOption ("sr", &sr, 1, SSI_DOUBLE, "sample rate in Hz");
			addOption ("size", &size, 1, SSI_DOUBLE, "block size in seconds");		
			addOption ("scale", &scale, 1, SSI_BOOL, "scale cursor position to interval [0..1]");
			addOption ("flip", &flip, 1, SSI_BOOL, "flip cursor vertically (origin is set to the bottom left corner of the screen)");
			addOption ("mask", &mask, 1, SSI_INT, "or'ed button mask (0=none, 1=left, 2=right, 4=middle)");
			addOption ("single", &single, 1, SSI_BOOL, "only mark when button is pressed");			
			addOption ("event", &sendEvent, 1, SSI_BOOL, "send an event when button after is pressed and released");			
			SSI_OPTIONLIST_ADD_ADDRESS(address);

			setAddress("click@mouse");
		};

		void setAddress(const ssi_char_t *address) {
			if (address) {
				ssi_strcpy(this->address, address);
			}
		}

		ssi_time_t sr;
		ssi_time_t size;
		bool scale;
		bool flip;		
		ssi_size_t mask;
		bool single;

		bool sendEvent;
		ssi_char_t address[SSI_MAX_CHAR];
	};

public:

	static const ssi_char_t *GetCreateName () { return "Mouse"; };
	static IObject *Create (const ssi_char_t *file) { return new Mouse (file); };
	~Mouse ();
	Mouse::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Provides mouse cursor position and button state."; };

	ssi_size_t getChannelSize () { return 2; };
	IChannel *getChannel (ssi_size_t index) { if (index == 0) return &_cursor_channel; if (index == 1) return &_button_channel; return 0; };
	bool setProvider (const ssi_char_t *name, IProvider *provider);
	bool connect ();
	bool start () { return Thread::start (); };
	bool stop () { return Thread::stop (); };
	void run ();
	bool disconnect ();

	bool setEventListener(IEventListener *listener);
	const ssi_char_t *getEventAddress() {
		return _eaddress.getAddress();
	}

	void setLogLevel (int level) {
		ssi_log_level = level;
	}
	
	#if __gnu_linux__
	
		double x,y,x2,y2;
        Display *display_name;
        X11Window root,win,win2,root2;
        int screen;
        int corep;
		XIButtonState buttons;
		XIModifierState mod;
		XIGroupState group;
	#endif
protected:

	Mouse (const ssi_char_t *file = 0);
	Mouse::Options _options;
	ssi_char_t *_file;

	int ssi_log_level;

	CursorChannel _cursor_channel;
	ButtonChannel _button_channel;
	void setCursorProvider (IProvider *cursor_provider);
	void setButtonProvider (IProvider *button_provider);

	POINT _cursor_pos;
	IProvider *_cursor_provider, *_button_provider;
	ssi_size_t _frame_size;
	SSI_MOUSE_CURSOR_SAMPLE_TYPE *_cursor_buffer, *_cursor_buffer_ptr;
	SSI_MOUSE_BUTTON_SAMPLE_TYPE *_button_buffer, *_button_buffer_ptr;
	bool _is_providing;

	ssi_size_t _counter;
	Timer *_timer;

	SSI_MOUSE_BUTTON_SAMPLE_TYPE _button_value;
	bool _button_on;
	ssi_size_t _event_start_time;

	SSI_MOUSE_CURSOR_SAMPLE_TYPE _max_x, _max_y;

	void sendEvent(bool continued);
	IEventListener *_elistener;
	ssi_event_t _event;
	EventAddress _eaddress;
};

}

#endif

