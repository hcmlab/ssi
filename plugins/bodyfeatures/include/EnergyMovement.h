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

#ifndef SSI_ENERGYMOVEMENT_H
#define SSI_ENERGYMOVEMENT_H

#include "base/IFeature.h"
#include "ioput/option/OptionList.h"
#include "SSI_SkeletonCons.h"

namespace ssi {
	class EnergyMovement : public IFeature {
	public:

		class Options : public OptionList {
		public:


			Options() : maxdim(11), norm(true), normmax(130.0) {
				addOption("maxdim", &maxdim, 1, SSI_INT, "calculate energy for dimensions (25=all, 11=upperbody only (no legs, no face), 19= full body without face)");
				addOption("norm", &norm, 1, SSI_BOOL, "normalize energy");
				addOption("normmax", &normmax, 1, SSI_FLOAT, "Max Value for normalization");
			}

			int maxdim;
			bool norm;
			float normmax;
		};

	public:

		static const ssi_char_t *GetCreateName() { return "EnergyMovement"; };
		static IObject *Create(const ssi_char_t *file) { return new EnergyMovement(file); };
		~EnergyMovement();

		EnergyMovement::Options *getOptions() { return &_options; };
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
			return _options.maxdim;
		}
		ssi_size_t getSampleBytesOut(ssi_size_t sample_bytes_in) {
			return sample_bytes_in;
		}
		ssi_type_t getSampleTypeOut(ssi_type_t sample_type_in) {
			if (sample_type_in != SSI_FLOAT) {
				ssi_err("type %s not supported", SSI_TYPE_NAMES[sample_type_in]);
				return SSI_UNDEF;
			}
			return SSI_REAL;
		}

		ssi_real_t normalizevalue(ssi_real_t value, ssi_real_t min, ssi_real_t max);

	protected:

		EnergyMovement(const ssi_char_t *file = 0);
		EnergyMovement::Options _options;
		ssi_char_t *_file;
		ssi_real_t energydata_temp[SSI_SKELETON_JOINT::NUM][SSI_SKELETON_JOINT_VALUE::NUM];
		ssi_real_t sumdata[SSI_SKELETON_JOINT::NUM];
		ssi_size_t sumcount[SSI_SKELETON_JOINT::NUM];
		ssi_real_t tmp_x;
		ssi_real_t tmp_y;
		ssi_real_t tmp_z;
	};
}

#endif
