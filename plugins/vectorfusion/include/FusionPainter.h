// FusionPainter.h
// author: Florian Lingenfelser <florian.lingenfelser@informatik.uni-augsburg.de>
// created: 2012/04/25
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute itand/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#pragma once

#ifndef SSI_FUSIONPAINTER_H
#define SSI_FUSIONPAINTER_H

#include "base/IObject.h"
#include "thread/Thread.h"
#include "thread/Lock.h"
#include "EVector.h"

#include <deque>

#include "base/ICanvas.h"

namespace ssi {

class FusionPainter : public ICanvasClient {

public:

	FusionPainter ();
	~FusionPainter ();

	void create(ICanvas *parent);
	void close();

	void paint(ssi_handle_t hdc, ssi_rect_t rect);
	void setData (ssi_size_t Dimension, ssi_real_t *Baseline, ssi_real_t Threshold, std::deque<EVector*> &VectorList, ssi_real_t* FusionPoint, EVector* FusionVector, bool DecayingVectors, bool PaintEvents);

	void initAxisCaption(ssi_size_t dim);
	void setAxisCaption(ssi_size_t dim, ssi_char_t* caption);
	void setWindowCaption (ssi_char_t *caption);

	void update_evectors(std::deque<EVector*> &VectorList);
	void update_fpoint(ssi_real_t *FusionPoint);
	void update_fvector(EVector *FusionVector);
	void update_baseline(ssi_real_t *Baseline);

	void createEventPen();
	void createAxisPen ();
	void createMassPen ();
	void createFusionPen ();
	void createFusionBrush ();
	void createBackBrush ();
	void createBaselinePen();

	void drawLine(HDC hdc, int x1, int y1, int x2, int y2);
	void drawRect(HDC hdc, int x, int y, int w, int h);
	void paintGrid(HDC hdc, RECT rect);
	void paintBaseline(HDC hdc, RECT rect);
	void paintEvents(HDC hdc, RECT rect);
	void paintMass(HDC hdc, RECT rect);
	void paintFusion(HDC hdc, RECT rect);
	
protected:

	static ssi_char_t *ssi_log_name;
	static int ssi_log_level;

	ICanvas *_parent;

	bool _decaying_vectors;
	bool _paint_events;

	std::deque<EVector*> EVectorList;
	ssi_size_t _dim;
	ssi_real_t *_baseline;
	ssi_real_t _threshold;
	ssi_real_t *_fvector;
	ssi_real_t *_fusion_point;

	ssi_char_t **_axis_captions;
	ssi_char_t *_window_caption;

	Mutex _mutex;

	int lineWidth;
	int lineStyle;
	int pointSize;
	COLORREF eventColor;
	HPEN eventPen;
	COLORREF axisColor;
	HPEN axisPen;
	COLORREF massColor;
	HPEN massPen;
	COLORREF backColor;
	HBRUSH backBrush;
	COLORREF fusionColor;
	HPEN fusionPen;
	HBRUSH fusionBrush;
	COLORREF baselineColor;
	HPEN baselinePen;

	int _p_perdim;

};

};

#endif

