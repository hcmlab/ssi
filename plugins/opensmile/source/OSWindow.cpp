// OSWindow.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/09/21 
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

// based on code of openSMILE 1.0.1
// http://opensmile.sourceforge.net/

/*F******************************************************************************
 *
 * openSMILE - open Speech and Music Interpretation by Large-space Extraction
 *       the open-source Munich Audio Feature Extraction Toolkit
 * Copyright (C) 2008-2009  Florian Eyben, Martin Woellmer, Bjoern Schuller
 *
 *
 * Institute for Human-Machine Communication
 * Technische Universitaet Muenchen (TUM)
 * D-80333 Munich, Germany
 *
 *
 * If you use openSMILE or any code from openSMILE in your research work,
 * you are kindly asked to acknowledge the use of openSMILE in your publications.
 * See the file CITING.txt for details.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 ******************************************************************************E*/

#include "OSWindow.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

OSWindow::OSWindow (const ssi_char_t *file)
	: _file (0),
	frameSizeFrames (0),
	win (0) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
}

OSWindow::~OSWindow () {

	release ();

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

void OSWindow::release () {

	frameSizeFrames = 0;
	delete[] win;
	win = 0;
}

bool OSWindow::apply (ssi_size_t num, ssi_size_t dim, ssi_real_t *ptr) {

	if (!compute (num)) {
		ssi_wrn ("could not apply window function");	
		return false;
	}

	double *w = win;

	for (ssi_size_t i = 0; i < num; i++) {
		for (ssi_size_t j = 0; j < dim; j++) {			
			*ptr = ssi_cast (ssi_real_t, *ptr * *w + offset);
			ptr++;
		}
		w++;
	}

	return true;
}

const double *OSWindow::compute (long size) {

	if (frameSizeFrames == size) {
		return win;
	}

	release ();

	frameSizeFrames = size;
	winFunc = ssi_cast (int, _options.type);
	offset = _options.offset;
	gain = _options.gain;
	sigma = _options.sigma;
	alpha = _options.alpha;
	alpha0 = _options.alphas[0];
	alpha1 = _options.alphas[1];
	alpha2 = _options.alphas[2];
	alpha3 = _options.alphas[3];
	squareRoot = _options.sqrt ? 1 : 0;

	// following code taken from openSMILE 1.0.1, windower.cpp
	// http://opensmile.sourceforge.net/

	long i;

	switch (winFunc) {
		case RECTANGULAR:	win = smileDsp_winRec(frameSizeFrames); break;
		case HANNING:		win = smileDsp_winHan(frameSizeFrames); break;
		case HAMMING:		win = smileDsp_winHam(frameSizeFrames); break;
		case TRIANGULAR:	win = smileDsp_winTri(frameSizeFrames); break;
		case BARTLETT:		win = smileDsp_winBar(frameSizeFrames); break;
		case SINE:			win = smileDsp_winSin(frameSizeFrames); break;
		case GAUSS:			win = smileDsp_winGau(frameSizeFrames,sigma); break;
		case BLACKMANN:		win = smileDsp_winBla(frameSizeFrames,alpha0,alpha1,alpha2); break;
		case BLACKHARR:		win = smileDsp_winBlH(frameSizeFrames,alpha0,alpha1,alpha2,alpha3); break;
		case BARTHANN:		win = smileDsp_winBaH(frameSizeFrames,alpha0,alpha1,alpha2); break;
		case LANCZOS:		win = smileDsp_winLac(frameSizeFrames); break;
		default: ssi_wrn ("unknown window function ID (%i)", winFunc); win=NULL;
	}
	
	if ((win != NULL)&&(squareRoot)) {
		for (i=0; i<frameSizeFrames; i++) {
			if (win[i] >= 0.0) win[i] = sqrt(win[i]);
			else {
				ssi_wrn ("window function '%d' apparently has negative values (%f) (bug?), taking the square root of this function is not possible, please correct your config! (at current, the square root of all non-negative values is computed and negative values are converted to zeros)", winFunc, win[i]);
				win[i] = 0.0;
			}
		}
	}
	
	if ((win != NULL)&&(gain!=1.0)) { // apply gain
		for (i=0; i<frameSizeFrames; i++) {
			win[i] *= gain;
		}
	}

	return win;
}


}

