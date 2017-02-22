// Browser.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 17/11/2014
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
// version 3 of the License, or any later version.
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

#pragma once

#ifndef SSI_BROWSER_BROWSER_H
#define SSI_BROWSER_BROWSER_H

#include "base/IObject.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class CWebBrowser;

class Browser : public IObject {

public:

	class Options : public OptionList {

	public:

		Options()
			: HTMLstr(false) 
		{

			setPos(0, 0, 100, 100);

			setTitle("Browser");
			setUrl("");

			addOption("url", url, SSI_MAX_CHAR, SSI_CHAR, "path of start page (can be empty)");
			addOption("HTMLstr", &HTMLstr, 1, SSI_BOOL, "display as html string");
			addOption("title", mname, SSI_MAX_CHAR, SSI_CHAR, "window caption");			
			addOption("pos", &mpos, 4, SSI_INT, "window position in pixels [left,top,width,height]");
		}

		void setPos(int x, int y, int width, int height) {
			mpos[0] = x;
			mpos[1] = y;
			mpos[2] = width;
			mpos[3] = height;
		}

		void setUrl(const ssi_char_t *url) {
			ssi_strcpy(this->url, url);
		}

		void setTitle(const ssi_char_t *name) {
			ssi_strcpy(mname, name);
		}

		int mpos[4];
		ssi_char_t mname[SSI_MAX_CHAR];

		bool HTMLstr;
		ssi_char_t url[SSI_MAX_CHAR];
	};

public:

	static const ssi_char_t *GetCreateName() { return "Browser"; };
	static IObject *Create(const ssi_char_t *file) { return new Browser(file); };
	virtual ~Browser();

	Options *getOptions() { return &_options; };
	const ssi_char_t *getName() { return GetCreateName(); };
	const ssi_char_t *getInfo() { return "Browser."; };

	void listen_enter();
	bool update(IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms);
	void listen_flush();

	bool notify(INotify::COMMAND::List command, const ssi_char_t *message);
	
protected:

	Browser(const ssi_char_t *file = 0);
	ssi_char_t *_file;
	Options _options;
	static char ssi_log_name[];

	CWebBrowser *_browser;
};

}

#endif
