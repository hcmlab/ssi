// PaintVideo.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/05/28
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
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

#include "PaintVideo.h"
#include "graphic/Painter.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif
		
namespace ssi {

PaintVideo::PaintVideo (ssi_video_params_t video_params,
	bool flipForOutput,
	bool scaleToOutput,
	bool mirrorForOutput,
	double maxValue)
	: _parent(0),
	_image_out (0),
	_image_out_tmp (0),
	size (0),
	_back_brush(0),
	_video_in_params (video_params),
	_video_in_stride (ssi_video_stride (video_params)),
	_video_in_size (ssi_video_size (video_params)),
	_flipForOutput(flipForOutput),
	_scaleToOutput(scaleToOutput),
	_mirrorForOutput(mirrorForOutput),
	_maxValue(maxValue),
	_draw_video(false)
{
	_painter = new Painter();
	setBackground(IPainter::ITool::COLORS::BLACK);	

	_video_out_params.widthInPixels = _video_in_params.widthInPixels;
	_video_out_params.heightInPixels = _video_in_params.heightInPixels;
	_video_out_params.depthInBitsPerChannel = SSI_VIDEO_DEPTH_8U;
	if(_video_in_params.numOfChannels == 1) {
		_video_out_params.numOfChannels = 3;
	} else {
		_video_out_params.numOfChannels = _video_in_params.numOfChannels;
	}
	_video_out_stride = ssi_video_stride (_video_out_params);
	_video_out_size = ssi_video_size (_video_out_params);
	
}

PaintVideo::~PaintVideo ()
{
	delete[] _image_out; _image_out = 0;
	delete[] _image_out_tmp; _image_out_tmp = 0;

	delete _back_brush; _back_brush = 0;
	delete _painter; _painter = 0;
}

void PaintVideo::create(ICanvas *canvas) {
	_parent = canvas;
}

void PaintVideo::setData (const void *image_in, ssi_size_t size_, ssi_time_t time)
{
	Lock lock (_mutex);

	if (size_ != 1) {
		ssi_wrn ("ignoring everything but first picture");
	}
	size = 1;

	const ssi_byte_t *p_image_in = reinterpret_cast<const ssi_byte_t *> (image_in);

	if (!_image_out) {
		_image_out = new ssi_byte_t[_video_out_size];
		_image_out_tmp = new ssi_byte_t[_video_out_size];
	}

	switch (_video_in_params.depthInBitsPerChannel)
	{
		case SSI_VIDEO_DEPTH_8U:
		{
			switch (_video_in_params.numOfChannels)
			{
				case 1:
					{
						ssi_byte_t *dstptr;
						const ssi_byte_t *srcptr;
						ssi_byte_t value;

						for (int i = 0; i < _video_in_params.heightInPixels; i++)
						{
							dstptr = _image_out + i * _video_out_stride;
							srcptr = p_image_in + i * _video_in_stride;
							for (int j = 0; j < _video_in_params.widthInPixels; j++)
							{
								value = *srcptr++;
								*dstptr++ = value;
								*dstptr++ = value;
								*dstptr++ = value;
							}
						}
						break;
					}
				case 3:
				case 4:
					{
						memcpy (_image_out, p_image_in, _video_in_size);
						break;
					}

				default:
					ssi_err ("#channels '%d' not supported", _video_in_params.numOfChannels);
			}

			break;
		}

		case SSI_VIDEO_DEPTH_32F:
		{
			switch (_video_in_params.numOfChannels)
			{
				case 1: {

					ssi_byte_t *dstptr;
					const float *srcptr;
					unsigned int value;
					for (int i = 0; i < _video_in_params.heightInPixels; i++)
					{
						dstptr = _image_out + i * _video_out_stride;
						srcptr = reinterpret_cast<const float *> (p_image_in + i * _video_in_stride);
						for (int j = 0; j < _video_in_params.widthInPixels; j++)
						{
							value = static_cast<ssi_byte_t> (*srcptr++ * 255);
							*dstptr++ = value;
							*dstptr++ = value;
							*dstptr++ = value;
						}
					}
					break;
				}
				default:
					ssi_err ("#channels '%d' not supported", _video_in_params.numOfChannels);
			}

			break;
		}

		case SSI_VIDEO_DEPTH_16U:
		{
			switch (_video_in_params.numOfChannels)
			{
				case 1: {

					ssi_byte_t *dstptr;
                    const int16_t *srcptr;
					ssi_byte_t value;

					for (int i = 0; i < _video_in_params.heightInPixels; i++)
					{
						dstptr = _image_out + i * _video_out_stride;
                        srcptr = reinterpret_cast<const int16_t *> (p_image_in + i * _video_in_stride) + (_mirrorForOutput ? (_video_in_params.widthInPixels - 1) : 0);
						for (int j = 0; j < _video_in_params.widthInPixels; j++)
						{
							// Convert 16 bit value to 8 bit value or use maxValue for this
							value = static_cast<unsigned char> (float(*srcptr) * (255.0f / ((_maxValue > 0) ? _maxValue : 65535.0f)));
							*dstptr++ = value;
							*dstptr++ = value;
							*dstptr++ = value;
							if (_mirrorForOutput)
								srcptr--;
							else
								srcptr++;
						}
					}
					break;

				default:
					ssi_err ("#channels '%d' not supported", _video_in_params.numOfChannels);
				}
			}

			break;
		}

		default:
			ssi_err ("pixel depth '%d' not supported", _video_in_params.depthInBitsPerChannel);
	}

	// mirror & flip

	if(_flipForOutput)
	{
		ssi_size_t stride = _video_out_stride;
		int _height = _video_out_params.heightInPixels;
		ssi_byte_t *dstptr = _image_out_tmp + (_height - 1) * stride;
		ssi_byte_t *srcptr = _image_out;
		for(int j = 0; j < _height; ++j)
		{
			memcpy(dstptr, srcptr, stride);
			dstptr -= stride;
			srcptr += stride;
		}
		ssi_byte_t *tmp = _image_out;
		_image_out = _image_out_tmp;
		_image_out_tmp = tmp;
	}

	if (_mirrorForOutput)
	{
		ssi_byte_t *dstptr = 0;
		const ssi_byte_t *srcptr = 0;
		int _height = _video_out_params.heightInPixels;
		int _width = _video_out_params.widthInPixels;
		int stride = _video_out_stride;
		for(int j = 0; j < _height; ++j)
		{
			dstptr = _image_out_tmp + j * stride;
			srcptr = _image_out + j * stride + (_width - 1) * 3;
			for (int i = 0; i < _width; ++i)
			{
				memcpy(dstptr, srcptr, 3);
				dstptr +=3;
				srcptr -=3;
			}
		}
		ssi_byte_t *tmp = _image_out;
		_image_out = _image_out_tmp;
		_image_out_tmp = tmp;
	}

	_draw_video = true;
}

void PaintVideo::clear() {
	_draw_video = false;
}

void PaintVideo::setBackground(ssi_rgb_t color) {

	Lock lock(_mutex);

	delete _back_brush;
	_back_brush = new Painter::Brush(IPainter::ITool::COLORS::BLACK);
}

void PaintVideo::paint(ssi_handle_t device, ssi_rect_t rect)
{
	if (_draw_video) {
		paintAsVideoImage(device, rect);
	} else {
		_painter->begin(device, rect);
		_painter->fill(*_back_brush, rect);
		_painter->end();
	}
}

void PaintVideo::paintAsVideoImage(ssi_handle_t device, ssi_rect_t rect)
{
	Lock lock (_mutex);

	if(!_image_out)
		return;

	_painter->begin(device, rect);
	_painter->image(_video_out_params, _image_out, _scaleToOutput);
	_painter->end();

}


}

