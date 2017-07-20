// EyeFixation.cpp
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

#include "EyeFixation.h"
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

char *EyeFixation::ssi_log_name = "fixation__";

EyeFixation::EyeFixation (const ssi_char_t *file)
	: _elistener (0),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT),
	_file (0) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}

	ssi_event_init (_fixation_event, SSI_ETYPE_MAP);
	ssi_event_adjust (_fixation_event, sizeof (ssi_event_map_t) * 2);

	ssi_event_init(_saccade_event, SSI_ETYPE_MAP);
	ssi_event_adjust(_saccade_event, sizeof(ssi_event_map_t) * 2);
}

EyeFixation::~EyeFixation () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}

	ssi_event_destroy (_fixation_event);
	ssi_event_destroy(_saccade_event);
}


bool EyeFixation::setEventListener (IEventListener *listener) {

	_elistener = listener;
	_fixation_event.sender_id = Factory::AddString (_options.sname);
	if (_fixation_event.sender_id == SSI_FACTORY_STRINGS_INVALID_ID) {
		return false;
	}
	_fixation_event.event_id = Factory::AddString (_options.eFixName);
	if (_fixation_event.event_id == SSI_FACTORY_STRINGS_INVALID_ID) {
		return false;
	}
	_saccade_event.sender_id = Factory::AddString(_options.sname);
	if (_saccade_event.sender_id == SSI_FACTORY_STRINGS_INVALID_ID) {
		return false;
	}
	_saccade_event.event_id = Factory::AddString(_options.eSacName);
	if (_fixation_event.event_id == SSI_FACTORY_STRINGS_INVALID_ID) {
		return false;
	}

	_event_address.setSender (_options.sname);
	_event_address.setEvents (_options.eFixName);
	_event_address.setEvents(_options.eSacName);
	
	ssi_event_map_t *ptr = ssi_pcast (ssi_event_map_t, _fixation_event.ptr);
	ptr[0].id = Factory::AddString ("x");
	ptr[1].id = Factory::AddString ("y");

	ssi_event_map_t *sac_ptr = ssi_pcast(ssi_event_map_t, _saccade_event.ptr);
	sac_ptr[SSI_EVENT_EYE_FIXATION_REAL_PATH_IDX].id = Factory::AddString("real_path");
	sac_ptr[SSI_EVENT_EYE_FIXATION_FIX_PATH_IDX].id = Factory::AddString("fixation_to_fixation_path");

	return true;
}


void EyeFixation::consume_enter (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	resetFixation (0);
}

void EyeFixation::consume (IConsumer::info consume_info,
	ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	ssi_time_t current_time;

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

	int *ptr = eyes;
	double time = consume_info.time;
	double dtime = 1.0 / stream_in[0].sr;

	for (ssi_size_t i = 0; i < stream_in[0].num; i++) {

		current_time = stream_in->time + (i / stream_in->sr);
		ssi_int_t x_pos = *ptr;
		ssi_int_t y_pos = *(ptr + 1);

		updateFixation(x_pos, y_pos);

		//leaving fixation area
		if (_infix && !isfix ()) {
			fixend (time);
			resetFixation (time);
			saccadeAddPoint(current_time, x_pos, y_pos);
		//starting a fixation
		} else if (!_infix && isfix ()) {
			//fixation complete
			if (time - _start >= _options.mindur) {
				if (_options.eager) {
					fixstart (_start);
				}
				_infix = true;
				saccadeEnd(current_time, x_pos, y_pos);
				saccadeStart(current_time, x_pos, y_pos);
			}
			else{
				saccadeAddPoint(current_time, x_pos, y_pos);
			}
		//out of fixation
		} else if (!_infix && !isfix ()) {
			resetFixation (time);
			saccadeAddPoint(current_time, x_pos, y_pos);
		}
		else if (_infix && isfix()){
			//restart saccade while in fixation
			saccadeStart(current_time, x_pos, y_pos);
		}

		ptr += 2;
		time += dtime;
	}

	if (stream_in[0].type != SSI_INT) {
		delete[] eyes;
	}
}

void EyeFixation::consume_flush (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {
}

SSI_INLINE void EyeFixation::updateFixation (int x, int y) {
	
	_minx = min (x, _minx);
	_maxx = max (x, _maxx);
	_miny = min (y, _miny);
	_maxy = max (y, _maxy);
}

SSI_INLINE void EyeFixation::resetFixation (double start) {

	_start = start;
	_infix = false;
	_minx = _miny = INT_MAX;
	_maxx = _maxy = INT_MIN;
}

SSI_INLINE bool EyeFixation::isfix () {

	return (_maxx - _minx) + (_maxy - _miny) <= _options.thres && (_minx + _miny > 0);
}

SSI_INLINE void EyeFixation::fixstart (double time) {
 
	if (_elistener) 
	{
		_fixation_event.time = ssi_cast (ssi_size_t, 1000 * _start + 0.5);
		_fixation_event.dur = ssi_cast (ssi_size_t, 1000 * (time - _start) + 0.5);
				
		float x = _minx + ssi_cast (float, (_maxx - _minx)) / 2;
		float y = _miny + ssi_cast (float, (_maxy - _miny)) / 2;

		ssi_event_map_t *ptr = ssi_pcast (ssi_event_map_t, _fixation_event.ptr);
		ptr[0].value = x;
		ptr[1].value = y;

		_fixation_event.state =  SSI_ESTATE_CONTINUED;
		_elistener->update (_fixation_event);
					
	}
}

SSI_INLINE void EyeFixation::fixend (double time) {

	if (_elistener) 
	{	
		_fixation_event.dur = ssi_cast (ssi_size_t, 1000 * (time - _start) + 0.5);
		_fixation_event.time = ssi_cast (ssi_size_t, 1000 * _start + 0.5) + _fixation_event.dur;
		
		float x = _minx + ssi_cast (float, (_maxx - _minx)) / 2;
		float y = _miny + ssi_cast (float, (_maxy - _miny)) / 2;

		ssi_event_map_t *ptr = ssi_pcast (ssi_event_map_t, _fixation_event.ptr);
		ptr[0].value = x;
		ptr[1].value = y;

		_fixation_event.state =  SSI_ESTATE_COMPLETED;
		_elistener->update (_fixation_event);
					
	}
}

void EyeFixation::saccadeStart(ssi_time_t time, ssi_int_t x_pos, ssi_int_t y_pos) {

	saccade_points.clear();
	saccadeAddPoint(time, x_pos, y_pos);
}

void EyeFixation::saccadeEnd(ssi_time_t time, ssi_int_t x_pos, ssi_int_t y_pos) {

	if (_elistener && saccade_points.size() > 0)
	{
		//calculate event data
		_saccade_event.dur = 1000 * (saccade_points.back().time - saccade_points.front().time);
		_saccade_event.time = 1000 * (saccade_points.front().time);


		//remove all samples in the fixation time duration
		ssi_time_t time_at_saccade_end = saccade_points.back().time;
		while (saccade_points.size() > 0 && saccade_points.back().time >= time_at_saccade_end - _options.mindur){
			saccade_points.pop_back();
		}
		if (saccade_points.size() < 2){
			return;
		}

		//calculate fix to fix path length (euclidean distance of first and last sample in saccade_points)
		ssi_real_t fixation_to_fixation_path = euclidean_distance(saccade_points.front(), saccade_points.back());
		//calculate the real path length
		ssi_real_t real_path = 0;
		view_point_t last_point = saccade_points.front();

		for (ssi_size_t i = 0; i < saccade_points.size() - 1; i++){
			real_path += euclidean_distance(saccade_points[i], saccade_points[i + 1]);
		}
		real_path = 0;
		saccade_points.erase(saccade_points.begin());
		for each (view_point_t current_point in saccade_points)
		{
			real_path += euclidean_distance(last_point, current_point);
			last_point = current_point;
		}
		
		


		ssi_event_map_t *ptr = ssi_pcast(ssi_event_map_t, _saccade_event.ptr);
		ptr[SSI_EVENT_EYE_FIXATION_FIX_PATH_IDX].value = fixation_to_fixation_path;
		ptr[SSI_EVENT_EYE_FIXATION_REAL_PATH_IDX].value = real_path;


		_saccade_event.state = SSI_ESTATE_COMPLETED;
		_elistener->update(_saccade_event);

	}
}


SSI_INLINE void EyeFixation::saccadeAddPoint(ssi_time_t time, ssi_int_t x_pos, ssi_int_t y_pos)
{
	view_point_t vpoint;

	vpoint.time = time;
	vpoint.x_pos = x_pos;
	vpoint.y_pos = y_pos;

	saccade_points.push_back(vpoint);
}

SSI_INLINE ssi_real_t EyeFixation::euclidean_distance(view_point_t p1, view_point_t p2){

	return ssi_cast(ssi_real_t, sqrt(ssi_cast(double, (p1.x_pos - p2.x_pos) * (p1.x_pos - p2.x_pos) + (p1.y_pos - p2.y_pos) * (p1.y_pos - p2.y_pos))));

}

}
