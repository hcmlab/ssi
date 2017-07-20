// OSFFTmagphase.cpp
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

#include "OSFFTmagphase.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

OSFFTmagphase::OSFFTmagphase (const ssi_char_t *file)
	: _file (0) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
}

OSFFTmagphase::~OSFFTmagphase () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

void OSFFTmagphase::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	switch (_options.type) {
		case MAGNITUDE:
			magnitude = 1;
			phase = 0;
			break;
		case PHASE:
			magnitude = 0;
			phase = 1;
			break;
		case BOTH:
			magnitude = 1;
			phase = 1;
			break;
	}
	
	power = _options.power || _options.dBpsd ? 1 : 0;
	normalise = _options.norm || _options.dBpsd ? 1 : 0;
	dBpsd = _options.dBpsd ? 1 : 0;
	dBpnorm = _options.dBpnorm;

}

void OSFFTmagphase::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	ssi_size_t num = stream_in.num;
	long Nsrc = stream_in.dim;
	long Ndst = stream_out.dim;

	static ssi_size_t count = 0;

	for (ssi_size_t i = 0; i < num; i++) { 

		count++;

		ssi_real_t *src = ssi_pcast (ssi_real_t, stream_in.ptr) + i * Nsrc;
		ssi_real_t *dst = ssi_pcast (ssi_real_t, stream_out.ptr) + i * Ndst;
		
		// following code taken from openSMILE 1.0.1, FFTmagphase.cpp
		// http://opensmile.sourceforge.net/

		long n;
		int p = phase ? 1 : 2;

		if (magnitude) {
			if (magnitude && !dBpsd && !normalise && !power) {
				dst[0] = fabs(src[0]);
				for(n=2; n<Nsrc; n += 2) {
					dst[n/p] = sqrt(src[n]*src[n] + src[n+1]*src[n+1]);
				}
				dst[Nsrc/p] = fabs(src[1]);
			} else if (magnitude && !dBpsd && normalise && !power) {
				dst[0] = (1.0f/(FLOAT_DMEM)Nsrc)*fabs(src[0]);
				for(n=2; n<Nsrc; n += 2) {
					dst[n/p] = (1.0f/(FLOAT_DMEM)Nsrc)*sqrt(src[n]*src[n] + src[n+1]*src[n+1]);
				}
				dst[Nsrc/p] = (1.0f/(FLOAT_DMEM)Nsrc)*fabs(src[1]);
			} else if (magnitude && !dBpsd && normalise && power) {
				dst[0] = (1.0f/(FLOAT_DMEM)Nsrc)*fabs(src[0]); dst[0] *= dst[0];
				for(n=2; n<Nsrc; n += 2) {
					dst[n/p] = (1.0f/((FLOAT_DMEM)Nsrc*(FLOAT_DMEM)Nsrc))*(src[n]*src[n] + src[n+1]*src[n+1]);
				}
				dst[Nsrc/p] = (1.0f/(FLOAT_DMEM)Nsrc)*fabs(src[1]);
				dst[Nsrc/p] *= dst[Nsrc/p];
			} else if (magnitude && !dBpsd && !normalise && power) {
				dst[0] = fabs(src[0]); dst[0] *= dst[0];
				for(n=2; n<Nsrc; n += 2) {
					dst[n/p] = (src[n]*src[n] + src[n+1]*src[n+1]);
				}
				dst[Nsrc/p] = fabs(src[1]);
				dst[Nsrc/p] *= dst[Nsrc/2];
			} else if (magnitude && dBpsd) {
				dst[0] = MAX(-10.0f, (dBpnorm + 20.0f * log10( (1.0f/(FLOAT_DMEM)Nsrc)*fabs(src[0]) )));
				for(n=2; n<Nsrc; n += 2) {
					dst[n/p] = MAX(-10.0f, (dBpnorm + 10.0f * log10( (1.0f/((FLOAT_DMEM)Nsrc*(FLOAT_DMEM)Nsrc))*(src[n]*src[n] + src[n+1]*src[n+1]) )) );
				}
				dst[Nsrc/p] = MAX(-10.0f, (dBpnorm + 20.0f * log10( (1.0f/(FLOAT_DMEM)Nsrc)*fabs(src[1]) )));
			}
		}
		if (phase) {
			if (magnitude) {
				dst[1] = 0.0;
				for(n=2; n<Nsrc; n+=2) {
					dst[n+1] = atan2(src[n+1],src[n]);
				}
			} else {
				dst[0] = 0.0;
				for(n=2; n<Nsrc; n+=2) {
					dst[n/2] = atan2(src[n+1],src[n]);
				}
			}
		}
	}
}

}
