// Iom.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/01/04
// Copyright (C) University of Augsburg
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

#ifndef SSI_SENSOR_IOM_H
#define	SSI_SENSOR_IOM_H

#include "base/ISensor.h"
#include "base/IProvider.h"
#include "thread/Thread.h"
#include "ioput/option/OptionList.h"
#include "lightstone.h"

namespace ssi {

#define SSI_IOM_SAMPLE_TYPE float
#define SSI_IOM_SAMPLE_RATE 27.0
#define SSI_IOM_BVP_PROVIDER_NAME "bvp"
#define SSI_IOM_SC_PROVIDER_NAME "sc"

class Iom : public ISensor, public Thread {

public:

	class HrvChannel : public IChannel {
		friend class Iom;
		public:
			HrvChannel () {
				ssi_stream_init (stream, 0, 1, sizeof(SSI_IOM_SAMPLE_TYPE), SSI_FLOAT, SSI_IOM_SAMPLE_RATE);
			}
			~HrvChannel () {
				ssi_stream_destroy (stream);
			}
			const ssi_char_t *getName () { return SSI_IOM_BVP_PROVIDER_NAME; };
			const ssi_char_t *getInfo () { return "BVP channel streamed as float values at a sample rate of 27 Hz."; };
			ssi_stream_t getStream () { return stream; };
		protected:
			ssi_stream_t stream;
	};

	class ScChannel : public IChannel {
		friend class Iom;
		public:
			ScChannel () {
				ssi_stream_init (stream, 0, 1, sizeof(SSI_IOM_SAMPLE_TYPE), SSI_FLOAT, SSI_IOM_SAMPLE_RATE);
			}
			~ScChannel () {
				ssi_stream_destroy (stream);
			}			
			const ssi_char_t *getName () { return SSI_IOM_SC_PROVIDER_NAME; };
			const ssi_char_t *getInfo () { return "SC channel streamed as float values at a sample rate of 27 Hz."; };
			ssi_stream_t getStream () { return stream; };
		protected:
			ssi_stream_t stream;
	};

public:

	class Options : public OptionList {

	public:

		Options () 
			: block (0.2), id (0) {

			addOption ("id", &id, 1, SSI_SIZE, "device id (0,1,..)");
			addOption ("block", &block, 1, SSI_DOUBLE, "block size in seconds");
		};

		ssi_size_t id;
		ssi_time_t block;
	};

public:

	static const ssi_char_t *GetCreateName () { return "Iom"; };
	static IObject *Create (const ssi_char_t *file) { return new Iom (file); };
	~Iom ();
	Iom::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Iom is a wearable finger sensor that measures HRV and SC."; };

	ssi_size_t getChannelSize () { return 2; };
	IChannel *getChannel (ssi_size_t index) { if (index == 0) return &_hrv; if (index == 1) return &_sc; return 0; };
	bool setProvider (const ssi_char_t *name, IProvider *provider);
	bool connect ();
	bool start () { return Thread::start (); };
	void run ();
	bool stop () { return Thread::stop (); };
	bool disconnect ();
	bool supportsReconnect () { return true; };

	void setLogLevel (int level) {
		ssi_log_level = level;
	}

protected:

	Iom (const ssi_char_t *file = 0);
	Options _options;
	ssi_char_t *_file;

	static ssi_char_t ssi_log_name[];
	int ssi_log_level;

	HrvChannel _hrv;
	ScChannel _sc;
	void setBvpProvider (IProvider *provider);
	void setScProvider (IProvider *provider);
	IProvider *_hrv_provider;
	IProvider *_sc_provider;

	ssi_size_t _buffer_size;		
	ssi_size_t _buffer_count;		
	float *_bvp_buffer;
	float *_bvp_buffer_ptr;
	float *_sc_buffer;
	float *_sc_buffer_ptr;
	float _last_bvp_value;
	float _last_sc_value;

	lightstone *_device; 
};

}

#endif

