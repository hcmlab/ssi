// Console.h
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

#ifndef SSI_GRAPHIC_CONSOLE_H
#define SSI_GRAPHIC_CONSOLE_H

#include "base/IObject.h"

namespace ssi {

class Console : public IObject {

public:

	static const ssi_char_t *GetCreateName() { return "Console"; };
	static IObject *Create(const ssi_char_t *file) { return new Console(); };
	~Console();

	IOptions *getOptions() { return 0; }
	const ssi_char_t *getName() { return GetCreateName(); }
	const ssi_char_t *getInfo() { return "The console window of the application."; }
	
	virtual void show();
	virtual void hide();

	virtual void setPosition(ssi_rect_t rect);
	virtual bool setPosition(const ssi_char_t *position);
	bool notify(INotify::COMMAND::List command, const ssi_char_t *message);

protected:

	Console();

	static ssi_char_t *ssi_log_name;
	static int ssi_log_level;

	static ssi_handle_t _hWnd;	
};

}

#endif
