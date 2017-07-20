// VideoPainter.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/02/04
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

#include "VideoPainter.h"
#include "base/Factory.h"
#include "graphic/Window.h"
#include "graphic/Canvas.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

const ssi_char_t *VideoPainter::ssi_log_name = "vidpainter_";

VideoPainter::VideoPainter (const ssi_char_t *file)
	: _window(0),
	_canvas(0),
	_video(0),
	_n_faces(0),
	_faces(0),
	_file (0) {
	
	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
}

VideoPainter::~VideoPainter () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

void VideoPainter::consume_enter (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	if (_video_format.framesPerSecond != stream_in[0].sr) {		
		_video_format.framesPerSecond = stream_in[0].sr;
		ssi_wrn ("sample rate has been adjusted");
	}

	if (ssi_video_size (_video_format) != stream_in[0].byte) {
		ssi_err ("input stream doesn't match video format");
	}

	if (stream_in[0].dim != 1) {
		ssi_wrn ("supports only one dimension");
	}

	_video = new PaintVideo (_video_format, _options.flip, _options.scale, _options.mirror, _options.maxValue);
	_canvas = new Canvas();
	_canvas->addClient(_video);

	if (stream_in_num > 1) {
		_n_faces = 0;
		for (ssi_size_t i = 1; i < stream_in_num; i++) {
			ssi_size_t dim = stream_in[i].dim;
			if (dim % 8 != 0) {
				ssi_wrn("to draw a face you need 4 points (i.e. a dimension of 8 or a mulitple of 8)");
			} else {
				_n_faces += dim / 8;
			}
		}		
		_faces = new PaintPoints *[_n_faces];
		for (ssi_size_t i = 0; i < _n_faces; i++) {
			_faces[i] = new PaintPoints(_options.type, _options.relative, _options.swap, _options.labels, false);
			_faces[i]->setPrecision(_options.precision);
			_canvas->addClient(_faces[i]);
		}
	}

	_window = new Window();
	_window->setClient(_canvas);
	_window->setTitle(_options.name);
	ssi_rect_t rect = ssi_rect(_options.pos[0], _options.pos[1], _options.pos[2], _options.pos[3]);

	if (_options.accurate) {
#if _WIN32|_WIN64
		RECT wndwRC, clntRC;
		HWND hWnd = (HWND) _window->getHandle();
		::GetWindowRect((HWND)hWnd, &wndwRC);
		::GetClientRect((HWND)hWnd, &clntRC);
		long dx = (wndwRC.right - wndwRC.left) - (clntRC.right - clntRC.left);
		long dy = (wndwRC.bottom - wndwRC.top) - (clntRC.bottom - clntRC.top);
		rect.width = rect.width + (ssi_int_t) dx;
		rect.height = rect.height + (ssi_int_t)dy;
#endif
	} 

	_window->setPosition(rect);
	_window->create();
	_window->show();
}

void VideoPainter::consume (IConsumer::info consume_info,
	ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	_video->setData (stream_in[0].ptr, stream_in[0].num, consume_info.time);
	if (stream_in_num > 1) {
		ssi_size_t n_face = 0;
		for (ssi_size_t i = 1; i < stream_in_num; i++) {
			ssi_size_t dim = stream_in[i].dim;
			ssi_real_t *ptr = ssi_pcast(ssi_real_t, stream_in[i].ptr);
			for (ssi_size_t j = 0; j < dim/8; j++) {
				_faces[n_face++]->setData(4, ptr + j*8, consume_info.time);
			}
		}
		
	}
	_window->update();

}

void VideoPainter::consume_flush (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	_window->close();
	delete _window; _window = 0;
	delete _canvas; _canvas = 0;
	delete _video; _video = 0;
	for (ssi_size_t i = 0; i < _n_faces; i++) {
		delete _faces[i];
	}
	delete[] _faces; _faces = 0;
	_n_faces = 0;
}

bool VideoPainter::notify(INotify::COMMAND::List command, const ssi_char_t *message) {

	switch (command) {
	case INotify::COMMAND::WINDOW_HIDE:
	{
		if (_window) {
			_window->hide();
			return true;
		}
		break;
	}
	case INotify::COMMAND::WINDOW_SHOW:
	{
		if (_window) {
			_window->show();
			return true;
		}
		break;
	}
	case INotify::COMMAND::WINDOW_MOVE:
	{
		if (_window) {
			return _window->setPosition(message);
		}
		break;
	}
	case INotify::COMMAND::MINMAX_SHOW:
	{
		if (_window) {
			_window->setIcons(_window->getIcons() | IWindow::ICONS::MINIMIZE | IWindow::ICONS::MAXIMIZE);
			return true;
		}
		break;
	}
	case INotify::COMMAND::MINMAX_HIDE:
	{
		if (_window) {
			_window->setIcons(_window->getIcons() & ~IWindow::ICONS::MINIMIZE & ~IWindow::ICONS::MAXIMIZE);
			return true;
		}
		break;
	}

	}

	return false;
}


}
