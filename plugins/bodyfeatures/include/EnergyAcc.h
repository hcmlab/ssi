// KinectFeatures.h
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

/**

Calculates energy from acceleration data.
Window size should be to be power of 2 (for FFT transformation)

*/

#pragma once

#ifndef SSI_NOVA_ENERGYACC_H
#define SSI_NOVA_ENERGYACC_H

#include "base/IFeature.h"
#include "ioput/option/OptionList.h"
#include "SSI_SkeletonCons.h"
#include "..\..\signal\include\FFTfeat.h"

namespace ssi {
	class EnergyAcc : public IFeature {
	public:

		struct JOIN {
			enum List {
				OFF = 0,
				MULT = 1,
				SUM = 2
			};
		};

	public:

		class Options : public OptionList {
		public:

			Options() : join(JOIN::SUM), nfft(256), totDim(0)//, thresLow(100), thresMedium(500), thresHigh(2000)
			{
				addOption("join", &join, 1, SSI_SIZE, "join the energies from all dimensions (0=off, 1=multiply, 2=sum up)");
				addOption("totDim", &totDim, 1, SSI_SIZE, "total number of dimensions including extra streams (required if join == OFF)");
				addOption("nfft", &nfft, 1, SSI_SIZE, "nfft");
			};
			
			ssi_size_t nfft;
			JOIN::List join;
			ssi_size_t totDim;
			ssi_real_t thresLow, thresMedium, thresHigh;
		};

	public:

		static const ssi_char_t *GetCreateName() { return "EnergyAcc"; };
		static IObject *Create(const ssi_char_t *file) { return new EnergyAcc(file); };
		~EnergyAcc();

		EnergyAcc::Options *getOptions() { return &_options; };
		const ssi_char_t *getName() { return GetCreateName(); };
		const ssi_char_t *getInfo() { return "Calculates energy from acceleration data."; };

		void transform(ITransformer::info info,
			ssi_stream_t &stream_in,
			ssi_stream_t &stream_out,
			ssi_size_t xtra_stream_in_num = 0,
			ssi_stream_t xtra_stream_in[] = 0);
		void transform_enter(
			ssi_stream_t &stream_in,
			ssi_stream_t &stream_out,
			ssi_size_t xtra_stream_in_num = 0,
			ssi_stream_t xtra_stream_in[] = 0);
		void transform_flush(ssi_stream_t &stream_in,
			ssi_stream_t &stream_out,
			ssi_size_t xtra_stream_in_num = 0,
			ssi_stream_t xtra_stream_in[] = 0);

		ssi_size_t getSampleDimensionOut(ssi_size_t sample_dimension_in) {
			ssi_size_t dim = _options.totDim;
			if (_options.join == JOIN::OFF && _options.totDim == 0)
			{
				ssi_wrn("Unknwon number of input dimensions (totDim not set). Assuming only 1 input stream. ");
				dim = sample_dimension_in;
			}

			return (_options.join == JOIN::OFF) ? dim : 1;
		}
		ssi_size_t getSampleBytesOut(ssi_size_t sample_bytes_in) {
			return sample_bytes_in;
		}
		ssi_type_t getSampleTypeOut(ssi_type_t sample_type_in) {
			if (sample_type_in != SSI_REAL) {
				ssi_err("type %s not supported", SSI_TYPE_NAMES[sample_type_in]);
				return SSI_UNDEF;
			}
			return SSI_REAL;
		}

		void setMetaData(ssi_size_t size, const void *meta) {
			memcpy(&_meta_in, meta, size); //if there are multiple metas, we will overwrite them
		}

		FFTfeat* getOSTransformFFT()	{
			return _fft;
		}

	protected:

		EnergyAcc(const ssi_char_t *file = 0);

		//void removeGravity1 (ssi_stream_t &stream_in, ssi_stream_t &stream_proc);
		//ssi_real_t sqrtsumsq(ssi_real_t x, ssi_real_t y);

		ssi_real_t energy(ITransformer::info info, ssi_stream_t &stream);

		EnergyAcc::Options _options;
		ssi_byte_t* _meta_in;

		ssi_char_t *_file;
		ssi_char_t _file_fft[256];

		FFTfeat* _fft;
	};
}

#endif
