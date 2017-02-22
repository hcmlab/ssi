// EyeFixation.h
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
// version 3 of the License, or any later version.
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
// Detects eye fixations and saccades and sends them to a socket connection or as a event
//************************************************************************************************* 

#pragma once

#ifndef SSI_EVENT_EYE_FIXATION_H
#define SSI_EVENT_EYE_FIXATION_H


#include "base/IConsumer.h"
#include "ioput/option/OptionList.h"
#include "event/EventAddress.h"

#include <climits>

namespace ssi {

#define SSI_EVENT_EYE_FIXATION_REAL_PATH_IDX 0
#define SSI_EVENT_EYE_FIXATION_FIX_PATH_IDX 1


class EyeFixation : public IConsumer {

public:

	struct 	view_point_t {
		ssi_time_t time;
		ssi_int_t x_pos;
		ssi_int_t y_pos;
	};

	class Options : public OptionList {

		

	public:

		Options () : thres (35), mindur (0.15), eager (true) {

			setSender ("eye");
			setFixEvent ("fixation");
			setSacEvent("saccade");

			addOption ("sname", sname, SSI_MAX_CHAR, SSI_CHAR, "name of sender (if sent to event board)");
			addOption ("eFixName", eFixName, SSI_MAX_CHAR, SSI_CHAR, "name of the fixation event (if sent to event board)");	
			addOption("eSacName", eSacName, SSI_MAX_CHAR, SSI_CHAR, "name of the saccade event (if sent to the event board)");
			addOption ("thres", &thres, 1, SSI_INT, "threshold for the difference in min and max position for a fixation to be detected");
			addOption ("mindur", &mindur, 1, SSI_DOUBLE, "minimum duration for a fixation to be detected in seconds.");
			addOption ("eager", &eager, 1, SSI_BOOL, "send an event as soon as area is entered");
		};


		void setSender (const ssi_char_t *sname) {			
			if (sname) {
				ssi_strcpy (this->sname, sname);
			}
		}

		void setFixEvent (const ssi_char_t *ename) {
			if (ename) {
				ssi_strcpy (this->eFixName, ename);
			}
		}
		void setSacEvent(const ssi_char_t *ename) {
			if (ename) {
				ssi_strcpy(this->eSacName, ename);
			}
		}

		ssi_char_t sname[SSI_MAX_CHAR];
		ssi_char_t eFixName[SSI_MAX_CHAR];	
		ssi_char_t eSacName[SSI_MAX_CHAR];

		int thres;
		double mindur;
		bool eager;
	};

public:

	static const ssi_char_t *GetCreateName () { return "EyeFixation"; };
	static IObject *Create (const ssi_char_t *file) { return new EyeFixation (file); };
	~EyeFixation ();
	EyeFixation::Options *getOptions () override
	{ return &_options; };
	const ssi_char_t *getName () override
	{ return GetCreateName (); };
	const ssi_char_t *getInfo () override
	{ return "detects eye fixations and sends them to a socket connection"; };

	void consume_enter (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]) override;
	void consume (IConsumer::info consume_info,
		ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]) override;
	void consume_flush (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]) override;

	//event sender
	bool setEventListener (IEventListener *listener) override;
	const ssi_char_t *getEventAddress () override
	{
		return _event_address.getAddress ();
	}

	void setLogLevel (int level) {
		ssi_log_level = level;
	}

protected:

	EyeFixation (const ssi_char_t *file = 0);
	EyeFixation::Options _options;
	ssi_char_t *_file;
	static char *ssi_log_name;
	int ssi_log_level;

	IEventListener *_elistener;
	ssi_event_t _fixation_event;
	ssi_event_t _saccade_event;
	EventAddress _event_address;

	//fixation 
	ssi_time_t _start, _last;
	bool _infix;
	int _minx, _miny, _maxx, _maxy;

	bool isfix ();
	void resetFixation (double start);
	void updateFixation (int x, int y);
	void fixstart (double time);
	void fixend (double time);

	// saccade 
	// contains all points since the last fixation.
	std::vector<view_point_t> saccade_points;
	// resets the sacccade_points vector and add the first point.
	void saccadeStart(ssi_time_t time, ssi_int_t x_pos, ssi_int_t y_pos);
	//calculates the event data with the point in saccade_points and send the event.
	void saccadeEnd(ssi_time_t time, ssi_int_t x_pos, ssi_int_t y_pos);
	//adds the current point to the saccade_points vector.
	void saccadeAddPoint(ssi_time_t time, ssi_int_t x_pos, ssi_int_t y_pos);
	//calculates the euclidean distance between p1 and p2
	ssi_real_t euclidean_distance(view_point_t p1, view_point_t p2);
};

}

#endif
