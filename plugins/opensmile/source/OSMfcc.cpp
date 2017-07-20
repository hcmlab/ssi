// OSMfcc.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/09/22
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

#include "OSMfcc.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

OSMfcc::OSMfcc (const ssi_char_t *file)
	: _file (0),
	_costable (0),
	_sintable (0) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
}

OSMfcc::~OSMfcc () {

	releaseTables ();

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

void OSMfcc::releaseTables () {
	
	delete[] _costable; _costable = 0;
	delete[] _sintable; _sintable = 0;
}

int OSMfcc::initTables (long blocksize)
{

	releaseTables ();

	// following code taken from openSMILE 1.0.1, mfcc.cpp
	// http://opensmile.sourceforge.net/

	int i,m;

	_costable = new FLOAT_DMEM[blocksize*nMfcc];	
	double fnM = (double)(blocksize);
	for (i=firstMfcc; i <= lastMfcc; i++) {
		double fi = (double)i;
		for (m=0; m<blocksize; m++) {
			_costable[m+(i-firstMfcc)*blocksize] = (FLOAT_DMEM)cos((double)M_PI*(fi/fnM) * ( (double)(m) + (double)0.5) );
		}
	}

	_sintable = new FLOAT_DMEM[nMfcc];
	if (cepLifter > 0.0) {
		for (i=firstMfcc; i <= lastMfcc; i++) {
			_sintable[i-firstMfcc] = ((FLOAT_DMEM)1.0 + cepLifter/(FLOAT_DMEM)2.0 * sin((FLOAT_DMEM)M_PI*((FLOAT_DMEM)(i))/cepLifter));
		}
	} else {
		for (i=firstMfcc; i <= lastMfcc; i++) {
			_sintable[i-firstMfcc] = 1.0;
		}
	}

	return 1;
}


void OSMfcc::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	lastMfcc = _options.last;
	firstMfcc = _options.first;
	nMfcc = lastMfcc - firstMfcc + 1;
	melfloor = _options.melFloor;
	cepLifter = _options.cepLifter;
	htkcompatible = _options.htk;
	if (htkcompatible) {
		ssi_msg (SSI_LOG_LEVEL_DETAIL, "HTK compatible output is enabled");
		melfloor = 1.0f;
	}
	
	initTables (stream_in.dim);
}

void OSMfcc::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	ssi_size_t num = stream_in.num;
	long Nsrc = stream_in.dim;
	long Ndst = stream_out.dim;
	ssi_real_t *src = ssi_pcast (ssi_real_t, stream_in.ptr);
	ssi_real_t *dst = ssi_pcast (ssi_real_t, stream_out.ptr);

	for (ssi_size_t j = 0; j < num; j++) { 

		ssi_real_t *src = ssi_pcast (ssi_real_t, stream_in.ptr) + j * Nsrc;
		ssi_real_t *dst = ssi_pcast (ssi_real_t, stream_out.ptr) + j * Ndst;

		// following code taken from openSMILE 1.0.1, mfcc.cpp
		// http://opensmile.sourceforge.net/

		int i,m;
  
		// compute log mel spectrum
		for (i=0; i<Nsrc; i++) {
			if (src[i] < melfloor) 
				src[i] = log (melfloor);
			else
				src[i] = (FLOAT_DMEM) log (src[i]);
		}
		
		// compute dct of mel data & do cepstral liftering:
		FLOAT_DMEM factor = (FLOAT_DMEM)sqrt((double)2.0/(double)(Nsrc));
		for (i=firstMfcc; i <= lastMfcc; i++) {
			int i0 = i-firstMfcc;
			FLOAT_DMEM * outc = dst+i0;  // = outp + (i-obj->firstMFCC);
			if (htkcompatible && (firstMfcc==0)) {
				if (i==lastMfcc) { i0 = 0; }
				else { i0 += 1; }
			}
			*outc = 0.0;
			for (m=0; m<Nsrc; m++) {
				*outc += src[m] * _costable[m + i0*Nsrc];
			}
			//*outc *= factor;   // use this line, if you want unliftered mfcc
			// do cepstral liftering:
			*outc *= _sintable[i0] * factor;
		}
	}
}

}
