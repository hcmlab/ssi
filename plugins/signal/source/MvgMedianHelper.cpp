// MvgMedianHelper.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/12/12
// Copyright (C) 2007-10 University of Augsburg, Johannes Wagner
//
// *************************************************************************************************
//
// This file is part of Smart Sensor Integration (SSI) developed at the 
// Lab for Multimedia Concepts and Applications of the University of Augsburg
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

#include "MvgMedianHelper.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

MvgMedianHelper::MvgMedianHelper (ssi_size_t nwin) 
	: _mpos (0),
	_nwin (nwin),
	_ndim (0),
	_vals (0),
	_order (0),
	_pointer (0) {

	if (nwin < 3) {
		ssi_err ("window size is too small, '%u' < '3'", nwin);
	}

	_mpos = _nwin / 2;
	_pointer = _nwin-1;
	_vals = new ssi_real_t[_nwin];
	_order = new ssi_size_t[_nwin];
}

MvgMedianHelper::~MvgMedianHelper () {

	delete[] _vals; _vals = 0;
	delete[] _order; _order = 0;
}

void MvgMedianHelper::init (ssi_real_t x) {

	/*
	
	if nargin < 3 || isempty (hist)        
		hist.mpos = floor (n/2+1); % position of median
		hist._vals = ones (1, n) * x; % values : [smaller median larger]
		hist._order = 1:n; % holds indices of values in temporal _order
		hist.pointer = n; % points to oldest value in _order
	end

	mpos = hist.mpos;
	_vals = hist._vals;
	_order = hist._order;
	pointer = hist.pointer;

	*/

	for (ssi_size_t j = 0; j <_nwin; j++) {
		_vals[j] = x;
		_order[j] = j;
	}

}

ssi_real_t MvgMedianHelper::move (ssi_real_t x) {

	ssi_size_t xpos;
	ssi_real_t mval;
	ssi_size_t oldpos;
	ssi_real_t oldval;
	ssi_size_t candidatepos;
	ssi_real_t candidate;
	
	/*

	% get current median
	mval = _vals(mpos);

	% replace oldest value with new value
	oldpos = _order(pointer);
	oldval = _vals(oldpos);
	_vals(oldpos) = x;
	xpos = oldpos;

	*/
		
	mval = _vals[_mpos];
	oldpos = _order[_pointer];
	oldval = _vals[oldpos];
	_vals[oldpos] = x;
	xpos = oldpos;

	/*

	% if NOT both oldest and new value <= or => median OR 
	% oldest value is current median
	if oldpos == mpos || ~ (oldval <= mval && x <= mval && oldval >= mval && x >= mval)
		% 1. swap new value with median
		[_order, _vals] = swap (xpos, mpos, _order, _vals);
		xpos = mpos;
		% 2. swap new value with new median
		if x <= mval
			[candidate, candidatepos] = max (_vals(1:mpos-1));
			if candidate > x
				[_order, _vals] = swap (xpos, candidatepos, _order, _vals);            
			end
		else
			[candidate, candidatepos] = min (_vals(mpos+1:end));
			if candidate < x
				[_order, _vals] = swap (xpos, candidatepos+mpos, _order, _vals);            
			end
		end    
	end

	*/

	if (oldpos == _mpos || ! (oldval <= mval && x <= mval && oldval >= mval && x >= mval)) {
		swap (xpos, _mpos, _nwin, _order, _vals);
		xpos = _mpos;
		if (x <= mval) {
			candidatepos = findmax (_mpos, _vals);
			candidate = _vals[candidatepos];
			if (candidate > x) {
				swap (xpos, candidatepos, _nwin, _order, _vals);
			}
		} else {
			candidatepos = findmin (_nwin - (_mpos+1), _vals + (_mpos+1));
			candidate = _vals[(_mpos+1) + candidatepos];
			if (candidate < x) {
				swap (xpos, (_mpos+1) + candidatepos, _nwin, _order, _vals);
			}
		}
	}

	/*

	% pick medium
	y = _vals(mpos); 

	% move pointer
	pointer = mod (pointer, n) + 1;

	% update history
	hist._vals = _vals;
	hist._order = _order;
	hist.pointer = pointer;

	*/

	_pointer = (++_pointer) % _nwin;	
	
	return _vals[_mpos];

}

ssi_size_t MvgMedianHelper::findmax (ssi_size_t n,
	ssi_real_t *values) {

	ssi_size_t ind = 0;		
	for (ssi_size_t i = 1; i < n; i++) {
		if (values[i] > values[ind]) {
			ind = i;
		}
	}

	return ind;
}

ssi_size_t MvgMedianHelper::findmin (ssi_size_t n,
	ssi_real_t *values) {

	ssi_size_t ind = 0;		
	for (ssi_size_t i = 1; i < n; i++) {
		if (values[i] < values[ind]) {
			ind = i;
		}
	}

	return ind;
}

void MvgMedianHelper::swap (ssi_size_t xpos,
	ssi_size_t ypos,
	ssi_size_t n,
	ssi_size_t *_order, 
	ssi_real_t *values) {

	/*

	x = values(xpos);
	y = values(ypos);
	xvalpos = _order == xpos;
	yvalpos = _order == ypos;
	values(xpos) = y;
	values(ypos) = x;
	_order(xvalpos) = ypos;
	_order(yvalpos) = xpos;

	*/

	ssi_real_t x = values[xpos];
	ssi_real_t y = values[ypos];
	ssi_size_t xvalpos, yvalpos;
	for (ssi_size_t i = 0; i < n; i++) {
		if (_order[i] == xpos) {
			xvalpos = i;
		}
		if (_order[i] == ypos) {
			yvalpos = i;
		}
	}
	values[xpos] = y;
	values[ypos] = x;
	_order[xvalpos] = ypos;
	_order[yvalpos] = xpos;

}

}
