// GSREventListener.h
// author: Fabian Hertwig <fabian.hertwig@student.uni-augsburg.de>,
//         Florian Obermayer <florian.obermayer@student.uni-augsburg.de>
// created: 2015/01/16
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

#ifndef GSREVENTLISTENER_H
#define GSREVENTLISTENER_H

#include "GSRResponseDetection.h"
#include "base/ISensor.h"
#include "thread/ClockThread.h"
#include "ioput/option/OptionList.h"
#include "base/IChannel.h"
#include "thread/Lock.h"


namespace ssi{

#define SSI_GSR_EVENTLISTENER_N_CHANNELS 5

#define SSI_GSR_EVENTLISTENER_NUMBER_OF_RESPONSES_PROVIDER_NAME "number of responses"
#define SSI_GSR_EVENTLISTENER_AMPLITUDE_PROVIDER_NAME "amplitude"
#define SSI_GSR_EVENTLISTENER_RISING_TIME_PROVIDER_NAME "rising time"
#define SSI_GSR_EVENTLISTENER_POWER_PROVIDER_NAME "power"
#define SSI_GSR_EVENTLISTENER_ENERGY_PROVIDER_NAME "energy"

#define SSI_GSR_EVENTLISTENER_NUMBER_OF_RESPONSES_INDEX 0
#define SSI_GSR_EVENTLISTENER_AMPLITUDE_INDEX 1
#define SSI_GSR_EVENTLISTENER_RISING_TIME_INDEX 2
#define SSI_GSR_EVENTLISTENER_ENERGY_INDEX 3
#define SSI_GSR_EVENTLISTENER_POWER_INDEX 4
#define SSI_GSR_EVENTLISTENER_CLOCK_THREAD_TIME_INDEX 5

#define SSI_GSR_EVENTLISTENER_NO_WINDOW 0


	class GSREventListener : public ISensor, public ClockThread{

	public:
		struct gsr_event_t{
			unsigned long ct_time;
			ssi_real_t event_features[SSI_GSR_EVENTLISTENER_N_CHANNELS];
		};
		
		class Options : public OptionList {
		
		public:
			Options() : sr(1.0), window(0), statisticalFn(GSR_SUM){

				addOption("sr", &sr, 1, SSI_TIME, "sampling rates (hz)");
				addOption("window", &window, 1, SSI_INT, "Time span for hrv calculation in ms. 0 means no window (all prior events are used). This option competes with listener span.");
				addOption("statisticalFunction", &statisticalFn, 1, SSI_INT, "0-Sum, 1-Mean, 2-Variance, 3-StdDev");
			};

			ssi_time_t sr;
			ssi_size_t window;
			gsr_response_feature_statistical_function_t statisticalFn;
		};

#pragma region Channels

		class NumberOfResponsesChannel : public IChannel {
		
		public:
			NumberOfResponsesChannel(ssi_time_t sr) {
				ssi_stream_init(stream, 0, 1, sizeof(ssi_real_t), SSI_REAL, sr);
			}
			~NumberOfResponsesChannel() {
				ssi_stream_destroy(stream);
			}
			const ssi_char_t *getName() { return SSI_GSR_EVENTLISTENER_NUMBER_OF_RESPONSES_PROVIDER_NAME; };
			const ssi_char_t *getInfo() { return "Provides statistical evaluation (sum only) for number of responses of a gsr signal"; };
			ssi_stream_t getStream() { return stream; };

		protected:
			ssi_stream_t stream;
		};

		class AmplitudeChannel : public IChannel {
		
		public:
			AmplitudeChannel(ssi_time_t sr) {
				ssi_stream_init(stream, 0, 1, sizeof(ssi_real_t), SSI_REAL, sr);
			}
			~AmplitudeChannel() {
				ssi_stream_destroy(stream);
			}
			const ssi_char_t *getName() { return SSI_GSR_EVENTLISTENER_AMPLITUDE_PROVIDER_NAME; };
			const ssi_char_t *getInfo() { return "Provides statistical evaluation for response amplitudes of a gsr signal"; };
			ssi_stream_t getStream() { return stream; };

		protected:
			ssi_stream_t stream;
		};

		class RisingTimeChannel : public IChannel {
		
		public:
			RisingTimeChannel(ssi_time_t sr) {
				ssi_stream_init(stream, 0, 1, sizeof(ssi_real_t), SSI_REAL, sr);
			}
			~RisingTimeChannel() {
				ssi_stream_destroy(stream);
			}
			const ssi_char_t *getName() { return SSI_GSR_EVENTLISTENER_RISING_TIME_PROVIDER_NAME; };
			const ssi_char_t *getInfo() { return "Provides statistical evaluation for response rising times of a gsr signal"; };
			ssi_stream_t getStream() { return stream; };

		protected:
			ssi_stream_t stream;
		};

		class EnergyChannel : public IChannel {
		
		public:
			EnergyChannel(ssi_time_t sr) {
				ssi_stream_init(stream, 0, 1, sizeof(ssi_real_t), SSI_REAL, sr);
			}
			~EnergyChannel() {
				ssi_stream_destroy(stream);
			}
			const ssi_char_t *getName() { return SSI_GSR_EVENTLISTENER_ENERGY_PROVIDER_NAME; };
			const ssi_char_t *getInfo() { return "Provides statistical evaluation for response energy of a gsr signal"; };
			ssi_stream_t getStream() { return stream; };

		protected:
			ssi_stream_t stream;
		};

		class PowerChannel : public IChannel {
		
		public:
			PowerChannel(ssi_time_t sr) {
				ssi_stream_init(stream, 0, 1, sizeof(ssi_real_t), SSI_REAL, sr);
			}
			~PowerChannel() {
				ssi_stream_destroy(stream);
			}
			const ssi_char_t *getName() { return SSI_GSR_EVENTLISTENER_POWER_PROVIDER_NAME; };
			const ssi_char_t *getInfo() { return "Provides statistical evaluation for response power of a gsr signal"; };
			ssi_stream_t getStream() { return stream; };

		protected:
			ssi_stream_t stream;
		};

#pragma endregion		

	public:
		static const ssi_char_t *GetCreateName() { return "GSREventListener"; }
		const ssi_char_t * getName(){ return GetCreateName(); }
		static IObject *Create(const ssi_char_t *file) { return new GSREventListener(file); };
		const ssi_char_t *getInfo(){
			return "provides statistical evaluations over a given window on gsr response events";
		}

		~GSREventListener();

		ssi_size_t getChannelSize();
		IChannel *getChannel(ssi_size_t index);
		bool setProvider(const ssi_char_t *name, IProvider *provider);
		bool connect();
		bool disconnect();

        bool start();
        bool stop();

		Options *getOptions() { return &_options; };

		void listen_enter(){}
		bool update(IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms);
		void listen_flush(){}

		/// Statistical functions
		ssi_real_t BaseSum(ssi_real_t * arr, ssi_size_t arr_len);
		ssi_real_t BaseMean(ssi_real_t * arr, ssi_size_t arr_len);
		ssi_real_t BaseStdD(ssi_real_t * arr, ssi_size_t arr_len);
		ssi_real_t BaseVar(ssi_real_t * arr, ssi_size_t arr_len);

	protected:
		GSREventListener(const ssi_char_t *file = 0);
		GSREventListener::Options _options;
		ssi_char_t *_file;

		static ssi_char_t *ssi_log_name;
		int ssi_log_level;

		IProvider * _provider[SSI_GSR_EVENTLISTENER_N_CHANNELS];
		IChannel * _channels[SSI_GSR_EVENTLISTENER_N_CHANNELS];

		/// Implements the clock thread function. Updates the vector with the responses to the window size and writes a output stream.
		void clock();
		/// Mutex to lock the fifo vector
		static Mutex _mutex;

	private:
		/// Gives a result with the statistical function and the channel index of the responses held in the fifo
		ssi_real_t calculateResult(gsr_response_feature_statistical_function_t s_fn_type, ssi_size_t channel_idx);
		/// checks if a channel is active
		bool hasChannel(ssi_size_t index);

		/// A vector which holds all the responses
		std::vector<gsr_event_t> fifo;

		/// Remove responses from the fifo vector which are not inside the window.
		void adaptToWindow();

		unsigned long  _clock_thread_time;
	};
}
#endif
