// GazeArea.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/12/20
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

#ifndef SSI_GAZEAREA_H
#define SSI_GAZEAREA_H

#include "base/IConsumer.h"
#include "ioput/option/OptionList.h"
#include "event/EventAddress.h"

namespace ssi {

class GazeArea : public IConsumer {

public:

	class Options : public OptionList {

	public:

		Options () : mindur (0), eager (true), norm (false) {

			setSender ("eye");
			setEvent ("area");

			setArea (0, 0, 100, 100);

			addOption ("sname", sname, SSI_MAX_CHAR, SSI_CHAR, "name of sender (if sent to event board)");
			addOption ("ename", ename, SSI_MAX_CHAR, SSI_CHAR, "name of event (if sent to event board)");	
			addOption ("area", area, 4, SSI_REAL, "area (left, top, width, height)");
			addOption ("norm", &norm, 1, SSI_BOOL, "area in normalized values [0..1] relative to screen resolution");
			addOption ("mindur", &mindur, 1, SSI_DOUBLE, "minimum duration in the area");
			addOption ("eager", &eager, 1, SSI_BOOL, "send an event as soon as area is entered");
		};

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
		
		void setArea (ssi_real_t left, ssi_real_t top, ssi_real_t width, ssi_real_t height) {
			area[0] = left;
			area[1] = top;
			area[2] = width;
			area[3] = height;
		}	

		ssi_real_t area[4];
		bool norm;
		double mindur;
		bool eager;

		ssi_char_t sname[SSI_MAX_CHAR];
		ssi_char_t ename[SSI_MAX_CHAR];	
	};

public:

	static const ssi_char_t *GetCreateName () { return "GazeArea"; };
	static IObject *Create (const ssi_char_t *file) { return new GazeArea (file); };
	~GazeArea ();

	GazeArea::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "detects eye fixations and sends them to a socket connection"; };

	void consume_enter (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);
	void consume (IConsumer::info consume_info,
		ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);
	void consume_flush (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);

	//event sender
	bool setEventListener (IEventListener *listener);
	const ssi_char_t *getEventAddress () {
		return _event_address.getAddress ();
	}

	void setLogLevel (int level) {
		ssi_log_level = level;
	}

protected:

	// Intern members
	GazeArea (const ssi_char_t *file = 0);
	GazeArea::Options _options;
	ssi_char_t *_file;	
	static ssi_char_t *ssi_log_name;

	// Event listenting
	ssi_event_t _event;
	IEventListener *_elistener;
	EventAddress _event_address;

	bool checkArea (int x, int y);

	int _area[4];
	bool _isInArea;
	ssi_time_t _lastEnter;

};

}

#endif
