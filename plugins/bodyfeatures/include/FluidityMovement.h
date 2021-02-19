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

#ifndef SSI_FLUIDITYMOVEMENT_H
#define SSI_FLUIDITYMOVEMENT_H

#include "base/IFeature.h"
#include "ioput/option/OptionList.h"
#include "SSI_SkeletonCons.h"

namespace ssi {
	class FluidityMovement : public IFeature {
	public:

		class Options : public OptionList {
		public:

			Options()
				: global(false), alternativeAlgorithm(false), legs(false) {
				addOption("global", &global, 1, SSI_BOOL, "calculate energy over dimensions");
				addOption("alternativeAlgorithm", &alternativeAlgorithm, 1, SSI_BOOL, "Use alernative algorithm (measures variance of acceleration instead of speed)");
				addOption("legs", &legs, 1, SSI_BOOL, "Measure fluidity of legs instead of arms");
			};

			bool global;
			bool alternativeAlgorithm;
			bool legs;
		};

	public:

		static const ssi_char_t *GetCreateName() { return "FluidityMovement"; };
		static IObject *Create(const ssi_char_t *file) { return new FluidityMovement(file); };
		~FluidityMovement();

		FluidityMovement::Options *getOptions() { return &_options; };
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

		float Variance(ssi_real_t *data, ssi_size_t items);
		float ArithmeticMean(ssi_real_t data[], ssi_size_t items);
		ssi_size_t getSampleDimensionOut(ssi_size_t sample_dimension_in) {
			return 1;
		}
		ssi_size_t getSampleBytesOut(ssi_size_t sample_bytes_in) {
			return sizeof(ssi_real_t);
		}
		ssi_type_t getSampleTypeOut(ssi_type_t sample_type_in) {
			if (sample_type_in != SSI_FLOAT) {
				ssi_err("type %s not supported", SSI_TYPE_NAMES[sample_type_in]);
				return SSI_UNDEF;
			}
			return SSI_REAL;
		}

	protected:

		FluidityMovement(const ssi_char_t *file = 0);
		FluidityMovement::Options _options;
		ssi_char_t *_file;
		ssi_real_t *sumdataleft;
		ssi_real_t *sumdataright;
		ssi_real_t energydata_temp[SSI_SKELETON_JOINT::NUM][SSI_SKELETON_JOINT_VALUE::NUM];
		ssi_size_t sumcount;
		ssi_real_t tmp_x;
		ssi_real_t tmp_y;
		ssi_real_t tmp_z;

		ssi_real_t tmp2_x;
		ssi_real_t tmp2_y;
		ssi_real_t tmp2_z;


		ssi_real_t tmpdiff_x;
		ssi_real_t tmpdiff_y;
		ssi_real_t tmpdiff_z;

	};
}

#endif
