// ControlSlider.h
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

#ifndef SSI_CONTROL_CONTROLSLIDER_H
#define SSI_CONTROL_CONTROLSLIDER_H

#include "SSI_Define.h"

#ifndef SSI_USE_SDL

#include "base/IRunnable.h"
#include "ioput/option/OptionList.h"
#include "event/EventAddress.h"
#include "base/IWindow.h"
#include "graphic/Slider.h"
namespace ssi {

class ControlSlider : public SSI_IRunnableObject, public Slider::ICallback {

public:

	class Options : public OptionList {

	public:

		Options ()
			: steps(100), defval(0.5f), minval(0.0f), maxval(1.0f), orientation(Slider::ORIENTATION::HORIZONTAL) {

			pos[0] = 0;
			pos[1] = 0;
			pos[2] = 100;
			pos[3] = 100;

			setTitle("");
			setId("");
			setName("");

			addOption ("title", &title, SSI_MAX_CHAR, SSI_CHAR, "window title (if empty set to 'id:option')");
			addOption ("pos", &pos, 4, SSI_INT, "position of check box on screen [posx,posy,width,height]");
			addOption ("id", id, SSI_MAX_CHAR, SSI_CHAR, "object id");
			addOption ("name", option, SSI_MAX_CHAR, SSI_CHAR, "option name");				
			addOption ("defval", &defval, 1, SSI_REAL, "default slider value");
			addOption ("minval", &minval, 1, SSI_REAL, "min slider value");
			addOption ("maxval", &maxval, 1, SSI_REAL, "max slider value");
			addOption ("steps", &steps, 1, SSI_UINT, "slider steps");
			addOption ("orientation", &orientation, 1, SSI_INT, "orientation (0=horizontal, 1=vertical)");

		}

		void setTitle(const ssi_char_t *string) {
			if (string) {
				ssi_strcpy(this->title, string);
			}
		}
		void setId (const ssi_char_t *string) {			
			if (string) {
				ssi_strcpy(this->id, string);
			}
		}
		void setName(const ssi_char_t *string) {
			if (string) {
				ssi_strcpy(this->option, string);
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
		ssi_char_t id[SSI_MAX_CHAR];
		ssi_char_t option[SSI_MAX_CHAR];			
		ssi_size_t steps;
		ssi_real_t defval, minval, maxval;
		Slider::ORIENTATION::List orientation;
	};

public:

	static const ssi_char_t *GetCreateName() { return "ControlSlider"; };
	static IObject *Create(const ssi_char_t *file) { return new ControlSlider(file); };
	~ControlSlider();

	static ssi_char_t *ssi_log_name;
	int ssi_log_level;

	Options *getOptions() { return &_options; };
	const ssi_char_t *getName() { return GetCreateName(); };
	const ssi_char_t *getInfo() { return "Slider to control option value of another object."; };

	bool start();
	bool stop();

	bool notify(INotify::COMMAND::List command, const ssi_char_t *message);

protected:

	ControlSlider (const ssi_char_t *file = 0);
	ControlSlider::Options _options;
	ssi_char_t *_file;

	IWindow *_window;
	Slider *_slider;

	bool _ready;
	IObject *_target;	
	void update (ssi_real_t value);
};

}

#endif

#endif