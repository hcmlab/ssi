// author: Fabian Hertwig <fabian.hertwig@student.uni-augsburg.de>,
//         Florian Obermayer <florian.obermayer@student.uni-augsburg.de>
// created: 2015/02/24
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
// This is a parent class for a BVP Beat Event listener. Classes which should listen to this type of 
// event should derive from this class.
// 
//************************************************************************************************* 




#ifndef _BVP_EVENTLISTENERBASE_H
#define  _BVP_EVENTLISTENERBASE_H

#include "base/ISensor.h"
#include "thread/ClockThread.h"
#include "ioput/option/OptionList.h"
#include "base/IChannel.h"
#include "base/ISensor.h"
#include "thread/Lock.h"

#include "BVPBeatDetection.h"

namespace ssi{

#define SSI_BVP_EVENTLISTENER_N_CHANNELS 3

#define SSI_BVP_EVENTLISTENER_CHANNEL_AMPLITUDE_PROVIDER_NAME "amplitude"
#define SSI_BVP_EVENTLISTENER_CHANNEL_AMPLITUDE_INDEX 0

#define SSI_BVP_EVENTLISTENER_CHANNEL_HEART_RATE_PROVIDER_NAME "heart rate"
#define SSI_BVP_EVENTLISTENER_CHANNEL_HEART_RATE_INDEX 1

#define SSI_BVP_EVENTLISTENER_CHANNEL_INTERBEAT_INTERVAL_PROVIDER_NAME "interbeat interval"
#define SSI_BVP_EVENTLISTENER_CHANNEL_INTERBEAT_INTERVAL_INDEX 2

	class BVPBeatEventListenerBase : public ISensor, public ClockThread{

	public:

		struct bvp_event_t{
			unsigned long ct_time;
			ssi_size_t event_start_time;
			ssi_size_t event_duration;
			ssi_real_t event_features[SSI_BVP_EVENTLISTENER_N_CHANNELS];
		};

		class Options : public OptionList {

		public:
			Options() : sr(1.0){

				addOption("sr", &sr, 1, SSI_TIME, "sampling rate of the output signal (Hz)");
			};

			ssi_time_t sr;
		};

#pragma region ChannelBases

		class AmplitudeChannelBase : public IChannel {

		public:
			AmplitudeChannelBase(ssi_time_t sr) {
				ssi_stream_init(stream, 0, 1, sizeof(ssi_real_t), SSI_REAL, sr);
			}
			~AmplitudeChannelBase() {
				ssi_stream_destroy(stream);
			}
			const ssi_char_t *getName() { return SSI_BVP_EVENTLISTENER_CHANNEL_AMPLITUDE_PROVIDER_NAME; };
			virtual const ssi_char_t *getInfo() = 0;
			ssi_stream_t getStream() { return stream; };

		protected:
			ssi_stream_t stream;
		};

		class HeartRateChannelBase : public IChannel {

		public:
			HeartRateChannelBase(ssi_time_t sr) {
				ssi_stream_init(stream, 0, 1, sizeof(ssi_real_t), SSI_REAL, sr);
			}
			~HeartRateChannelBase() {
				ssi_stream_destroy(stream);
			}
			const ssi_char_t *getName() { return SSI_BVP_EVENTLISTENER_CHANNEL_HEART_RATE_PROVIDER_NAME; };
			virtual const ssi_char_t *getInfo() = 0;
			ssi_stream_t getStream() { return stream; };

		protected:
			ssi_stream_t stream;
		};

		class InterbeatIntervalChannelBase : public IChannel {

		public:
			InterbeatIntervalChannelBase(ssi_time_t sr) {
				ssi_stream_init(stream, 0, 1, sizeof(ssi_real_t), SSI_REAL, sr);
			}
			~InterbeatIntervalChannelBase() {
				ssi_stream_destroy(stream);
			}
			const ssi_char_t *getName() { return SSI_BVP_EVENTLISTENER_CHANNEL_INTERBEAT_INTERVAL_PROVIDER_NAME; };
			virtual const ssi_char_t *getInfo() = 0;
			ssi_stream_t getStream() { return stream; };

		protected:
			ssi_stream_t stream;
		};


#pragma endregion		

	public:

		~BVPBeatEventListenerBase();

		ssi_size_t getChannelSize();
		bool setProvider(const ssi_char_t *name, IProvider *provider);
		bool connect();
		bool disconnect();

        bool start();
        bool stop();

		virtual IOptions *getOptions() = 0;

		void listen_enter(){}
		virtual bool update(IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms) = 0;
		void listen_flush(){}

		/// Statistical functions
		ssi_real_t BaseSum(ssi_real_t * arr, ssi_size_t arr_len);
		ssi_real_t BaseMean(ssi_real_t * arr, ssi_size_t arr_len);
		ssi_real_t BaseStdD(ssi_real_t * arr, ssi_size_t arr_len);
		ssi_real_t BaseVar(ssi_real_t * arr, ssi_size_t arr_len);

	protected:
		BVPBeatEventListenerBase::Options _options;

		BVPBeatEventListenerBase(const ssi_char_t *file = 0);
		ssi_char_t *_file;

		IProvider * _provider[SSI_BVP_EVENTLISTENER_N_CHANNELS];
		IChannel * _channels[SSI_BVP_EVENTLISTENER_N_CHANNELS];

		static Mutex _mutex;

		/// Implements the clock thread function.
		void clock();

		/// Is called after clock thread time is calculated and before calculateResults is invoked. Thread-safe.
		virtual void clockUpdate() = 0;

		unsigned long getClockThreadTime();

		/// checks if a channel is active
		bool hasChannel(ssi_size_t index);

		/// Gives a result with the channel index of the beats 
		virtual ssi_real_t calculateResult(ssi_size_t channel_idx) = 0;
		virtual ssi_time_t getSampleRate() = 0;
        uint64_t  _clock_thread_time;
	private:
		
	};
}

#endif // !_BVP_EVENTLISTENERBASE_H
