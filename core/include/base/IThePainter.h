// FileProvider.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2010/03/11 
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

#ifndef SSI_BASE_ITHEPAINTER_H
#define SSI_BASE_ITHEPAINTER_H

#include "base/IObject.h"
#include "base/ICanvas.h"
#include "base/IColormap.h"

namespace ssi {

class IThePainter : public IObject {

public:

	virtual ~IThePainter () {};

	virtual int AddCanvas(const char* name) = 0;
	virtual bool RemCanvas(int convas_id) = 0;
	virtual bool AddObject(int canvas_id, ICanvasClient *object) = 0;
	virtual bool Update(int canvas_id) = 0;
	virtual bool Show(int canvas_id) = 0;
	virtual bool Hide(int canvas_id) = 0;
	virtual bool Clear(int canvas_id) = 0;
	virtual bool Move(int canvas_id, int x, int y, int width, int height, bool paramsDescribeClientRect = false) = 0;
	virtual void Arrange (int numh, int numv, int start_x, int start_y, int width, int height) = 0;
	virtual void ArrangeRelative (int numh, int numv, ssi_real_t start_x, ssi_real_t start_y, ssi_real_t width, ssi_real_t height) = 0;
	virtual void MoveConsole (int x, int y, int width, int height) = 0;
	virtual void MoveConsoleRelative (ssi_real_t x, ssi_real_t y, ssi_real_t width, ssi_real_t height) = 0;
	virtual void SetScreen (ssi_real_t width, ssi_real_t height) = 0;
	virtual void Clear () = 0;

};

}

#endif
