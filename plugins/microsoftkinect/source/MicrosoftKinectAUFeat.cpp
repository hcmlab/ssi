// MicrosoftKinectAUFeat.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2013/10/21
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

#include "MicrosoftKinectAUFeat.h"
#include "base/Factory.h"
#include "../../signal/include/Functionals.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi
{

MicrosoftKinectAUFeat::MicrosoftKinectAUFeat (const ssi_char_t *file)
: _functionals (0), 
_file (0) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
}

MicrosoftKinectAUFeat::~MicrosoftKinectAUFeat () {

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}

void MicrosoftKinectAUFeat::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	ssi_stream_init (_valid_samples, 0, stream_in.dim, stream_in.byte, stream_in.type, stream_in.sr);

	Functionals *functionals = ssi_pcast (Functionals, Factory::Create (Functionals::GetCreateName (), 0, false));
	if (!functionals) {
		ssi_err ("%s not found, include 'ssisignal.dll'", Functionals::GetCreateName ());
	}
	ssi_strcpy (functionals->getOptions ()->names, "mean,energy,std,min,max,range");
	_functionals = functionals;

	_functionals->transform_enter (stream_in, stream_out);
}

void MicrosoftKinectAUFeat::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	ssi_size_t sample_dimension = stream_in.dim;
	ssi_size_t sample_number = stream_in.num;

	ssi_stream_adjust (_valid_samples, sample_number);

	ssi_real_t *srcptr = ssi_pcast (ssi_real_t, stream_in.ptr);
	ssi_real_t *dstptr = ssi_pcast (ssi_real_t, stream_out.ptr);
	ssi_real_t *tmpptr = ssi_pcast (ssi_real_t, _valid_samples.ptr);

	for (ssi_size_t i = 0; i < stream_out.dim; i++) {
		dstptr[i] = SSI_MICROSOFTKINECT_INVALID_ACTIONUNIT_VALUE;
	}

	ssi_size_t n_valid = 0;
	for (ssi_size_t i = 0; i < sample_number*sample_dimension; i+=sample_dimension) {
		if (srcptr[i] != SSI_MICROSOFTKINECT_INVALID_ACTIONUNIT_VALUE) {
			memcpy (tmpptr + (n_valid++ * sample_dimension), srcptr+i, sample_dimension * sizeof (ssi_real_t));
		}
	}

	if (n_valid > 0) {
		ssi_stream_adjust (_valid_samples, n_valid);
		_functionals->transform (info, _valid_samples, stream_out);
	}
}

void MicrosoftKinectAUFeat::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	ssi_stream_destroy (_valid_samples);

	_functionals->transform_flush (stream_in, stream_out);
	delete _functionals; _functionals = 0;
}

}
