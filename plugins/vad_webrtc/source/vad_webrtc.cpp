// VadWebRTC.cpp
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

#include "vad_webrtc.h"

namespace ssi {

	char VadWebRTC::ssi_log_name[] = "vad_webrtc";

	VadWebRTC::VadWebRTC(const ssi_char_t *file)
		: _file(0), vad_state_sent(false), _elistener(0), start_time(0), convert(0) {
			getOptions()->setSender("webrtc_vad");
			getOptions()->setEvent("voice activity");

		if (file) {
			if (!OptionList::LoadXML(file, &_options)) {
				OptionList::SaveXML(file, &_options);
			}
			_file = ssi_strcpy(file);
		}

		ssi_event_init(_event, SSI_ETYPE_EMPTY);
	}

	VadWebRTC::~VadWebRTC() {

		if (_file) {
			OptionList::SaveXML(_file, &_options);
			delete[] _file;
		}

		ssi_event_destroy(_event);
		
		if (convert) delete[] convert; 
		convert = 0;
	}


	ssi_size_t VadWebRTC::getSampleDimensionOut (ssi_size_t sample_dimension_in) {
	
		SSI_ASSERT (sample_dimension_in == 1);

		return 1;
	}

	ssi_size_t VadWebRTC::getSampleBytesOut (ssi_size_t sample_bytes_in) {
		return sizeof (short);
	}

	ssi_type_t VadWebRTC::getSampleTypeOut (ssi_type_t sample_type_in) {
		return SSI_SHORT;
	}


	void VadWebRTC::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

		if (stream_in.dim != 1)
			ssi_err("Can just handle one dimension!");

		handle = WebRtcVad_Create();

		if (handle == 0 )
			ssi_err("Could not create webRTC vad!");

		if ( WebRtcVad_Init(handle) != 0 )
			ssi_err("Could not init webRTC vad!");

		if (getOptions()->aggr_mode > 3 || getOptions()->aggr_mode < 0) {
			getOptions()->aggr_mode = 0;			
			ssi_wrn("VAD aggressiveness mode out of range [0;3]! Using default value (0).");
		}

		WebRtcVad_set_mode(handle, getOptions()->aggr_mode);

		//allowed frame sizes: 10, 20, 30 ms @ 48 kHz = 480, 960, 1440 samples
		//if (WebRtcVad_ValidRateAndFrameLength(stream_in.sr,stream_in.num_real) != 0)
			//ssi_err("Invalid sample rate / frame length combination! Allowed frame sizes: 10, 20 or 30 ms");

	}

	void VadWebRTC::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

		ssi_size_t time = ssi_cast(ssi_size_t, info.time * 1000.0 + 0.5);

		if (WebRtcVad_ValidRateAndFrameLength(stream_in.sr,stream_in.num) != 0)
			ssi_err("Invalid sample rate / frame length combination! Allowed frame sizes: 10, 20 or 30 ms. Detected sr: %f, num_real: %i", stream_in.sr, stream_in.num);
	

		short result = -1;
		short *dstptr = ssi_pcast (short, stream_out.ptr);

		//convert to int16
		if (stream_in.type == SSI_FLOAT ) {
			float *srcptr = ssi_pcast (float, stream_in.ptr);

			if (!convert) 
				convert = new short[stream_in.num];

			for (int i = 0; i < stream_in.num; i++)
				convert[i] = *srcptr++ * 32768;

			result = WebRtcVad_Process(handle, stream_in.sr, convert, stream_in.num);
		}
		else
		//already have int16
		if (stream_in.type == SSI_SHORT ) {

			short *srcptr = ssi_pcast (short, stream_in.ptr);		
			result = WebRtcVad_Process(handle, stream_in.sr, srcptr, stream_in.num);
		}

		*dstptr = result;

		if (result == -1) {
			ssi_err("webRTC vad error on process!");
		}
		else if (result == 0) {


			if (vad_state_sent) {
				//ssi_print("VAD: No voice\n");
				sendEvent(true, time);
				vad_state_sent = false;
			}
		}
		else	{

			if (!vad_state_sent) {
				//ssi_print("VAD: Voice\n");
				sendEvent(false, time);
				vad_state_sent = true;
			}
		}

		
	}


	void VadWebRTC::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

		WebRtcVad_Free(handle);
		handle = 0;
	}


	bool VadWebRTC::setEventListener(IEventListener *listener) {

		_elistener = listener;
		_event.sender_id = Factory::AddString(_options.sname);
		if (_event.sender_id == SSI_FACTORY_STRINGS_INVALID_ID) {
			return false;
		}
		_event.event_id = Factory::AddString(_options.ename);
		if (_event.event_id == SSI_FACTORY_STRINGS_INVALID_ID) {
			return false;
		}

		_eaddress.setSender(_options.sname);
		_eaddress.setEvents(_options.ename);

		return true;
	}


	void VadWebRTC::sendEvent(bool end, ssi_size_t time) {

	if (_elistener && _options.sendEvent) {

		if (end) {
			_event.time = start_time;
			_event.dur = time - start_time;
			_event.state = SSI_ESTATE_COMPLETED;
			_elistener->update(_event);
		}
		else {
			start_time = time;

			_event.time = start_time;
			_event.dur = 0;
			_event.state = SSI_ESTATE_CONTINUED;
			_elistener->update(_event);
		}
	}
}

}
