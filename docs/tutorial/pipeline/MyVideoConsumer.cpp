// MyVideoConsumer.cpp
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

#include "MyVideoConsumer.h"
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

ssi_size_t MyVideoConsumer::_window_counter = 0;

MyVideoConsumer::MyVideoConsumer (const ssi_char_t *file) 
	: _stride_in (0),
	_image_in (0),
	_file (0) {

	ssi_sprint(_window_name, "window%u", _window_counter++);
}

MyVideoConsumer::~MyVideoConsumer () {
}

void MyVideoConsumer::consume_enter (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	_stride_in = ssi_video_stride(_format_in);
	_image_in = cvCreateImageHeader(cvSize(_format_in.widthInPixels, _format_in.heightInPixels), _format_in.depthInBitsPerChannel, _format_in.numOfChannels);

	cvNamedWindow(_window_name, cv::WINDOW_NORMAL);
}

void MyVideoConsumer::consume(IConsumer::info consume_info,
	ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	cvSetData (_image_in, stream_in[0].ptr, _stride_in);
	cvShowImage(_window_name, _image_in);
	cvWaitKey(1);
}

void MyVideoConsumer::consume_flush (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	cvReleaseImageHeader(&_image_in);
	cvDestroyWindow(_window_name);
}

bool MyVideoConsumer::getPosition(const ssi_char_t *message, ssi_rect_t &pos)
{
	ssi_real_t posf[4] = { 0, 0, 0, 0 };
	ssi_size_t n = ssi_string2array_count(message, ',');
	if (n == 4) {
		ssi_string2array(n, posf, message, ',');
	}
	else {
		ssi_wrn("could not parse position '%s'", message);
		return false;
	}

	pos.left = (int)(posf[0] + 0.5f);
	pos.top = (int)(posf[1] + 0.5f);
	pos.width = (int)(posf[2] + 0.5f);
	pos.height = (int)(posf[3] + 0.5f);

	return true;
}

bool MyVideoConsumer::notify(INotify::COMMAND::List command, const ssi_char_t *message) {

	switch (command) {
	case INotify::COMMAND::WINDOW_MOVE:
	{
		ssi_rect_t pos;
		if (getPosition(message, pos))
		{			
			int db = ::GetSystemMetrics(SM_CXSIZEFRAME);
			int dm = ::GetSystemMetrics(SM_CYMENU);
			cvMoveWindow(_window_name, pos.left, pos.top);
			cvResizeWindow(_window_name, pos.width - 2 * db, pos.height - dm - db);
		}
		break;
	}

	}

	return false;
}

}
