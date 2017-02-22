// Decorator.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2016/01/29
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

#include "Decorator.h"
#include "ioput/xml/tinyxml.h"
#include "base/Factory.h"

namespace ssi {

const ssi_char_t *Decorator::ssi_log_name = "decorator_";
int Decorator::ssi_log_level = SSI_LOG_LEVEL_DEFAULT;

Decorator::Decorator(const ssi_char_t *file)
: _scale_x(1.0f),
_scale_y(1.0f),
_show(true),
_minmax_show(true),
_window(0),
_file(0) {

	_origin = ssi_pointf(0, 0);
	_screen = ssi_getscreen();

	if (file) {
		if (!OptionList::LoadXML(file, _options)) {
			OptionList::SaveXML(file, _options);
		}
		_file = ssi_strcpy(file);
	}
}

Decorator::~Decorator() {

	clear();

	if (_window) {
		_window->close();
		delete _window; _window = 0;
	}

	if (_file) {
		OptionList::SaveXML(_file, _options);
		delete[] _file;
	}
}

void Decorator::readOptions() {

	_options.lock();

	_show = _options.show;
	_minmax_show = _options.minmax;
	_scale_x = _options.scale[0];
	_scale_y = _options.scale[1];
	_origin.x = _options.origin[0];
	_origin.y = _options.origin[1];

	_options.unlock();
}

void Decorator::clear() {
	_items.clear();
}

void Decorator::add(const ssi_char_t *ids, int nh, int nv, int x, int y, int width, int height) {

	ssi_rectf_t pos = ssi_rectf((ssi_real_t)x, (ssi_real_t)y, (ssi_real_t)width, (ssi_real_t)height);
	add(ids, pos, (ssi_size_t)nv, (ssi_size_t)nh);
}

void Decorator::add(const ssi_char_t *ids, int x, int y, int width, int height) {

	ssi_rectf_t pos = ssi_rectf((ssi_real_t)x, (ssi_real_t)y, (ssi_real_t)width, (ssi_real_t)height);
	add(ids, pos);
}

bool Decorator::add(const ssi_char_t *ids, const ssi_char_t *pos_s, const ssi_char_t *nh_s, const ssi_char_t *nv_s) {

	ssi_rectf_t pos;
	
	if (!pos_s || ssi_strcmp(pos_s, SSI_DECORATOR_SCREEN_NAME, false)) {
		pos.left = 0;
		pos.top = 0;
		pos.width = _screen.x;
		pos.height = _screen.y;
	} else {
		ssi_real_t posf[4] = { 0, 0, 0, 0 };
		ssi_size_t n = ssi_string2array_count(pos_s, ',');
		if (n == 4) {
			ssi_string2array(n, posf, pos_s, ',');
		} else {
			ssi_wrn("could not parse position '%s'", pos_s);
			return false;
		}
		pos.left = posf[0];
		pos.top = posf[1];
		pos.width = posf[2];
		pos.height = posf[3];
	}

	ssi_size_t nh = 0;	
	if (nh_s && sscanf(nh_s, "%u", &nh) != 1) {
		ssi_wrn("could not parse nh '%s'", nh_s);
		return false;
	}

	ssi_size_t nv = 0;
	if (nv_s && sscanf(nv_s, "%u", &nv) != 1) {
		ssi_wrn("could not parse nv '%s'", nv_s);
		return false;
	}

	add(ids, pos, nv, nh);

	return true;
}

void Decorator::add(const ssi_char_t *ids, ssi_rect_t pos, ssi_size_t nv, ssi_size_t nh) {
	add(ids, ssi_rectf((ssi_real_t)pos.left, (ssi_real_t)pos.top, (ssi_real_t)pos.width, (ssi_real_t)pos.height));
}

void Decorator::add(const ssi_char_t *ids, ssi_rectf_t pos, ssi_size_t nv, ssi_size_t nh) {

	ssi_size_t n_tokens = ssi_split_string_count(ids, ',');
	if (n_tokens == 0) {
		ssi_wrn("no objects selected")
		return;
	}

	ssi_char_t **tokens = new ssi_char_t *[n_tokens];
	ssi_split_string(n_tokens, tokens, ids, ',');

	std::vector<ssi_char_t *> names;
	ssi_size_t n_names = 0;
	for (ssi_size_t n = 0; n < n_tokens; n++) {

		ssi_size_t n_names_tmp;
		ssi_char_t **names_tmp = 0;
		n_names_tmp = Factory::GetObjectIds(&names_tmp, tokens[n]);

		for (ssi_size_t i = 0; i < n_names_tmp; i++) {
			names.push_back(names_tmp[n_names_tmp - i - 1]);
		}

		delete[] names_tmp;
	}
	
	for (ssi_size_t n = 0; n < n_tokens; n++) {
		delete[] tokens[n];
	}
	delete[] tokens;

	n_names = (ssi_size_t) names.size();
	if (n_names == 0) {
		return;
	}

	if (nv == 0 && nh == 0) {		
        nh = ssi_max2((ssi_size_t)1, (ssi_size_t)(sqrt((ssi_real_t)n_names) + 0.5f));
	}
	if (nh == 0) {
		nh = ssi_max2((ssi_size_t)1, n_names / nv);
		if (nh * nv < n_names) {
			nh++;
		}
	}
	else if (nv == 0) {
		nv = ssi_max2((ssi_size_t)1, n_names / nh);
		if (nh * nv < n_names) {
			nv++;
		}
	}

	IObject *object;

	ssi_real_t x = _origin.x + pos.left;
	ssi_real_t y = _origin.y + pos.top;
	ssi_real_t w = pos.width / nh;
	ssi_real_t h = pos.height / nv;

	ssi_size_t count = 0;

	for (ssi_size_t i = 0; i < nv; i++) {

		for (ssi_size_t j = 0; j < nh; j++) {

			if (count < n_names) {

				object = Factory::GetObjectFromId(names[count]);
				item_s item;
				item.obj = object;
				item.pos = ssi_rectf(x + j*w, y + i*h, w, h);
				_items.push_back(item);
			}

			++count;
		}
	}

	std::vector<ssi_char_t *>::iterator it;
	for (it = names.begin(); it != names.end(); it++) {
		delete[](*it);
	}
	names.clear();

}

void Decorator::update() {

   #ifndef SSI_USE_SDL
	if (_options.icon && !_window) {
		_window = new Window();
		_window->setIcons(IWindow::ICONS::SYSTEMTRAY);
		_window->setCallback(this);
		_window->setTitle(_options.title);
		_window->create();
	}
    #endif

	readOptions();

	ssi_char_t message[SSI_MAX_CHAR];

	std::vector<item_s>::iterator it;

	IObject *obj;
	ssi_rectf_t pos;
	for (it = _items.begin(); it != _items.end(); it++) {
		obj = it->obj;
		pos = it->pos;
		pos.left += _origin.x;
		pos.top += _origin.y;
		pos.left *= _scale_x;
		pos.top *= _scale_y;
		pos.width *= _scale_x;
		pos.height *= _scale_y;
		if (obj) {
			ssi_sprint(message, "%f,%f,%f,%f", pos.left, pos.top, pos.width, pos.height);
			ssi_msg(SSI_LOG_LEVEL_DETAIL, "move '%s' to '%.0f,%.0f,%.0f,%.0f'", Factory::GetObjectId(obj), pos.left, pos.top, pos.width, pos.height);
			obj->notify(INotify::COMMAND::WINDOW_MOVE, message);
			obj->notify(_options.show ? INotify::COMMAND::WINDOW_SHOW : INotify::COMMAND::WINDOW_HIDE);
			obj->notify(_options.minmax ? INotify::COMMAND::MINMAX_SHOW : INotify::COMMAND::MINMAX_HIDE);
		}
	}
}

bool Decorator::parse(TiXmlElement *node) {

	TiXmlElement *area = node->FirstChildElement("area");
	while (area) {

		const ssi_char_t *ids = area->GetText();
		const ssi_char_t *pos = area->Attribute("pos");
		const ssi_char_t *nh = area->Attribute("nh");
		const ssi_char_t *nv = area->Attribute("nv");
		
		add(ids, pos, nh, nv);

		area = area->NextSiblingElement("area");
	}

	return true;
}

bool Decorator::notify(INotify::COMMAND::List command, const ssi_char_t *message) {

	switch (command) {
	case INotify::COMMAND::OPTIONS_CHANGE:
	{
		update();
		return true;
	}	
	}

	return false;
}

bool Decorator::isVisible() {
	return _show;
}
void Decorator::show(Window *w) {

	_options.lock();
	_options.show = true;
	_options.unlock();

	update();
}
void Decorator::hide(Window *w) {
	
	_options.lock();
	_options.show = false;
	_options.unlock();

	update();
}

bool Decorator::isMinMaxVisible() {
	return _minmax_show;
}
void Decorator::minmax_show(Window *w) {

	_options.lock();
	_options.minmax = true;
	_options.unlock();

	update();
}

void Decorator::minmax_hide(Window *w) {

	_options.lock();
	_options.minmax = false;
	_options.unlock();

	update();
}

}
