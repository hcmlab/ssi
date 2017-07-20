// CVSave.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/05/27
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

#include "CVSave.h"
#include "ssiocv.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

//const ssi_char_t *CVSave::FORMAT_NAMES[] = { "bmp", "jpg", "png", "pbm", "ras", "tif", "exr", "jp2" };
const ssi_char_t *CVSave::FORMAT_NAMES[] = { "bmp", "tif", "ras", "pbm" };
ssi_char_t *CVSave::ssi_log_name = "ocvsave___";

CVSave::CVSave (const ssi_char_t *file) 
	: _file (0),
	_path (0),
	_counter (0) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
}

CVSave::~CVSave () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

void CVSave::setFormat (ssi_video_params_t format_in) {

	ICVConsumer::setFormatIn (format_in);
}

void CVSave::consume_enter (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	if (_options.path[0] == '\0') {
		_path = ssi_strcpy ("noname");
	} else {
		_path = ssi_strcpy (_options.path);
	}

	_zeros = ssi_cast (int, _options.zeros);
	_counter = _options.start;
	_format = _options.format;
	_flip = _options.flip;
}

void CVSave::consume (ssi_time_t frame_rate,
	const IplImage *_image_in) {

	if (_options.number) {
		ssi_sprint (_string, "%s%0*d.%s", _path, _zeros, _counter++, FORMAT_NAMES[_format]);
	} else {
		ssi_sprint (_string, "%s.%s", _path, FORMAT_NAMES[_format]);
	}
	if (_flip) {
		cvFlip (_image_in, NULL, 0);
	}
	cvSaveImage (_string, _image_in);
	ssi_msg (SSI_LOG_LEVEL_DETAIL, "saved image '%s'", _string);
}

void CVSave::consume_flush (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	delete[] _path; _path = 0;
	if (_options.remember) {
		_options.start = _counter;
	}
}

}
