// FusionPainter.cpp
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
#include "../include/FusionPainter.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

	ssi_char_t *FusionPainter::ssi_log_name = "painter___";
	int FusionPainter::ssi_log_level = SSI_LOG_LEVEL_DEFAULT;

	//std::deque<EVector*> FusionPainter::EVectorList;

FusionPainter::FusionPainter(){
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
	_p_perdim = 0;

}

FusionPainter::~FusionPainter(){

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

void FusionPainter::create(ICanvas *parent)
{
	_parent = parent;
}

void FusionPainter::close()
{

}

void FusionPainter::createEventPen () {
	::DeleteObject (eventPen);
	eventPen = ::CreatePen (lineStyle, lineWidth, eventColor);
}

void FusionPainter::createAxisPen () {
	::DeleteObject (axisPen);
	axisPen = ::CreatePen (lineStyle, lineWidth, axisColor);
}

void FusionPainter::createMassPen () {
	::DeleteObject (massPen);
	massPen = ::CreatePen (lineStyle, lineWidth, massColor);
}

void FusionPainter::createFusionPen () {
	::DeleteObject (fusionPen);
	fusionPen = ::CreatePen (lineStyle, lineWidth, fusionColor);
}

void FusionPainter::createFusionBrush () {
	::DeleteObject (fusionBrush);
	fusionBrush = ::CreateSolidBrush (fusionColor);
}

void FusionPainter::createBaselinePen () {
	::DeleteObject (baselinePen);
	baselinePen = ::CreatePen (lineStyle, lineWidth, baselineColor);
}

void FusionPainter::createBackBrush () {
	::DeleteObject (backBrush);
	backBrush = ::CreateSolidBrush (backColor);
}

void FusionPainter::update_evectors(std::deque<EVector*> &VectorList){

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

void FusionPainter::update_fpoint(ssi_real_t* FusionPoint){

	if(_fusion_point){
		for(ssi_size_t i = 0; i < _dim; i++){
			_fusion_point[i] = FusionPoint[i];
		}
	}

}

void FusionPainter::update_fvector(EVector* FusionVector){

	if(_fvector){
		for(ssi_size_t i = 0; i < _dim; i++){
			_fvector[i] = FusionVector->get_value_decay()[i];
		}
	}

}

void FusionPainter::update_baseline(ssi_real_t *Baseline){

	if(_baseline){
		for(ssi_size_t i = 0; i < _dim; i++){
			_baseline[i] = Baseline[i];
		}
	}

}

void FusionPainter::setData(ssi_size_t Dimension, ssi_real_t *Baseline, ssi_real_t Threshold, std::deque<EVector*> &VectorList, ssi_real_t* FusionPoint, EVector* FusionVector, bool DecayingVectors, bool PaintEvents){

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

void FusionPainter::initAxisCaption(ssi_size_t dim){

	if(_axis_captions == 0){
		_axis_captions = new ssi_char_t*[dim];
		for(ssi_size_t i = 0; i < dim; i++){
			_axis_captions[i] = new ssi_char_t[SSI_MAX_CHAR];
		}
	}

}

void FusionPainter::setAxisCaption(ssi_size_t dim, ssi_char_t* caption){

	ssi_sprint(_axis_captions[dim], caption);

}

void FusionPainter::setWindowCaption (ssi_char_t *caption) {
	_window_caption = ssi_strcpy (caption);
}

void FusionPainter::paint(ssi_handle_t hdc, ssi_rect_t rect){

	Lock lock(_mutex);

	if(_dim != 0){

		RECT r;
		r.top = rect.top;
		r.left = rect.left;
		r.bottom = rect.top + rect.height;
		r.right = rect.left + rect.width;

		// paint it black
		::FillRect ((HDC) hdc, &r, backBrush);

		//paint the components
		paintFusion((HDC)hdc, r);
		paintGrid((HDC)hdc, r);
		if(_paint_events){
			paintEvents((HDC)hdc, r);
		}
		paintMass((HDC)hdc, r);
		paintBaseline((HDC)hdc, r);
		
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

void FusionPainter::drawLine(HDC hdc, int x1, int y1, int x2, int y2){

	::MoveToEx(hdc, x1, y1, 0);
	::LineTo(hdc, x2, y2);

}

void FusionPainter::drawRect(HDC hdc, int x, int y, int w, int h){

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

void FusionPainter::paintGrid(HDC hdc, RECT rect){

	TextOut(hdc, 0, 0, "Fusion", GetTextSize("Fusion"));

	//paint variables
	int x1, x2, y1, y2, w, h = 0;

	// get dimension of the canvas
	int width = rect.right;
	int height = rect.bottom;

	_p_perdim = (int) width / _dim;

	// set pen
	HGDIOBJ old = ::SelectObject (hdc, axisPen);

	//paint
	for(ssi_size_t ndim = 0; ndim < _dim; ndim++){

		//draw rect
		x1 = (int) ( (_p_perdim*ndim) + (_p_perdim/3) );
		y1 = (int) (height/2-height/4);
		w  = (int) (_p_perdim/3);
		h  = (int) (height/2 + 1);
		drawRect(hdc, x1, y1, w, h);

		TextOut(hdc, x1, y1+h, _axis_captions[ndim], GetTextSize( _axis_captions[ndim]));

		//threshold(s)
		/*x1 = ssi_size_t(_p_perdim*ndim);
		x2 = ssi_size_t(_p_perdim*(ndim+1));
		y1 = y2 = ssi_size_t(height/2 - _threshold*(height/4));
		drawLine(hdc, x1, y1, x2, y2);

		y1 = y2 = ssi_size_t(height/2 + _threshold*(height/4) + 1);
		drawLine(hdc, x1, y1, x2, y2);*/
		
		//line from start to 1/3
		x1 = (int) _p_perdim*ndim;
		x2 = (int) ( (_p_perdim * ndim) + (_p_perdim / 3) );
		y1 = y2 = (int) (height/2)/* - (int) ((h/2) * _baseline[ndim])*/;
		drawLine(hdc, x1, y1, x2, y2);

		//line from 2/3 to end
		x1 = (int) ( (_p_perdim * ndim) + (_p_perdim/3) * 2 );
		x2 = (int) ( _p_perdim * (ndim+1) );
		y1 = y2 = (int) (height/2)/* - (int) ((h/2) * _baseline[ndim])*/;
		drawLine(hdc, x1, y1, x2, y2);

	}

	// switch to old pen
	::SelectObject(hdc,old);

}

void FusionPainter::paintBaseline(HDC hdc, RECT rect){

	

	//paint variables
	int x1, x2, y1, y2 = 0;

	// get dimension of the canvas
	int width = rect.right;
	int height = rect.bottom;

	_p_perdim = (int) width / _dim;

	// set pen
	HGDIOBJ old = ::SelectObject (hdc, baselinePen);

	//paint
	for(ssi_size_t ndim = 0; ndim < _dim; ndim++){

		//line from start to 1/3
		x1 = (int) ( (_p_perdim * ndim) + (_p_perdim / 3));
		x2 = (int) ( (_p_perdim * ndim) + (_p_perdim/3) * 2 );
		y1 = y2 = (int) (height/2) - (int) ((height/4) * _baseline[ndim]);
		drawLine(hdc, x1, y1, x2, y2);

	}

	// switch to old pen
	::SelectObject(hdc,old);

}

void FusionPainter::paintEvents(HDC hdc, RECT rect){

	//paint variables
	int x1, x2, y1, y2, h = 0;

	// get dimension of the canvas
	int width = rect.right;
	int height = rect.bottom;

	_p_perdim = (int) width / _dim;

	// set pen
	HGDIOBJ old = ::SelectObject (hdc, eventPen);

	//vectors
	for(ssi_size_t ndim = 0; ndim < _dim; ndim++){

		for(ssi_size_t nvec = 0; nvec < EVectorList.size(); nvec++){
			ssi_real_t dim_val = EVectorList[nvec]->get_value_decay()[ndim];
			if(dim_val >= 0){
				x1 = (int) (_p_perdim * ndim + _p_perdim/4);
				y1 = (int) (height/2 - (dim_val*(height/4)) + 1);
				x2 = (int) (_p_perdim*(ndim+1) - _p_perdim/4);
				y2 = (int) (height/2 - (dim_val*(height/4)) + 1);
				drawLine(hdc, x1, y1,  x2, y2);
			}else{
				x1 = (int) (_p_perdim * ndim + _p_perdim/4);
				y1 = (int) (height/2 + (abs(dim_val)*(height/4)));
				x2 = (int) (_p_perdim*(ndim+1) - _p_perdim/4);
				y2 = (int) (height/2 + (abs(dim_val)*(height/4)));

				drawLine(hdc, x1, y1,  x2, y2);
			}
		}
	
	}//dim
	
	// switch to old pen
	::SelectObject(hdc,old);

}

void FusionPainter::paintMass(HDC hdc, RECT rect){

	//paint variables
	int x1, x2, y1, y2, h = 0;

	// get dimension of the canvas
	int width = rect.right;
	int height = rect.bottom;

	_p_perdim = (int) width / _dim;

	// set pen
	HGDIOBJ old = ::SelectObject (hdc, massPen);
	
	for(ssi_size_t ndim = 0; ndim < _dim; ndim++){

		//mass point
		ssi_real_t dim_val = _fusion_point[ndim];
		if(dim_val > 0){
			x1 = (int) (_p_perdim * ndim + _p_perdim/3);
			y1 = (int) (height/2 - (dim_val*(height/4)) + 1);
			x2 = (int) (_p_perdim*(ndim+1) - _p_perdim/3);
			y2 = (int) (height/2 - (dim_val*(height/4)) + 1);
			drawLine(hdc, x1, y1,  x2, y2);
		}else if(dim_val < 0){
			x1 = (int) (_p_perdim * ndim + _p_perdim/3);
			y1 = (int) (height/2 + (abs(dim_val)*(height/4)));
			x2 = (int) (_p_perdim*(ndim+1) - _p_perdim/3);
			y2 = (int) (height/2 + (abs(dim_val)*(height/4)));
			drawLine(hdc, x1, y1,  x2, y2);
		}

	}//dim

	// switch to old pen
	::SelectObject(hdc,old);

}

void FusionPainter::paintFusion(HDC hdc, RECT rect){

	//paint variables
	int x1, y1, w, h = 0;

	// get dimension of the canvas
	int width = rect.right;
	int height = rect.bottom;

	_p_perdim = (int) width / _dim;

	// set pen
	HGDIOBJ old = ::SelectObject (hdc, fusionPen);
	
	for(ssi_size_t ndim = 0; ndim < _dim; ndim++){

		//fusion
		if(_fvector[ndim] != 0){
			if(_fvector[ndim] > 0){
				x1 = ssi_size_t(_p_perdim * ndim + _p_perdim/3 + 1);
				y1 = ssi_size_t(height/2 - (_fvector[ndim]*(height/4)) + 1);
				w  = ssi_size_t(_p_perdim/3 - 2);
				/*h  = ssi_size_t(_fvector[ndim]*(height/4) + 1);*/
				h  = ssi_size_t(_fvector[ndim]*(height/4) + 1) - ssi_size_t(_baseline[ndim]*(height/4) + 1);
				drawRect(hdc, x1, y1, w, h);

				RECT frect;
				frect.left = x1;
				frect.top = y1;
				frect.right = x1 + w;
				frect.bottom = y1 + h;
				::FillRect(hdc, &frect, fusionBrush);
			
			}else{
				x1 = ssi_size_t(_p_perdim * ndim + _p_perdim/3 + 1);
				//y1 = ssi_size_t(height/2);
				y1 = ssi_size_t(height/2) - ssi_size_t(ssi_size_t(height/4)*_baseline[ndim]);
				w  = ssi_size_t(_p_perdim/3 - 2);
				/*h  = ssi_size_t(abs(_fvector[ndim])*(height/4));*/
				h  = ssi_size_t(abs(_fvector[ndim])*(height/4)) + ssi_size_t(ssi_size_t(height/4)*_baseline[ndim]);
				drawRect(hdc, x1, y1, w, h);

				RECT frect;
				frect.left = x1;
				frect.top = y1;
				frect.right = x1 + w;
				frect.bottom = y1 + h;
				::FillRect(hdc, &frect, fusionBrush);
			}
		}
	
	}//dim
	
	// switch to old pen
	::SelectObject(hdc,old);

}

};
