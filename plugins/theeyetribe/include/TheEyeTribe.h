// eyetribe.h
// author: Tobias Baur <baur@hcm-lab.de>
// created: 2013/02/28
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

// Provides eyetribe eyegaze tracking

#pragma once

#ifndef SSI_SENSOR_THEEYETRIBE_H
#define	SSI_SENSOR_THEEYETRIBE_H

#include "base/ISensor.h"
#include "base/IProvider.h"
#include "thread/ClockThread.h"
#include "ioput/option/OptionList.h"

namespace ssi {

#define SSI_THEEYETRIBE_INVALID_VALUE -1.0f

class GazeListener;

class TheEyeTribe : public ISensor, public ClockThread {

public:

	typedef ssi_real_t confidence_t;
	typedef ssi_real_t gaze_raw_t[2];
	typedef ssi_real_t gaze_avg_t[2];
	typedef ssi_real_t eye_raw_t[2];
	typedef ssi_real_t eye_avg_t[2];
	typedef ssi_real_t pupil_size_t;
	typedef ssi_real_t pupil_center_t[2];

public:

	struct CHANNELS {
		enum List {
			CONFIDENCE,
			GAZE_RAW,
			GAZE_AVG,
			EYE_LEFT_RAW,
			EYE_RIGHT_RAW,
			EYE_LEFT_AVG,
			EYE_RIGHT_AVG,
			PUPIL_LEFT_SIZE,
			PUPIL_RIGHT_SIZE,
			PUPIL_LEFT_CENTER,
			PUPIL_RIGHT_CENTER,
			NUM
		};
	};
	static const char *CHANNELS_STRING[];

	class Channel : public IChannel {
		friend class TheEyeTribe;
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

	class ConfidenceChannel : public Channel {
	public:
		ConfidenceChannel() 
		: Channel (1) {			
		}
		static const ssi_char_t *GetName() { return "confidence"; };
		const ssi_char_t *getName() { return GetName(); };
		const ssi_char_t *getInfo() { return "Confidence value"; };		
	};

	class GazeRawChannel : public Channel {
	public:
		GazeRawChannel()
			: Channel(2) {
		}
		static const ssi_char_t *GetName() { return "gazeraw"; };
		const ssi_char_t *getName() { return GetName(); };
		const ssi_char_t *getInfo() { return "Raw eye gaze"; };
	};

	class GazeAvgChannel : public Channel {
	public:
		GazeAvgChannel()
			: Channel(2) {
		}
			static const ssi_char_t *GetName() { return "gazeavg"; };
			const ssi_char_t *getName() { return GetName(); };
			const ssi_char_t *getInfo () { return "Average eye gaze"; };
			ssi_stream_t getStream () { return stream; };
	};

	class EyeLeftRawChannel : public Channel {
	public:
		EyeLeftRawChannel()
			: Channel(2) {
		}
		static const ssi_char_t *GetName() { return "eyeleftraw"; };
		const ssi_char_t *getName() { return GetName(); };
		const ssi_char_t *getInfo() { return "Raw position of left eye"; };
		ssi_stream_t getStream() { return stream; };
	};

	class EyeRightRawChannel : public Channel {
	public:
		EyeRightRawChannel()
			: Channel(2) {
		}
		static const ssi_char_t *GetName() { return "eyerightraw"; };
		const ssi_char_t *getName() { return GetName(); };
		const ssi_char_t *getInfo() { return "Raw position of right eye"; };
		ssi_stream_t getStream() { return stream; };
	};

	class EyeLeftAvgChannel : public Channel {
	public:
		EyeLeftAvgChannel()
			: Channel(2) {
		}
		static const ssi_char_t *GetName() { return "eyeleftavg"; };
		const ssi_char_t *getName() { return GetName(); };
		const ssi_char_t *getInfo() { return "Average position of left eye"; };
		ssi_stream_t getStream() { return stream; };
	};

	class EyeRightAvgChannel : public Channel {
	public:
		EyeRightAvgChannel()
			: Channel(2) {
		}
		static const ssi_char_t *GetName() { return "eyerightavg"; };
		const ssi_char_t *getName() { return GetName(); };
		const ssi_char_t *getInfo() { return "Average position of right eye"; };
		ssi_stream_t getStream() { return stream; };
	};

	class PupilLeftSizeChannel : public Channel {
	public:
		PupilLeftSizeChannel()
			: Channel(1) {
		}
		static const ssi_char_t *GetName() { return "pupilleftsize"; };
		const ssi_char_t *getName() { return GetName(); };
		const ssi_char_t *getInfo() { return "Pupil size of left eye"; };
		ssi_stream_t getStream() { return stream; };
	};

	class PupilRightSizeChannel : public Channel {
	public:
		PupilRightSizeChannel()
			: Channel(1) {
		}
		static const ssi_char_t *GetName() { return "pupilrightsize"; };
		const ssi_char_t *getName() { return GetName(); };
		const ssi_char_t *getInfo() { return "Pupil size of right eye"; };
		ssi_stream_t getStream() { return stream; };
	};

	class PupilLeftCenterChannel : public Channel {
	public:
		PupilLeftCenterChannel()
			: Channel(2) {
		}
		static const ssi_char_t *GetName() { return "pupilleftcenter"; };
		const ssi_char_t *getName() { return GetName(); };
		const ssi_char_t *getInfo() { return "Pupil center of left eye"; };
		ssi_stream_t getStream() { return stream; };
	};

	class PupilRightCenterChannel : public Channel {
	public:
		PupilRightCenterChannel()
			: Channel(2) {
		}
		static const ssi_char_t *GetName() { return "pupilrightcenter"; };
		const ssi_char_t *getName() { return GetName(); };
		const ssi_char_t *getInfo() { return "Pupil center of right eye"; };
		ssi_stream_t getStream() { return stream; };
	};

public:

	class Options : public OptionList {

	public:

		Options ()
			: sr(30), port(6555), keep(1)
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

	static const ssi_char_t *GetCreateName () { return "TheEyeTribe"; };
	static IObject *Create (const ssi_char_t *file) { return new TheEyeTribe (file); };
	~TheEyeTribe ();
	TheEyeTribe::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "TheEyeTribe tracker"; };

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

	TheEyeTribe (const ssi_char_t *file = 0);
	TheEyeTribe::Options _options;
	ssi_char_t *_file;

	static ssi_char_t *ssi_log_name;
	int ssi_log_level;

	void setProvider(int channel, IProvider *provider);
	IChannel *_channel[CHANNELS::NUM];
	IProvider *_provider[CHANNELS::NUM];

	confidence_t _confidence;
	gaze_raw_t _gaze_raw;
	gaze_avg_t _gaze_avg;
	eye_raw_t _eye_left_raw;
	eye_raw_t _eye_right_raw;
	eye_avg_t _eye_left_avg;
	eye_avg_t _eye_right_avg;
	pupil_size_t _pupil_left_size;
	pupil_size_t _pupil_right_size;
	pupil_center_t _pupil_left_center;
	pupil_center_t _pupil_right_center;

	GazeListener *_gaze_listener;
};

}

#endif

