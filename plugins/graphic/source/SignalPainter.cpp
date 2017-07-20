// SignalPainter.cpp
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

#include "SignalPainter.h"
#include "PaintSignal.h"
#include "PaintBars.h"
#include "graphic/Window.h"
#include "graphic/Canvas.h"
#include "base/Factory.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

const ssi_char_t *SignalPainter::ssi_log_name = "sigpainter_";
int SignalPainter::ssi_log_level = SSI_LOG_LEVEL_DEFAULT;

SignalPainter::SignalPainter (const ssi_char_t *file)
	: _n_windows(0),
	_window (0),
	_canvas (0),
	_client (0),
	_file (0) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
}

SignalPainter::~SignalPainter () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

void SignalPainter::consume_enter (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	_n_windows = stream_in_num;
	_window = new IWindow *[_n_windows];	
	_canvas = new ICanvas *[_n_windows];
	_client = new ICanvasClient *[_n_windows];

	ssi_char_t **titles = new ssi_char_t *[_n_windows];
	if (_n_windows == 1)
	{
		titles[0] = ssi_strcpy(_options.title);
	} 
	else
	{
		ssi_char_t n_titles = ssi_split_string_count(_options.title, ';');
		if (n_titles == _n_windows)
		{
			ssi_split_string(_n_windows, titles, _options.title, ';');
		}
		else
		{
			ssi_char_t string[SSI_MAX_CHAR];
			for (ssi_size_t i = 0; i < _n_windows; i++)
			{
				ssi_sprint(string, "%s#%02d", _options.title, i);
				titles[i] = ssi_strcpy(string);
			}
		}
	}

	int pos_top = _options.pos[1];
	int pos_height = _options.pos[3] / _n_windows;
	for (ssi_size_t i = 0; i < _n_windows; i++) {

		if (_options.type == PaintSignalType::BAR || _options.type == PaintSignalType::BAR_POS) {
			PaintBars *plot = new PaintBars(_options.type == PaintSignalType::BAR ? PaintBars::TYPE::BAR : PaintBars::TYPE::BAR_POS);
			if (!_options.autoscale){
				plot->setFixedLimit(ssi_cast(ssi_real_t, _options.fix[0]));
			} else {
				if (!_options.reset) {
					plot->setGlobalLimit(false);
				}
			}
			plot->setPrecision(_options.axisPrecision);
			_client[i] = plot;
		} else {
			PaintSignal *plot = new PaintSignal(stream_in[i].sr, stream_in[i].dim, _options.size, _options.type, _options.indx, _options.indy, _options.staticImage);
			plot->setIndices(_options.indx, _options.indy);
			plot->setPrecision(_options.axisPrecision);
			plot->setColormap(_options.colormap);
			if (!_options.autoscale){
				plot->setLimits(ssi_cast(ssi_real_t, _options.fix[0]), ssi_cast(ssi_real_t, _options.fix[1]));
			}
			_client[i] = plot;
		}		

		_canvas[i] = new Canvas();
		_canvas[i]->addClient(_client[i]);

		_window[i] = new Window();
		_window[i]->setClient(_canvas[i]);
			
		_window[i]->setTitle(titles[i]);				
		_window[i]->setPosition(ssi_rect(_options.pos[0], pos_top + i * pos_height, _options.pos[2], pos_height));

		_window[i]->create();
		_window[i]->show();	

		delete[] titles[i];
	}
	delete[] titles;
}

void SignalPainter::consume (IConsumer::info consume_info,
	ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	for (ssi_size_t i = 0; i < _n_windows; i++) {
		if (_options.type == PaintSignalType::BAR || _options.type == PaintSignalType::BAR_POS) {	
			if (_options.barNames[0] != '\0') {
				ssi_size_t n = ssi_split_string_count(_options.barNames, ',');
				ssi_char_t **tokens = new ssi_char_t *[n];
				ssi_split_string(n, tokens, _options.barNames, ',');
				ssi_pcast(PaintBars, _client[i])->setExternalAxisCaptions(n, tokens);
				for (ssi_size_t j = 0; j < n; j++) {
					delete[] tokens[j];
				}
				delete[] tokens;
			}
			if (_options.reset) {
				ssi_pcast(PaintBars, _client[i])->reset();
			}
			ssi_pcast(PaintBars, _client[i])->setData(stream_in[i]);
		} else {
			if (_options.reset) {
				ssi_pcast(PaintSignal, _client[i])->resetLimits();
			}
			ssi_pcast(PaintSignal, _client[i])->setData(stream_in[i], consume_info.time);
		}
		_canvas[i]->update();
	}
}

void SignalPainter::consume_flush (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	for (ssi_size_t i = 0; i < _n_windows; i++) {
		_window[i]->close();
		delete _window[i];
		delete _canvas[i];
		delete _client[i];		
	}
	delete[] _window; _window = 0;
	delete[] _canvas; _canvas = 0;
	delete[] _client; _client = 0;
	_n_windows = 0;
}

bool SignalPainter::notify(INotify::COMMAND::List command, const ssi_char_t *message) {

	switch (command) {
	case INotify::COMMAND::WINDOW_HIDE:
	{
		if (_window) {
			for (ssi_size_t i = 0; i < _n_windows; i++) {
				_window[i]->hide();
			}
			return true;
		}
		break;
	}
	case INotify::COMMAND::WINDOW_SHOW:
	{
		if (_window) {
			for (ssi_size_t i = 0; i < _n_windows; i++) {
				_window[i]->show();
			}
			return true;
		}
		break;
	}
	case INotify::COMMAND::WINDOW_MOVE:
	{
		if (_window) {
			ssi_real_t posf[4] = { 0, 0, 0, 0 };
			ssi_size_t n = ssi_string2array_count(message, ',');
			if (n == 4) {
				ssi_string2array(n, posf, message, ',');
			}
			else {
				ssi_wrn("could not parse position '%s'", message);
				return false;
			}
			ssi_rect_t pos;
			pos.left = (int)posf[0];
			pos.top = (int)posf[1];
			pos.width = (int)posf[2];
			pos.height = (int)(posf[3] / _n_windows);
			for (ssi_size_t i = 0; i < _n_windows; i++) {
				_window[i]->setPosition(pos);
				pos.top += pos.height;
			}
			return true;
		}
		break;
	}
	case INotify::COMMAND::MINMAX_SHOW:
	{
		if (_window) {
			for (ssi_size_t i = 0; i < _n_windows; i++) {
				_window[i]->setIcons(_window[i]->getIcons() | IWindow::ICONS::MINIMIZE | IWindow::ICONS::MAXIMIZE);
			}
			return true;
		}
		break;
	}
	case INotify::COMMAND::MINMAX_HIDE:
	{
		if (_window) {
			for (ssi_size_t i = 0; i < _n_windows; i++) {
				_window[i]->setIcons(_window[i]->getIcons() & ~IWindow::ICONS::MINIMIZE & ~IWindow::ICONS::MAXIMIZE);
			}
			return true;
		}
		break;
	}
	}

	return false;
}

}
