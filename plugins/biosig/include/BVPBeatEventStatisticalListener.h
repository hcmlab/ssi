// GSRResponseEventSender.h
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
// This class listens to the BVP Beat events and calculates sum, mean, var, sdtdev of the beats in a window.
//************************************************************************************************* 

#pragma once

#include "BVPBeatEventListenerBase.h"


#ifndef _BVP_EVENTSTATISTICALLISTENER_H
#define  _BVP_EVENTSTATISTICALLISTENER_H

namespace ssi{

#define SSI_BVP_EVENTLISTENER_NO_WINDOW 0


	class BVPBeatEventStatisticalListener : public BVPBeatEventListenerBase {
#pragma region Channels
		class AmplitudeChannel : public BVPBeatEventListenerBase::AmplitudeChannelBase{
		public:
			AmplitudeChannel(ssi_time_t sr) : AmplitudeChannelBase(sr) { }
			virtual const char * getInfo() override{
				return "statistical evaluations of the amplitude of the beats in a bvp signal in the given window";
			}
		};
		class HeartRateChannel : public BVPBeatEventListenerBase::HeartRateChannelBase{
		public:
			HeartRateChannel(ssi_time_t sr) : HeartRateChannelBase(sr) { }
			virtual const char * getInfo() override{
				return "statistical evaluations of the heart rate of the bvp signal in the given window";
			}
		};
		class InterbeatIntervalChannel : public BVPBeatEventListenerBase::InterbeatIntervalChannelBase{
		public:
			InterbeatIntervalChannel(ssi_time_t sr) : InterbeatIntervalChannelBase(sr) { }
			virtual const char * getInfo() override{
				return "statistical evaluations of the interbeat interval of the bvp signal in the given window";
			}
		};
#pragma endregion
	public:

		class Options : public BVPBeatEventListenerBase::Options {

		public:
			Options() : window(0), statisticalFn(BVP_SUM){
				addOption("window", &window, 1, SSI_INT, "Time span for hrv calculation in ms. 0 means no window (all prior events are used). This option competes with listener span.");
				addOption("statisticalFunction", &statisticalFn, 1, SSI_INT, "0-Sum, 1-Mean, 2-Variance, 3-StdDev");
			};

			ssi_size_t window;
			bvp_statistical_function_t statisticalFn;
		};

	public:

		static const ssi_char_t *GetCreateName() { return "BVPEventStatisticalListener"; }
		const ssi_char_t * getName()override{ return GetCreateName(); }
		static IObject *Create(const ssi_char_t *file) { return new BVPBeatEventStatisticalListener(file); };
		const ssi_char_t *getInfo() override{
			return "Listens to the events of the BVPEventSender and provides statistical evaluations over a given window.";
		}

		Options *getOptions() override{ return &_options; };

		bool update(IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms) override;

	protected:
		BVPBeatEventStatisticalListener(const ssi_char_t *file = 0) : BVPBeatEventListenerBase(file){};
		BVPBeatEventStatisticalListener::Options _options;

		static ssi_char_t *ssi_log_name;
		int ssi_log_level;

	private:
		/// Gives a result with the statistical function and the channel index of the responses held in the fifo
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

#endif // !_BVP_EVENTSTATISTICALLISTENER_H
