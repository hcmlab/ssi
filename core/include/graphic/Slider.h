// Slider.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2007/10/15 
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

#ifndef SSI_GRAPHIC_SLIDER_H
#define SSI_GRAPHIC_SLIDER_H

#include "SSI_Define.h"

#ifndef SSI_USE_SDL

#include "base/IWindow.h"
#include "graphic/Tab.h"

namespace ssi {

class Slider : public ITabClient {

public:

	struct ORIENTATION {
		enum List {
			HORIZONTAL = 0,
			VERTICAL
		};
	};

public:
	
	class ICallback {

	public:
	
		virtual void update (ssi_real_t value) = 0;
	};

public:

	Slider (const ssi_char_t *name,
		ssi_real_t value,
		ssi_real_t min,
		ssi_real_t max,
		ssi_size_t steps = 100,
		ORIENTATION::List orientation = ORIENTATION::HORIZONTAL);
	~Slider ();

	void create(IWindow *parent);
	void create(HWND hwnd, HMENU id);
	void close() {};
	ssi_handle_t getHandle();
	void update();
	void setPosition(ssi_rect_t rect);
	
	void set (ssi_real_t value);
	ssi_real_t get ();

	void setCallback (ICallback *callback) {
		_callback = callback;
	};

protected:

	static ssi_char_t *ssi_log_name;

	LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	HWND _hWnd;
	HWND _hSlider;
	IWindow *_parent;

	ssi_size_t _steps;
	ssi_real_t _value, _min, _max;
	ssi_char_t *_name;
	ORIENTATION::List _orientation;

	ICallback *_callback;
};

}

#endif

#endif