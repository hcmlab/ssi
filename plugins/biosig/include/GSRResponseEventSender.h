// GSRResponseEventSender.h
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

#ifndef _GSRRESPONSEEVENTSENDER_H
#define _GSRRESPONSEEVENTSENDER_H

#include "GSRResponseDetection.h"
#include "ioput/option/OptionList.h"
#include "base/IConsumer.h"
#include "base/IEvents.h"
#include "event/EventAddress.h"

namespace ssi {

	/// This class uses the callback of GSRResponse.h to send found responses as events.
	class GSRResponseEventSender : public IConsumer, GSRResponse::ICallback {

	public:

		class Options : public OptionList {
		public:

			Options()
				: print(false), tuple(false), minAmplitude(0.0), minRisingTime(0.0), minAllowedRegression(0.0){

				setSenderName("gsr");
				setEventName("response");

				addOption("sname", s_name, SSI_MAX_CHAR, SSI_CHAR, "name of sender");
				addOption("ename", e_name, SSI_MAX_CHAR, SSI_CHAR, "name of event");
				addOption("print", &print, 1, SSI_BOOL, "print response information to console");
				addOption("tuple", &tuple, 1, SSI_BOOL, "send tuple events instead of array");
				addOption("minAmplitude", &minAmplitude, 1, SSI_REAL, "minimum amplitude for a response to be detected");
				addOption("minRisingTime", &minRisingTime, 1, SSI_TIME, "minimum rising time for a response to be detected (seconds)");
				addOption("minAllowedRegression", &minAllowedRegression, 1, SSI_REAL, "minimum allowed regression after a peak to continue the response's peak search");
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

			ssi_char_t s_name[SSI_MAX_CHAR];
			ssi_char_t e_name[SSI_MAX_CHAR];
			bool print;
			bool tuple;
			ssi_real_t minAmplitude;
			ssi_time_t minRisingTime;
			ssi_real_t minAllowedRegression;
		};

		static const ssi_char_t *GetCreateName() { return "GSRResponseEventSender"; };
		static IObject *Create(const ssi_char_t *file) { return new GSRResponseEventSender(file); };
		~GSRResponseEventSender();

		Options *getOptions() { return &_options; };
		const ssi_char_t *getName() { return GetCreateName(); };
		const ssi_char_t *getInfo() { return "gsr response event sender"; };

		void consume_enter(ssi_size_t stream_in_num, ssi_stream_t stream_in[]);
		void consume(IConsumer::info consume_info, ssi_size_t stream_in_num, ssi_stream_t stream_in[]);
		void consume_flush(ssi_size_t stream_in_num, ssi_stream_t stream_in[]);

		bool setEventListener(IEventListener *listener);
		const ssi_char_t *getEventAddress() {
			return _event_address.getAddress();
		}

	protected:

		GSRResponseEventSender(const ssi_char_t *file = 0);
		ssi_char_t *_file;
		static char ssi_log_name[];
		Options _options;

		IEventListener *_elistener;
		ssi_event_t _event;
		EventAddress _event_address;

	private:
		/// The callback
		void handleResponse(gsr_response_t response);

		GSRResponse *_response;

		bool _send_as_tuple;
		ssi_size_t _etuple_dur_id;
		ssi_size_t _etuple_min_id;
		ssi_size_t _etuple_max_id;
		ssi_size_t _etuple_amplitude_id;
		ssi_size_t _etuple_energy_id;
		ssi_size_t _etuple_power_id;

	};
}

#endif
