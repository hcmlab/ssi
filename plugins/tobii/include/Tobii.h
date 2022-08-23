//
// author: Alexander Heimerl <heimerl@hcai.eu>, Tobias Baur <baur@hcai.eu
// created: 2022/08/23
// Copyright (C) University of Augsburg, Lab for Human Centered Artificial Intelligence
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

// Provides tobii eyegaze tracking

#pragma once

#ifndef SSI_SENSOR_TOBII_H
#define	SSI_SENSOR_TOBII_H

#include "base/ISensor.h"
#include "base/IProvider.h"
#include "thread/ClockThread.h"
#include "thread/Lock.h"
#include "ioput/option/OptionList.h"

#include "tobii_research.h"
#include "tobii_research_eyetracker.h"
#include "tobii_research_streams.h"


namespace ssi {

#define SSI_Tobii_INVALID_VALUE -1.0f
#define SSI_Tobii_GAZE_DATA_LENGTH 22

class GazeListener;

class Tobii : public ISensor, public ClockThread {

public:

	typedef ssi_real_t gaze_data_t[SSI_Tobii_GAZE_DATA_LENGTH];

public:


	struct CHANNELS {
		enum List {
			GAZE_DATA,
			NUM
		};
	};
	static const char *CHANNELS_STRING[];

	class Channel : public IChannel {
		friend class Tobii;
	public:
		Channel(ssi_size_t dim) {
			ssi_stream_init(stream, 0, dim, sizeof(ssi_real_t), SSI_REAL, 0);
		}
		~Channel() {
			ssi_stream_destroy(stream);
		}
		ssi_stream_t getStream() { return stream; };
	protected:
		ssi_stream_t stream;
	};

	class GazeChannel : public Channel {
	public:
		GazeChannel() 
		: Channel (SSI_Tobii_GAZE_DATA_LENGTH) {
		}
		static const ssi_char_t *GetName() { return "gaze"; };
		const ssi_char_t *getName() { return GetName(); };
		const ssi_char_t *getInfo() { return "gaze data"; };		
	};

public:

	class Options : public OptionList {

	public:

		Options ()
			: sr(120), port(6555), keep(1)
		{
			addOption ("sr", &sr, 1, SSI_TIME, "sample rate in hz");
			addOption("port", &port, 1, SSI_SIZE, "server port");
			addOption("keep", &keep, 1, SSI_SIZE, "keep last valid result for x samples");
		};		

		ssi_time_t sr;
		ssi_size_t port;
		ssi_size_t keep;
	};

public:

	static const ssi_char_t *GetCreateName () { return "tobii"; };
	static IObject *Create (const ssi_char_t *file) { return new Tobii (file); };
	~Tobii ();
	Tobii::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Tobii tracker"; };

	ssi_size_t getChannelSize () { return CHANNELS::NUM; };
	IChannel *getChannel (ssi_size_t index) { 
		if (index < CHANNELS::NUM) {		
			return _channel[index];			
		}
		return 0;
	};
	bool setProvider (const ssi_char_t *name, IProvider *provider);
	bool connect ();
	bool start () { return Thread::start (); };
	bool stop () { return Thread::stop (); };
	void clock ();
	bool disconnect ();

	void setLogLevel (int level) {
		ssi_log_level = level;
	}


protected:

	Tobii (const ssi_char_t *file = 0);
	Tobii::Options _options;
	ssi_char_t *_file;

	static ssi_char_t *ssi_log_name;
	int ssi_log_level;

	void setProvider(int channel, IProvider *provider);
	IChannel *_channel[CHANNELS::NUM];
	IProvider *_provider[CHANNELS::NUM];

	gaze_data_t _gaze_data;
	TobiiResearchEyeTracker* eyetracker;
	TobiiResearchGazeData  gaze_data;

};

}

#endif

