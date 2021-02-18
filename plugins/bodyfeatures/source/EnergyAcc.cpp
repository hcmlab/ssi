// KinectFeatures.cpp
// author: Tobias Baur <tobias.baur@informatik.uni-augsburg.de>
// created: 2012/08/10
// Copyright (C) 2007-12 University of Augsburg, Johannes Wagner
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute itand/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#include "EnergyAcc.h"
#include "signal/mathext.h"
#include <math.h>
#include "ssi.h"

#ifdef USE_SSI_LEAK_DETECTOR
#include "SSI_LeakWatcher.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

namespace ssi {
	EnergyAcc::EnergyAcc(const ssi_char_t *file)
		: _file(0), _meta_in(0) {
		if (file) {
			if (!OptionList::LoadXML(file, &_options)) {
				OptionList::SaveXML(file, &_options);
			}
			_file = ssi_strcpy(file);
			sprintf(_file_fft, "%s.fft", file);
		}

		_fft = ssi_create(FFTfeat, _file_fft, true);
	}

	EnergyAcc::~EnergyAcc() {
		if (_file) {
			OptionList::SaveXML(_file, &_options);
			delete[] _file;
		}
	}

	void EnergyAcc::transform_enter(
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[])
	{
		_fft->getOptions()->nfft = _options.nfft;

		//fft can be computed from 1 dimension only
		ssi_size_t dim = 1;
		ssi_stream_t stream_in_single;
		ssi_stream_select(stream_in, stream_in_single, 1, &dim);

		ssi_stream_t fft_out;
		ssi_stream_init(fft_out, 1, _fft->getSampleDimensionOut(stream_in_single.dim), _fft->getSampleBytesOut(stream_in_single.byte), _fft->getSampleTypeOut(stream_in_single.type), stream_in_single.sr);

		_fft->transform_enter(stream_in_single, fft_out);
	}

	void EnergyAcc::transform(ITransformer::info info,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[])
	{
		//concatenate streams
		ssi_stream_t** streams = new ssi_stream_t*[1 + xtra_stream_in_num];
		streams[0] = &stream_in;
		for (ssi_size_t i = 0; i < xtra_stream_in_num; ++i)
			streams[i + 1] = &xtra_stream_in[i];

		ssi_real_t *dst_ptr = ssi_pcast(ssi_real_t, stream_out.ptr);
		ssi_real_t result = (_options.join == JOIN::MULT) ? 1 : 0;

		//we will handle each dimension of each stream separately and then just sum up the results
		//we expect 3 accelation values (3 axis) from 1 or more sensors per stream
		for (ssi_size_t i = 0; i < 1 + xtra_stream_in_num; ++i) //streams loop
			for (ssi_size_t j = 0; j < streams[i]->dim; ++j) //dimensions loop
			{
				//select a dimension
				ssi_stream_t stream;
				ssi_stream_select(*streams[i], stream, 1, &j);

				//compute energy
				ssi_real_t e = energy(info, stream);

				switch (_options.join)
				{
				case JOIN::SUM:
					result += e;
					break;
				case JOIN::MULT:
					result *= e;
					break;
				case JOIN::OFF:
					*dst_ptr++ = e;
				}
			}

		if (_options.join != JOIN::OFF)
			*dst_ptr = result;

		delete[] streams;
	}

	ssi_real_t EnergyAcc::energy(ITransformer::info info, ssi_stream_t &stream)
	{
		ssi_stream_t fft_out;
		ssi_stream_init(fft_out, 1, _fft->getSampleDimensionOut(stream.dim), _fft->getSampleBytesOut(stream.byte), _fft->getSampleTypeOut(stream.type), stream.sr);

		//compute fft
		_fft->transform(info, stream, fft_out);

		//sum up squared fft components and normalize them
		ssi_real_t sum = 0;
		ssi_real_t *ffts = ssi_pcast(ssi_real_t, fft_out.ptr);
		for (ssi_size_t i = 0; i < fft_out.dim; ++i)
			sum += pow(ffts[i], 2);

		ssi_time_t winLen = (info.frame_num + info.delta_num);// / stream.sr;

		return sum / winLen;
	}

	void EnergyAcc::transform_flush(
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[])
	{
		//fft can be computed from 1 dimension only
		ssi_size_t dim = 1;
		ssi_stream_t stream_in_single;
		ssi_stream_select(stream_in, stream_in_single, 1, &dim);

		ssi_stream_t fft_out;
		ssi_stream_init(fft_out, 1, _fft->getSampleDimensionOut(stream_in_single.dim), _fft->getSampleBytesOut(stream_in_single.byte), _fft->getSampleTypeOut(stream_in_single.type), stream_in_single.sr);

		_fft->transform_flush(stream_in_single, fft_out);
	}
}