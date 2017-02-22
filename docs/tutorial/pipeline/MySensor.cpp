// MySensor.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/09/17
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

#include "MySensor.h"

namespace ssi {

char MySensor::ssi_log_name[] = "mysensor__";

MySensor::MySensor(const ssi_char_t *file)
	: _provider(0),
	_timer(0),
	_file(0) {

	if (file) {
		if (!OptionList::LoadXML(file, _options)) {
			OptionList::SaveXML(file, _options);
		}
		_file = ssi_strcpy(file);
	}

	Thread::setName(getName());
}

MySensor::~MySensor() {

	if (_file) {
		OptionList::SaveXML(_file, _options);
		delete[] _file;
	}
}

bool MySensor::setProvider(const ssi_char_t *name, IProvider *provider) {

	if (strcmp(name, MYSENSOR_PROVIDER_NAME) == 0) {
		_provider = provider;
		_channel.stream.sr = _options.sr;
		_provider->init(&_channel);
		return true;
	}

	ssi_wrn("unkown provider name '%s'", name);
	return false;
}

bool MySensor::connect() {

	if (!_provider) {
		ssi_err("provider not set");
	}

	RECT rect;
	HWND desktop = ::GetDesktopWindow();
	::GetWindowRect(desktop, &rect);
	_max_x = ssi_cast(float, rect.right);
	_max_y = ssi_cast(float, rect.bottom);

	_timer = new Timer(1 / _options.sr);

	ssi_msg(SSI_LOG_LEVEL_BASIC, "connect()..ok");

	return true;
}

void MySensor::run() {

	POINT point;
	float cursor[2];
	::GetCursorPos(&point);
	cursor[0] = point.x / _max_x;
	cursor[1] = point.y / _max_y;

	_provider->provide(ssi_pcast(ssi_byte_t, cursor), 1);
	_timer->wait();
}

bool MySensor::disconnect() {

	delete _timer; _timer = 0;

	ssi_msg(SSI_LOG_LEVEL_BASIC, "disconnect()..ok");

	return true;
}

}
