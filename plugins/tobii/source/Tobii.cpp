// author: Alexander Heimerl <heimerl@hcai.eu>, Tobias Baur <baur@hcai.eu
// created: 2022/08/23
// Copyright (C) University of Augsburg, Lab for Human Centered Artificial Intelligence
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Artificial Intelligence of the University of Augsburg
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
#include <inttypes.h>
#include "Tobii.h"



#ifdef USE_SSI_LEAK_DETECTOR
#include "SSI_LeakWatcher.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

namespace ssi {

ssi_char_t *Tobii::ssi_log_name = "tobiieye__";
const ssi_char_t *Tobii::CHANNELS_STRING[] = { "gaze" };

Tobii::Tobii (const ssi_char_t *file) 
	:  	
	_file (0), eyetracker(0), gaze_data(), _gaze_data(),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

	for (ssi_size_t i = 0; i < CHANNELS::NUM; i++) {
		_provider[i] = 0;
		switch (i) {
			case CHANNELS::GAZE_DATA:
				_channel[i] = new GazeChannel();
				break;
		}
 	}

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
}

Tobii::~Tobii () {

	for (ssi_size_t i = 0; i < CHANNELS::NUM; i++) {
		delete _channel[i]; _channel[i] = 0;
	}
}

bool Tobii::setProvider (const ssi_char_t *name, IProvider *provider) {

	for (ssi_size_t i = 0; i < CHANNELS::NUM; i++) {
		if (ssi_strcmp(name, _channel[i]->getName(), false)) {
			setProvider(i, provider);
			return true;
		}
	}

	ssi_print("unkown provider name '%s'", name);

	return false;
}

void gaze_data_callback(TobiiResearchGazeData* gaze_data, void* user_data) {

	memcpy(user_data, gaze_data, sizeof(*gaze_data));

}

void Tobii::setProvider(int channel, IProvider *provider) {

	if (_provider[channel]) {
		ssi_print("eyegazeraw provider already set");
	}

	_provider[channel] = provider;
	ssi_pcast(Channel, _channel[channel])->stream.sr = _options.sr;
	_provider[channel]->init(_channel[channel]);

}


bool Tobii::connect () 
{
	ssi_msg(SSI_LOG_LEVEL_DETAIL, "connect 'port=%u'", _options.port);

	setClockHz(_options.sr);

	TobiiResearchEyeTrackers* eyetrackers = NULL;

	TobiiResearchStatus result;
	size_t i = 0;

	result = tobii_research_find_all_eyetrackers(&eyetrackers);

	if (result != TOBII_RESEARCH_STATUS_OK) {
		printf("Finding trackers failed. Error: %d\n", result);
		return result;
	}

	for (i = 0; i < eyetrackers->count; i++) {
		eyetracker = eyetrackers->eyetrackers[i];
		char* address = NULL;
		char* serial_number = NULL;
		char* device_name = NULL;

		tobii_research_get_address(eyetracker, &address);
		tobii_research_get_serial_number(eyetracker, &serial_number);
		tobii_research_get_device_name(eyetracker, &device_name);

		printf("%s\t%s\t%s\n", address, serial_number, device_name);

		tobii_research_free_string(address);
		tobii_research_free_string(serial_number);
		tobii_research_free_string(device_name);
	}
	printf("Found %d Eye Trackers \n\n", (int)eyetrackers->count);
	tobii_research_free_eyetrackers(eyetrackers);

	float initial_gaze_output_frequency;

	TobiiResearchStatus status_test = tobii_research_set_gaze_output_frequency(eyetracker, _options.sr);

	TobiiResearchStatus status_hz = tobii_research_get_gaze_output_frequency(eyetracker, &initial_gaze_output_frequency);
	printf("The eye tracker's initial gaze output frequency is %f Hz with status %i.\n",
		initial_gaze_output_frequency, status_hz);

	TobiiResearchStatus status = tobii_research_subscribe_to_gaze_data(eyetracker, gaze_data_callback, &gaze_data);



	   return true;
	



}


void Tobii::clock() {

	if (_provider[CHANNELS::GAZE_DATA]) {
		
		 _gaze_data[0] = gaze_data.left_eye.pupil_data.diameter;
		 _gaze_data[1] = gaze_data.right_eye.pupil_data.diameter;
		 _gaze_data[2] = gaze_data.left_eye.pupil_data.validity;
		 _gaze_data[3] = gaze_data.right_eye.pupil_data.validity;

		 _gaze_data[4] = gaze_data.left_eye.gaze_origin.position_in_track_box_coordinates.x;
		 _gaze_data[5] = gaze_data.left_eye.gaze_origin.position_in_track_box_coordinates.y;
		 _gaze_data[6] = gaze_data.left_eye.gaze_origin.position_in_track_box_coordinates.z;

		 _gaze_data[7] = gaze_data.right_eye.gaze_origin.position_in_track_box_coordinates.x;
		 _gaze_data[8] = gaze_data.right_eye.gaze_origin.position_in_track_box_coordinates.y;
		 _gaze_data[9] = gaze_data.right_eye.gaze_origin.position_in_track_box_coordinates.z;

		 _gaze_data[10] = gaze_data.left_eye.gaze_origin.position_in_user_coordinates.x;
		 _gaze_data[11] = gaze_data.left_eye.gaze_origin.position_in_user_coordinates.y;
		 _gaze_data[12] = gaze_data.left_eye.gaze_origin.position_in_user_coordinates.z;

		 _gaze_data[13] = gaze_data.right_eye.gaze_origin.position_in_user_coordinates.x;
		 _gaze_data[14] = gaze_data.right_eye.gaze_origin.position_in_user_coordinates.y;
		 _gaze_data[15] = gaze_data.right_eye.gaze_origin.position_in_user_coordinates.z;

		 _gaze_data[16] = gaze_data.right_eye.gaze_origin.validity;
		 _gaze_data[17] = gaze_data.left_eye.gaze_origin.validity;

		 _gaze_data[18] = gaze_data.left_eye.gaze_point.position_on_display_area.x;
		 _gaze_data[19] = gaze_data.left_eye.gaze_point.position_on_display_area.y;

		 _gaze_data[20] = gaze_data.right_eye.gaze_point.position_on_display_area.x;
		 _gaze_data[21] = gaze_data.right_eye.gaze_point.position_on_display_area.y;

		for (int i = 0; i < SSI_Tobii_GAZE_DATA_LENGTH; i++) {
			if (_gaze_data[i] != _gaze_data[i]) {
				_gaze_data[i] = SSI_Tobii_INVALID_VALUE;
			}
		}

		_provider[CHANNELS::GAZE_DATA]->provide(ssi_pcast(ssi_byte_t, &_gaze_data), 1);
	}

}

bool Tobii::disconnect () {	

	ssi_msg(SSI_LOG_LEVEL_DETAIL, "disconnect 'port=%u'", _options.port);
	TobiiResearchStatus status = tobii_research_unsubscribe_from_gaze_data(eyetracker, gaze_data_callback);
	printf("Unsubscribed from gaze data with status %i.\n", status);

	return true;
}	
	




}

