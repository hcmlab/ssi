// Sensor.cpp
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

#include "Sensor.h"
#include "TheFramework.h"
#include "base/Factory.h"

namespace ssi {

int Sensor::ssi_log_level = SSI_LOG_LEVEL_DEFAULT;
ssi_char_t *Sensor::ssi_log_name = "sensor____";

Sensor::Sensor (ISensor *sensor)
	: _sensor (sensor),
	_connected (false) {

	_frame = ssi_pcast (TheFramework, Factory::GetFramework ());

	// add consumer to framework
	if (_frame->IsAutoRun ()) {
		_frame->AddRunnable (this);
	}
}

Sensor::~Sensor () {
}

bool Sensor::start () {

	ssi_msg (SSI_LOG_LEVEL_BASIC, "connect '%s'", _sensor->getName ());

	if (_connected) {
		ssi_wrn ("already connected '%s'", _sensor->getName ());
		return false;
	}

	_connected = _sensor->connect ();

	if (!_connected) {
		ssi_wrn ("could not connect '%s'", _sensor->getName ());
		return false;
	}	

	ssi_msg (SSI_LOG_LEVEL_BASIC, "start '%s'", _sensor->getName ());

	if (!_sensor->start ()) {
		ssi_wrn ("start failed '%s'", _sensor->getName ());
	}

	return true;
}

bool Sensor::stop () {

	ssi_msg (SSI_LOG_LEVEL_BASIC, "stop '%s'", _sensor->getName ());	

	if (!_connected) {
		ssi_wrn ("not connected '%s'", _sensor->getName ());
		return false;
	}	

	if (!_sensor->stop ()) {
		ssi_wrn ("stop failed '%s'", _sensor->getName ());
		return false;
	}

	ssi_msg (SSI_LOG_LEVEL_BASIC, "disconnect '%s'", _sensor->getName ());	

	_connected = !_sensor->disconnect ();

	if (_connected) {
		ssi_wrn ("could not disconnect '%s'", _sensor->getName ());
		return false;
	}

	return true;

}

}
