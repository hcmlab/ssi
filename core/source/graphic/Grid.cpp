// Grid.cpp
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

#include "SSI_Define.h"

#ifndef SSI_USE_SDL

#include "graphic/Grid.h"
#include "base/Factory.h"

#include <shlwapi.h> 
#include <commctrl.h>

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif


namespace ssi {

ssi_char_t *Grid::ssi_log_name = "grid______";

Grid::Grid(const ssi_char_t *name)
	: _parent(0),	
	_callback (0),
	_id((HMENU)1),
	_hGrid (0) {

	WNDCLASS wndcls;
	BOOL result = ::GetClassInfo((HINSTANCE)::GetModuleHandle(NULL), "BABYGRID", &wndcls);

	if (result == 0) {
		RegisterGridClass(::GetModuleHandle(NULL));
	}

	_buffer[0] = '\0';
	_name = ssi_strcpy(name);
}

Grid::~Grid () {

	delete[] _name;

	if (_hGrid) {
		::DestroyWindow(_hGrid);
	}
}

ssi_handle_t Grid::getHandle() {
	return _hGrid;
}

void Grid::create(IWindow *parent) {
	_parent = parent;
}

void Grid::create(HWND hwnd, HMENU id) {

	if (id > 0) {
		_id = id;
	}	

	_hGrid = ::CreateWindowEx(WS_EX_CLIENTEDGE,
		"BABYGRID",
		_name,
		WS_VISIBLE | WS_CHILD,
		0, 
		0, 
		0, 
		0,
		hwnd,
		_id,
		::GetModuleHandle(NULL),
		0);

	if (!_hGrid) {
		//PrintLastError();
		ssi_wrn("could not create grid");
	}

	setGridDim(0, 0);
}

void Grid::setGridDim(ssi_size_t n_rows, ssi_size_t n_cols) {

	::SendMessage(_hGrid, BGM_SETGRIDDIM, n_rows, n_cols);
}

void Grid::getText(ssi_size_t row, ssi_size_t col, ssi_size_t n, ssi_char_t *buffer) {

	_BGCELL cell;
	cell.row = row;
	cell.col = col;

	buffer[0] = '\0';
	::SendMessage(_hGrid, BGM_GETCELLDATA, (UINT)&cell, (LPARAM)buffer);
	buffer[n-1] = '\0';
}

void Grid::setText(ssi_size_t row, ssi_size_t col, const char *text, bool editable)
{	

	_BGCELL cell;

	cell.row = (int) row;
	cell.col = (int) col;

	if (!editable) {
		SendMessage(_hGrid, BGM_SETPROTECT, TRUE, 0);
	}
	SendMessage(_hGrid, BGM_SETCELLDATA, (UINT)&cell, (LPARAM)text);
	if (!editable) {
		SendMessage(_hGrid, BGM_SETPROTECT, FALSE, 0);
	}
}

void Grid::setCheckBox(ssi_size_t row, ssi_size_t col, bool checked, bool editable) {

	_BGCELL cell;

	cell.row = (int)row;
	cell.col = (int)col;

	if (!editable) {
		SendMessage(_hGrid, BGM_SETPROTECT, TRUE, 0);
	}
	SendMessage(_hGrid, BGM_SETCELLDATA, (UINT)&cell, checked ? (LPARAM)"TRUE" : (LPARAM)"FALSE");
	if (!editable) {
		SendMessage(_hGrid, BGM_SETPROTECT, FALSE, 0);
	}
}

void Grid::setEditable(bool flag) {

	::SendMessage( _hGrid, BGM_SETEDITABLE, flag ? TRUE : FALSE, 0);
}

void Grid::setExtendLastColumn(bool flag) {

	::SendMessage(_hGrid, BGM_EXTENDLASTCOLUMN, flag ? TRUE : FALSE, 0);
}

void Grid::setAllowColumnResize(bool flag) {

	::SendMessage(_hGrid, BGM_SETALLOWCOLRESIZE, flag ? TRUE : FALSE, 0);
}

void Grid::setEllipsis(bool flag) {

	::SendMessage(_hGrid, BGM_SETELLIPSIS, flag ? TRUE : FALSE, 0);
}

void Grid::setColumnsAutoWidth(bool flag) {

	::SendMessage(_hGrid, BGM_SETCOLAUTOWIDTH, flag ? TRUE : FALSE, 0);
}

void Grid::setColumnsNumbered(bool flag) {

	::SendMessage(_hGrid, BGM_SETCOLSNUMBERED, flag ? TRUE : FALSE, 0);
}

void Grid::setRowsNumbered(bool flag) {

	::SendMessage(_hGrid, BGM_SETROWSNUMBERED, flag ? TRUE : FALSE, 0);
}

void Grid::setShowHilight(bool flag) {

	::SendMessage(_hGrid, BGM_SHOWHILIGHT, flag ? TRUE : FALSE, 0);
}

void Grid::setHeaderRowHeight(ssi_size_t height) {

	::SendMessage(_hGrid, BGM_SETHEADERROWHEIGHT, height, 0);
}

void Grid::setColumnWidth(ssi_size_t column, ssi_size_t width) {

	::SendMessage(_hGrid, BGM_SETCOLWIDTH, column, width);
}

void Grid::setCursorColor(ssi_rgb_t rgb) {
	
	::SendMessage(_hGrid, BGM_SETCURSORCOLOR, (UINT)rgb, 0);
}

void Grid::setGridLineColor(ssi_rgb_t rgb) {

	::SendMessage(_hGrid, BGM_SETGRIDLINECOLOR, (UINT)rgb, 0);
}

void Grid::setProtectColor(ssi_rgb_t rgb) {

	::SendMessage(_hGrid, BGM_SETPROTECTCOLOR, (UINT)rgb, 0);
}
void Grid::setUnProtectColor(ssi_rgb_t rgb) {

	::SendMessage(_hGrid, BGM_SETUNPROTECTCOLOR, (UINT)rgb, 0);
}

void Grid::setPosition(ssi_rect_t rect) {

	::MoveWindow(_hGrid, 0, 0, rect.width, rect.height, TRUE);
}

void Grid::update() {
}

LRESULT CALLBACK Grid::windowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

	switch (msg) {

	case WM_CREATE: {

		create(hWnd, (HMENU)1);

		return 0;
	}

	case WM_COMMAND: {
		
		int wmId = LOWORD(wParam);
		int wmEvent = HIWORD(wParam);

		if (wmId == (int) _id) {

			if (HIWORD(wParam) == BGN_EDITEND) {

				_BGCELL cell;
				cell.row = LOWORD(lParam);
				cell.col = HIWORD(lParam);

				LRESULT dtype = SendMessage(_hGrid, BGM_GETTYPE, (UINT)&cell, 0);

				//datatype 1 is alphanumeric data
				//datatype 2 is numeric data
				//datatype 3 is BOOLEAN TRUE data
				//datatype 4 is BOOLEAN FALSE data				
				if (dtype == 1 || dtype == 2) {
					if (_callback) {
						_buffer[0] = '\0';
						::SendMessage(_hGrid, BGM_GETCELLDATA, (UINT)&cell, (LPARAM)_buffer);
						_buffer[SSI_MAX_CHAR - 1] = '\0';
						_callback->update((ssi_size_t)cell.row, (ssi_size_t)cell.col, _buffer);
					}					
				}

				return 0;
			}

			if (HIWORD(wParam) == BGN_CELLCLICKED) {

				_BGCELL cell;
				cell.row = LOWORD(lParam);
				cell.col = HIWORD(lParam);

				LRESULT locked = ::SendMessage(_hGrid, BGM_GETPROTECTION, (UINT)&cell, 0);
				if (!locked) {

					//datatype 1 is alphanumeric data
					//datatype 2 is numeric data
					//datatype 3 is BOOLEAN TRUE data
					//datatype 4 is BOOLEAN FALSE data
					LRESULT dtype = ::SendMessage(_hGrid, BGM_GETTYPE, (UINT)&cell, 0);
					if (dtype == 3) {
						::SendMessage(_hGrid, BGM_SETCELLDATA, (UINT)&cell, (LPARAM)"FALSE");
						if (_callback) {
							_callback->update((ssi_size_t)cell.row, (ssi_size_t)cell.col, false);
						}
					} else if (dtype == 4) {
						::SendMessage(_hGrid, BGM_SETCELLDATA, (UINT)&cell, (LPARAM)"TRUE");
						if (_callback) {
							_callback->update((ssi_size_t)cell.row, (ssi_size_t)cell.col, true);
						}
					}
				}

				return 0;
			}
		}
	}

	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

}

#endif