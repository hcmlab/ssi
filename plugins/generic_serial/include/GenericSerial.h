// GenericSerial.h
// author: Andreas Seiderer <seiderer@hcm-lab.de>
// created: 9/3/2015
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************


#pragma once

#ifndef SSI_GERNERICSERIAL_H
#define	SSI_GERNERICSERIAL_H

#include "base/ISensor.h"
#include "base/IProvider.h"
#include "thread/Thread.h"
#include "ioput/option/OptionList.h"

#include <map>
#include <sstream>


// see for linux implementation
// https://github.com/voodootikigod/node-serialport


#include <Windows.h>

#include "listComPorts.h"


class Serial;

#define SSI_GENERICSERIAL_PROVIDER_NAME "serialrawdata"

namespace ssi {

class GenericSerial : public ISensor, public Thread {

public:

	class SerialChannel : public IChannel {

		friend class GenericSerial;

		public:

			SerialChannel (ssi_size_t dim, ssi_time_t sr) {
				ssi_stream_init (stream, 0, dim, sizeof (ssi_real_t), SSI_REAL, sr);
			}
			~SerialChannel () {
				ssi_stream_destroy (stream);
			}

			const ssi_char_t *getName () { return SSI_GENERICSERIAL_PROVIDER_NAME; };
			const ssi_char_t *getInfo () { return "Gets all serial data."; };
			ssi_stream_t getStream () { return stream; };

		protected:
			ssi_stream_t stream;
	};

public:

	class Options : public OptionList {

	public:
		Options ()
			: port (1), baud (9600), sr (50), dim(3), separator(','), size(0.2), skipLinesAfterStart(0), skipLinesAfterStartCMD(0), useId(false), showDebugSR(false) {

			addOption ("port", &port, 1, SSI_SIZE, "port number");
			addOption ("useId", &useId, 1, SSI_BOOL, "use device instance id instead of port number");
			addOption ("deviceInstanceId", &deviceInstanceId, SSI_MAX_CHAR, SSI_CHAR, "device instance id");


			addOption ("baud", &baud, 1, SSI_SIZE, "baud rate");									
			addOption ("separator", &separator, 1, SSI_CHAR, "separator used to split input data");
			addOption ("sr", &sr, 1, SSI_SIZE, "sampling rates (hz)");
			addOption ("dim", &dim, 1, SSI_SIZE, "value count");
			addOption ("size", &size, 1, SSI_DOUBLE, "block size in seconds");

			addOption ("startCMD", &startCMD, SSI_MAX_CHAR, SSI_CHAR, "command to be sent after first connection with controller (leave empty to send no command)");
			addOption ("skipLinesAfterStart", &skipLinesAfterStart, 1, SSI_INT, "skip lines from controller after connection" );
			addOption ("skipLinesAfterStartCMD", &skipLinesAfterStartCMD, 1, SSI_INT, "skip lines from controller after start command" );
			addOption ("stopCMD", &stopCMD, SSI_MAX_CHAR, SSI_CHAR, "command to be sent to controller during pipeline shutdown (leave empty to send no command)");

			addOption("showDebugSR", &showDebugSR, 1, SSI_BOOL, "show mean sample rate of input data");
		};

		void setStartCMD (const ssi_char_t *startCMD) {
			this->startCMD[0] = '\0';
			if (startCMD) {
				ssi_strcpy (this->startCMD, startCMD);
			}
		};
		
		void setStopCMD (const ssi_char_t *stopCMD) {
			this->stopCMD[0] = '\0';
			if (stopCMD) {
				ssi_strcpy (this->stopCMD, stopCMD);
			}
		};

		void setDeviceInstanceId (const ssi_char_t *deviceInstanceId) {
			this->deviceInstanceId[0] = '\0';
			if (deviceInstanceId) {
				ssi_strcpy (this->deviceInstanceId, deviceInstanceId);
			}
		};

		ssi_size_t port;
		ssi_size_t baud;
		ssi_char_t separator;
		ssi_size_t sr;
		ssi_size_t dim;
		ssi_time_t size;

		ssi_char_t startCMD[SSI_MAX_CHAR];
		ssi_char_t stopCMD[SSI_MAX_CHAR];
		ssi_size_t skipLinesAfterStart;
		ssi_size_t skipLinesAfterStartCMD;

		bool useId;
		ssi_char_t deviceInstanceId[SSI_MAX_CHAR];

		bool showDebugSR;

	};

public:

	static const ssi_char_t *GetCreateName () { return "GenericSerial"; };
	static IObject *Create (const ssi_char_t *file) { return new GenericSerial (file); };
	~GenericSerial ();
	GenericSerial::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Generic Serial Sensor"; };

	ssi_size_t getChannelSize () { return 1; };

	IChannel *getChannel (ssi_size_t index) {  
		if (!_serial_channel) _serial_channel = new SerialChannel(_options.dim, _options.sr); 
		return _serial_channel; 
	};

	bool setProvider (const ssi_char_t *name, IProvider *provider);
	bool connect ();
	bool start () { return Thread::start (); };
	bool stop () { return Thread::stop (); };
	void run ();
	bool disconnect ();


	//event listener
	void listen_enter();
	bool update(IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms);
	void listen_flush();


	void setLogLevel (int level) {
		ssi_log_level = level;
	}

protected:

	GenericSerial (const ssi_char_t *file = 0);
	GenericSerial::Options _options;
	ssi_char_t *_file;

	int ssi_log_level;

	SerialChannel *_serial_channel;	
	void setSerialProvider (IProvider *provider);		
	IProvider *_serial_provider;

	Serial *_serial;
	bool _is_connected;
	ssi_char_t _port_s[16];

	ssi_real_t *_buffer, *_buffer_ptr;

	ssi_size_t _counter;
	ssi_size_t _frame_size;

	std::map<int, unsigned long> baudRates;


	_int64 lastCall;
	_int64 fpsValuecount;
	double avgFps;
};

}

#endif

