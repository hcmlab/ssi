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

Provides KinectFeatures analysis.

*/

#pragma once

#ifndef SSI_Openness_H
#define SSI_Openness_H

#include "base/IFeature.h"
#include "ioput/option/OptionList.h"
#include "SSI_SkeletonCons.h"

namespace ssi {
	class Openness : public IFeature {
	public:

		class Options : public OptionList {
		public:

			Options()
				: global(false), normalize(true), normalizemaxval(1350), normalizeminval(0) {
				addOption("global", &global, 1, SSI_BOOL, "calculate energy over dimensions");
				addOption("normalize", &normalize, 1, SSI_BOOL, "Normalize output between 0 and 1");
				addOption("normalizemaxValue", &normalizemaxval, 1, SSI_FLOAT, "Normalize between 0 and this max value");
				addOption("normalizeminValue", &normalizemaxval, 1, SSI_FLOAT, "Normalize between this minValue and this max value");
			};

			bool global, normalize;
			ssi_real_t normalizemaxval;
			ssi_real_t normalizeminval;;
		};

	public:

		static const ssi_char_t *GetCreateName() { return "Openness"; };
		static IObject *Create(const ssi_char_t *file) { return new Openness(file); };
		~Openness();

		Openness::Options *getOptions() { return &_options; };
		const ssi_char_t *getName() { return GetCreateName(); };
		const ssi_char_t *getInfo() { return "Calculates the Energy of the Skeleton's movements."; };

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

		ssi_size_t getSampleDimensionOut(ssi_size_t sample_dimension_in) {
			return 1;
		}
		ssi_size_t getSampleBytesOut(ssi_size_t sample_bytes_in) {
			return sizeof(float);
		}
		ssi_type_t getSampleTypeOut(ssi_type_t sample_type_in) {
			if (sample_type_in != SSI_FLOAT) {
				ssi_err("type %s not supported", SSI_TYPE_NAMES[sample_type_in]);
				return SSI_UNDEF;
			}
			return SSI_REAL;
		}

	protected:

		Openness(const ssi_char_t *file = 0);
		Openness::Options _options;
		ssi_char_t *_file;
		ssi_real_t normalizevalue(ssi_real_t value, ssi_real_t min, ssi_real_t max);
		ssi_real_t energydata_temp[SSI_SKELETON_JOINT::NUM][SSI_SKELETON_JOINT_VALUE::NUM];
		ssi_real_t sumdata[SSI_SKELETON_JOINT::NUM];
		ssi_size_t sumcount[SSI_SKELETON_JOINT::NUM];
		ssi_real_t tmp_x;
		ssi_real_t tmp_y;
		ssi_real_t tmp_z;
	};
}

#endif
