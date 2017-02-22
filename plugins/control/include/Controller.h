// Controller.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/05/26
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

#ifndef SSI_CONTROL_CONTROLLER_H
#define SSI_CONTROL_CONTROLLER_H

#include "SSI_Define.h"

#ifndef SSI_USE_SDL

#include "base/IRunnable.h"
#include "ioput/option/OptionList.h"
#include "event/EventAddress.h"
#include "base/IWindow.h"
#include "graphic/TextBox.h"

namespace ssi {

class Controller : public SSI_IRunnableObject, public TextBox::ICallback {

public:

	class Options : public OptionList {

	public:

		Options () : buffer (10240), history(10) {

			pos[0] = 0;
			pos[1] = 0;
			pos[2] = 100;
			pos[3] = 100;

			setTitle("Controller");

			addOption ("title", title, SSI_MAX_CHAR, SSI_CHAR, "window title");
			addOption ("pos", &pos, 4, SSI_INT, "position on screen [posx,posy,width,height]");			
			addOption ("buffer", &buffer, 1, SSI_SIZE, "size of character buffer");			
			addOption ("history", &history, 1, SSI_SIZE, "length of history");
		}

		void setTitle(const ssi_char_t *string) {
			if (string) {
				ssi_strcpy(this->title, string);
			}
		}

		void setPos (int x, int y, int width, int height) {
			pos[0] = x;
			pos[1] = y;
			pos[2] = width;
			pos[3] = height;
		}

		int pos[4];
		ssi_char_t title[SSI_MAX_CHAR];
		ssi_size_t buffer;
		ssi_size_t history;
	};

public:

	static const ssi_char_t *GetCreateName() { return "Controller"; };
	static IObject *Create(const ssi_char_t *file) { return new Controller(file); };
	~Controller();

	static ssi_char_t *ssi_log_name;
	int ssi_log_level;

	Options *getOptions() { return &_options; };
	const ssi_char_t *getName() { return GetCreateName(); };
	const ssi_char_t *getInfo() { return "Allows to dis/enable objects and change their options at run-time."; };

	void update(const ssi_char_t *text);
	bool keyDown(IWindow::KEY key, IWindow::KEY vkey);
	void mouseUp(ssi_size_t position, IWindow::KEY vkey);
	void showHistory(int move);

	bool start();
	bool stop();

	bool notify(INotify::COMMAND::List command, const ssi_char_t *message);

protected:

	Controller (const ssi_char_t *file = 0);
	Controller::Options _options;
	ssi_char_t *_file;

	void autoComplete();
	void autoCompleteObject(const ssi_char_t *id);
	void autoCompleteOption(const ssi_char_t *id, const ssi_char_t *arg);
	ssi_char_t *lastLine(const ssi_char_t *line);
	IObject *parseCommand(const ssi_char_t *line, ssi_char_t **id, ssi_char_t **arg);
	bool sendCommand(IObject *object, const ssi_char_t *id, const ssi_char_t *arg);
	void showHelp(IObject *object);

	IWindow *_window;
	TextBox *_console;
	ssi_size_t _n_buffer;
	ssi_char_t *_buffer;
	ssi_size_t _n_history;
	ssi_char_t **_history;
	int _history_insert;
	int _history_last;

	ssi_size_t _n_object_ids;
	ssi_char_t **_object_ids;
};

}

#endif

#endif