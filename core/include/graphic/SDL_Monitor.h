// Monitor.h
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

#ifndef SSI_GRAPHIC_SDL_MONITOR_H
#define SSI_GRAPHIC_SDL_MONITOR_H

#include "SSI_Define.h"

#ifdef SSI_USE_SDL

#include "base/IMonitor.h"
#include "base/IPainter.h"
#include "thread/Lock.h"

struct SDL_Renderer;
struct SDL_Texture;
struct FTT_Font;

namespace ssi {

class Monitor : public IMonitor {

public:

	Monitor (ssi_size_t maxlen = SSI_MAX_CHAR);
	virtual ~Monitor ();
	
	void create(IWindow *parent);
	void close();
	ssi_handle_t getHandle();
	void update();
	void setPosition(ssi_rect_t rect);

	void print (const ssi_char_t *str);	
	void clear();
	void setFont(const ssi_char_t *name, ssi_size_t size);

protected:
	
	void update_safe();

	IPainter *_painter;
	SDL_Renderer *_renderer;
	SDL_Texture *_texture;
	IPainter::ITool *_pen;
	IPainter::ITool *_brush;
	IPainter::ITool *_font;
	ssi_char_t *_font_name;
	ssi_size_t _font_size;	
	IWindow *_parent;

	int _width, _height;

	Mutex _mutex;
	ssi_char_t *_buffer;
	ssi_size_t _n_buffer;
	ssi_size_t _buffer_count;
	ssi_char_t *_buffer_safe;
};

}

#endif

#endif