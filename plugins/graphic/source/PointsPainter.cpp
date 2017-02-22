// PointsPainter.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2015/07/06
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

#include "PointsPainter.h"
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

const ssi_char_t *PointsPainter::ssi_log_name = "poipainter_";
int PointsPainter::ssi_log_level = SSI_LOG_LEVEL_DEFAULT;

PointsPainter::PointsPainter (const ssi_char_t *file)
	: _window(0),
	_canvas(0),
	_client(0),
	_file (0) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
}

PointsPainter::~PointsPainter () {

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}

void PointsPainter::consume_enter (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	_client = new PaintPoints(_options.type, _options.relative, _options.swap, _options.labels, true);
	_client->setPrecision(_options.precision);
		
	_canvas = new Canvas();
	_canvas->addClient(_client);

	_window = new Window();
	_window->setClient(_canvas);
	_window->setTitle(_options.name);
	_window->setPosition(ssi_rect(_options.pos[0], _options.pos[1], _options.pos[2], _options.pos[3]));
	_window->create();
	_window->show();
}

void PointsPainter::consume (IConsumer::info consume_info,
	ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {
	
	_client->setData(stream_in[0], consume_info.time);
 	_window->update();
}

void PointsPainter::consume_flush (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	_window->close();
	delete _window; _window = 0;
	delete _canvas; _canvas = 0;
	delete _client; _client = 0;
}

bool PointsPainter::notify(INotify::COMMAND::List command, const ssi_char_t *message) {

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
