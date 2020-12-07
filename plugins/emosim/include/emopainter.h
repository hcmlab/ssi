// emopainter.h
// author: Florian Lingenfelser <florian.lingenfelser@informatik.uni-augsburg.de>
// created: 2020/10/26
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

#ifndef SSI_EMOPAINTER_H
#define SSI_EMOPAINTER_H

#include "base/IObject.h"
#include "thread/Thread.h"
#include "thread/Lock.h"
#include "base/ICanvas.h"

namespace ssi {

class EmoPainter : public ICanvasClient {

public:

	EmoPainter ();
	~EmoPainter ();

	void create(ICanvas *parent);
	void close();

	void paint(ssi_handle_t hdc, ssi_rect_t rect);
	void setWindowCaption (ssi_char_t *caption);

	void setData(ssi_char_t* emotion_label, ssi_char_t* emotion_label_user, ssi_char_t* emotion_label_base,
		ssi_real_t valence, ssi_real_t arousal,
		ssi_real_t valence_user, ssi_real_t arousal_user,
		ssi_real_t empathy_factor_valence, ssi_real_t empathy_factor_arousal,
		ssi_real_t valence_base, ssi_real_t arousal_base,
		ssi_real_t habituation_pos, ssi_real_t habituation_neg,
		ssi_real_t openness, ssi_real_t conscientiousness, ssi_real_t extraversion, ssi_real_t agreeableness, ssi_real_t neuroticism);

	void createAxisPen ();
	void createEmoPenRed ();
	void createEmoPenGreen();
	void createEmoBrushRed ();
	void createEmoBrushGreen ();
	void createBackBrushBlack ();
	void createBackBrushWhite ();

	void drawLine(HDC hdc, int x1, int y1, int x2, int y2);
	void drawRect(HDC hdc, int x, int y, int w, int h);
	void drawCircle(HDC hdc, int x, int y, int r);
	void paintVAGrid(HDC hdc, RECT rect);
	void paintBlackBox(HDC hdc, RECT rect);
	void paintWhiteBox(HDC hdc, RECT rect);
	void paintEmotion(HDC hdc, RECT rect);
	void printInfo(HDC hdc, RECT rect);
	void paintBars(HDC hdc, RECT rect, int nBars);
	
protected:

	static ssi_char_t *ssi_log_name;
	static int ssi_log_level;

	ICanvas *_parent;

	ssi_char_t *_window_caption;

	Mutex _mutex;

	ssi_char_t* _emotion_label;
	ssi_char_t* _emotion_label_user;
	ssi_char_t* _emotion_label_base;
	ssi_real_t* _emotion_point;
	ssi_real_t* _emotion_point_base;
	ssi_real_t* _emotion_point_user;
	ssi_real_t _empathy_factor_valence;
	ssi_real_t _empathy_factor_arousal;
	ssi_real_t _habituation_pos;
	ssi_real_t _habituation_neg;
	ssi_real_t* _ocean;

	int lineWidth;
	int lineStyle;
	int pointSize;
	COLORREF axisColor;
	HPEN axisPen;
	COLORREF emoColorRed;
	HPEN emoPenRed;
	COLORREF emoColorGreen;
	HPEN emoPenGreen;
	HBRUSH emoBrushRed;
	HBRUSH emoBrushGreen;
	COLORREF backColorBlack;
	HBRUSH backBrushBlack;
	COLORREF backColorWhite;
	HBRUSH backBrushWhite;

};

};

#endif

