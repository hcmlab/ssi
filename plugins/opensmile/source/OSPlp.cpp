// OSPlp.cpp
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

#include "OSPlp.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

OSPlp::OSPlp (const ssi_char_t *file)
	: _file (0),
	_costable (0),
	_sintable (0),
	_acf(0),
	_ceps(0),
	_lpc(0){

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
}

OSPlp::~OSPlp () {

	releaseTables ();

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}

void OSPlp::releaseTables () {
	
	delete[] _costable; _costable = 0;
	delete[] _sintable; _sintable = 0;
	delete[] _acf; _acf = 0;
	delete[] _lpc; _lpc = 0;
	delete[] _ceps; _ceps = 0;
	
}

int OSPlp::initTables (long blocksize)
{

	releaseTables ();

	// following code taken from openSMILE 1.0.1, plp.cpp
	// http://opensmile.sourceforge.net/

	int i,m;
	
	
	nFreq = blocksize+2; // +DC + Nyquist...?
	if (doIDFT) {
		nAuto = lpOrder + 1;
		_costable = new FLOAT_DMEM [nAuto*nFreq];
		
		// memory for acf:
		_acf = new FLOAT_DMEM [nAuto];
		// IDFT costable:
		FLOAT_DMEM a = (FLOAT_DMEM) M_PI / (FLOAT_DMEM)(nFreq-1);
		for (i=0; i<nAuto; i++) {
			int ib = i*nFreq;
			_costable[ib] = 1.0;
			for (m=1; m<(nFreq-1); m++) {
				_costable[m+ib] = (FLOAT_DMEM)( 2.0 * cos(a * (double)i * (double)m ) );
			}
			_costable[m+ib] = (FLOAT_DMEM)( cos(a * (double)i * (double)m ) );
		}
	}

	// memory for lp coefficients
	if (_options.doLP) {
		_lpc = new FLOAT_DMEM[lpOrder];
	}

	// sintable for cepstral liftering
	if (_options.doLpToCeps) {
		_sintable = new FLOAT_DMEM[nCeps];
		
		if (cepLifter > 0.0) {
			for (i=firstCC; i <= lastCC; i++) {
				_sintable[i-firstCC] = ((FLOAT_DMEM)1.0 + cepLifter/(FLOAT_DMEM)2.0 * sin((FLOAT_DMEM)M_PI*((FLOAT_DMEM)(i))/cepLifter));
			}
		} else {
			for (i=firstCC; i <= lastCC; i++) {
				_sintable[i-firstCC] = 1.0;
			}
		}
		_ceps = new FLOAT_DMEM [nCeps];
	}

	// equal loudness curve:
	//not Implemented

	// rasta filter
	//not Implemented

	return 1;
}


void OSPlp::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	firstCC = _options.first;
	lastCC = _options.last;
	nCeps = lastCC - firstCC + 1;
	melfloor = _options.melFloor;
	cepLifter = _options.cepLifter;
	lpOrder = _options.lporder;
	htkcompatible = _options.htk;
	if (htkcompatible) {
		ssi_msg (SSI_LOG_LEVEL_DETAIL, "HTK compatible output is enabled");
		melfloor = 1.0f;
	}
	
	initTables (stream_in.dim);
}

void OSPlp::transform (ITransformer::info info,
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
  
		// compute log spectrum
		if (doLog) {
			for (i=0; i<Nsrc; i++) {
				if (src[i] < melfloor) src[i] = log(melfloor);
				else src[i] = (FLOAT_DMEM) log(src[i]);
				//if (!htkcompatible) _src[i] += 21;
			}
		}
		
		// inverse log
		if (doInvLog) {
			for (i=0; i<Nsrc; i++) {
				src[i] = exp(src[i]);
			}
		}

		if (doIDFT) {  // TODO: check nAuto<=nDst!
		for (i=0; i<nAuto; i++) {
			double tmp = 0;
			if (htkcompatible) { tmp = (double)_costable[i*nFreq] * (double)src[0]; }

			for (m=1; m<nFreq-1; m++) {
				tmp += (double)_costable[m+i*nFreq] * (double)src[m-1];
			}
			tmp += (double)_costable[m+i*nFreq] * (double)src[nFreq-3];
			_acf[i] = (FLOAT_DMEM)(tmp / (2.0*(nFreq-1)));
		}

		// linear prediction analysis on ACF
		FLOAT_DMEM lpGain=0.0;
		if (doLP) {
			smileDsp_calcLpcAcf(_acf, _lpc, lpOrder, &lpGain, NULL);

			// convert lp filter coefficients to cepstral representation
			if (doLpToCeps) {
				if (lpGain <= 0) {
					lpGain = (FLOAT_DMEM)1.0;
				}

				// lp to ceps
				FLOAT_DMEM *__ceps = _ceps;
				if (!htkcompatible && (firstCC==0)) __ceps++;
				FLOAT_DMEM zeroth = smileDsp_lpToCeps(_lpc,lpOrder,lpGain,__ceps,firstCC,lastCC);
				if (firstCC==0) {
					if (!htkcompatible) { _ceps[0] = zeroth; }
					else { _ceps[nCeps-1] = zeroth; }
				}

				// cepstral liftering:
				FLOAT_DMEM factor;
				if (cepLifter > 0.0) {
					factor = (FLOAT_DMEM)sqrt((double)2.0/(double)(Nsrc));
				}
				for (i=firstCC; i<=lastCC; i++) {
					int i0 = i-firstCC;
					int i1 = i0;
					if (htkcompatible && (firstCC==0)) {
						if (i==lastCC) { i1 = 0; }
						else { i1 += 1; }
					}
					if (cepLifter > 0.0) {
						dst[i0] = _ceps[i0] * _sintable[i1];
					} else {
						dst[i0] = _ceps[i0];// * _sintable[i1] * factor;
					}
				}

			} else { // save lp coeffs to dst[]
				for (i=0; i<lpOrder; i++) {
					dst[i] = _lpc[i];
				}
			}
		} else { // save data in acf[] to dst[]
			for (i=0; i<nAuto; i++) {
				dst[i] = _acf[i];
			}
		}
	} else { // no IDFT , save spectrum in _src[] to dst[]
		for (i=0; i<Nsrc; i++) {
			dst[i] = src[i];
		}
	}
			
		
	}
}

}
