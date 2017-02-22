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
// This class listens to the BVP beat events and creates an output stream with the raw data.
//************************************************************************************************* 

#pragma once

#include "BVPBeatEventListenerBase.h"

#ifndef _BVP_EVENTRAWLISTENER_H
#define  _BVP_EVENTRAWLISTENER_H

namespace ssi{

	class BVPBeatEventRawListener : public BVPBeatEventListenerBase {
#pragma region Channels
		class AmplitudeChannel : public BVPBeatEventListenerBase::AmplitudeChannelBase{
		public:
			AmplitudeChannel(ssi_time_t sr) : AmplitudeChannelBase(sr) { }
			virtual const char * getInfo() override{
				return "amplitude of the last beat of the bvp signal";
			}
		};
		class HeartRateChannel : public BVPBeatEventListenerBase::HeartRateChannelBase{
		public:
			HeartRateChannel(ssi_time_t sr) : HeartRateChannelBase(sr) { }
			virtual const char * getInfo() override{
				return "heart rate measured from the last two beats of the bvp signal";
			}
		};
		class InterbeatIntervalChannel : public BVPBeatEventListenerBase::InterbeatIntervalChannelBase{
		public:
			InterbeatIntervalChannel(ssi_time_t sr) : InterbeatIntervalChannelBase(sr) { }
			virtual const char * getInfo() override{
				return "interbeat interval measured from the last two beats of the bvp signal";
			}
		};
#pragma endregion
	public:

		static ssi_char_t *ssi_log_name;

		static const ssi_char_t *GetCreateName() { return "BVPEventRawListener"; }
		virtual const ssi_char_t * getName()override{ return GetCreateName(); }
		static IObject *Create(const ssi_char_t *file) { return new BVPBeatEventRawListener(file); };
		const ssi_char_t *getInfo() override{
			return "Listens to the events of the BVPEventSender and provides bvp beat event features";
		}

		Options * getOptions() override
		{
			return &_options;
		}

		virtual bool update(IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms) override;

		ssi_real_t calculateResult(ssi_size_t channel_idx) override;

		virtual void clockUpdate() override{}

		virtual IChannel * getChannel(ssi_size_t index) override;

		BVPBeatEventRawListener(const ssi_char_t *file = 0);

		virtual ssi_time_t getSampleRate() override;

	private:
		bvp_event_t _last_event;
		bvp_event_t _current_event;
	};

}

#endif // !_BVP_EVENTRAWLISTENER_H
