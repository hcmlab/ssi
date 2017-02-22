// ImageViewer.h
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

#ifndef SSI_BROWSER_IMAGEVIEWER_H
#define SSI_BROWSER_IMAGEVIEWER_H

#include "base/IObject.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class ImageCanvas;
class Window;
class Canvas;

class ImageViewer : public IObject {

public:

	class Options : public OptionList {

	public:

		Options() {

			setPos(0, 0, 100, 100);

			setTitle("ImageViewer");
			setUrl("");
			setDirectory(".\\");
			setExtension("jpg");

			addOption("url", url, SSI_MAX_CHAR, SSI_CHAR, "path of start page (can be empty)");
			addOption("title", mname, SSI_MAX_CHAR, SSI_CHAR, "window caption");
			addOption("pos", &mpos, 4, SSI_INT, "window position in pixels [left,top,width,height]");						
			addOption("directory", &directory, SSI_MAX_CHAR, SSI_CHAR, "default directory (if file is not found it will be searched there)");
			addOption("extension", &extension, 10, SSI_CHAR, "default extension (if file is not found it will be added)");
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

		void setDirectory(const ssi_char_t *dir) {
			ssi_strcpy(this->directory, dir);
		}

		void setExtension(const ssi_char_t *ext) {
			ssi_strcpy(this->extension, ext);
		}

		int mpos[4];
		ssi_char_t mname[SSI_MAX_CHAR];
		ssi_char_t url[SSI_MAX_CHAR];
		ssi_char_t directory[SSI_MAX_CHAR];
		ssi_char_t extension[10];
	};

public:

	static const ssi_char_t *GetCreateName() { return "ImageViewer"; };
	static IObject *Create(const ssi_char_t *file) { return new ImageViewer(file); };
	virtual ~ImageViewer();

	Options *getOptions() { return &_options; };
	const ssi_char_t *getName() { return GetCreateName(); };
	const ssi_char_t *getInfo() { return "Image viewer."; };

	void listen_enter();
	bool update(IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms);
	void listen_flush();

	bool notify(INotify::COMMAND::List command, const ssi_char_t *message);
	
protected:

	ImageViewer(const ssi_char_t *file = 0);
	ssi_char_t *_file;
	Options _options;
	static char ssi_log_name[];

	Window *_window;
	Canvas *_canvas;
	ImageCanvas *_img_canvas;
};

}

#endif
