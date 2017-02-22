// WiiRemote.h
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

#ifndef SSI_SENSOR_WIIREMOTE_H
#define	SSI_SENSOR_WIIREMOTE_H

#include "base/ISensor.h"
#include "base/IProvider.h"
#include "thread/Thread.h"
#include "thread/Timer.h"
#include "ioput/option/OptionList.h"
#include "wiimote.h"

class wiimote;

namespace ssi {

#define SSI_WII_SAMPLE_RATE				50.0
#define SSI_WII_CHANNEL_NUM				7
#define SSI_WII_ACC_CHANNEL_NUM			0
#define SSI_WII_ACC_SAMPLE_TYPE			float
#define SSI_WII_ACC_PROVIDER_NAME		"acceleration"
#define SSI_WII_ORI_CHANNEL_NUM			1
#define SSI_WII_ORI_SAMPLE_TYPE			float
#define SSI_WII_ORI_PROVIDER_NAME		"orientation"
#define SSI_WII_BUT_CHANNEL_NUM			2
#define SSI_WII_BUT_SAMPLE_TYPE			unsigned short
#define SSI_WII_BUT_PROVIDER_NAME		"button"
#define SSI_WII_MPRAW_CHANNEL_NUM		3
#define SSI_WII_MPRAW_SAMPLE_TYPE		short
#define SSI_WII_MPRAW_PROVIDER_NAME	"motionplusraw"
#define SSI_WII_MPFLT_CHANNEL_NUM		4
#define SSI_WII_MPFLT_SAMPLE_TYPE		float
#define SSI_WII_MPFLT_PROVIDER_NAME	"motionplusfloat"
#define SSI_WII_IRFLT_CHANNEL_NUM		5
#define SSI_WII_IRFLT_SAMPLE_TYPE		float
#define SSI_WII_IRFLT_PROVIDER_NAME		"infrared"
#define SSI_WII_IRRAW_CHANNEL_NUM		6
#define SSI_WII_IRRAW_SAMPLE_TYPE		unsigned int
#define SSI_WII_IRRAW_PROVIDER_NAME		"infraredraw"

#define SSI_WII_FIRST_AVAILABLE  0xffffffff

class WiiRemote : public ISensor, public Thread {

public:

	class AccChannel : public IChannel {
		friend class WiiRemote;
		public:
			AccChannel () {
				ssi_stream_init (stream, 0, 3, sizeof(SSI_WII_ACC_SAMPLE_TYPE), SSI_FLOAT, SSI_WII_SAMPLE_RATE);
			}
			~AccChannel () {
				ssi_stream_destroy (stream);
			}
			const ssi_char_t *getName () { return SSI_WII_ACC_PROVIDER_NAME; };
			const ssi_char_t *getInfo () { return "3-axis acceleration (X, Y, and Z) streamed as float values."; };
			ssi_stream_t getStream () { return stream; };
		protected:
			ssi_stream_t stream;
	};

	class OriChannel : public IChannel {
		friend class WiiRemote;
		public:
			OriChannel () {
				ssi_stream_init (stream, 0, 2, sizeof(SSI_WII_ORI_SAMPLE_TYPE), SSI_FLOAT, SSI_WII_SAMPLE_RATE);
			}
			~OriChannel () {
				ssi_stream_destroy (stream);
			}
			const ssi_char_t *getName () { return SSI_WII_ORI_PROVIDER_NAME; };
			const ssi_char_t *getInfo () { return "Pitch and roll as float values estimated if the controller isn't accelerating."; };
			ssi_stream_t getStream () { return stream; };
		protected:
			ssi_stream_t stream;
	};

	class ButChannel : public IChannel {
		friend class WiiRemote;
		public:
			ButChannel () {
				ssi_stream_init (stream, 0, 1, sizeof(SSI_WII_BUT_SAMPLE_TYPE), SSI_SHORT, SSI_WII_SAMPLE_RATE);
			}
			~ButChannel () {
				ssi_stream_destroy (stream);
			}
		public:
			const ssi_char_t *getName () { return SSI_WII_BUT_PROVIDER_NAME; };
			const ssi_char_t *getInfo () { return "State of 11 buttons (LEFT|RIGHT|DOWN|UP|PLUS|TWO|ONE|A|B|MINUS|HOME) stored as bits in a single unsigned short value."; };
			ssi_stream_t getStream () { return stream; };
		protected:
			ssi_stream_t stream;
	};

	class IrRawChannel : public IChannel {
		friend class WiiRemote;
		public:
			IrRawChannel () {
				ssi_stream_init (stream, 0, 2, sizeof(SSI_WII_IRRAW_SAMPLE_TYPE), SSI_UINT, SSI_WII_SAMPLE_RATE);
			}
			~IrRawChannel () {
				ssi_stream_destroy (stream);
			}
			const ssi_char_t *getName () { return SSI_WII_IRRAW_PROVIDER_NAME; };
			const ssi_char_t *getInfo () { return "Raw X and Y coordinates measured by infrared sensor as unsigned int values [1024x768]"; };
			ssi_stream_t getStream () { return stream; };
		protected:
			ssi_stream_t stream;
	};

	class IrFltChannel : public IChannel {
		friend class WiiRemote;
		public:
			IrFltChannel () {
				ssi_stream_init (stream, 0, 2, sizeof(SSI_WII_IRFLT_SAMPLE_TYPE), SSI_FLOAT, SSI_WII_SAMPLE_RATE);
			}
			~IrFltChannel () {
				ssi_stream_destroy (stream);
			}
			const ssi_char_t *getName () { return SSI_WII_IRFLT_PROVIDER_NAME; };
			const ssi_char_t *getInfo () { return "X and Y coordinates measured by infrared sensor as float values in range [0.0 1.0], where X = left->right and Y = top->bottom."; };
			ssi_stream_t getStream () { return stream; };
		protected:
			ssi_stream_t stream;
	};

	class MPRawChannel : public IChannel {
		friend class WiiRemote;
		public:
			MPRawChannel () {
				ssi_stream_init (stream, 0, 3, sizeof(SSI_WII_MPRAW_SAMPLE_TYPE), SSI_SHORT, SSI_WII_SAMPLE_RATE);
			}
			~MPRawChannel () {
				ssi_stream_destroy (stream);
			}
		public:
			const ssi_char_t *getName () { return SSI_WII_MPRAW_PROVIDER_NAME; };
			const ssi_char_t *getInfo () { return "Raw yaw, roll and pitch as short values (requires MotionPlus extension)."; };
			ssi_stream_t getStream () { return stream; };
		protected:
			ssi_stream_t stream;
	};

	class MPFltChannel : public IChannel {
		friend class WiiRemote;
		public:
			MPFltChannel () {
				ssi_stream_init (stream, 0, 3, sizeof(SSI_WII_MPFLT_SAMPLE_TYPE), SSI_FLOAT, SSI_WII_SAMPLE_RATE);
			}
			~MPFltChannel () {
				ssi_stream_destroy (stream);
			}
			const ssi_char_t *getName () { return SSI_WII_MPFLT_PROVIDER_NAME; };
			const ssi_char_t *getInfo () { return "Raw yaw, roll and pitch as float values (requires MotionPlus extension)."; };
			ssi_stream_t getStream () { return stream; };
		protected:
			ssi_stream_t stream;
	};

public:

	enum BUTTON {
		LEFT	= 0x0001,
		RIGHT	= 0x0002,
		DOWN	= 0x0004,
		UP		= 0x0008,
		PLUS	= 0x0010,
		TWO		= 0x0100,
		ONE		= 0x0200,
		B		= 0x0400,
		A		= 0x0800,
		MINUS	= 0x1000,
		HOME	= 0x8000,
		ALL		= LEFT|RIGHT|DOWN|UP|PLUS|TWO|ONE|A|B|MINUS|HOME
	};

	class Options : public OptionList {

	public:

		Options ()
			: device (SSI_WII_FIRST_AVAILABLE), size (0.1) {

			addOption ("device", &device, 1, SSI_INT, "device id (first available = -1)");
			addOption ("size", &size, 1, SSI_DOUBLE, "block size in seconds");	
		};		

		int device;
		ssi_time_t size;
	};

public:

	static const ssi_char_t *GetCreateName () { return "WiiRemote"; };
	static IObject *Create (const ssi_char_t *file) { return new WiiRemote (file); };
	~WiiRemote ();
	WiiRemote::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Connects Nintendo's WiiRemote Device and reports acceleration, orientation and 2D position."; };

	ssi_size_t getChannelSize () { return 7; };
	IChannel *getChannel (ssi_size_t index) { return _channels[index]; };
	bool setProvider (const ssi_char_t *name, IProvider *provider);
	bool connect ();
	bool start () { return Thread::start (); }
	void run ();
	bool stop () { return Thread::stop (); }
	bool disconnect ();

	int	 getDeviceId() const { return _options.device; }
	
  wiimote *getDevice () { return &_wii; }

	void setLogLevel (int level) {
		ssi_log_level = level;
	}
  
 
  static void on_state_change (wiimote &_wiimote, state_change_flags changed, const wiimote_state &remote_state);
 

protected:
  
  int counterMotionPlus;
	WiiRemote (const ssi_char_t *file = 0);
	WiiRemote::Options _options;
	ssi_char_t *_file;

	IChannel *_channels[7];
	void setAccelerationProvider (IProvider *provider);
	void setOrientationProvider (IProvider *provider);
	void setButtonProvider (IProvider *provider);
	void setMotionPlusRawProvider (IProvider *provider);
	void setMotionPlusFloatProvider (IProvider *provider);
	void setInfraredProvider (IProvider *provider);
	void setInfraredRawProvider (IProvider *provider);

	static ssi_char_t *ssi_log_name;
	int ssi_log_level;

	ssi_size_t _buffer_size;
	ssi_size_t _buffer_counter;

	IProvider *_acc_provider;
	SSI_WII_ACC_SAMPLE_TYPE *_acc_buffer;
	SSI_WII_ACC_SAMPLE_TYPE *_acc_buffer_ptr;

	IProvider *_ori_provider;
	SSI_WII_ORI_SAMPLE_TYPE *_ori_buffer;
	SSI_WII_ORI_SAMPLE_TYPE *_ori_buffer_ptr;

	IProvider *_but_provider;
	SSI_WII_BUT_SAMPLE_TYPE *_but_buffer;
	SSI_WII_BUT_SAMPLE_TYPE *_but_buffer_ptr;

	bool _motion_plus_enabled;

	IProvider *_mpraw_provider;
	SSI_WII_MPRAW_SAMPLE_TYPE *_mpraw_buffer;
	SSI_WII_MPRAW_SAMPLE_TYPE *_mpraw_buffer_ptr;

	IProvider *_mpflt_provider;
	SSI_WII_MPFLT_SAMPLE_TYPE *_mpflt_buffer;
	SSI_WII_MPFLT_SAMPLE_TYPE *_mpflt_buffer_ptr;

	IProvider *_ir_provider;
	SSI_WII_IRFLT_SAMPLE_TYPE *_ir_buffer;
	SSI_WII_IRFLT_SAMPLE_TYPE *_ir_buffer_ptr;

	IProvider *_irraw_provider;
	SSI_WII_IRRAW_SAMPLE_TYPE *_irraw_buffer;
	SSI_WII_IRRAW_SAMPLE_TYPE *_irraw_buffer_ptr;

  wiimote _wii;
	Timer *_timer;	
};

}

#endif

