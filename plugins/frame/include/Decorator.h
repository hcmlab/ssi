// Decorator.h
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

#pragma once

#ifndef SSI_GRAPHIC_DECORATOR_H
#define SSI_GRAPHIC_DECORATOR_H

#include "base/IObject.h"
#include "ioput/option/OptionList.h"
#if _WIN32||_WIN64
#include "graphic/Window.h"
#else
#ifdef SSI_USE_SDL
#include "graphic/Window.h"
#else
#include "graphic/WindowFallback.h"
#endif
#endif

#define SSI_DECORATOR_SCREEN_NAME "SCREEN"

namespace ssi {

class TiXmlElement;

class Decorator : public IObject, public Window::ShowAndHideCallback {

public:

	class Options : public OptionList {

	public:

		Options() : show(true), icon(false), minmax(false) {

			setOrigin(0, 0);
			setScale(1.0f, 1.0f);
			setTitle("");

			addOption("icon", &icon, 1, SSI_BOOL, "show system tray icon");
			addOption("title", title, SSI_MAX_CHAR, SSI_CHAR, "title of system tray icon");
			addOption("show", &show, 1, SSI_BOOL, "show windows", false);
			addOption("origin", origin, 2, SSI_REAL, "origin in pixels (x,y)", false);
			addOption("scale", scale, 2, SSI_REAL, "scale factor (x,y)", false);
			addOption("minmax", &minmax, 1, SSI_BOOL, "show minimize and maximize icons in window title bars");


		};

		void setTitle(const ssi_char_t *string) {
			if (string) {
				ssi_strcpy(this->title, string);
			}
		}
		void setOrigin(ssi_real_t x, ssi_real_t y) {
			origin[0] = x;
			origin[1] = y;
		}
		void setScale(ssi_real_t x, ssi_real_t y) {
			scale[0] = x;
			scale[1] = y;
		}

		bool icon;
		bool minmax;
		ssi_char_t title[SSI_MAX_CHAR];
		bool show;
		ssi_real_t origin[2];
		ssi_real_t scale[2];
	};

public:

	static const ssi_char_t *GetCreateName() { return "Decorator"; };
	static IObject *Create(const ssi_char_t *file) { return new Decorator(file); };
	~Decorator();

	Options *getOptions() { return &_options; };
	const ssi_char_t *getName() { return GetCreateName(); };
	const ssi_char_t *getInfo() { return "Allows to arrange windows on the desktop."; };

	virtual bool add(const ssi_char_t *ids, const ssi_char_t *pos = 0, const ssi_char_t *nh = 0, const ssi_char_t *nv = 0);
	virtual void add(const ssi_char_t *ids, ssi_rect_t pos, ssi_size_t nh = 0, ssi_size_t nv = 0);
	virtual void add(const ssi_char_t *ids, ssi_rectf_t pos, ssi_size_t nh = 0, ssi_size_t nv = 0);

	// added for compatibility reasons to replace ThePainter::Arrange()
	virtual void add(const ssi_char_t *ids, int nh, int nv, int x, int y, int width, int height);
	virtual void add(const ssi_char_t *ids, int x, int y, int width, int height);

	bool notify(INotify::COMMAND::List command, const ssi_char_t *message);
	virtual void update();
	virtual void clear();
		
	virtual bool parse(TiXmlElement *node);

protected:

	Decorator(const ssi_char_t *file = 0);
	Options _options;
	ssi_char_t *_file;

	void readOptions();

	bool _show;
	bool _minmax_show;
	ssi_pointf_t _origin;
	ssi_real_t _scale_x;
	ssi_real_t _scale_y;
	ssi_pointf_t _screen;

	struct item_s {
		IObject *obj;
		ssi_rectf_t pos;
	};
	std::vector<item_s> _items;

	Window *_window;
	bool isVisible();
	bool isMinMaxVisible();
	void show(Window *w);
	void hide(Window *w);

	void minmax_show(Window *w);
	void minmax_hide(Window *w);

	static const ssi_char_t *ssi_log_name;
	static int ssi_log_level;

};
}

#endif
