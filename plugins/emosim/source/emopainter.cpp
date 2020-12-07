// emopainter.cpp
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
#include "../include/emopainter.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

	ssi_char_t *EmoPainter::ssi_log_name = "painter___";
	int EmoPainter::ssi_log_level = SSI_LOG_LEVEL_DEFAULT;

EmoPainter::EmoPainter(){

	_parent = 0;
	_window_caption = 0;
	_emotion_label = new ssi_char_t[SSI_MAX_CHAR];
	ssi_sprint(_emotion_label, "neutral");
	_emotion_label_user = new ssi_char_t[SSI_MAX_CHAR];
	ssi_sprint(_emotion_label_user, "neutral");
	_emotion_label_base = new ssi_char_t[SSI_MAX_CHAR];
	ssi_sprint(_emotion_label_base, "neutral");
	_emotion_point = new ssi_real_t[2];
	_emotion_point[0] = 0.0f;
	_emotion_point[1] = 0.0f;
	_emotion_point_base = new ssi_real_t[2];
	_emotion_point_base[0] = 0.0f;
	_emotion_point_base[1] = 0.0f;
	_emotion_point_user = new ssi_real_t[2];
	_emotion_point_user[0] = 0.0f;
	_emotion_point_user[1] = 0.0f;
	_empathy_factor_valence = 0.0f;
	_empathy_factor_arousal = 0.0f;
	_habituation_pos = 0.0f;
	_habituation_neg = 0.0f;
	_ocean = new ssi_real_t[5];
	_ocean[0] = 0.0f;
	_ocean[1] = 0.0f;
	_ocean[2] = 0.0f;
	_ocean[3] = 0.0f;
	_ocean[4] = 0.0f;

	
	// create default pen and brush
	lineStyle = PS_SOLID;
	lineWidth = 1;
	pointSize = 5;
	axisColor = RGB (255, 255, 255);
	emoColorRed = RGB (255, 0, 0);
	emoColorGreen = RGB(0, 255, 0);
	backColorBlack = RGB (0, 0, 0);
	backColorWhite = RGB(255, 255, 255);
	createAxisPen ();
	createEmoPenRed();
	createEmoPenGreen();
	createEmoBrushRed();
	createEmoBrushGreen();
	createBackBrushBlack ();
	createBackBrushWhite();
	
}

EmoPainter::~EmoPainter(){

	Lock lock(_mutex);

	if (_window_caption) {
		delete[] _window_caption;
		_window_caption = 0;
	}
	if (_emotion_label) {
		delete[] _emotion_label;
		_emotion_label = 0;
	}
	if (_emotion_label_user) {
		delete[] _emotion_label_user;
		_emotion_label_user = 0;
	}
	if (_emotion_label_base) {
		delete[] _emotion_label_base;
		_emotion_label_base = 0;
	}
	if (_emotion_point) {
		delete[] _emotion_point;
		_emotion_point = 0;
	}
	if (_emotion_point_base) {
		delete[] _emotion_point_base;
		_emotion_point_base = 0;
	}
	if (_emotion_point_user) {
		delete[] _emotion_point_user;
		_emotion_point_user = 0;
	}
	if (_ocean) {
		delete[] _ocean;
		_ocean = 0;
	}

}

void EmoPainter::create(ICanvas *parent)
{
	_parent = parent;
}

void EmoPainter::close()
{
	if (_window_caption) {
		delete[] _window_caption;
		_window_caption = 0;
	}
	if (_emotion_label) {
		delete[] _emotion_label;
		_emotion_label = 0;
	}
	if (_emotion_label_user) {
		delete[] _emotion_label_user;
		_emotion_label_user = 0;
	}
	if (_emotion_label_base) {
		delete[] _emotion_label_base;
		_emotion_label_base = 0;
	}
	if (_emotion_point) {
		delete[] _emotion_point;
		_emotion_point = 0;
	}
	if (_emotion_point_base) {
		delete[] _emotion_point_base;
		_emotion_point_base = 0;
	}
	if (_emotion_point_user) {
		delete[] _emotion_point_user;
		_emotion_point_user = 0;
	}
	if (_ocean) {
		delete[] _ocean;
		_ocean = 0;
	}
}

void EmoPainter::createAxisPen () {
	::DeleteObject (axisPen);
	axisPen = ::CreatePen (lineStyle, lineWidth, axisColor);
}

void EmoPainter::createEmoPenRed () {
	::DeleteObject (emoPenRed);
	emoPenRed = ::CreatePen (lineStyle, lineWidth, emoColorRed);
}

void EmoPainter::createEmoBrushRed () {
	::DeleteObject (emoBrushRed);
	emoBrushRed = ::CreateSolidBrush (emoColorRed);
}

void EmoPainter::createEmoPenGreen() {
	::DeleteObject(emoPenGreen);
	emoPenGreen = ::CreatePen(lineStyle, lineWidth, emoColorGreen);
}

void EmoPainter::createEmoBrushGreen() {
	::DeleteObject(emoBrushGreen);
	emoBrushGreen = ::CreateSolidBrush(emoColorGreen);
}

void EmoPainter::createBackBrushBlack () {
	::DeleteObject (backBrushBlack);
	backBrushBlack = ::CreateSolidBrush (backColorBlack);
}

void EmoPainter::createBackBrushWhite() {
	::DeleteObject(backBrushWhite);
	backBrushWhite = ::CreateSolidBrush(backColorWhite);
}

void EmoPainter::setData(ssi_char_t* emotion_label, ssi_char_t* emotion_label_user, ssi_char_t* emotion_label_base,
							ssi_real_t valence, ssi_real_t arousal,
							ssi_real_t valence_user, ssi_real_t arousal_user,
							ssi_real_t empathy_factor_valence, ssi_real_t empathy_factor_arousal,
							ssi_real_t valence_base, ssi_real_t arousal_base,
							ssi_real_t habituation_pos, ssi_real_t habituation_neg,
							ssi_real_t openness, ssi_real_t conscientiousness, ssi_real_t extraversion, ssi_real_t agreeableness, ssi_real_t neuroticism) {

	Lock lock(_mutex);

	_emotion_label = ssi_strcpy(emotion_label);
	_emotion_point[0] = valence;
	_emotion_point[1] = arousal;
	_emotion_label_user = ssi_strcpy(emotion_label_user);
	_emotion_label_base = ssi_strcpy(emotion_label_base);
	_emotion_point_user[0] = valence_user;
	_emotion_point_user[1] = arousal_user;
	_empathy_factor_valence = empathy_factor_valence;
	_empathy_factor_arousal = empathy_factor_arousal;
	_emotion_point_base[0] = valence_base;
	_emotion_point_base[1] = arousal_base;
	_habituation_pos = habituation_pos;
	_habituation_neg = habituation_neg;
	_ocean[0] = openness;
	_ocean[1] = conscientiousness;
	_ocean[2] = extraversion;
	_ocean[3] = agreeableness;
	_ocean[4] = neuroticism;

}

void EmoPainter::setWindowCaption (ssi_char_t *caption) {
	_window_caption = ssi_strcpy (caption);
}

void EmoPainter::paint(ssi_handle_t hdc, ssi_rect_t rect){

		Lock lock(_mutex);

		RECT r;
		r.top = rect.top;
		r.left = rect.left;
		r.bottom = rect.top + rect.height;
		r.right = rect.left + rect.width;

		// paint it black (whole rect)
		::FillRect ((HDC) hdc, &r, backBrushBlack);

		// left upper
		r.top = rect.top;
		r.left = rect.left;
		r.bottom = rect.top + rect.height / 2;
		r.right = rect.left + rect.width / 2;
				
		paintVAGrid((HDC)hdc, r);
		paintEmotion((HDC)hdc, r);

		// right upper & right lower
		r.top = rect.top;
		r.left = rect.left + rect.width / 2;
		/*r.bottom = rect.top + rect.height / 2;
		r.right = rect.left + rect.width;*/
		r.bottom = rect.top + rect.height;
		r.right = rect.left + rect.width;

		paintWhiteBox((HDC)hdc, r);
		printInfo((HDC)hdc, r);

		// left lower
		r.top = rect.top + rect.height / 2;
		r.left = rect.left;
		r.bottom = rect.top + rect.height;
		r.right = rect.left + rect.width / 2;

		paintBlackBox((HDC)hdc, r);
		paintBars((HDC)hdc, r, 2);

		// right lower
		/*r.top = rect.top + rect.height / 2;
		r.left = rect.left + rect.width / 2;
		r.bottom = rect.top + rect.height;
		r.right = rect.left + rect.width;

		paintBlackBox((HDC)hdc, r);*/
	
}

SSI_INLINE int GetTextSize (LPSTR a0)
{
    for (int iLoopCounter = 0; ;iLoopCounter++)
    {
        if (a0 [iLoopCounter] == '\0')
            return iLoopCounter;
    }
}

void EmoPainter::drawLine(HDC hdc, int x1, int y1, int x2, int y2){

	::MoveToEx(hdc, x1, y1, 0);
	::LineTo(hdc, x2, y2);

}

void EmoPainter::drawRect(HDC hdc, int x, int y, int w, int h){

	//origin
	::MoveToEx(hdc, x, y, 0);
	//right
	::LineTo(hdc, x + w, y);
	//down
	::MoveToEx(hdc, x + w, y, 0);
	::LineTo(hdc, x + w, y + h);
	//left
	::MoveToEx(hdc, x + w, y + h, 0);
	::LineTo(hdc, x, y + h);
	//up
	::MoveToEx(hdc, x, y + h, 0);
	::LineTo(hdc, x, y);

}

void EmoPainter::drawCircle(HDC hdc, int x, int y, int r){

	::Ellipse(hdc, x - r, y - r, x + r, y + r);

}

void EmoPainter::paintVAGrid(HDC hdc, RECT rect){

	//paint variables
	int x1, x2, y1, y2 = 0;

	// get dimension of the canvas
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;

	//area
	HGDIOBJ old = ::SelectObject(hdc, axisPen);
	::SelectObject(hdc, backBrushBlack);
	::SelectObject(hdc, axisPen);
	::Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);
	::Ellipse(hdc, rect.left, rect.top, rect.right, rect.bottom);

	TextOut(hdc, rect.left, rect.top, "Valence Arousal Wheel", GetTextSize("Valence Arousal Wheel"));

	//valence
	x1 = rect.left;
	y1 = rect.top + height / 2;
	x2 = rect.left + width;
	y2 = rect.top + height / 2;
	drawLine(hdc, x1, y1, x2, y2);
	TextOut(hdc, x1, y1, "Valence", GetTextSize("Valence"));
	
	//arousal
	x1 = rect.left + width / 2;
	y1 = rect.top;
	x2 = rect.left + width / 2;
	y2 = rect.top + height;
	drawLine(hdc, x1, y1, x2, y2);
	TextOut(hdc, x1, y1, "Arousal", GetTextSize("Arousal"));
	

	// switch to old pen
	::SelectObject(hdc,old);

}

void EmoPainter::paintBlackBox(HDC hdc, RECT rect) {

	// set pen
	HGDIOBJ old = ::SelectObject(hdc, axisPen);

	//paint
	//area
	::SelectObject(hdc, backBrushBlack);
	::SelectObject(hdc, axisPen);
	::Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);

	// switch to old pen
	::SelectObject(hdc, old);

}

void EmoPainter::paintWhiteBox(HDC hdc, RECT rect) {

	// set pen
	HGDIOBJ old = ::SelectObject(hdc, axisPen);

	//paint
	//area
	::SelectObject(hdc, backBrushWhite);
	::SelectObject(hdc, axisPen);
	::Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);

	// switch to old pen
	::SelectObject(hdc, old);

}

void EmoPainter::paintEmotion(HDC hdc, RECT rect){

	ssi_char_t string[SSI_MAX_CHAR];

	// get dimension of the canvas
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;

	int x_base = (int)((rect.left + width / 2) + 0.9f * _emotion_point_base[0] * (width / 2));
	int y_base = (int)((rect.top + height / 2) - 0.9f * _emotion_point_base[1] * (height / 2));

	int x_final = (int) ((rect.left + width / 2) + 0.9f * _emotion_point[0] * (width / 2));
	int y_final = (int) ((rect.top + height / 2) - 0.9f * _emotion_point[1] * (height / 2));

	HGDIOBJ old = ::SelectObject(hdc, emoPenRed);
	::SelectObject(hdc, emoBrushRed);
	drawCircle(hdc, x_base, y_base, width / 50);
	::SelectObject(hdc, emoPenGreen);
	::SelectObject(hdc, emoBrushGreen);
	drawCircle(hdc, x_final, y_final, width / 25);
	::SelectObject(hdc, backBrushBlack);
	int charSize = 6;
	ssi_sprint(string, "[%.2f, %.2f]", _emotion_point[0], _emotion_point[1]);
	TextOut(hdc, x_final - GetTextSize(string) * charSize / 2 , y_final, string, GetTextSize(string));

	// drawLine(hdc, rect.left + width / 2, rect.top + height / 2, x, y);
	::SelectObject(hdc, emoPenRed);
	drawLine(hdc, rect.left + width / 2, rect.top + height / 2, x_base, y_base);
	::SelectObject(hdc, emoPenGreen);
	drawLine(hdc, x_base, y_base, x_final, y_final);
	
	// switch to old pen
	::SelectObject(hdc,old);

}

void EmoPainter::printInfo(HDC hdc, RECT rect) {

	ssi_char_t string[SSI_MAX_CHAR];
	int charSize = 6;
	int lineSkip = 18;
	int lineCount = 0;

	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;

	int x = (int)(rect.left + 3 * charSize);
	int y = (int)(rect.top + lineSkip);

	TextOut(hdc, rect.left, rect.top, "InfoBox", GetTextSize("InfoBox"));

	lineCount++;

	ssi_sprint(string, "Personality and Baseline Emotion");
	TextOut(hdc, rect.left, y + (lineCount * lineSkip), string, GetTextSize(string));
	lineCount++;

	lineCount++;

	/* openness to experience	(cautious vs. curious)
	conscientiousness			(careless vs. organized)
	extraversion				(reserved vs. energetic)
	agreeableness				(challenging vs. friendly)
	neuroticism					(confident vs. nervous) */

	if (_ocean[0] > 0.5f) {
		ssi_sprint(string, "generally, I'm very curious (openness [%.1f])", _ocean[0]);
	}
	else if (_ocean[0] >= 0.0f) {
		ssi_sprint(string, "generally, I'm curious (openness [%.1f])", _ocean[0]);
	}
	else if (_ocean[0] > -0.5f) {
		ssi_sprint(string, "generally, I'm cautious (openness [%.1f])", _ocean[0]);
	}
	else {
		ssi_sprint(string, "generally, I'm very cautious (openness [%.1f])", _ocean[0]);
	}
	TextOut(hdc, x, y + (lineCount * lineSkip), string, GetTextSize(string));
	lineCount++;

	if (_ocean[1] > 0.5f) {
		ssi_sprint(string, "generally, I'm very organized (conscientiousness [%.1f])", _ocean[1]);
	}
	else if (_ocean[1] >= 0.0f) {
		ssi_sprint(string, "generally, I'm organized (conscientiousness [%.1f])", _ocean[1]);
	}
	else if (_ocean[1] > -0.5f) {
		ssi_sprint(string, "generally, I'm careless (conscientiousness [%.1f])", _ocean[1]);
	}
	else {
		ssi_sprint(string, "generally, I'm very careless (conscientiousness [%.1f])", _ocean[1]);
	}
	TextOut(hdc, x, y + (lineCount * lineSkip), string, GetTextSize(string));
	lineCount++;

	if (_ocean[2] > 0.5f) {
		ssi_sprint(string, "generally, I'm very energetic (extraversion [%.1f])", _ocean[2]);
	}
	else if (_ocean[2] >= 0.0f) {
		ssi_sprint(string, "generally, I'm energetic (extraversion [%.1f])", _ocean[2]);
	}
	else if (_ocean[2] > -0.5f) {
		ssi_sprint(string, "generally, I'm reserved (extraversion [%.1f])", _ocean[2]);
	}
	else {
		ssi_sprint(string, "generally, I'm very reserved (extraversion [%.1f])", _ocean[2]);
	}
	TextOut(hdc, x, y + (lineCount * lineSkip), string, GetTextSize(string));
	lineCount++;

	if (_ocean[3] > 0.5f) {
		ssi_sprint(string, "generally, I'm very friendly (agreeableness [%.1f])", _ocean[3]);
	}
	else if (_ocean[3] >= 0.0f) {
		ssi_sprint(string, "generally, I'm friendly (agreeableness [%.1f])", _ocean[3]);
	}
	else if (_ocean[3] > -0.5f) {
		ssi_sprint(string, "generally, I'm challenging (agreeableness [%.1f])", _ocean[3]);
	}
	else {
		ssi_sprint(string, "generally, I'm very challenging (agreeableness [%.1f])", _ocean[3]);
	}
	TextOut(hdc, x, y + (lineCount * lineSkip), string, GetTextSize(string));
	lineCount++;

	if (_ocean[4] > 0.5f) {
		ssi_sprint(string, "generally, I'm very nervous (neuroticism [%.1f])", _ocean[4]);
	}
	else if (_ocean[4] >= 0.0f) {
		ssi_sprint(string, "generally, I'm nervous (neuroticism [%.1f])", _ocean[4]);
	}
	else if (_ocean[4] > -0.5f) {
		ssi_sprint(string, "generally, I'm confident (neuroticism [%.1f])", _ocean[4]);
	}
	else {
		ssi_sprint(string, "generally, I'm very confident (neuroticism [%.1f])", _ocean[4]);
	}
	TextOut(hdc, x, y + (lineCount * lineSkip), string, GetTextSize(string));
	lineCount++;
	
	ssi_sprint(string, "consequently, my baseline valence is [%.2f]", _emotion_point_base[0]);
	TextOut(hdc, x, y + (lineCount * lineSkip), string, GetTextSize(string));
	lineCount++;

	ssi_sprint(string, "consequently, my baseline arousal is [%.2f]", _emotion_point_base[1]);
	TextOut(hdc, x, y + (lineCount * lineSkip), string, GetTextSize(string));
	lineCount++;

	ssi_sprint(string, "this makes my baseline emotion [%s]", _emotion_label_base);
	TextOut(hdc, x, y + (lineCount * lineSkip), string, GetTextSize(string));
	lineCount++;

	lineCount++;

	ssi_sprint(string, "User Emotion (current) and Empathy");
	TextOut(hdc, rect.left, y + (lineCount * lineSkip), string, GetTextSize(string));
	lineCount++;

	lineCount++;

	ssi_sprint(string, "I think your valence is [%.2f]", _emotion_point_user[0]);
	TextOut(hdc, x, y + (lineCount * lineSkip), string, GetTextSize(string));
	lineCount++;

	ssi_sprint(string, "I think your arousal is [%.2f]", _emotion_point_user[1]);
	TextOut(hdc, x, y + (lineCount * lineSkip), string, GetTextSize(string));
	lineCount++;

	ssi_sprint(string, "I think you feel [%s]", _emotion_label_user);
	TextOut(hdc, x, y + (lineCount * lineSkip), string, GetTextSize(string));
	lineCount++;

	ssi_sprint(string, "my empathy for your valence is [%.2f]", _empathy_factor_valence);
	TextOut(hdc, x, y + (lineCount * lineSkip), string, GetTextSize(string));
	lineCount++;

	ssi_sprint(string, "my empathy for your arousal is [%.2f]", _empathy_factor_arousal);
	TextOut(hdc, x, y + (lineCount * lineSkip), string, GetTextSize(string));
	lineCount++;

	lineCount++;

	ssi_sprint(string, "Feedback and Habituation (current)");
	TextOut(hdc, rect.left, y + (lineCount * lineSkip), string, GetTextSize(string));
	lineCount++;
	
	lineCount++;

	if (_habituation_pos > 0.5f) {
		ssi_sprint(string, "my habituation to positive feedback is high [%.2f]", _habituation_pos);
	}
	else if (_habituation_pos > 0.0f) {
		ssi_sprint(string, "my habituation to positive feedback is low [%.2f]", _habituation_pos);
	}
	else {
		ssi_sprint(string, "my habituation to positive feedback is very low [%.2f]", _habituation_pos);
	}

	TextOut(hdc, x, y + (lineCount * lineSkip), string, GetTextSize(string));
	lineCount++;

	if (_habituation_neg > 0.5f) {
		ssi_sprint(string, "my habituation to negative feedback is high [%.2f]", _habituation_neg);
	}
	else if (_habituation_neg > 0.0f) {
		ssi_sprint(string, "my habituation to negative feedback is low [%.2f]", _habituation_pos);
	}
	else {
		ssi_sprint(string, "my habituation to negative feedback is very low [%.2f]", _habituation_neg);
	}

	TextOut(hdc, x, y + (lineCount * lineSkip), string, GetTextSize(string));
	lineCount++;
	
	lineCount++;

	ssi_sprint(string, "Resulting Emotional State (current)");
	TextOut(hdc, rect.left, y + (lineCount * lineSkip), string, GetTextSize(string));
	lineCount++;

	lineCount++;

	ssi_sprint(string, "in all, this makes my valence [%.2f]", _emotion_point[0]);
	TextOut(hdc, x, y + (lineCount * lineSkip), string, GetTextSize(string));
	lineCount++;

	ssi_sprint(string, "in all, this makes my arousal [%.2f]", _emotion_point[1]);
	TextOut(hdc, x, y + (lineCount * lineSkip), string, GetTextSize(string));
	lineCount++;

	ssi_sprint(string, "in all, my current emotion is [%s]", _emotion_label);
	TextOut(hdc, x, y + (lineCount * lineSkip), string, GetTextSize(string));
	lineCount++;
}

void EmoPainter::paintBars(HDC hdc, RECT rect, int nBars) {

	TextOut(hdc, rect.left, rect.top, "Habituation", GetTextSize("Habituation"));

	ssi_real_t* habituation_vector = new ssi_real_t[2];
	habituation_vector[0] = _habituation_pos;
	habituation_vector[1] = _habituation_neg;

	//paint variables
	int x1, x2, y1, y2, w, h = 0;

	// get dimension of the canvas
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;

	int p_perdim = (int)width / nBars;

	// set pen
	HGDIOBJ old = ::SelectObject(hdc, axisPen);

	//paint
	for (int ndim = 0; ndim < nBars; ndim++) {

		//draw rect
		x1 = rect.left + (int)((p_perdim * ndim) + (p_perdim / 3));
		y1 = rect.top + (int)(height / 2 - height / 4);
		w = (int)(p_perdim / 3);
		h = (int)(height / 2 + 1);
		drawRect(hdc, x1, y1, w, h);

		if (ndim == 0) {
			TextOut(hdc, x1, y1 + h, "Positive", GetTextSize("Positive"));
		}
		else {
			TextOut(hdc, x1, y1 + h, "Negative", GetTextSize("Negative"));
		}

		//draw thresh
		x1 = rect.left;
		x2 = rect.right;
		y1 = y2 = y1 = rect.top + int(height / 2 - (0.5f * (height / 4)) + 1);
		drawLine(hdc, x1, y1, x2, y2);

		//line from start to 1/3
		x1 = rect.left + (int)p_perdim * ndim;
		x2 = rect.left + (int)((p_perdim * ndim) + (p_perdim / 3));
		y1 = y2 = rect.top + (int)(height / 2);
		drawLine(hdc, x1, y1, x2, y2);

		//line from 2/3 to end
		x1 = rect.left + (int)((p_perdim * ndim) + (p_perdim / 3) * 2);
		x2 = rect.left + (int)(p_perdim * (ndim + 1));
		y1 = y2 = rect.top + (int)(height / 2);
		drawLine(hdc, x1, y1, x2, y2);

		if (habituation_vector[ndim] > 0) {
			x1 = rect.left + int(p_perdim * ndim + p_perdim / 3 + 1);
			y1 = rect.top + int(height / 2 - (habituation_vector[ndim] * (height / 4)) + 1);
			w = int(p_perdim / 3 - 2);
			h = int(habituation_vector[ndim] * (height / 4) + 1);
			drawRect(hdc, x1, y1, w, h);

			RECT frect;
			frect.left = x1;
			frect.top = y1;
			frect.right = x1 + w;
			frect.bottom = y1 + h;
			if (habituation_vector[ndim] > 0.5f) {
				::FillRect(hdc, &frect, emoBrushGreen);
			}
			else {
				::FillRect(hdc, &frect, emoBrushRed);
			}
			

		}
		else {
			x1 = rect.left + int(p_perdim * ndim + p_perdim / 3 + 1);
			y1 = rect.top + int(height / 2);
			w = int(p_perdim / 3 - 2);
			h = int(abs(habituation_vector[ndim]) * (height / 4));
			drawRect(hdc, x1, y1, w, h);

			RECT frect;
			frect.left = x1;
			frect.top = y1;
			frect.right = x1 + w;
			frect.bottom = y1 + h;
			::FillRect(hdc, &frect, emoBrushRed);
		}

	}

	// switch to old pen
	::SelectObject(hdc, old);

	//clean up
	delete[] habituation_vector;
	habituation_vector = 0;

}

};
