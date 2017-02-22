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
// This class reads the BVP stream and uses the BVPBeatDetectionClass to find beats. It sends 
// the found beats as events.
//************************************************************************************************* 

#pragma once


#ifndef _BVP_EVENTSENDER_H
#define  _BVP_EVENTSENDER_H


#include "BVPBeatDetection.h"
#include "base/IConsumer.h"
#include "ioput/option/OptionList.h"
#include "event/EventAddress.h"
#include "base/IEvents.h"
#include "base/Factory.h"

namespace ssi{
	class BVPBeatEventSender : public IConsumer, BVPBeatDetection::ICallback
	{
		class Options : public OptionList {
		public:

			Options()
				: print(false), tuple(false), mean_window(3), beep(false) {

				setSenderName("bvp");
				setEventName("beats");

				addOption("sname", s_name, SSI_MAX_CHAR, SSI_CHAR, "name of sender");
				addOption("ename", e_name, SSI_MAX_CHAR, SSI_CHAR, "name of event");
				addOption("mean_window", &mean_window, 1, SSI_INT, "mean window (in seconds) to get a baseline for beat detection");
				addOption("print", &print, 1, SSI_BOOL, "print response information to console");
				addOption("tuple", &tuple, 1, SSI_BOOL, "send tuple events instead of array");
				addOption("beep", &beep, 1, SSI_BOOL, "beep on heart beat received");


				};

			void setSenderName(const ssi_char_t *sname) {
				if (sname) {
					ssi_strcpy(this->s_name, sname);
				}
			}
			void setEventName(const ssi_char_t *epeakname) {
				if (epeakname) {
					ssi_strcpy(this->e_name, epeakname);
				}
			}
			
			ssi_size_t mean_window;
			ssi_char_t s_name[SSI_MAX_CHAR];
			ssi_char_t e_name[SSI_MAX_CHAR];
			bool print;
			bool tuple;
			bool beep;
		};

	public:

		static const ssi_char_t *GetCreateName() { return "BVPEventSender"; };
		static IObject *Create(const ssi_char_t *file) { return new BVPBeatEventSender(file); };
		~BVPBeatEventSender();

		virtual Options *getOptions() override { return &_options; }
		virtual const ssi_char_t * getName() override { return GetCreateName(); }
		virtual const ssi_char_t * getInfo() override{
			return "sends bvp beat events";
		}


		const ssi_char_t *getEventAddress() override
		{
			return _event_address.getAddress();
		}

	protected:

		BVPBeatEventSender(const ssi_char_t *file = 0);
		ssi_char_t *_file;
		static char ssi_log_name[];
		Options _options;

		IEventListener *_elistener;
		ssi_event_t _event;
		EventAddress _event_address;

	private:

		BVPBeatDetection * _bvp_beat_detection;

		virtual void handleBeat(bvp_beat_t beat) override;

		virtual void consume_enter(ssi_size_t stream_in_num, ssi_stream_t stream_in[]) override;

		virtual void consume(IConsumer::info consume_info, ssi_size_t stream_in_num, ssi_stream_t stream_in[]) override;

		virtual void consume_flush(ssi_size_t stream_in_num, ssi_stream_t stream_in[]) override;

		virtual bool setEventListener(IEventListener *listener) override;

		bool _send_as_tuple;
		ssi_size_t _etuple_minimum_id;
		ssi_size_t _etuple_maximum_id;
		ssi_size_t _etuple_amplitude_id;
		ssi_size_t _etuple_energy_id;
		ssi_size_t _etuple_power_id;

	};
}

#endif // !_BVP_EVENTSENDER_H
