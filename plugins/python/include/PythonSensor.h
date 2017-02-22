// PythonSensor.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2016/03/02
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

#pragma once

#ifndef SSI_PYTHON_SENSOR_H
#define SSI_PYTHON_SENSOR_H

#include "base/ISensor.h"
#include "thread/Thread.h"
#include "PythonOptions.h"

namespace ssi {

class Timer;
class PythonHelper;

class PythonSensor : public ISensor, public Thread {

public:

	class Options : public PythonOptions 
	{

	public:

		Options()
		: block(1.0)
		{
			addOption("block", &block, 1, SSI_TIME, "block size in seconds");
		}

		ssi_time_t block;
	};

public:

	static const ssi_char_t *GetCreateName () { return "PythonSensor"; };
	static IObject *Create (const ssi_char_t *file) { return new PythonSensor (file); };
	~PythonSensor ();

	PythonSensor::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Python sensor wrapper"; };

	ssi_size_t getChannelSize();
	IChannel *getChannel(ssi_size_t index);
	bool setProvider(const ssi_char_t *name, IProvider *provider);
	bool connect();
	bool disconnect();

	bool start() { return Thread::start(); };
	bool stop() { return Thread::stop(); };	

	bool setEventListener(IEventListener *listener);
	const ssi_char_t *getEventAddress();
	void send_enter();
	void send_flush();

	void listen_enter();
	bool update(ssi_event_t &e);
	bool update(IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms);
	void listen_flush();

protected:

	PythonSensor (const ssi_char_t *file = 0);
	PythonSensor::Options _options;
	ssi_char_t *_file;

	static ssi_char_t *ssi_log_name;

	Timer *_timer;
	bool _is_providing;
	void enter();
	void run();
	void flush();

	void initHelper();
	PythonHelper *_helper;
};

}

#endif
