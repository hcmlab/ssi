// OSEnergy.cpp
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

#include "OSEnergy.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

OSEnergy::OSEnergy (const ssi_char_t *file)
	: _file (0) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
}

OSEnergy::~OSEnergy () {

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}

void OSEnergy::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	switch (_options.type) {
		case RMS:
			elog = 0;
			erms = 1;
			break;
		case LOG:
			elog = 1;
			erms = 0;
			break;
		case BOTH:
			elog = 1;
			erms = 1;
			break;
	}
	ebiasRms = _options.biasRms;
	ebiasLog = _options.biasLog;
	escaleRms = _options.scaleRms;
	escaleLog = _options.scaleLog;	
	htkcompatible = _options.htk ? 1 : 0;	
}

void OSEnergy::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	long Nsrc = stream_in.num;
	ssi_real_t *src = ssi_pcast (ssi_real_t, stream_in.ptr);
	ssi_real_t *dst = ssi_pcast (ssi_real_t, stream_out.ptr);
	double minE = 8.674676e-019;

	// following code taken from openSMILE 1.0.1, energy.cpp
	// http://opensmile.sourceforge.net/

	long i;
	double d=0.0;
	for (i=0; i<Nsrc; i++) {
		FLOAT_DMEM tmp=*(src++);
		d += tmp*tmp;
	}

	int n=0;
	if (erms) {
		dst[n++] = (FLOAT_DMEM)sqrt(d/(FLOAT_DMEM)Nsrc) * escaleRms + ebiasRms;
	}

	//double minE = 8.674676e-019;
	float log_val = 0;

	if (elog) {
		if (!htkcompatible) {
			d /= (FLOAT_DMEM)Nsrc;
			if (d<minE) d = minE;
			dst[n++] = (FLOAT_DMEM)log(d) * escaleLog + ebiasLog;
		} else {
			d *= 32767.0*32767.0;
			if (d<=1.0) d = 1.0;
			dst[n++] = (FLOAT_DMEM)log(d) * escaleLog + ebiasLog;
		}
	}
}

}

