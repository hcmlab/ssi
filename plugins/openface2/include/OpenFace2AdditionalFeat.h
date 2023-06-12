#pragma once
// Openface2AdditionalFeat.h
// author: Tobias Baur
// created: 12/06/2023
// Copyright (C) University of Augsburg, Lab for Human Centered Artificial Intelligence
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************
#ifndef SSI_OPENFACE2ADDITIONALFEAT_H
#define SSI_OPENFACE2ADDITIONALFEAT_H

#include "base/IFeature.h"
#include "ioput/option/OptionList.h"

namespace ssi {

	class Openface2AdditionalFeat : public IFeature {
	public:
		class Options : public OptionList {
		public:

			Options() :
				sd_area_mouth(true),
				sum_area_mouth(true),
				sd_dist_nose_lower_lip(true),
				sum_dist_nose_lower_lip(true),
				sd_dist_nose_chin(true),
				sum_dist_nose_chin(true),
				sd_dist_chin_pose(true),
				sum_dist_chin_pose(true),
				magnitudes_lips(true),
				sum_rotations_pose(true)

			{
				addOption("SD_Area_Mouth", &sd_area_mouth, 1, SSI_BOOL, "Calculates standard deviation of the area over mouth");
				addOption("Sum_Area_Mouth", &sum_area_mouth, 1, SSI_BOOL, "Calculates sum of the area over mouth");
				addOption("SD_Dist_Lips", &sd_dist_nose_lower_lip, 1, SSI_BOOL, "Calculates standard deviation of the distance between lips");
				addOption("Sum_Dist_Lips", &sum_dist_nose_lower_lip, 1, SSI_BOOL, "Calculates sum of the distance between lips");
				addOption("SD_Dist_Nose_Chin", &sd_dist_nose_chin, 1, SSI_BOOL, "Calculates standard deviation of the distance nose to chin");
				addOption("Sum_Dist_Nose_Chin", &sum_dist_nose_chin, 1, SSI_BOOL, "Calculates sum of the distance nose to chin");
				addOption("SD_Dist_Chin_Pose", &sd_dist_chin_pose, 1, SSI_BOOL, "Calculates standard deviation of the distance chin to pose");
				addOption("Sum_Dist_Chin_Pose", &sum_dist_chin_pose, 1, SSI_BOOL, "Calculates sum of the distance chin to pose");
				addOption("Magnitudes_Lips", &magnitudes_lips, 1, SSI_BOOL, "Calculates the magnitudes of 20 facial landmarks on lips");
				addOption("Sum_Rotations_Head", &sum_rotations_pose, 1, SSI_BOOL, "Calculates sum of rotations of pose");
			};

			bool
				sd_area_mouth,
				sum_area_mouth,
				sd_dist_nose_lower_lip,
				sum_dist_nose_lower_lip,
				sd_dist_nose_chin,
				sum_dist_nose_chin,
				sd_dist_chin_pose,
				sum_dist_chin_pose,
				magnitudes_lips,
				sum_rotations_pose;
		};

	public:
		static const ssi_char_t* GetCreateName() { return "Openface2AdditionalFeat"; };
		static IObject* Create(const ssi_char_t* file) { return new Openface2AdditionalFeat(file); };
		Openface2AdditionalFeat::Options* getOptions() { return &_options; };
		~Openface2AdditionalFeat();
		const ssi_char_t* getName() { return GetCreateName(); };
		const ssi_char_t* getInfo() { return "Openface2 additional features"; };

		ssi_size_t getSampleDimensionOut(ssi_size_t sample_dimension_in) {
			int dimensions = 0;
			dimensions += _options.sd_area_mouth ? 1 : 0;
			dimensions += _options.sum_area_mouth ? 1 : 0;
			dimensions += _options.sd_dist_nose_lower_lip ? 1 : 0;
			dimensions += _options.sum_dist_nose_lower_lip ? 1 : 0;
			dimensions += _options.sd_dist_nose_chin ? 1 : 0;
			dimensions += _options.sum_dist_nose_chin ? 1 : 0;
			dimensions += _options.sd_dist_chin_pose ? 1 : 0;
			dimensions += _options.sum_dist_chin_pose ? 1 : 0;
			dimensions += _options.magnitudes_lips ? 20 : 0;
			dimensions += _options.sum_rotations_pose ? 3 : 0;

			return dimensions;
		};
		ssi_size_t getSampleBytesOut(ssi_size_t sample_bytes_in) { return sample_bytes_in; };
		ssi_type_t getSampleTypeOut(ssi_type_t sample_type_in) { return sample_type_in; };

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

		float calc_max_norm_mag(ssi_real_t* in, int i, int sample_dimension);

		float rot_diff(float rot_1, float rot_2);

		float calc_dist_r2(float point_1_1, float point_1_2, float point_2_1, float point_2_2);

		float calc_variance(float data[], float sum, float size);

		float calc_area(float height, float width);

	protected:
		Openface2AdditionalFeat(const ssi_char_t* file = 0);
		Openface2AdditionalFeat::Options _options;
		ssi_char_t* _file;

	};

}

#endif