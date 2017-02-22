// VadWebRTC.h
// author: Andreas Seiderer <seiderer@hcm-lab.de>
// created: 7/3/2015
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

#ifndef SSI_VAD_WEBRTC_H
#define SSI_VAD_WEBRTC_H

#include "base/ITransformer.h"
#include "ioput/option/OptionList.h"
#include "event/EventAddress.h"
#include "base/Factory.h"


#include "webrtc/common_audio/vad/include/webrtc_vad.h"


namespace ssi {

	class VadWebRTC : public IFeature {

	public:

		class Options : public OptionList {

		public:

			Options()
				: aggr_mode(0), sendEvent(false) {
				addOption("aggressiveness", &aggr_mode, 1, SSI_INT,  "Range: [0;3]. A more aggressive (higher mode) VAD is more restrictive in reporting speech.");
				
				addOption("event", &sendEvent, 1, SSI_BOOL, "send an event on voice activity");
				addOption("sname", sname, SSI_MAX_CHAR, SSI_CHAR, "name of sender");
				addOption("ename", ename, SSI_MAX_CHAR, SSI_CHAR, "name of event");

			}

			void setSender(const ssi_char_t *sname) {
				if (sname) {
					ssi_strcpy(this->sname, sname);
				}
			}
			void setEvent(const ssi_char_t *ename) {
				if (ename) {
					ssi_strcpy(this->ename, ename);
				}
			}

			ssi_size_t aggr_mode;

			bool sendEvent;
			ssi_char_t sname[SSI_MAX_CHAR];
			ssi_char_t ename[SSI_MAX_CHAR];
		};

	public:

		static const ssi_char_t *GetCreateName() { return "VadWebRTC"; };
		static IObject *Create(const ssi_char_t *file) { return new VadWebRTC(file); };
		~VadWebRTC();

		Options *getOptions() { return &_options; };
		const ssi_char_t *getName() { return GetCreateName(); };
		const ssi_char_t *getInfo() { return "VAD of webRTC"; };

		ssi_size_t getSampleDimensionOut (ssi_size_t sample_dimension_in);
		ssi_size_t getSampleBytesOut (ssi_size_t sample_bytes_in);
		ssi_type_t getSampleTypeOut (ssi_type_t sample_type_in);

	void transform_enter (ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);

	void transform (ITransformer::info info,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);

	void transform_flush (ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);


	bool setEventListener(IEventListener *listener);
	const ssi_char_t *getEventAddress() {
		return _eaddress.getAddress();
	}

	protected:

		VadWebRTC(const ssi_char_t *file = 0);
		ssi_char_t *_file;
		Options _options;
		static char ssi_log_name[];


		VadInst* handle;

		void sendEvent(bool end, ssi_size_t time);
		IEventListener *_elistener;
		ssi_event_t _event;
		EventAddress _eaddress;

		bool vad_state_sent;
		ssi_size_t start_time;

		short *convert;
	};

}

#endif
