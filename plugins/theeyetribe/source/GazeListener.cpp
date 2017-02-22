// GazeListener.cpp
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
#include "GazeListener.h"
#include "thread/Lock.h"

#ifdef USE_SSI_LEAK_DETECTOR
#include "SSI_LeakWatcher.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif


namespace ssi {

GazeListener::GazeListener(ssi_size_t port, 
		ssi_size_t keep) :
	_port(port),
	_keep(keep) {

	_confidence = 0;
	_gaze_raw[0] = 0;
	_gaze_raw[1] = 0;
	_gaze_avg[0] = 0;
	_gaze_avg[1] = 0;
}

GazeListener::~GazeListener() {
}

bool GazeListener::connect () {

	bool connected = m_api.connect(true, _port);
	if (connected)
	{
		m_api.add_listener(*this);
	}

	return connected;
}

void GazeListener::disconnect () {
	
	if (m_api.is_connected())
	{
		m_api.remove_listener(*this);
	}
	//m_api.disconnect();
}

void GazeListener::getGaze(TheEyeTribe::confidence_t &confidence,
	TheEyeTribe::gaze_avg_t &gaze_raw,
	TheEyeTribe::gaze_avg_t &gaze_avg,
	TheEyeTribe::eye_raw_t &eye_left_raw,
	TheEyeTribe::eye_raw_t &eye_right_raw,
	TheEyeTribe::eye_avg_t &eye_left_avg,
	TheEyeTribe::eye_avg_t &eye_right_avg,
	TheEyeTribe::pupil_size_t &pupil_left_size,
	TheEyeTribe::pupil_size_t &pupil_right_size,
	TheEyeTribe::pupil_center_t &pupil_left_center,
	TheEyeTribe::pupil_center_t &pupil_right_center) {

	Lock lock(m_mutex);

	confidence = _confidence;

	gaze_raw[0] = _gaze_raw[0];
	gaze_raw[1] = _gaze_raw[1];

	gaze_avg[0] = _gaze_avg[0];
	gaze_avg[1] = _gaze_avg[1];

	eye_left_raw[0] = _eye_left_raw[0];
	eye_left_raw[1] = _eye_left_raw[1];

	eye_right_raw[0] = _eye_right_raw[0];
	eye_right_raw[1] = _eye_right_raw[1];

	eye_left_avg[0] = _eye_left_avg[0];
	eye_left_avg[1] = _eye_left_avg[1];

	eye_right_avg[0] = _eye_right_avg[0];
	eye_right_avg[1] = _eye_right_avg[1];

	pupil_left_size = _pupil_left_size;
	pupil_right_size = _pupil_right_size;

	pupil_left_center[0] = _pupil_left_center[0];
	pupil_left_center[1] = _pupil_left_center[1];

	pupil_right_center[0] = _pupil_right_center[0];
	pupil_right_center[1] = _pupil_right_center[1];
}

void GazeListener::on_gaze_data(gtl::GazeData const &gaze_data)
{		
    if (gaze_data.state & gtl::GazeData::GD_STATE_TRACKING_GAZE)
    {
		Lock lock(m_mutex);   

		_confidence = 1;
		_counter = _keep;

		_gaze_raw[0] = gaze_data.raw.x;
		_gaze_raw[1] = gaze_data.raw.y;

		_gaze_avg[0] = gaze_data.avg.x;
		_gaze_avg[1] = gaze_data.avg.y;

		_eye_left_raw[0] = gaze_data.lefteye.raw.x;
		_eye_left_raw[1] = gaze_data.lefteye.raw.y;

		_eye_right_raw[0] = gaze_data.righteye.raw.x;
		_eye_right_raw[1] = gaze_data.righteye.raw.x;

		_eye_left_avg[0] = gaze_data.lefteye.avg.x;
		_eye_left_avg[1] = gaze_data.lefteye.avg.y;

		_eye_right_avg[0] = gaze_data.righteye.avg.x;
		_eye_right_avg[1] = gaze_data.righteye.avg.x;

		_pupil_left_size = gaze_data.lefteye.psize;
		_pupil_right_size = gaze_data.righteye.psize;

		_pupil_left_center[0] = gaze_data.lefteye.pcenter.x;
		_pupil_left_center[1] = gaze_data.lefteye.pcenter.y;

		_pupil_right_center[0] = gaze_data.righteye.pcenter.x;
		_pupil_right_center[1] = gaze_data.righteye.pcenter.x;
		
	} else {		
		
		_confidence = 0;

		if (_keep > 0 && --_counter == 0) {

			_gaze_raw[0] = SSI_THEEYETRIBE_INVALID_VALUE;
			_gaze_raw[1] = SSI_THEEYETRIBE_INVALID_VALUE;

			_gaze_avg[0] = SSI_THEEYETRIBE_INVALID_VALUE;
			_gaze_avg[1] = SSI_THEEYETRIBE_INVALID_VALUE;
		}
	}
}




}

