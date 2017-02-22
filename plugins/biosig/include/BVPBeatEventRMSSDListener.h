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
// This class listens to the BVP beat events and creates an output stream with the RMSSD value 
// calculated from the beats in a window.
// RMSSD is the square root of the mean squared differences of successive interbeat intervals.
//************************************************************************************************* 

#pragma once

#include "BVPBeatEventListenerBase.h"


#ifndef _BVP_EVENTRMSSDLISTENER_H
#define  _BVP_EVENTRMSSDLISTENER_H

namespace ssi{

#define SSI_BVP_RMSSDEVENTLISTENER_CHANNEL_PROVIDER_NAME "RMSSD"

#define SSI_BVP_RMSSDEVENTLISTENER_N_CHANNELS 1
#define SSI_BVP_RMSSDEVENTLISTENER_CHANNEL_INDEX 0

#define SSI_BVP_RMSSDEVENTLISTENER_NO_WINDOW 0



	class BVPBeatEventRMSSDListener : public BVPBeatEventListenerBase {

	public:

		class RMSSDChannel : public IChannel {
		public:
			RMSSDChannel(ssi_time_t sr) {
				ssi_stream_init(stream, 0, 1, sizeof(ssi_real_t), SSI_REAL, sr);
			}
			~RMSSDChannel() {
				ssi_stream_destroy(stream);
			}
			const ssi_char_t *getName() override{ return SSI_BVP_RMSSDEVENTLISTENER_CHANNEL_PROVIDER_NAME; };
			const ssi_char_t *getInfo() override{
				return "RMSSD is the square root of the mean squared differences of successive NN intervals.";
			}
			ssi_stream_t getStream() { return stream; };

		protected:
			ssi_stream_t stream;
		};

		class Options : public BVPBeatEventListenerBase::Options {

		public:
			Options() : window(0){
				addOption("window", &window, 1, SSI_INT, "Time span for RMSSD calculation in ms. 0 means no window (all prior events are used). This option competes with listener span.");
			};

			ssi_size_t window;
		};

	public:
		
		bool setProvider(const ssi_char_t *name, IProvider *provider);
		static const ssi_char_t *GetCreateName() { return "BVPEventRMSSDListener"; }
		const ssi_char_t * getName()override{ return GetCreateName(); }
		static IObject *Create(const ssi_char_t *file) { return new BVPBeatEventRMSSDListener(file); };
		const ssi_char_t *getInfo() override{
			return "Listens to the events of the BVPEventSender and calculates the RMSSD. RMSSD is the square root of the mean squared differences of successive interbeat intervals.";
		}
		ssi_size_t getChannelSize()override{ return SSI_BVP_RMSSDEVENTLISTENER_N_CHANNELS; };


		Options *getOptions() override{ return &_options; };

		bool update(IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms) override;

	protected:
		BVPBeatEventRMSSDListener(const ssi_char_t *file = 0);

		BVPBeatEventRMSSDListener::Options _options;

		IProvider * _providerRMSSD[SSI_BVP_RMSSDEVENTLISTENER_N_CHANNELS];
		IChannel * _channels[SSI_BVP_RMSSDEVENTLISTENER_N_CHANNELS];

		static ssi_char_t *ssi_log_name;
		int ssi_log_level;

		/// Implements the clock thread function.
		void clock() override;


	private:
		/// Calculates the RMSSD
		ssi_real_t calculateResult(ssi_size_t channel_idx) override;

		/// A vector which holds all the beats
		std::vector<bvp_event_t> fifo;

		/// Remove responses from the fifo vector which are not inside the window.
		void adaptToWindow();

		virtual void clockUpdate() override;

		virtual IChannel * getChannel(ssi_size_t index) override;

		virtual ssi_time_t getSampleRate() override;




	};
}


#endif //_BVP_EVENTRMSSDLISTENER_H
