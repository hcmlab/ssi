// FusionPainterVA.cpp
// author: Florian Lingenfelser <florian.lingenfelser@informatik.uni-augsburg.de>
// created: 2015/12/01
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
#include "../include/FusionPainterVA.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

	ssi_char_t *FusionPainterVA::ssi_log_name = "painter___";
	int FusionPainterVA::ssi_log_level = SSI_LOG_LEVEL_DEFAULT;

	//std::deque<EVector*> FusionPainterVA::EVectorList;

FusionPainterVA::FusionPainterVA(){

	_parent = 0;
	_decaying_vectors = false;
	_paint_events = true;
	_dim = 0;
	_baseline = 0;
	_fvector = 0;
	_fusion_point = 0;
	_baseline = 0;
	_axis_captions = 0;
	_window_caption = 0;
	
	// create default pen and brush
	lineStyle = PS_SOLID;
	lineWidth = 1;
	pointSize = 5;
	eventColor = RGB (0, 255, 0);
	axisColor = RGB (255, 255, 255);
	massColor = RGB (255, 0, 255);
	fusionColor = RGB (255, 0, 0);
	baselineColor = RGB(0, 0, 255);
	backColor = RGB (0, 0, 0);
	createEventPen ();
	createAxisPen ();
	createMassPen();
	createFusionPen();
	createBaselinePen();
	createFusionBrush();
	createBackBrush ();

	_threshold = 0;

}

FusionPainterVA::~FusionPainterVA(){

	Lock lock(_mutex);

	if(_fvector){
		delete _fvector;
		_fvector = 0;
	}

	if(_fusion_point){
		delete _fusion_point;
		_fusion_point = 0;
	}

	if(_baseline){
		delete _baseline;
		_baseline = 0;
	}

	if(_axis_captions){
		for(ssi_size_t i = 0; i < _dim; i++){
			delete[] _axis_captions[i];
			_axis_captions[i] = 0;
		}
		delete[] _axis_captions;
		_axis_captions = 0;
	}

	delete[] _window_caption;
	_window_caption = 0;

	{
		for(ssi_size_t i = 0; i < EVectorList.size(); i++){
			delete EVectorList[i];
		}
		EVectorList.clear();
	}

}

void FusionPainterVA::create(ICanvas *parent)
{
	_parent = parent;
}

void FusionPainterVA::close()
{
}

void FusionPainterVA::createEventPen () {
	::DeleteObject (eventPen);
	eventPen = ::CreatePen (lineStyle, lineWidth, eventColor);
}

void FusionPainterVA::createAxisPen () {
	::DeleteObject (axisPen);
	axisPen = ::CreatePen (lineStyle, lineWidth, axisColor);
}

void FusionPainterVA::createMassPen () {
	::DeleteObject (massPen);
	massPen = ::CreatePen (lineStyle, lineWidth, massColor);
}

void FusionPainterVA::createFusionPen () {
	::DeleteObject (fusionPen);
	fusionPen = ::CreatePen (lineStyle, lineWidth, fusionColor);
}

void FusionPainterVA::createFusionBrush () {
	::DeleteObject (fusionBrush);
	fusionBrush = ::CreateSolidBrush (fusionColor);
}

void FusionPainterVA::createBaselinePen () {
	::DeleteObject (baselinePen);
	baselinePen = ::CreatePen (lineStyle, lineWidth, baselineColor);
}

void FusionPainterVA::createBackBrush () {
	::DeleteObject (backBrush);
	backBrush = ::CreateSolidBrush (backColor);
}

void FusionPainterVA::update_evectors(std::deque<EVector*> &VectorList){

	for(ssi_size_t i = 0; i < EVectorList.size(); i++){
		delete EVectorList[i];
	}
	EVectorList.clear();
	for(ssi_size_t i = 0; i < VectorList.size(); i++){
		if(!VectorList[i]) continue;
		EVector* vec = new EVector( _dim, VectorList[i]->get_weight(), VectorList[i]->get_speed(), VectorList[i]->get_type(), VectorList[i]->get_gradient(), VectorList[i]->get_time(), VectorList[i]->get_does_decay_weight());
		if(_decaying_vectors){
			vec->set_values_decay(_dim, VectorList[i]->get_value_decay());
		}else{
			vec->set_values_decay(_dim, VectorList[i]->get_value());
		}
		
		EVectorList.push_back(vec);
	}	

}

void FusionPainterVA::update_fpoint(ssi_real_t* FusionPoint){

	if(_fusion_point){
		for(ssi_size_t i = 0; i < _dim; i++){
			_fusion_point[i] = FusionPoint[i];
		}
	}

}

void FusionPainterVA::update_fvector(EVector* FusionVector){

	if(_fvector){
		for(ssi_size_t i = 0; i < _dim; i++){
			_fvector[i] = FusionVector->get_value_decay()[i];
		}
	}

}

void FusionPainterVA::update_baseline(ssi_real_t *Baseline){

	if(_baseline){
		for(ssi_size_t i = 0; i < _dim; i++){
			_baseline[i] = Baseline[i];
		}
	}

}

void FusionPainterVA::setData(ssi_size_t Dimension, ssi_real_t *Baseline, ssi_real_t Threshold, std::deque<EVector*> &VectorList, ssi_real_t* FusionPoint, EVector* FusionVector, bool DecayingVectors, bool PaintEvents){

	Lock lock(_mutex);

	_decaying_vectors = DecayingVectors;
	_paint_events = PaintEvents;
	_dim = Dimension;
	_threshold = Threshold;
	
	if(!_fvector && _dim != 0){
		_fvector = new ssi_real_t[_dim];
		for(ssi_size_t i = 0; i < _dim; i++){
			_fvector[i] = 0.0f;
		}
	}

	if(!_fusion_point && _dim != 0){
		_fusion_point = new ssi_real_t[_dim];
		for(ssi_size_t i = 0; i < _dim; i++){
			_fusion_point[i] = 0.0f;
		}
	}

	if(!_baseline && _dim != 0){
		_baseline = new ssi_real_t[_dim];
		for(ssi_size_t i = 0; i < _dim; i++){
			_baseline[i] = 0.0f;
		}
	}

	update_evectors(VectorList);
	update_fpoint(FusionPoint);
	update_fvector(FusionVector);
	update_baseline(Baseline);

}

void FusionPainterVA::setWindowCaption (ssi_char_t *caption) {
	_window_caption = ssi_strcpy (caption);
}

void FusionPainterVA::paint(ssi_handle_t hdc, ssi_rect_t rect){

	if(_dim != 0){

		Lock lock(_mutex);

		RECT r;
		r.top = rect.top;
		r.left = rect.left;
		r.bottom = rect.top + rect.height;
		r.right = rect.left + rect.width;

		// paint it black
		::FillRect ((HDC) hdc, &r, backBrush);

		//paint the components
		paintGrid((HDC)hdc, r);
		if(_paint_events){
			paintEvents((HDC)hdc, r);
		}
		paintMass((HDC)hdc, r);
		paintFusion((HDC)hdc, r);
		//paintBaseline((HDC)hdc, r);
		
	}
}

SSI_INLINE int GetTextSize (LPSTR a0)
{
    for (int iLoopCounter = 0; ;iLoopCounter++)
    {
        if (a0 [iLoopCounter] == '\0')
            return iLoopCounter;
    }
}

void FusionPainterVA::drawLine(HDC hdc, int x1, int y1, int x2, int y2){

	::MoveToEx(hdc, x1, y1, 0);
	::LineTo(hdc, x2, y2);

}

void FusionPainterVA::drawRect(HDC hdc, int x, int y, int w, int h){

	//origin
	::MoveToEx(hdc, x, y, 0);
	//right
	::LineTo(hdc, x+w, y);
	//down
	::MoveToEx(hdc, x+w, y, 0);
	::LineTo(hdc, x+w, y+h);
	//left
	::MoveToEx(hdc, x+w, y+h, 0);
	::LineTo(hdc, x, y+h);
	//up
	::MoveToEx(hdc, x, y+h, 0);
	::LineTo(hdc, x, y);

}

void FusionPainterVA::drawCircle(HDC hdc, int x, int y, int r){

	::Ellipse(hdc, x-r, y-r, x+r, y+r);

}

void FusionPainterVA::paintGrid(HDC hdc, RECT rect){

	//TextOut(hdc, 0, 0, "Valence Arousal Fusion", GetTextSize("Valence Arousal Fusion"));

	//paint variables
	int x1, x2, y1, y2, w, h = 0;

	// get dimension of the canvas
	int width = rect.right;
	int height = rect.bottom;

	// set pen
	HGDIOBJ old = ::SelectObject (hdc, axisPen);

	//paint
	//area
	::Rectangle(hdc, width/2 - width/4 - 20, height/2 - height/4 - 20, width/2 + width/4 + 20, height/2 + height/4 + 20);
	::SelectObject(hdc, backBrush);
	::Rectangle(hdc, width/2 - width/4 - 19, height/2 - height/4 - 19, width/2 + width/4 + 19, height/2 + height/4 + 19);
	::SelectObject(hdc, axisPen);

	//threshold
	::Rectangle(hdc, width/2 - width/4*_threshold, height/2 - height/4*_threshold, width/2 + width/4*_threshold, height/2 + height/4*_threshold);
	::SelectObject(hdc, backBrush);
	::Rectangle(hdc, width/2 - width/4*_threshold, height/2 - height/4*_threshold, width/2 + width/4*_threshold + 2, height/2 + height/4*_threshold + 2);
	::SelectObject(hdc, axisPen);

	//valence
	x1 = 0;
	y1 = height/2;
	x2 = width;
	y2 = height/2;
	drawLine(hdc, x1, y1, x2, y2);
	TextOut(hdc, x1, y1, "Valence", GetTextSize("Valence"));
	
	//arousal
	x1 = width/2;
	y1 = 0;
	x2 = width/2;
	y2 = height;
	drawLine(hdc, x1, y1, x2, y2);
	TextOut(hdc, x1, y1, "Arousal", GetTextSize("Arousal"));
	

	// switch to old pen
	::SelectObject(hdc,old);

}

void FusionPainterVA::paintBaseline(HDC hdc, RECT rect){


}

void FusionPainterVA::paintEvents(HDC hdc, RECT rect){

	//paint variables
	int x, y = 0;

	// get dimension of the canvas
	int width = rect.right;
	int height = rect.bottom;

	// set pen
	HGDIOBJ old = ::SelectObject (hdc, eventPen);

	//vectors
	for(ssi_size_t nvec = 0; nvec < EVectorList.size(); nvec++){
		Lock lock (_mutex);
		ssi_real_t dim_valence = EVectorList[nvec]->get_value_decay()[0];
		ssi_real_t dim_arousal = EVectorList[nvec]->get_value_decay()[1];
		ssi_real_t weight = EVectorList[nvec]->get_weight();
		
		x = (int) ((width/2) + dim_valence * (width/4));
		y = (int) ((height/2) - dim_arousal * (height/4));
		drawCircle(hdc, x, y, 10*weight);
	}
		
	// switch to old pen
	::SelectObject(hdc,old);

}

void FusionPainterVA::paintMass(HDC hdc, RECT rect){

	//paint variables
	int x, y = 0;

	// get dimension of the canvas
	int width = rect.right;
	int height = rect.bottom;

	// set pen
	HGDIOBJ old = ::SelectObject (hdc, massPen);

	ssi_real_t dim_valence = _fusion_point[0];
	ssi_real_t dim_arousal = _fusion_point[1];

	x = (int) ((width/2) + dim_valence * (width/4));
	y = (int) ((height/2) - dim_arousal * (height/4));
	drawCircle(hdc, x, y, 15);
	
	// switch to old pen
	::SelectObject(hdc,old);

}

void FusionPainterVA::paintFusion(HDC hdc, RECT rect){
	
	//paint variables
	int x, y = 0;

	// get dimension of the canvas
	int width = rect.right;
	int height = rect.bottom;

	// set pen
	HGDIOBJ old = ::SelectObject (hdc, fusionPen);

	ssi_real_t dim_valence = _fvector[0];
	ssi_real_t dim_arousal = _fvector[1];

	x = (int) ((width/2) + dim_valence * (width/4));
	y = (int) ((height/2) - dim_arousal * (height/4));
	drawCircle(hdc, x, y, 10);
	drawLine(hdc, width/2, height/2, x, y);
	
	// switch to old pen
	::SelectObject(hdc,old);

}

};
