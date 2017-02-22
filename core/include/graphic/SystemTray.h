// SystemTray.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2016/02/02
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

#ifndef SSI_GRAPHIC_SYSTEMTRAY_H
#define SSI_GRAPHIC_SYSTEMTRAY_H

#include "SSI_Define.h"

#ifndef SSI_USE_SDL

#include "SSI_Cons.h"
#include <shellapi.h>

#define SSI_GRAPHIC_SYSTEMTRAY_MSG   (WM_USER + 1000)
#define SSI_GRAPHIC_SYSTEMTRAY_SHOW  (SSI_GRAPHIC_SYSTEMTRAY_MSG + 1)
#define SSI_GRAPHIC_SYSTEMTRAY_HIDE  (SSI_GRAPHIC_SYSTEMTRAY_MSG + 2)
#define SSI_GRAPHIC_SYSTEMTRAY_ABOUT (SSI_GRAPHIC_SYSTEMTRAY_MSG + 3)
#define SSI_GRAPHIC_SYSTEMTRAY_EXIT  (SSI_GRAPHIC_SYSTEMTRAY_MSG + 4)

#define SSI_GRAPHIC_MINMAX_SHOW		 (SSI_GRAPHIC_SYSTEMTRAY_MSG + 5)
#define SSI_GRAPHIC_MINMAX_HIDE      (SSI_GRAPHIC_SYSTEMTRAY_MSG + 6)

namespace ssi {

class SystemTray {

public:

	SystemTray(HWND hWnd, HICON icon, const ssi_char_t *tooltip);
	virtual ~SystemTray();

	void create();
	void showMenu(bool isVisible, bool minMaxVisible);
	void showAbout();
	void close();

protected:

	NOTIFYICONDATA _nid;
	HWND _hWnd;
	ssi_char_t *_title;
};

}

#endif

#endif