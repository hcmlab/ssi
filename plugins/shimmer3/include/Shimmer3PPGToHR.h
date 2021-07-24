// Shimmer3PPGToHR.h
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

#pragma once

#ifndef SSI_SHIMMER3_PPGTOHR_H
#define SSI_SHIMMER3_PPGTOHR_H

#include "base/IFilter.h"
#include "signal/SignalCons.h"
#include "ioput/option/OptionList.h"

#include "IShimmerPPGToHRAlgorithm.h"

namespace ssi {

	class Shimmer3PPGToHR : public IFilter {

	public:

		class Options : public OptionList {

		public:
			Options(){};
		};

	public:

		static const ssi_char_t* GetCreateName() { return "Shimmer3PPGToHR"; };
		static IObject* Create(const ssi_char_t* file) { return new Shimmer3PPGToHR(file); };
		~Shimmer3PPGToHR();

		Options* getOptions() { return &_options; };
		const ssi_char_t* getName() { return GetCreateName(); };
		const ssi_char_t* getInfo() { return "Estimates Heart rate Shimmer3 ppg values. Expects input values to be bandpass filtered within 0.5-5Hz"; };

		void transform_enter(ssi_stream_t& stream_in,
			ssi_stream_t& stream_out,
			ssi_size_t xtra_stream_in_num = 0,
			ssi_stream_t xtra_stream_in[] = 0);
		void transform(ITransformer::info info,
			ssi_stream_t& stream_in,
			ssi_stream_t& stream_out,
			ssi_size_t xtra_stream_in_num = 0,
			ssi_stream_t xtra_stream_in[] = 0);
		void transform_flush(ssi_stream_t& stream_in,
			ssi_stream_t& stream_out,
			ssi_size_t xtra_stream_in_num = 0,
			ssi_stream_t xtra_stream_in[] = 0);

		ssi_size_t getSampleDimensionOut(ssi_size_t sample_dimension_in) {
			return sample_dimension_in;
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

	protected:

		Shimmer3PPGToHR(const ssi_char_t* file = 0);
		Shimmer3PPGToHR::Options _options;
		ssi_char_t* _file;

		PPGToHRAlgorithmUniquePtr _ppgToHRAlgorithm;
		double _sessionTimestamp; //dummy timestamp to satisfy interface of Shimmer interal PPGtoHR algorithm
		double _msPerSample;
	};

}

#endif //SSI_SHIMMER3_PPGTOHR_H
