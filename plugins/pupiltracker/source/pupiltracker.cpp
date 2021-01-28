// facecrop.cpp
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

#include "ioput/file/FileTools.h"
#include "base/ITheFramework.h"
#include "base/Factory.h"

#include <ssiocv.h>
#include "../include/pupiltracker.h"

namespace ssi {

	PupilTracker::PupilTracker(const ssi_char_t *file)
		: _stride_in(0),
		_stride_out(0),
		_listener(0),
		_face_detector(0),
		_file(0) {

		if (file) {
			if (!OptionList::LoadXML(file, &_options)) {
				OptionList::SaveXML(file, &_options);
			}
			_file = ssi_strcpy(file);
		}

		ssi_event_init(_event, SSI_ETYPE_MAP);
	}

	bool PupilTracker::setEventListener(IEventListener *listener) {

		_listener = listener;

		if (_options.address[0] != '\0') {

			_event_address.setAddress(_options.address);
			_event.sender_id = Factory::AddString(_event_address.getSender(0));
			_event.event_id = Factory::AddString(_event_address.getEvent(0));

		}
		else {

			ssi_wrn("use of deprecated option 'sname' and 'ename', use 'address' instead")

			_event.sender_id = Factory::AddString(_options.sname);
			if (_event.sender_id == SSI_FACTORY_STRINGS_INVALID_ID) {
				return false;
			}
			_event.event_id = Factory::AddString(_options.ename);
			if (_event.event_id == SSI_FACTORY_STRINGS_INVALID_ID) {
				return false;
			}

			_event_address.setSender(_options.sname);
			_event_address.setEvents(_options.ename);
		}

		return true;

	}

	PupilTracker::~PupilTracker() {

		if (_file) {
			OptionList::SaveXML(_file, &_options);
			delete[] _file;
		}

		ssi_event_destroy(_event);

	}

	void PupilTracker::setFormatIn(ssi_video_params_t format) {

		_format_in = format;
		_stride_in = ssi_video_stride(_format_in);

	}

	void PupilTracker::setFormatOut(ssi_video_params_t format) {

		_format_out = format;
		_stride_out = ssi_video_stride(_format_out);

	}

	void PupilTracker::setFormat(ssi_video_params_t format_in) {

		PupilTracker::setFormatIn(format_in);
		PupilTracker::setFormatOut(format_in);
	}

	void PupilTracker::transform_enter(ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {

		cv::String path = _options.dependencies;
		path += "haarcascade_frontalface_alt.xml";
		_face_detector = new cv::CascadeClassifier(path);
		
		ssi_event_adjust(_event, 4 * sizeof (ssi_event_map_t));
		ssi_event_map_t *tuple = ssi_pcast(ssi_event_map_t, _event.ptr);
		tuple[0].id = Factory::AddString("face_x");
		tuple[1].id = Factory::AddString("face_y");
		tuple[2].id = Factory::AddString("face_w");
		tuple[3].id = Factory::AddString("face_h");

		_last_x = 0;
		_last_y = 0;
		_last_w = 0;
		_last_h = 0;

		_firstrun = true;
	}

	void PupilTracker::transform(ITransformer::info info,
		
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {

		ssi_size_t frameWidth = _format_in.widthInPixels;
		ssi_size_t frameHeight = _format_in.heightInPixels;
		
		cv::Mat frame;
		cv::Mat grayframe;
		cv::Mat roi;

		frame = cv::Mat(frameHeight, frameWidth, CV_8UC3, stream_in.ptr, _stride_in);
		cvtColor(frame, grayframe, cv::COLOR_RGB2GRAY);
		std::vector<cv::Rect> faces;
		_face_detector->detectMultiScale(grayframe, faces, 1.2, 1, 0, cvSize(64, 48), cv::Size(frameWidth, frameHeight));

		if (faces.size() > 0)
		{
			if (_firstrun)
			{
				if (_options.color_code) rectangle(frame, faces[0], cv::Scalar(0, 255, 255), 5, cv::LINE_AA);
				roi = frame(faces[0]);
				cv::resize(roi, roi, cv::Size(frameWidth, frameHeight), faces[0].x, faces[0].y, 1);

				// event
				_event.time = info.time * 1000;
				_event.dur = (ssi_size_t)1.0f / stream_in.sr * 1000.0f;
				ssi_event_map_t *tuple = ssi_pcast(ssi_event_map_t, _event.ptr);
				tuple[0].value = (ssi_real_t)faces[0].x;
				tuple[1].value = (ssi_real_t)faces[0].y;
				tuple[2].value = (ssi_real_t)faces[0].width;
				tuple[3].value = (ssi_real_t)faces[0].height;
				if (_listener != 0)
				{
					_listener->update(_event);
				}

				_last_x = faces[0].x;
				_last_y = faces[0].y;
				_last_w = faces[0].width;
				_last_h = faces[0].height;

				_firstrun = false;
			}
			else
			{
				cv::Rect size(cv::Point((faces[0].x + _last_x)/2, (faces[0].y + _last_y) / 2), cv::Size((_last_w + faces[0].width)/2, (_last_h + faces[0].height)/2));

				int offset = _options.resize_offset;
				if (offset < 0) offset = 0;

				size.x -= offset;
				if (size.x < 0) size.x = 0;
				size.y -= offset;
				if (size.y < 0) size.y = 0;
				size.width += offset * 2;
				if (size.x + size.width > frameWidth) size.width -= (size.x + size.width - frameWidth);
				size.height += offset * 2;
				if (size.y + size.height > frameHeight) size.height -= (size.y + size.height - frameHeight);

				if (_options.color_code) rectangle(frame, size, cv::Scalar(0, 255, 0), 5, cv::LINE_AA);
				roi = frame(size);
				cv::resize(roi, roi, cv::Size(frameWidth, frameHeight), size.x, size.y, 1);

				// event
				_event.time = info.time * 1000;
				_event.dur = (ssi_size_t)1.0f / stream_in.sr * 1000.0f;
				ssi_event_map_t *tuple = ssi_pcast(ssi_event_map_t, _event.ptr);
				tuple[0].value = (ssi_real_t)faces[0].x;
				tuple[1].value = (ssi_real_t)faces[0].y;
				tuple[2].value = (ssi_real_t)faces[0].width;
				tuple[3].value = (ssi_real_t)faces[0].height;
				if (_listener != 0)
				{
					_listener->update(_event);
				}

				_last_x = size.x;
				_last_y = size.y;
				_last_w = size.width;
				_last_h = size.height;
			}
		}
		else
		{
			if (_firstrun)
			{
				if (_options.color_code) rectangle(frame, cv::Rect(cv::Point(0,0), cv::Size(frameWidth, frameHeight)), cv::Scalar(0, 255, 255), 5, cv::LINE_AA);
				roi = frame;
			}
			else
			{
				cv::Rect _last_face(cv::Point(_last_x, _last_y), cv::Size(_last_w, _last_h));
				if (_options.color_code) rectangle(frame, _last_face, cv::Scalar(0, 0, 255), 5, cv::LINE_AA);
				roi = frame(_last_face);
				cv::resize(roi, roi, cv::Size(frameWidth, frameHeight), _last_x, _last_y, 1);
			}
		}

		

		// Fin
		ssi_byte_t* outptr = stream_out.ptr;
		for (ssi_size_t rows = 0; rows < roi.rows; rows++) {
			memcpy(outptr, roi.ptr(rows), roi.cols * roi.elemSize());
			outptr += _stride_out;
		}
		
		// Release
		frame.release();
		grayframe.release();	
	}

	void PupilTracker::transform_flush(ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {

		delete _face_detector;
	}

}