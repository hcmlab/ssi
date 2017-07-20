// ARTKPlusTracker.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/10/29
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

#include "ARTKPlusTracker.h"
#include "ARToolKitPlus/TrackerMultiMarkerImpl.h"
#include "ARToolKitPlus/TrackerMultiMarker.h"

namespace ssi {

ARTKPlusTracker::ARTKPlusTracker (const ssi_char_t *file) 
	: _tracker (0),
	_flip_buffer (0),
	_file (0) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
}

ARTKPlusTracker::~ARTKPlusTracker () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

void ARTKPlusTracker::transform_enter(
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	_tracker = new ARToolKitPlus::TrackerMultiMarkerImpl<6,6,6,1,MAX_MARKER> (_video_format.widthInPixels, _video_format.heightInPixels);

	if (!ssi_exists (_options.camcfg)) {
		ssi_err ("camera config file '%s' not found", _options.camcfg);
	}

	if (!ssi_exists (_options.markcfg)) {
		ssi_err ("marker config file '%s' not found", _options.markcfg);
	}

	if (!_tracker->init (_options.camcfg, _options.markcfg, 1.0f, 1000.0f)) {
		ssi_err ("tracker initalization failed");		
	}

	_tracker->setPixelFormat(ARToolKitPlus::PIXEL_FORMAT_BGR);

	// the marker in the BCH test image has a thiner border...
    _tracker->setBorderWidth(0.125f);

    // set a threshold. we could also activate automatic thresholding
	_tracker->activateAutoThreshold(true);
    //_tracker->setThreshold(160);

    // let's use lookup-table undistortion for high-speed
    // note: LUT only works with images up to 1024x1024
    _tracker->setUndistortionMode(ARToolKitPlus::UNDIST_LUT);

    // RPP is more robust than ARToolKit's standard pose estimator
    //_tracker->setPoseEstimator(ARToolKitPlus::POSE_ESTIMATOR_RPP);
    // switch to simple ID based markers
    // use the tool in tools/IdPatGen to generate markers
    _tracker->setMarkerMode(ARToolKitPlus::MARKER_ID_SIMPLE);

	if (!_video_format.flipImage && !_options.flip || _video_format.flipImage && _options.flip) {
		_flip_buffer = new BYTE[ssi_video_size (_video_format)];
	}
}

void ARTKPlusTracker::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	// get input
	BYTE *image = 0;
	if (!_video_format.flipImage && !_options.flip || _video_format.flipImage && _options.flip) {
		ARTKPlusTools::FlipImage (_flip_buffer, ssi_pcast (BYTE, stream_in.ptr), _video_format);
		image = _flip_buffer;
	} else {
		image = ssi_pcast (BYTE, stream_in.ptr);
	}

	// get output
	ARTKPlusTools::marker_s *ss = ssi_pcast (ARTKPlusTools::marker_s, stream_out.ptr);
	for (ssi_size_t i = 0; i < _options.n_marker; i++) {
		ARTKPlusTools::ClearStruct (ss[i]);
	}

    // here we go, just one call to find the camera pose
	int numDetected = _tracker->calc (image);
	ARToolKitPlus::ARMarkerInfo markerInfo;

	for (int i = 0; i < min ((int) _options.n_marker, numDetected); i++){

		markerInfo = _tracker->getDetectedMarker(i);
		ssi_byte_t *outptr = stream_out.ptr;

		if (markerInfo.id > 0 && markerInfo.id <= ssi_cast (int, _options.n_marker)) {

			ss[i].id = markerInfo.id;
			ss[i].visible = true;
			ss[i].center.x = markerInfo.pos[0];
			ss[i].center.y = markerInfo.pos[1];
			ss[i].scaled = _options.scale;
			memcpy (ss[i].vertex, markerInfo.vertex, 8*sizeof (float));

			if (_options.flip) {
				ss[i].center.y = _video_format.heightInPixels - ss[i].center.y;
				for (int j = 0; j < 4; j++) {
					ss[i].vertex[j].y = _video_format.heightInPixels - ss[i].vertex[j].y;
				}
			}

			if (_options.scale) {
				ss[i].center.x /= _video_format.widthInPixels;
				ss[i].center.y /= _video_format.heightInPixels;
				for (int j = 0; j < 4; j++) {
					ss[i].vertex[j].x /= _video_format.widthInPixels;
					ss[i].vertex[j].y /= _video_format.heightInPixels;
				}
			}
		}
	}
}

void ARTKPlusTracker::transform_flush(
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	_tracker->cleanup ();
	delete _tracker; _tracker = 0;
	delete[] _flip_buffer; _flip_buffer = 0;
}

}
