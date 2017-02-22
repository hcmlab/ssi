// EventSlider.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/05/26
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

#ifndef SSI_EVENT_EVENTSLIDER_H
#define SSI_EVENT_EVENTSLIDER_H

#include "base/IObject.h"
#include "ioput/option/OptionList.h"
#include "event/EventAddress.h"
#include "base/IWindow.h"
#include "graphic/Slider.h"
namespace ssi {

class EventSlider : public IObject, public Slider::ICallback {

public:

	class Options : public OptionList {

	public:

		Options ()
		: steps (100), defval (0.5f), minval (0.0f), maxval (1.0f) {

			pos[0] = 0;
			pos[1] = 0;
			pos[2] = 300;
			pos[3] = 75;

			setSender ("tsender");
			setEvent ("tevent");
			setValue ("value");

			addOption ("pos", &pos, 4, SSI_INT, "position of monitor on screen [posx,posy,width,height]");
			addOption ("sname", sname, SSI_MAX_CHAR, SSI_CHAR, "name of sender (if sent to event board)");
			addOption ("ename", ename, SSI_MAX_CHAR, SSI_CHAR, "name of event (if sent to event board)");	
			addOption ("vname", vname, SSI_MAX_CHAR, SSI_CHAR, "name of tuple value");
			addOption ("defval", &defval, 1, SSI_REAL, "default slider value");
			addOption ("minval", &minval, 1, SSI_REAL, "min slider value");
			addOption ("maxval", &maxval, 1, SSI_REAL, "max slider value");
			addOption ("steps", &steps, 1, SSI_UINT, "slider steps");

		}

		void setSender (const ssi_char_t *sname) {			
			if (sname) {
				ssi_strcpy (this->sname, sname);
			}
		}
		void setEvent (const ssi_char_t *ename) {
			if (ename) {
				ssi_strcpy (this->ename, ename);
			}
		}
		void setValue (const ssi_char_t *vname) {
			if (vname) {
				ssi_strcpy (this->vname, vname);
			}
		}
		void setSliderPos (int x, int y, int width, int height) {
			pos[0] = x;
			pos[1] = y;
			pos[2] = width;
			pos[3] = height;
		}

		int pos[4];
		ssi_char_t sname[SSI_MAX_CHAR];
		ssi_char_t ename[SSI_MAX_CHAR];	
		ssi_char_t vname[SSI_MAX_CHAR];	
		ssi_size_t steps;
		ssi_real_t defval, minval, maxval;
	};

public:

	static const ssi_char_t *GetCreateName () { return "EventSlider"; };
	static IObject *Create (const ssi_char_t *file) { return new EventSlider (file); };
	~EventSlider ();

	static ssi_char_t *ssi_log_name;
	int ssi_log_level;

	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Detects events if samples exceed a certain threshold."; };

	void send_enter();
	void send_flush();
	bool setEventListener (IEventListener *listener);
	const ssi_char_t *getEventAddress () {
		return _event_address.getAddress ();
	}

protected:

	EventSlider (const ssi_char_t *file = 0);
	EventSlider::Options _options;
	ssi_char_t *_file;

	EventAddress _event_address;
	ssi_event_t _event;
	IEventListener *_elistener;

	IWindow *_window;
	Slider *_slider;
	void update (ssi_real_t value);
};

}

#endif
