// ThresClassEventSender.h
// author: Ionut Damianr <damian@hcm-lab.de>
// created: 2013/08/21
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

#include "ThresClassEventSender.h"
#include "base/Factory.h"
#include <sstream>

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif


namespace ssi {

ssi_char_t *ThresClassEventSender::ssi_log_name = "thresclsev";

ThresClassEventSender::ThresClassEventSender (const ssi_char_t *file)
:	_file (0),
	_elistener (0),
	_thres(0),
	_classes(0),
	_num_classes(0),
	_lastClass(-1),
	_lastValue(FLT_MAX),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}

	ssi_event_init (_event, SSI_ETYPE_STRING, 0,0,0,0, 256);
}

ThresClassEventSender::~ThresClassEventSender () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
	
	ssi_event_destroy (_event);
	if(_thres != 0)	delete[] _thres;
	for(ssi_size_t i=0; i < _num_classes; ++i)
		delete _classes[i];
	delete[] _classes;
}

bool ThresClassEventSender::setEventListener (IEventListener *listener) {

	_elistener = listener;
	
	if (_options.address[0] != '\0') {

		SSI_OPTIONLIST_SET_ADDRESS(_options.address, _event_address, _event);

	} else {

		_event.sender_id = Factory::AddString(_options.sname);
		_event.event_id = Factory::AddString(_options.ename);

		_event_address.setSender(_options.sname);
		_event_address.setEvents(_options.ename);
	}

	return true;
}

int ThresClassEventSender::classify(ssi_real_t value, ssi_real_t* thresholds, ssi_size_t n_thresholds)
{
	int class_id = -1;
	for(int i =  n_thresholds - 1; i >= 0; i--)
	{
		if (value > thresholds[i]) //value exceed threshold for this class
		{
			if(_lastClass != i && abs(value - _lastValue) > _options.minDiff) //significantly different than last event
			{
				class_id = i;
					
				_lastClass = i;
				_lastValue = value;					
			}
			break;
		}
	}
	return class_id;
}

bool ThresClassEventSender::handleEvent(IEventListener *listener, ssi_event_t* ev, const ssi_char_t* class_name, ssi_time_t time)
{
	if (listener)
	{
		strcpy(ev->ptr, class_name);
		ev->time = ssi_cast (ssi_size_t, 1000 * time + 0.5);
		ev->dur = 0;				
		ev->state = SSI_ESTATE_COMPLETED;			
		listener->update (*ev);

		return true;
	}	
	return false;
}

void ThresClassEventSender::consume_enter (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	if(stream_in[0].dim > 1)
		ssi_wrn("Dimension > 1 unsupported");
	
	ssi_size_t numThres = 0;
	_classes = parseStrings(_options.classes, _num_classes);
	_thres = parseFloats(_options.thres, numThres, true);

	if (numThres != _num_classes)
		ssi_wrn("invalid thresholds (%d classes, %d thresholds)", _num_classes, numThres);
}

void ThresClassEventSender::consume (IConsumer::info consume_info,
	ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	ssi_byte_t *dataptr = stream_in[0].ptr;
	ssi_time_t step = 1.0 / stream_in[0].sr; 
	ssi_time_t t = consume_info.time; 
	ssi_real_t sum = 0;

	for (ssi_size_t k = 0; k < stream_in[0].num; k++) 
	{
		ssi_real_t value = *ssi_pcast(ssi_real_t, stream_in[0].ptr + k * stream_in[0].dim * stream_in[0].byte);

		if(!_options.mean)
		{
			int id = classify(value, _thres, _num_classes);
			if(id >= 0)
				handleEvent(_elistener, &_event, _classes[id], t);
		} else
			sum += value;

		t += step;
	}

	if(_options.mean)
	{
		int id = classify(sum / stream_in[0].num, _thres, _num_classes);
		if(id >= 0)
			handleEvent(_elistener, &_event, _classes[id], consume_info.time);
	}
}

void ThresClassEventSender::consume_flush (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {
}

ssi_real_t *ThresClassEventSender::parseFloats (const ssi_char_t *str, ssi_size_t &n, bool sort, const ssi_char_t *delims) {

	n = 0;

	if (!str || str[0] == '\0') {
		return 0;
	}

	ssi_char_t *string = ssi_strcpy (str);
	
	char *pch;
	strcpy (string, str);
	pch = strtok (string, delims);
	float index;

	std::vector<float> items;
	
	while (pch != NULL) {
		index = (float) atof (pch);
		items.push_back(index);
		pch = strtok (NULL, delims);
	}

	if (sort) {
		std::sort (items.begin(), items.end());		
	}

	n = (ssi_size_t) items.size();
	float *values = new float[n];

	for(size_t i = 0; i < items.size(); i++) {
		values[i] = items[i];		
	}

	delete[] string;

	return values;
}

const ssi_char_t **ThresClassEventSender::parseStrings (const ssi_char_t *str, ssi_size_t &n, const ssi_char_t *delims) {

	n = 0;

	if (!str || str[0] == '\0') {
		return 0;
	}

	ssi_char_t *string = ssi_strcpy (str);
	
	char *pch;
	strcpy (string, str);
	pch = strtok (string, delims);

	std::vector<char*> items;
	
	while (pch != NULL) {
		items.push_back(pch);
		pch = strtok (NULL, delims);
	}

	n = (ssi_size_t) items.size();
	char **values = new char*[n];

	for(size_t i = 0; i < items.size(); i++) {
		values[i] = new char[SSI_MAX_CHAR];
		strcpy(values[i], items[i]);		
	}
	delete[] string;

	return (const ssi_char_t**)values;
}

}
