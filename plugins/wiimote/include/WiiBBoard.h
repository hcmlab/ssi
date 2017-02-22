// WiiBBoard.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/05/20
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

// Provides live recording from Alive Heart Rate Monitor via Bluetooth

#pragma once

#ifndef SSI_SENSOR_WIIBBOARD_H
#define	SSI_SENSOR_WIIBBOARD_H

#include "base/ISensor.h"
#include "base/IProvider.h"
#include "thread/Thread.h"
#include "thread/Timer.h"
#include "ioput/option/OptionList.h"

class wiimote;

namespace ssi {

#define SSI_WIIBBOARD_SAMPLE_RATE			50.0
#define SSI_WIIBBOARD_RAW_SAMPLE_TYPE		short
#define SSI_WIIBBOARD_RAW_PROVIDER_NAME		"raw"
#define SSI_WIIBBOARD_FLT_SAMPLE_TYPE		float
#define SSI_WIIBBOARD_FLT_PROVIDER_NAME		"flt"

#define SSI_WIIBBOARD_FIRST_AVAILABLE		0xffffffff

class WiiBBoard : public ISensor, public Thread {

public:

	class RawChannel : public IChannel {
		friend class WiiBBoard;
		public:
			RawChannel () {
				ssi_stream_init (stream, 0, 4, sizeof(SSI_WIIBBOARD_RAW_SAMPLE_TYPE), SSI_SHORT, SSI_WIIBBOARD_SAMPLE_RATE);
			}
			~RawChannel () {
				ssi_stream_destroy (stream);
			}
			const ssi_char_t *getName () { return SSI_WIIBBOARD_RAW_PROVIDER_NAME; };
			const ssi_char_t *getInfo () { return "Reports corner weights as 4 short values in order Top-Left, Top-Right, Bottom-Left, Bottom-Right."; };
			ssi_stream_t getStream () { return stream; };
		protected:
			ssi_stream_t stream;
	};

	class FltChannel : public IChannel {
		friend class WiiBBoard;
		public:
			FltChannel () {
				ssi_stream_init (stream, 0, 4, sizeof(SSI_WIIBBOARD_FLT_SAMPLE_TYPE), SSI_FLOAT, SSI_WIIBBOARD_SAMPLE_RATE);
			}
			~FltChannel () {
				ssi_stream_destroy (stream);
			}
			const ssi_char_t *getName () { return SSI_WIIBBOARD_RAW_PROVIDER_NAME; };
			const ssi_char_t *getInfo () { return "Reports corner weights as 4 float vlaues in order Top-Left, Top-Right, Bottom-Left, Bottom-Right."; };
			ssi_stream_t getStream () { return stream; };
		protected:
			ssi_stream_t stream;
	};

public:

	class Options : public OptionList {

	public:

		Options ()
			: device (SSI_WIIBBOARD_FIRST_AVAILABLE), size (0.2) {

			addOption ("device", &device, 1, SSI_INT, "device id (first available = -1)");
			addOption ("size", &size, 1, SSI_DOUBLE, "block size in seconds");		
		};		

		int device;
		ssi_time_t size;
	};

public:

	static const ssi_char_t *GetCreateName () { return "WiiBBoard"; };
	static IObject *Create (const ssi_char_t *file) { return new WiiBBoard (file); };
	~WiiBBoard ();
	WiiBBoard::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Connects Nintendo's Balance Board and reports values from the four pressure sensors."; };

	ssi_size_t getChannelSize () { return 2; };
	IChannel *getChannel (ssi_size_t index) { if (index == 0) return &_raw_channel; if (index == 1) return &_raw_channel; return 0; };
	bool setProvider (const ssi_char_t *name, IProvider *provider);
	bool connect ();
	
	bool disconnect ();
	bool start () { return Thread::start (); }
	void run ();
	bool stop () { return Thread::stop (); }
	int	 getDeviceId() const { return _options.device; }
	wiimote *getDevice () { return _wiimote; }

	void setLogLevel (int level) {
		ssi_log_level = level;
	}

protected:

	WiiBBoard (const ssi_char_t *file = 0);
	WiiBBoard::Options _options;
	ssi_char_t *_file;

	RawChannel _raw_channel;
	FltChannel _flt_channel;
	void setRawProvider (IProvider *provider);
	void setFltProvider (IProvider *provider);

	static ssi_char_t *ssi_log_name;
	int ssi_log_level;

	ssi_size_t _buffer_size;
	ssi_size_t _buffer_counter;

	IProvider *_raw_provider;
	SSI_WIIBBOARD_RAW_SAMPLE_TYPE *_raw_buffer;
	SSI_WIIBBOARD_RAW_SAMPLE_TYPE *_raw_buffer_ptr;

	IProvider *_flt_provider;
	SSI_WIIBBOARD_FLT_SAMPLE_TYPE *_flt_buffer;
	SSI_WIIBBOARD_FLT_SAMPLE_TYPE *_flt_buffer_ptr;

	wiimote *_wiimote;
	Timer *_timer;	
};

}

#endif

