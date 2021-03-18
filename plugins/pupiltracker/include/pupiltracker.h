// facecrop.h
// author: Florian Lingenfelser <lingenfelser@hcm-lab.de>
// created: 2018/12/06
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

#pragma once

#ifndef PUPILTRA_H
#define PUPILTRA_H

#include "base/IFilter.h"
#include "ioput/option/OptionList.h"
#include "event/EventAddress.h"

#include "boost/asio.hpp"

using boost::asio::ip::tcp;

namespace cv {
	class Mat;
	class CascadeClassifier;
};

namespace ssi {

	class PupilTracker : public IFilter {

	public:

		class Options : public OptionList {

		public:

			Options() {
				setAddress("");
				setSenderName("sender");
				setEventName("event");
			};

			void setAddress(const ssi_char_t *address) {
				if (address) {
					ssi_strcpy(this->address, address);
				}
			}
			void setSenderName(const ssi_char_t *sname) {
				if (sname) {
					ssi_strcpy(this->sname, sname);
				}
			}
			void setEventName(const ssi_char_t *ename) {
				if (ename) {
					ssi_strcpy(this->ename, ename);
				}
			}

			ssi_char_t address[SSI_MAX_CHAR];
			ssi_char_t sname[SSI_MAX_CHAR];
			ssi_char_t ename[SSI_MAX_CHAR];

		};

	public:

		static const ssi_char_t *GetCreateName() { return "PupilTracker"; };
		static IObject *Create(const ssi_char_t *file) { return new PupilTracker(file); };
		~PupilTracker();
		PupilTracker::Options *getOptions() { return &_options; };
		const ssi_char_t *getName() { return GetCreateName(); };
		const ssi_char_t *getInfo() { return "detects pupil from video stream"; };

		void transform_enter(ssi_stream_t &stream_in,
			ssi_stream_t &stream_out,
			ssi_size_t xtra_stream_in_num = 0,
			ssi_stream_t xtra_stream_in[] = 0);
		void transform(ITransformer::info info,
			ssi_stream_t &stream_in,
			ssi_stream_t &stream_out,
			ssi_size_t xtra_stream_in_num = 0,
			ssi_stream_t xtra_stream_in[] = 0);
		void transform_flush(ssi_stream_t &stream_in,
			ssi_stream_t &stream_out,
			ssi_size_t xtra_stream_in_num = 0,
			ssi_stream_t xtra_stream_in[] = 0);

		ssi_size_t getSampleDimensionOut(ssi_size_t sample_dimension_in) {
			if (sample_dimension_in > 1) {
				ssi_err("#dimension > 1 not supported");
			}
			return _sampleDimensionsOut;
		};
		ssi_size_t getSampleBytesOut(ssi_size_t sample_bytes_in) {
			if (sample_bytes_in != ssi_video_size(_format_in)) {
				ssi_err("#bytes not compatible");
			}
			return sizeof(SSI_FLOAT);
		};
		ssi_type_t getSampleTypeOut(ssi_type_t sample_type_in) {
			if (sample_type_in != SSI_IMAGE) {
				ssi_err("unsupported type");
			}
			return SSI_FLOAT;
		};

		bool setEventListener(IEventListener *listener);
		const ssi_char_t *getEventAddress() {
			return _event_address.getAddress();
		}

		//video stuff
		ssi_video_params_t getFormatIn() { return _format_in; };
		ssi_video_params_t getFormatOut() { return _format_out; };

		void setFormatIn(ssi_video_params_t format);
		void setFormatOut(ssi_video_params_t format);
		void setFormat(ssi_video_params_t format_in);

		const void *getMetaData(ssi_size_t &size) {
			size = sizeof (_format_out);
			return &_format_out;
		};
		void setMetaData(ssi_size_t size, const void *meta) {
			if (sizeof (_format_in) != size) {
				ssi_err("invalid meta size");
				return;
			}
			memcpy(&_format_in, meta, size);
			setFormat(_format_in);
		};

	protected:

		PupilTracker(const ssi_char_t *file = 0);
		PupilTracker::Options _options;
		ssi_char_t *_file;

		EventAddress _event_address;
		IEventListener *_listener;
		ssi_event_t _event;

		ssi_video_params_t _format_in, _format_out;
		ssi_size_t _stride_in, _stride_out;

	private:
		char* _sendFrameBuffer;
		size_t _ts;
		boost::asio::io_context _ioContext;
		tcp::socket _serverSocket;
		boost::asio::streambuf _response_buffer;

		ssi_size_t _frameWidthIn;
		ssi_size_t _frameHeightIn;
		double _bytesPerPixelIn;

		ssi_size_t _sampleDimensionsOut = 6;

	};

}

#endif