// Sensor.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/02/28
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

#pragma once

#ifndef SSI_FRAME_SENSOR_H
#define	SSI_FRAME_SENSOR_H

#include "base/ISensor.h"
#include "base/IRunnable.h"
#include "thread/Lock.h"
#include "thread/Timer.h"

namespace ssi {

class TheFramework;

class Sensor : public IRunnable {

public:

	Sensor (ISensor *sensor);
	virtual ~Sensor ();

	static void SetLogLevel (int level) {
		ssi_log_level = level;
	}

protected:

	static ssi_char_t *ssi_log_name;
	static int ssi_log_level;

	bool start ();
	bool stop ();

	ISensor *_sensor;
	bool _connected;
	TheFramework *_frame;

};

}

#endif
