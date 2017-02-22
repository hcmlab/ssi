// MySensor.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/09/17
// Copyright (C) 2007-14 University of Augsburg, Lab for Human Centered Multimedia
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

#ifndef _MYSENSOR_H
#define _MYSENSOR_H

#include "base/ISensor.h"
#include "ioput/option/OptionList.h"
#include "thread/Timer.h"
#include "thread/Thread.h"

namespace ssi {

#define MYSENSOR_PROVIDER_NAME "cursor"
#define MYSENSOR_SAMPLE_TYPE ssi_real_t

class MySensor : public ISensor, public Thread {

public:

	class MyChannel : public IChannel {

		friend class MySensor;

	public:

		MyChannel() {
			ssi_stream_init(stream, 0, 2, sizeof(MYSENSOR_SAMPLE_TYPE), SSI_REAL, 0);
		}
		~MyChannel() {
			ssi_stream_destroy(stream);
		}

		const ssi_char_t *getName() { return MYSENSOR_PROVIDER_NAME; };
		const ssi_char_t *getInfo() { return "mouse cursor as float values in the interval [0..1]"; };
		ssi_stream_t getStream() { return stream; };

	protected:

		ssi_stream_t stream;
	};

public:

	class Options : public OptionList {

	public:

		Options()
			: sr(50.0) {

			addOption("sr", &sr, 1, SSI_DOUBLE, "sample rate in Hz");
		};

		ssi_time_t sr;
	};

public:

	static const ssi_char_t *GetCreateName() { return "ssi_sensor_MySensor"; };
	static IObject *Create(const ssi_char_t *file) { return new MySensor(file); };
	~MySensor();

	Options *getOptions() { return &_options; };
	const ssi_char_t *getName() { return GetCreateName(); };
	const ssi_char_t *getInfo() { return "captures mouse coordinates from screen"; };

	ssi_size_t getChannelSize() { return 1; };
	IChannel *getChannel(ssi_size_t index) { return &_channel; };
	bool setProvider(const ssi_char_t *name, IProvider *provider);

	bool connect();
	bool start() { return Thread::start(); };
	bool stop() { return Thread::stop(); };
	void run();
	bool disconnect();

protected:

	MySensor(const ssi_char_t *file = 0);
	Options _options;
	ssi_char_t *_file;
	static char ssi_log_name[];

	MyChannel _channel;
	IProvider *_provider;
	ssi_real_t _max_x, _max_y;
	Timer *_timer;

};

}

#endif
