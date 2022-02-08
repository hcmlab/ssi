// Shimmer3PPGToHR.cpp
// author: Fabian Wildgrube
// created: 2021/07/24
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

#include "Shimmer3PPGToHR.h"

#ifdef USE_SSI_LEAK_DETECTOR
#include "SSI_LeakWatcher.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

#include <ShimmerClosedLibraryAlgoFactory.h>

namespace ssi {

	Shimmer3PPGToHR::Shimmer3PPGToHR(const ssi_char_t* file)
		: _sessionTimestamp(0),
		_msPerSample(1),
		_file(0) {

		if (file) {
			if (!OptionList::LoadXML(file, &_options)) {
				OptionList::SaveXML(file, &_options);
			}
			_file = ssi_strcpy(file);
		}
	}

	Shimmer3PPGToHR::~Shimmer3PPGToHR() {

		if (_file) {
			OptionList::SaveXML(_file, &_options);
			delete[] _file;
		}
	}

	void Shimmer3PPGToHR::transform_enter(ssi_stream_t& stream_in,
		ssi_stream_t& stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {

		_msPerSample = 1000.0 / stream_in.sr;
		_sessionTimestamp = 0.0;

		_ppgToHRAlgorithm = ShimmerClosedLibraryAlgoFactory::createPPGToHRAlgoInstance(stream_in.sr);
	}

	void Shimmer3PPGToHR::transform(ITransformer::info info,
		ssi_stream_t& stream_in,
		ssi_stream_t& stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {

		ssi_real_t* srcptr = ssi_pcast(ssi_real_t, stream_in.ptr);
		ssi_real_t* dstptr = ssi_pcast(ssi_real_t, stream_out.ptr);

		_sessionTimestamp += _msPerSample;
		float ppgValue = *srcptr;

		auto hr = _ppgToHRAlgorithm->ppgToHrConversion(ppgValue, _sessionTimestamp);

		*dstptr = static_cast<float>(hr);
	}

	void Shimmer3PPGToHR::transform_flush(ssi_stream_t& stream_in,
		ssi_stream_t& stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {
	}


}
