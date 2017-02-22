// OSWindow.h
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

#pragma once

#ifndef SSI_OPENSMILE_WINDOW_H
#define SSI_OPENSMILE_WINDOW_H

#include "base/IObject.h"
#include "OSTools.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class OSWindow : public IObject {

public:

	enum TYPE {
		HANNING = 0,
		HAMMING,
		RECTANGULAR,
		SINE,
		GAUSS,
		TRIANGULAR,
		BARTLETT,
		LANCZOS,
		BARTHANN,
		BLACKMANN,
		BLACKHARR
	};

	#define WINF_HANNING    0
#define WINF_HAMMING    1
#define WINF_RECTANGLE  2
#define WINF_RECTANGULAR 2
#define WINF_SINE       3
#define WINF_COSINE     3
#define WINF_GAUSS      4
#define WINF_TRIANGULAR 5
#define WINF_TRIANGLE   5
#define WINF_BARTLETT   6
#define WINF_LANCZOS    7
#define WINF_BARTHANN   8
#define WINF_BLACKMAN   9
#define WINF_BLACKHARR  10

public:

	class Options : public OptionList {

	public:

		Options ()
			: type (HANNING), offset (0), gain (1.0), sigma (0.4), alpha (0.16), sqrt (false) {

			alphas[0] = 0;
			alphas[1] = 0;
			alphas[2] = 0;
			alphas[3] = 0;

			addOption ("type", &type, 1, SSI_INT, "0=Hann, 1=Hamming, 2=Rectangular, 3=Sine, 4=Gauss, 5=Triangular, 6=Bartlett, 7=Lanczos, 8=Bartlett-Hann 9=Blackmann, 10=Blackmann-Harris");		
			addOption ("offset", &offset, 1, SSI_DOUBLE, "This specifies an offset which will be added to the samples after multiplying with the window function.");		
			addOption ("gain", &gain, 1, SSI_DOUBLE, "This option allows you to specify a scaling factor by which window function (which is by default normalised to max. 1) should be multiplied by.");		
			addOption ("sigma", &sigma, 1, SSI_DOUBLE, "Standard deviation for the Gaussian window.");		
			addOption ("alpha", &alpha, 1, SSI_DOUBLE, "alpha for the Blackmann window.");		
			addOption ("sqrt", &sqrt, 1, SSI_BOOL, "use square root of 'winFunc' as actual window function (e.g. to get a root raised cosine window).");		
			addOption ("alphas", &alphas, 4, SSI_DOUBLE, "alphas for Blackmann(-Harris) / Bartlett-Hann windows (optional!).");		
		};

		TYPE   type;
		double offset, gain;
		double sigma, alpha;
		double alphas[4];
		bool   sqrt;
	};

public:

	static const ssi_char_t *GetCreateName () { return "OSWindow"; };
	static IObject *Create (const ssi_char_t *file) { return new OSWindow (file); };
	~OSWindow ();

	OSWindow::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "This component applies window functions."; };

	virtual bool apply (ssi_size_t num, ssi_size_t dim, ssi_real_t *ptr);
	virtual const double *compute (long size);
	virtual void release ();

protected:

	OSWindow (const ssi_char_t *file = 0);
	OSWindow::Options _options;
	ssi_char_t *_file;

	double offset, gain;
    double sigma, alpha, alpha0, alpha1, alpha2, alpha3;
    long frameSizeFrames;
    int winFunc;    // winFunc as numeric constant (see #defines above)
    int squareRoot;
    double *win;
};

}

#endif
