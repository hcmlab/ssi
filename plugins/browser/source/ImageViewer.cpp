// ImageViewer.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 29/11/2015
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
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#include "ImageViewer.h"
#include "ImageCanvas.h"
#include "graphic/Window.h"
#include "base/Factory.h"

namespace ssi {

char ImageViewer::ssi_log_name[] = "imageview_";

ImageViewer::ImageViewer(const ssi_char_t *file)
	: _window(0),
	_canvas(0),
	_img_canvas(0),
	_file(0) {

	if (file) {
		if (!OptionList::LoadXML(file, _options)) {
			OptionList::SaveXML(file, _options);
		}
		_file = ssi_strcpy(file);
	}
}

ImageViewer::~ImageViewer() {

	delete _img_canvas; _img_canvas = 0;
	delete _canvas; _canvas = 0;
	delete _window; _window = 0;

	if (_file) {
		OptionList::SaveXML(_file, _options);
		delete[] _file;
	}
}

void ImageViewer::listen_enter() {

	ssi_rect_t rect;
	rect.left = _options.mpos[0];
	rect.top = _options.mpos[1];
	rect.width = _options.mpos[2];
	rect.height = _options.mpos[3];

	_window = new Window();
	_window->setTitle(_options.mname);	
	_window->setPosition(rect);
	_img_canvas = new ImageCanvas();
	_img_canvas->setDefaultDirectory(_options.directory);
	_img_canvas->setDefaultExtension(_options.extension);
	_canvas = new Canvas();
	_canvas->addClient(_img_canvas);
	_window->setClient(_canvas);
	_window->create();

	if (_options.url[0] != '\0') {
		_img_canvas->navigate(_options.url);
	}

	_window->show();
	_window->setPosition(rect);
}

bool ImageViewer::update(IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms)
{
	if (n_new_events > 0) {

		ssi_event_t *e = events.next();
		if (e->type == SSI_ETYPE_STRING) 
		{
			ssi_msg(SSI_LOG_LEVEL_BASIC, "load image '%s'", e->ptr);
			_img_canvas->navigate(e->ptr);
			_canvas->update();			

			return true;
		} else if (e->type == SSI_ETYPE_NTUPLE) {

			ssi_event_tuple_t *ptr = ssi_pcast(ssi_event_tuple_t, e->ptr);
			ssi_size_t n = e->tot / sizeof (ssi_event_tuple_t);

			if (n > 0) {
				ssi_real_t maxval = ptr->value;
				ssi_size_t maxind = 0;
				for (ssi_size_t i = 1; i < n; i++) {
					if (ptr[i].value > maxval) {
						maxval = ptr[i].value;
						maxind = i;
					}
				}
				_img_canvas->navigate(Factory::GetString(ptr[maxind].id));
				_window->update();
			}
		}
	}

	return false;
}

void ImageViewer::listen_flush() {

	_window->close();
}

bool ImageViewer::notify(INotify::COMMAND::List command, const ssi_char_t *message) {

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

	}

	return false;
}

}
