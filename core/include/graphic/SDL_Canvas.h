// SDL_Canvas.h
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

#ifndef SSI_GRAPHIC_SDL_CANVAS_H
#define SSI_GRAPHIC_SDL_CANVAS_H

#include "SSI_Define.h"

#ifdef SSI_USE_SDL

#include "base/ICanvas.h"

struct SDL_Renderer;
struct SDL_Window;
struct SDL_Texture;

namespace ssi {

class Canvas : public ICanvas
{
	
public:

	Canvas();
	~Canvas();

	void create(IWindow *parent);
	ssi_handle_t getHandle();
	void update();
	void update_safe();
	void setPosition(ssi_rect_t rect);
	void addClient(ICanvasClient *client);
	void close();
	
protected:

	void paint();
	void paint_safe();

	void mouseDown(ssi_point_t position, IWindow::KEY vkey);
	void keyDown(IWindow::KEY key, IWindow::KEY vkey);

	std::vector<ICanvasClient *> _objects;

	IWindow *_parent;
	SDL_Texture *_texture;
	SDL_Renderer *_renderer;

	int _width, _height;
};

} 

#endif

#endif
