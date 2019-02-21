// GazeListener.h
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

// Provides GazeListener eyegaze tracking

#pragma once

#ifndef SSI_THEEYETRIBE_GAZELISTENER_H
#define	SSI_THEEYETRIBE_GAZELISTENER_H

#include "thread/Lock.h"
#include "TheEyeTribe.h"
#include "gazeapi.h"

namespace ssi {

class GazeListener : public gtl::IGazeListener {

public:

	GazeListener(ssi_size_t port, ssi_size_t keep);
	virtual ~GazeListener();

	void getGaze(TheEyeTribe::confidence_t &confidence,
		TheEyeTribe::gaze_avg_t &gaze_raw,
		TheEyeTribe::gaze_avg_t &gaze_avg,
		TheEyeTribe::eye_raw_t &eye_left_raw,
		TheEyeTribe::eye_raw_t &eye_right_raw,
		TheEyeTribe::eye_avg_t &eye_left_avg,
		TheEyeTribe::eye_avg_t &eye_right_avg,
		TheEyeTribe::pupil_size_t &pupil_left_size,
		TheEyeTribe::pupil_size_t &pupil_right_size,
		TheEyeTribe::pupil_center_t &pupil_left_center,
		TheEyeTribe::pupil_center_t &pupil_right_center);
	bool connect();
	void disconnect();

protected:

	void on_gaze_data(gtl::GazeData const &gaze_data);

	gtl::GazeApi m_api;
	Mutex m_mutex;

	ssi_size_t _port;
	ssi_size_t _keep;
	ssi_size_t _counter;

	TheEyeTribe::confidence_t _confidence;
	TheEyeTribe::gaze_raw_t _gaze_raw;
	TheEyeTribe::gaze_avg_t _gaze_avg;
	TheEyeTribe::eye_raw_t _eye_left_raw;
	TheEyeTribe::eye_raw_t _eye_right_raw;
	TheEyeTribe::eye_avg_t _eye_left_avg;
	TheEyeTribe::eye_avg_t _eye_right_avg;
	TheEyeTribe::pupil_size_t _pupil_left_size;
	TheEyeTribe::pupil_size_t _pupil_right_size;
	TheEyeTribe::pupil_center_t _pupil_left_center;
	TheEyeTribe::pupil_center_t _pupil_right_center;
	
};



}

#endif

