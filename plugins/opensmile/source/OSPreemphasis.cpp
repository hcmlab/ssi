// OSPreemphasis.cpp
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

#include "OSPreemphasis.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

OSPreemphasis::OSPreemphasis (const ssi_char_t *file)
	: _file (0),
	k (0.97f),
	de (false),
	first_call (true),
	hist (0) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
}

OSPreemphasis::~OSPreemphasis () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

void OSPreemphasis::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	// following code taken from openSMILE 1.0.1, preemphasis.cpp
	// http://opensmile.sourceforge.net/

	if (_options.f >= 0.0) {
		double _T = 1.0 / stream_in.sr;
		k = (FLOAT_DMEM) exp ( -2.0 * M_PI * _options.f * _T );
		ssi_msg (SSI_LOG_LEVEL_DETAIL, "computed k from f (%f Hz) : k = %f (sr = %f Hz)",_options.f,_options.k,1.0/_T);
	} else {
		k = _options.k;
	}

	first_call = true;
}

void OSPreemphasis::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	long Nsrc = stream_in.num;
	ssi_real_t *src = ssi_pcast (ssi_real_t, stream_in.ptr);
	ssi_real_t *dst = ssi_pcast (ssi_real_t, stream_out.ptr);

	if (first_call) {
		hist = *src;
		first_call = false;
	}
	
	// following code taken from openSMILE 1.0.1, energy.cpp
	// http://opensmile.sourceforge.net/

	long n;

	FLOAT_DMEM *x = src;
	FLOAT_DMEM *y = dst;

	if (de) {
		for (n=0; n<Nsrc; n++) {
			*(y++) = *(x) + k * hist;
			hist = *x++;
		}
	} else {
		for (n=0; n<Nsrc; n++) {
			*(y++) = *(x) - k * hist;
			hist = *x++;
		}
	}

}

}
