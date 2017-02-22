// EHealth.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2013/06/05
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

#ifndef SSI_SENSOR_EHEALTH_H
#define	SSI_SENSOR_EHEALTH_H

#include "base/ISensor.h"
#include "base/IProvider.h"
#include "thread/Thread.h"
#include "ioput/option/OptionList.h"
#include "Serial.h"

namespace ssi {

#define SSI_EHEALTH_ECG_PROVIDER_NAME "ecg"
#define SSI_EHEALTH_ECG_INDEX 0
#define SSI_EHEALTH_GSR_PROVIDER_NAME "gsr"
#define SSI_EHEALTH_GSR_INDEX 1
#define SSI_EHEALTH_AIR_PROVIDER_NAME "air"
#define SSI_EHEALTH_AIR_INDEX 2
#define SSI_EHEALTH_TMP_PROVIDER_NAME "tmp"
#define SSI_EHEALTH_TMP_INDEX 3
#define SSI_EHEALTH_BPM_PROVIDER_NAME "bpm"
#define SSI_EHEALTH_BPM_INDEX 4
#define SSI_EHEALTH_OXY_PROVIDER_NAME "oxy"
#define SSI_EHEALTH_OXY_INDEX 5
#define SSI_EHEALTH_N_CHANNELS 6
#define SSI_EHEALTH_N_BUFFER 100
#define SSI_EHEALTH_CMD_START "start"
#define SSI_EHEALTH_CMD_STOP "stop"

class EHealth : public ISensor, public Thread {

public:

	class Options : public OptionList {

	public:

		Options () : port (0), sr (100) {
						
			addOption ("port", &port, 1, SSI_SIZE, "number of com port");		
			addOption ("sr", &sr, 1, SSI_SIZE, "sampling rates (hz)");		
		};		

		ssi_size_t port;
		ssi_size_t sr;
	};

public:

	class GsrChannel : public IChannel {
		public:
			GsrChannel (ssi_time_t sr) {				
				ssi_stream_init (stream, 0, 1, sizeof (ssi_real_t), SSI_REAL, sr);
			}
			~GsrChannel () {
				ssi_stream_destroy (stream);
			}
			const ssi_char_t *getName () { return SSI_EHEALTH_GSR_PROVIDER_NAME; };
			const ssi_char_t *getInfo () { return "Provides measure of skin conductance, also known as galvanic skin response (GSR)."; };
			ssi_stream_t getStream () { return stream; };
		protected:
			ssi_stream_t stream;			
	};
	
	class EcgChannel : public IChannel {
		public:
			EcgChannel (ssi_time_t sr) {				
				ssi_stream_init (stream, 0, 1, sizeof (ssi_real_t), SSI_REAL, sr);
			}
			~EcgChannel () {
				ssi_stream_destroy (stream);
			}
			const ssi_char_t *getName () { return SSI_EHEALTH_ECG_PROVIDER_NAME; };
			const ssi_char_t *getInfo () { return "Provides measure of electrocardiogram (ECG)."; };
			ssi_stream_t getStream () { return stream; };
		protected:
			ssi_stream_t stream;			
	};

	class AirChannel : public IChannel {
		public:
			AirChannel (ssi_time_t sr) {				
				ssi_stream_init (stream, 0, 1, sizeof (ssi_real_t), SSI_REAL, sr);
			}
			~AirChannel () {
				ssi_stream_destroy (stream);
			}
			const ssi_char_t *getName () { return SSI_EHEALTH_AIR_PROVIDER_NAME; };
			const ssi_char_t *getInfo () { return "Provides measure of airflow."; };
			ssi_stream_t getStream () { return stream; };
		protected:
			ssi_stream_t stream;			
	};

	class TmpChannel : public IChannel {
		public:
			TmpChannel (ssi_time_t sr) {				
				ssi_stream_init (stream, 0, 1, sizeof (ssi_real_t), SSI_REAL, sr);
			}
			~TmpChannel () {
				ssi_stream_destroy (stream);
			}
			const ssi_char_t *getName () { return SSI_EHEALTH_TMP_PROVIDER_NAME; };
			const ssi_char_t *getInfo () { return "Provides measure of temperature."; };
			ssi_stream_t getStream () { return stream; };
		protected:
			ssi_stream_t stream;			
	};

	class BpmChannel : public IChannel {
		public:
			BpmChannel (ssi_time_t sr) {				
				ssi_stream_init (stream, 0, 1, sizeof (ssi_real_t), SSI_REAL, sr);
			}
			~BpmChannel () {
				ssi_stream_destroy (stream);
			}
			const ssi_char_t *getName () { return SSI_EHEALTH_BPM_PROVIDER_NAME; };
			const ssi_char_t *getInfo () { return "Provides measure pulse (bpm)."; };
			ssi_stream_t getStream () { return stream; };
		protected:
			ssi_stream_t stream;			
	};

	class OxyChannel : public IChannel {
		public:
			OxyChannel (ssi_time_t sr) {				
				ssi_stream_init (stream, 0, 1, sizeof (ssi_real_t), SSI_REAL, sr);
			}
			~OxyChannel () {
				ssi_stream_destroy (stream);
			}
			const ssi_char_t *getName () { return SSI_EHEALTH_OXY_PROVIDER_NAME; };
			const ssi_char_t *getInfo () { return "Provides measure of oxygen saturation."; };
			ssi_stream_t getStream () { return stream; };
		protected:
			ssi_stream_t stream;			
	};


public:

	static const ssi_char_t *GetCreateName () { return "EHealth"; };
	static IObject *Create (const ssi_char_t *file) { return new EHealth (file); };
	~EHealth ();
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Connects glove device via serial com port."; };
	Options *getOptions () { return &_options; };

	bool setProvider (const ssi_char_t *name, IProvider *provider);
	ssi_size_t getChannelSize () { return SSI_EHEALTH_N_CHANNELS; };
	IChannel *getChannel (ssi_size_t index);

	bool connect ();
	bool start () { return Thread::start (); };
	void run ();
	bool stop () { return Thread::stop (); };
	bool disconnect ();
	bool supportsReconnect () { return false; };

	void setLogLevel (int level) {
		ssi_log_level = level;
	}

protected:

	EHealth (const ssi_char_t *file);	
	Options _options;
	ssi_char_t *_file;
	
	static ssi_char_t ssi_log_name[];
	int ssi_log_level;

	void sendCommand (const char *cmd);

	Serial *_serial;
	char _com_port[16];
	char _buffer[SSI_EHEALTH_N_BUFFER];
	ssi_real_t _values[SSI_EHEALTH_N_CHANNELS];

	IProvider *_provider[SSI_EHEALTH_N_CHANNELS];
	IChannel *_channel[SSI_EHEALTH_N_CHANNELS];
	
};

}

#endif

