// SensingTex.h
// author: Andreas Seiderer <seiderer@hcm-lab.de>
// created: 2014/09/02
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

#ifndef SSI_ENTERFACE12_SENSINGTEXSENSOR_H
#define	SSI_ENTERFACE12_SENSINGTEXSENSOR_H

#include "base/ISensor.h"
#include "base/IProvider.h"
#include "thread/Thread.h"
#include "thread/Timer.h"
#include "ioput/option/OptionList.h"

#include <Windows.h>

class Serial;

#define SSI_SENSINGTEX_PROVIDER_NAME "pressurematrix"

namespace ssi {

	typedef struct {
		unsigned char nrowspercol;
		unsigned char col;
		short value[16];
	} RMS_Matrix_Col_t;

	

class SensingTex : public ISensor, public Thread {

public:

	class PressureMatrixChannel : public IChannel {

		friend class SensingTex;

		public:

			PressureMatrixChannel () {
				ssi_stream_init (stream, 0, 1, sizeof (ssi_real_t), SSI_REAL, 5.0);
			}
			~PressureMatrixChannel() {
				ssi_stream_destroy (stream);
			}

			const ssi_char_t *getName () { return SSI_SENSINGTEX_PROVIDER_NAME; };
			const ssi_char_t *getInfo () { return "Reports pressure matrix."; };
			ssi_stream_t getStream () { return stream; };

		protected:
			ssi_stream_t stream;
	};

public:

	class Options : public OptionList {

	public:

		Options ()
			: port(1), size(0.1), scale (true), cols(1), rows(1), sr(5.0) {

			addOption ("sr", &sr, 1, SSI_DOUBLE, "sample rate in Hz [1,21]");
			addOption ("port", &port, 1, SSI_SIZE, "port number");
			addOption ("cols", &cols, 1, SSI_SIZE, "columns [1,16]");
			addOption ("rows", &rows, 1, SSI_SIZE, "rows [1,16]");
			addOption ("size", &size, 1, SSI_DOUBLE, "block size in seconds");
			addOption ("scale", &scale, 1, SSI_BOOL, "scale cursor to interval [0.0,1.0]");
		};		

		ssi_size_t port;
		char cols, rows;
		double sr;
		ssi_time_t size;
		bool scale;
	};

public:

	static const ssi_char_t *GetCreateName () { return "SensingTex"; };
	static IObject *Create(const ssi_char_t *file) { return new SensingTex(file); };
	~SensingTex();

	SensingTex::Options *getOptions() { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "SensingTex Sensor"; };

	ssi_size_t getChannelSize () { return 64; };
	IChannel *getChannel (ssi_size_t index) { return &_pressurematrix_channel; };
	bool setProvider (const ssi_char_t *name, IProvider *provider);

	bool connect ();
	bool start () { return Thread::start (); };
	bool stop () { return Thread::stop (); };
	void run ();
	bool disconnect ();

	void setLogLevel (int level) {
		ssi_log_level = level;
	}

	void checkOptions ();

protected:

	SensingTex(const ssi_char_t *file = 0);
	SensingTex::Options _options;
	ssi_char_t *_file;

	int ssi_log_level;

	PressureMatrixChannel _pressurematrix_channel;
	void setPressureMatrixProvider(IProvider *provider);
	IProvider *_pressurematrix_provider;
	ssi_size_t _n_serial_buffer;
	unsigned char *_serial_buffer;

	ssi_size_t _frame_size;
	ssi_size_t _counter;
	
	ssi_real_t *_pressurematrix_buffer;
	bool *_received_cols;

	Serial *_serial;
	bool _is_connected;
	ssi_char_t _port_s[16];

	Timer *_timer;

	_int64 lastCall;
	_int64 fpsValuecount;
	double avgFps;

};

}

#endif

