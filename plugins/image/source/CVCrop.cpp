// CVCrop.cpp
// author: Johannes Wagner <johannes.wagner@informatik.uni-augsburg.de>
// created: 2008/07/29
// Copyright (C) 2007-11 University of Augsburg, Johannes Wagner
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute itand/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#include "CVCrop.h"
#include "ssiocv.h"

#ifdef USE_SSI_LEAK_DETECTOR
#include "SSI_LeakWatcher.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

#define OCVCROP_MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define OCVCROP_MAX(X,Y) ((X) >= (Y) ? (X) : (Y))

namespace ssi {

ssi_char_t *CVCrop::ssi_log_name = "ocvcrop___";	

CVCrop::CVCrop (const ssi_char_t *file) 
	: _tmpImage (0),
	_file (0) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}

	_region = new CvRect;
}

CVCrop::~CVCrop () {

	if (_tmpImage) {
		cvReleaseImage (&_tmpImage);
		_tmpImage = 0;
	}

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

void CVCrop::setFormat(ssi_video_params_t format) {

	ICVFilter::setFormatIn(format);

	if (_tmpImage) {
		cvReleaseImage(&_tmpImage);
		_tmpImage = 0;
	}

	if (_options.scaled) {
		_region->x = ssi_cast(int, _options.region[0] * format.widthInPixels);
		_region->y = ssi_cast(int, _options.region[1] * format.heightInPixels);
		_region->width = ssi_cast(int, _options.region[2] * format.widthInPixels);
		_region->height = ssi_cast(int, _options.region[3] * format.heightInPixels);
	}
	else {
		_region->x = ssi_cast(int, _options.region[0]);
		_region->y = ssi_cast(int, _options.region[1]);
		_region->width = ssi_cast(int, _options.region[2]);
		_region->height = ssi_cast(int, _options.region[3]);
	}

	if (_options.width == 0)
	{
		_options.width = _region->width;
	}
	if (_options.height == 0)
	{
		_options.height = _region->height;
	}

	format.widthInPixels = _options.width;
	format.heightInPixels = _options.height;

	ICVFilter::setFormatOut(format);
}

void CVCrop::transform (ssi_time_t frame_rate,
	IplImage *image_in, 
	IplImage *image_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	if (xtra_stream_in_num > 0)
	{
		if (_options.scaled) {
			_region->x = ssi_cast(int, _options.region[0] * image_in->width);
			_region->y = ssi_cast(int, _options.region[1] * image_in->height);
			_region->width = ssi_cast(int, _options.region[2] * image_in->width);
			_region->height = ssi_cast(int, _options.region[3] * image_in->height);
		}
		else 
		{
			_region->x = ssi_cast(int, _options.region[0]);
			_region->y = ssi_cast(int, _options.region[1]);
			_region->width = ssi_cast(int, _options.region[2]);
			_region->height = ssi_cast(int, _options.region[3]);
		}

		if (xtra_stream_in[0].type == SSI_FLOAT)
		{
			ssi_real_t *region = ssi_pcast(ssi_real_t, xtra_stream_in[0].ptr);
			if (xtra_stream_in[0].dim > 0) {
				_region->x = ssi_cast(int, _options.scaled ? *region++ * image_in->width : *region++);
			}

			if (xtra_stream_in[0].dim > 1) {
				_region->y = ssi_cast(int, _options.scaled ? *region++ * image_in->height : *region++);
			}

			if (xtra_stream_in[0].dim > 2) {
				_region->width = ssi_cast(int, _options.scaled ? *region++ * image_in->width : *region++);
			}

			if (xtra_stream_in[0].dim > 3) {
				_region->height = ssi_cast(int, _options.scaled ? *region++ * image_in->height : *region++);
			}
		}
		else if (xtra_stream_in[0].type == SSI_INT) 
		{
			int *region = ssi_pcast(int, xtra_stream_in[0].ptr);
			if (xtra_stream_in[0].dim > 0) {
				_region->x = *region++;
			}

			if (xtra_stream_in[0].dim > 1) {
				_region->y = *region++;
			}

			if (xtra_stream_in[0].dim > 2) {
				_region->width = *region++;
			}

			if (xtra_stream_in[0].dim > 3) {
				_region->height = *region++;
			}
		}
		else
		{
			ssi_err("type '%s' in xstream not supported, float or int required", SSI_TYPE_NAMES[xtra_stream_in[0].type]);
		}
	}

	cropAndResize (image_in, image_out);
}

void CVCrop::cropAndResize (IplImage *image_in, IplImage *image_out)
{
	if (_region->x < 0 || _region->y < 0 || _region->width == 0 || _region->height == 0) {
		if (!_options.keep) {
			cvSet (image_out, CV_RGB (0,0,0));
		}
		return;
	}

	if (_options.flip) {
		_region->y = image_in->height - (_region->height + _region->y);
	}

	switch (_options.origin) {
		case ORIGIN::LEFTTOP:
			break;
		case ORIGIN::CENTER:
			_region->x -= _region->width >> 1;
			_region->y -= _region->height >> 1;
			break;
	}

	_region->x = OCVCROP_MAX (_region->x, 0);
	_region->y = OCVCROP_MAX (_region->y, 0);	
	_region->x = OCVCROP_MIN (_region->x, image_in->width-1);
	_region->y = OCVCROP_MIN (_region->y, image_in->height-1);
	_region->width = OCVCROP_MIN (_region->width, image_in->width - _region->x);
	_region->height = OCVCROP_MIN (_region->height, image_in->height - _region->y);	

	if (!_tmpImage || _region->width != _tmpImage->width || _region->height != _tmpImage->height) {
		if (_tmpImage) {
			cvReleaseImage (&_tmpImage);
		}
		_tmpImage = cvCreateImage (cvSize (_region->width, _region->height), _format_in.depthInBitsPerChannel, _format_in.numOfChannels);
	}

	cvSetImageROI (image_in, *_region);
	cvCopy (image_in, _tmpImage, NULL);
	cvResize (_tmpImage, image_out, _options.method);
	cvResize (image_in, image_out, _options.method);
}

}
