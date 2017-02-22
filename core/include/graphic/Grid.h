// Grid.h
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

#ifndef SSI_GRAPHIC_GRID_H
#define SSI_GRAPHIC_GRID_H

#include "SSI_Define.h"

#ifndef SSI_USE_SDL

#include "base/IWindow.h"
#include "graphic/BabyGrid.h"
#include "graphic/Tab.h"

namespace ssi {

class Grid : public ITabClient {

public:
	
	class ICallback {

	public:
	
		virtual void update(ssi_size_t row, ssi_size_t column, const ssi_char_t *text) = 0;
		virtual void update(ssi_size_t row, ssi_size_t column, bool checked) = 0;
	};

public:

	Grid (const ssi_char_t *name);
	~Grid ();

	void create(IWindow *parent);	
	void create(HWND hwnd, HMENU id);
	void close() {};
	ssi_handle_t getHandle();
	void setPosition(ssi_rect_t rect);
	void update();

	void setGridDim(ssi_size_t n_rows, ssi_size_t n_cols);
	void setText(ssi_size_t row, ssi_size_t col, const char *text, bool editable = true);
	void setCheckBox(ssi_size_t row, ssi_size_t col, bool checked, bool editable = true);

	void getText(ssi_size_t row, ssi_size_t col, ssi_size_t n, ssi_char_t *buffer);

	void setEditable(bool flag);
	void setExtendLastColumn(bool flag);
	void setAllowColumnResize(bool flag);
	void setEllipsis(bool flag);
	void setColumnsAutoWidth(bool flag);
	void setColumnsNumbered(bool flag);
	void setRowsNumbered(bool flag);
	void setShowHilight(bool flag);
	void setHeaderRowHeight(ssi_size_t height);
	void setColumnWidth(ssi_size_t column, ssi_size_t width);
	void setCursorColor(ssi_rgb_t rgb);
	void setGridLineColor(ssi_rgb_t rgb);
	void setProtectColor(ssi_rgb_t rgb);
	void setUnProtectColor(ssi_rgb_t rgb);

	void setCallback (ICallback *callback) {
		_callback = callback;
	};

protected:

	static ssi_char_t *ssi_log_name;

	LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	IWindow *_parent;
	HWND _hGrid;
	HMENU _id;

	ssi_char_t _buffer[SSI_MAX_CHAR];

	ssi_char_t *_name;	
	ICallback *_callback;
};

}

#endif

#endif