// GazeArea.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/02/04
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

#include "GazeArea.h"
#include "base/Factory.h"

#ifdef USE_SSI_LEAK_DETECTOR
#include "SSI_LeakWatcher.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

namespace ssi {

ssi_char_t *GazeArea::ssi_log_name = "gazearea__";

GazeArea::GazeArea (const ssi_char_t * file) 
	: _file (0),
	_elistener (0)	{

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
	ssi_event_init (_event, SSI_ETYPE_EMPTY);
}

GazeArea::~GazeArea () {
	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
	ssi_event_destroy (_event);
}

bool GazeArea::setEventListener (IEventListener *listener) {

	_elistener = listener;
	_event.sender_id = Factory::AddString (_options.sname);
	if (_event.sender_id == SSI_FACTORY_STRINGS_INVALID_ID) {
		return false;
	}
	_event.event_id = Factory::AddString (_options.ename);
	if (_event.event_id == SSI_FACTORY_STRINGS_INVALID_ID) {
		return false;
	}

	_event_address.setSender (_options.sname);
	_event_address.setEvents (_options.ename);
	_event.prob = ssi_cast (ssi_real_t, 1.0);
	_event.type = SSI_ETYPE_EMPTY;
	return true;
}

void GazeArea::consume_enter (
	ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	if (stream_in[0].type != SSI_INT && stream_in[0].type != SSI_REAL) {
		ssi_err ("type of input stream has to be int or float");
	}

	if (_options.norm) {

		HWND desktop = GetDesktopWindow ();
		RECT rect;
		GetWindowRect (desktop, &rect);
			
		ssi_real_t width  = ssi_cast (ssi_real_t, rect.right);
		ssi_real_t height = ssi_cast (ssi_real_t, rect.bottom);

		_area[0] = ssi_cast (int, _options.area[0] * width);
		_area[1] = ssi_cast (int, _options.area[1] * height);
		_area[2] = ssi_cast (int, _options.area[2] * width);
		_area[3] = ssi_cast (int, _options.area[3] * height);

	} else {
		_area[0] = ssi_cast (int, _options.area[0] + 0.5f);
		_area[1] = ssi_cast (int, _options.area[1] + 0.5f);
		_area[2] = ssi_cast (int, _options.area[2] + 0.5f);
		_area[3] = ssi_cast (int, _options.area[3] + 0.5f);
	}

	_isInArea = false;
	_lastEnter = 0;
}

bool GazeArea::checkArea (int x, int y) {
	return x >= _area[0] && x <= _area[0] + _area[2] &&
		y >= _area[1] && y <= _area[1] + _area[3];
}

void GazeArea::consume (
	IConsumer::info consume_info,
	ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	int *eyes = 0;
	if (stream_in[0].type != SSI_INT) {
		ssi_size_t n = stream_in[0].num * stream_in[0].dim;
		ssi_real_t *ptr = ssi_pcast (ssi_real_t, stream_in[0].ptr);
		eyes = new int[n];
		for (ssi_size_t i = 0; i < n; i++) {
			eyes[i] = ssi_cast (int, *ptr++);
		}
	} else {
		eyes = ssi_pcast (int, stream_in[0].ptr);
	}

	// Get the timestamp of the current frame
	double time = consume_info.time;
	double dtime = 1.0 / stream_in[0].sr;

	// Create an adequate event structure
	// and add the right data to the event 
	_event.time = ssi_cast (ssi_size_t, 1000 * time + 0.5);
	_event.dur = ssi_cast (ssi_size_t, 0);
	_event.prob = ssi_cast (ssi_real_t, 1.0);

	int x;
	int y;
	ssi_time_t now = time;
	int *ptr = eyes;

	// Process all the points in this frame
	for (ssi_size_t i = 0; i < stream_in[0].num; i++) {

		// Get x and y values of the gaze
		x = *ptr;
		y = *(ptr+1);

		bool check = checkArea (x,y);
				
		if (!_isInArea && check) {

			_isInArea = true;
			_lastEnter = now;

			if (_options.eager) {
				_event.state = SSI_ESTATE_CONTINUED;
				_event.time = ssi_cast (ssi_size_t, now * 1000 + 0.5);
				_event.dur = 0;
				_elistener->update(_event);
			}

		} else if (_isInArea && !check) {
					
			if (now - _lastEnter >= _options.mindur) { 
				_event.state = SSI_ESTATE_COMPLETED;
				_event.time = ssi_cast (ssi_size_t, _lastEnter * 1000 + 0.5);
				_event.dur = ssi_cast (ssi_size_t, (now - _lastEnter) * 1000 + 0.5);
				_elistener->update(_event);
			}

			_isInArea = false;
		}

		ptr += 2;
		time += dtime;
		now += dtime;
	}

	if (stream_in[0].type != SSI_INT) {
		delete[] eyes;
	}
}

void GazeArea::consume_flush (
	ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {
}

}
