// ComboBox.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2015/11/23
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

#ifndef SSI_GRAPHIC_COMBOBOX_H
#define SSI_GRAPHIC_COMBOBOX_H

#include "SSI_Define.h"

#ifndef SSI_USE_SDL

#include "base/IWindow.h"
#include "graphic/Tab.h"

namespace ssi {

class ComboBox : public ITabClient {

public:
	
	class ICallback {

	public:
	
		virtual void update (const ssi_char_t *item) = 0;
	};

public:

	ComboBox (ssi_size_t n_items,
		const ssi_char_t **items,
		ssi_size_t current = 0);
	~ComboBox ();
	
	void create(IWindow *parent);
	void create(HWND hwnd, HMENU id);
	void close() {};
	ssi_handle_t getHandle();
	void update();
	void setPosition(ssi_rect_t rect);
	
	void set (ssi_size_t index);
	void set (const ssi_char_t *item);
	const ssi_char_t *get ();

	void setCallback (ICallback *callback) {
		_callback = callback;
	};

protected:

	static ssi_char_t *ssi_log_name;

	LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	IWindow *_parent;
	HWND _hComboBox;

	ssi_size_t _n_items;
	ssi_size_t _current;
	ssi_char_t **_items;

	ICallback *_callback;
};

}

#endif

#endif