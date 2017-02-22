// eyetribe.cpp
// author: Tobias Baur <baur@hcm-lab.de>
// created: 2013/02/28
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

#include <stdio.h>
#include <string.h>
#include "TheEyeTribe.h"
#include "GazeListener.h"

#ifdef USE_SSI_LEAK_DETECTOR
#include "SSI_LeakWatcher.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

namespace ssi {

ssi_char_t *TheEyeTribe::ssi_log_name = "theeyetrib";
const ssi_char_t *TheEyeTribe::CHANNELS_STRING[] = { "gaze raw", "gaze avg" };

TheEyeTribe::TheEyeTribe (const ssi_char_t *file) 
	: _gaze_listener(0), 	
	_file (0),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

	for (ssi_size_t i = 0; i < CHANNELS::NUM; i++) {
		_provider[i] = 0;
		switch (i) {
			case CHANNELS::CONFIDENCE:
				_channel[i] = new ConfidenceChannel();
				break;
			case CHANNELS::GAZE_RAW:
				_channel[i] = new GazeRawChannel();
				break;
			case CHANNELS::GAZE_AVG:
				_channel[i] = new GazeAvgChannel();
				break;			
			case CHANNELS::EYE_LEFT_RAW:
				_channel[i] = new EyeLeftRawChannel();
				break;
			case CHANNELS::EYE_RIGHT_RAW:
				_channel[i] = new EyeRightRawChannel();
				break;
			case CHANNELS::EYE_LEFT_AVG:
				_channel[i] = new EyeLeftAvgChannel();
				break;
			case CHANNELS::EYE_RIGHT_AVG:
				_channel[i] = new EyeRightAvgChannel();
				break;
			case CHANNELS::PUPIL_LEFT_SIZE:
				_channel[i] = new PupilLeftSizeChannel();
				break;
			case CHANNELS::PUPIL_RIGHT_SIZE:
				_channel[i] = new PupilRightSizeChannel();
				break;
			case CHANNELS::PUPIL_LEFT_CENTER:
				_channel[i] = new PupilLeftCenterChannel();
				break;
			case CHANNELS::PUPIL_RIGHT_CENTER:
				_channel[i] = new PupilRightCenterChannel();
				break;
		}
 	}

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
}

TheEyeTribe::~TheEyeTribe () {

	for (ssi_size_t i = 0; i < CHANNELS::NUM; i++) {
		delete _channel[i]; _channel[i] = 0;
	}
}

bool TheEyeTribe::setProvider (const ssi_char_t *name, IProvider *provider) {

	for (ssi_size_t i = 0; i < CHANNELS::NUM; i++) {
		if (ssi_strcmp(name, _channel[i]->getName(), false)) {
			setProvider(i, provider);
			return true;
		}
	}

	ssi_wrn("unkown provider name '%s'", name);

	return false;
}

void TheEyeTribe::setProvider(int channel, IProvider *provider) {

	if (_provider[channel]) {
		ssi_wrn("eyegazeraw provider already set");
	}

	_provider[channel] = provider;
	ssi_pcast(Channel, _channel[channel])->stream.sr = _options.sr;
	_provider[channel]->init(_channel[channel]);

	ssi_msg(SSI_LOG_LEVEL_DETAIL, "provider '%s' set", );	
}

bool TheEyeTribe::connect () 
{
	ssi_msg(SSI_LOG_LEVEL_DETAIL, "connect 'port=%u'", _options.port);

	setClockHz(_options.sr);

	_gaze_listener = new GazeListener(_options.port, _options.keep);
	return _gaze_listener->connect ();
}

void TheEyeTribe::clock() {

	_gaze_listener->getGaze(_confidence, 
		_gaze_raw,
		_gaze_avg,
		_eye_left_raw,
		_eye_right_raw,
		_eye_left_avg,
		_eye_right_avg,
		_pupil_left_size,
		_pupil_right_size,
		_pupil_left_center,
		_pupil_right_center);

	if (_provider[CHANNELS::CONFIDENCE]) {
		_provider[CHANNELS::CONFIDENCE]->provide(ssi_pcast(ssi_byte_t, &_confidence), 1);
	}

	if (_provider[CHANNELS::GAZE_RAW]) {
		_provider[CHANNELS::GAZE_RAW]->provide(ssi_pcast(ssi_byte_t, _gaze_raw), 1);
	}

	if (_provider[CHANNELS::GAZE_AVG]) {		
		_provider[CHANNELS::GAZE_AVG]->provide(ssi_pcast(ssi_byte_t, _gaze_avg), 1);
	}

	if (_provider[CHANNELS::EYE_LEFT_RAW]) {
		_provider[CHANNELS::EYE_LEFT_RAW]->provide(ssi_pcast(ssi_byte_t, _eye_left_raw), 1);
	}

	if (_provider[CHANNELS::EYE_RIGHT_RAW]) {
		_provider[CHANNELS::EYE_RIGHT_RAW]->provide(ssi_pcast(ssi_byte_t, _eye_right_raw), 1);
	}

	if (_provider[CHANNELS::EYE_LEFT_AVG]) {
		_provider[CHANNELS::EYE_LEFT_AVG]->provide(ssi_pcast(ssi_byte_t, _eye_left_avg), 1);
	}

	if (_provider[CHANNELS::EYE_RIGHT_AVG]) {
		_provider[CHANNELS::EYE_RIGHT_AVG]->provide(ssi_pcast(ssi_byte_t, _eye_right_avg), 1);
	}

	if (_provider[CHANNELS::PUPIL_LEFT_SIZE]) {
		_provider[CHANNELS::PUPIL_LEFT_SIZE]->provide(ssi_pcast(ssi_byte_t, &_pupil_left_size), 1);
	}

	if (_provider[CHANNELS::PUPIL_RIGHT_SIZE]) {
		_provider[CHANNELS::PUPIL_RIGHT_SIZE]->provide(ssi_pcast(ssi_byte_t, &_pupil_right_size), 1);
	}

	if (_provider[CHANNELS::PUPIL_LEFT_CENTER]) {
		_provider[CHANNELS::PUPIL_LEFT_CENTER]->provide(ssi_pcast(ssi_byte_t, _pupil_left_center), 1);
	}

	if (_provider[CHANNELS::PUPIL_RIGHT_CENTER]) {
		_provider[CHANNELS::PUPIL_RIGHT_CENTER]->provide(ssi_pcast(ssi_byte_t, _pupil_right_center), 1);
	}
}

bool TheEyeTribe::disconnect () {	

	ssi_msg(SSI_LOG_LEVEL_DETAIL, "disconnect 'port=%u'", _options.port);

	_gaze_listener->disconnect();
	delete _gaze_listener; _gaze_listener = 0;

	return true;
}	
	




}

