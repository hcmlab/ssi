// Openface2.h
// author: Johannes Wagner <wagner@hcm-lab.de>, Björn Bittner <bittner@hcm-lab.de>
// created: 14/6/2016
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
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

#pragma once

#ifndef SSI_OPENFACE_OPENFACE2_H
#define SSI_OPENFACE_OPENFACE2_H

#include "base/IFilter.h"
#include "ioput/option/OptionList.h"


namespace ssi {

	class Openface2Helper;

	class Openface2 : public IFilter {

	public:

		class Options : public OptionList {

		public:

			Options() {

				setModelPath(".\\");
				setAuPath(".\\");
				pose = true;
				landmarks = true;
				landmarks3d = true;
				eye = true;
				eye3d = true;
				gaze = true;
				actionunits = true;

				addOption("modelPath", modelPath, SSI_MAX_CHAR, SSI_CHAR, "Path to model");
				addOption("auPath", AuPath, SSI_MAX_CHAR, SSI_CHAR, "Path to action units");
				addOption("pose", &pose,1, SSI_BOOL, "Calculate head position (x,y,z,rot_x,rot_y,rot_z)");
				addOption("landmarks", &landmarks, 1, SSI_BOOL, "Calculate landmarks (x_1,y_1,...,x_68,y_68)");
				addOption("landmarks3d", &landmarks3d, 1, SSI_BOOL, "Calculate 3D landmarks (x_1,y_1,z_1,...,x_68,y_68,z_68)");
				addOption("eye", &eye, 1, SSI_BOOL, "Calculate 2D eye landmarks (x_1,y_1,...,x_56,y_56)");
				addOption("eye3d", &eye3d, 1, SSI_BOOL, "Calculate 3D eye landmarks (x_1,y_1,z_1,...,x_56,y_56,z_56)");
				addOption("gaze", &gaze, 1, SSI_BOOL, "Calculate gaze direction (x_left,y_left,z_left,x_right,y_right,z_right)");
				addOption("actionunits", &actionunits, 1, SSI_BOOL, "Calculate action units");
			}

			void setModelPath(const ssi_char_t *string) {
				this->modelPath[0] = '\0';
				if (string) {
					ssi_strcpy(this->modelPath, string);
				}
			}

			void setAuPath(const ssi_char_t *string) {
				this->AuPath[0] = '\0';
				if (string) {
					ssi_strcpy(this->AuPath, string);
				}
			}

			ssi_char_t modelPath[SSI_MAX_CHAR], AuPath[SSI_MAX_CHAR];
			bool pose, landmarks, landmarks3d, eye, eye3d, gaze, actionunits;
		};

	public:

		static const ssi_char_t *GetCreateName() { return "Openface2"; };
		static IObject *Create(const ssi_char_t *file) { return new Openface2(file); };
		~Openface2();

		Options *getOptions() { return &_options; };
		const ssi_char_t *getName() { return GetCreateName(); };
		const ssi_char_t *getInfo() { return "Facial landmark detection, head pose estimation, facial action unit recognition, and eye-gaze estimation."; };

		void setMetaData(ssi_size_t size, const void *meta) {
			if (sizeof(_video_format) != size) {
				ssi_err("invalid meta size");
				return;
			}
			memcpy(&_video_format, meta, size);
			_stride = ssi_video_stride(_video_format);
		};

		ssi_video_params_t getVideoFormat() { return _video_format; };
		

		void transform_enter(ssi_stream_t &stream_in,
			ssi_stream_t &stream_out,
			ssi_size_t xtra_stream_in_num = 0,
			ssi_stream_t xtra_stream_in[] = 0);
		void transform(ITransformer::info info,
			ssi_stream_t &stream_in,
			ssi_stream_t &stream_out,
			ssi_size_t xtra_stream_in_num = 0,
			ssi_stream_t xtra_stream_in[] = 0);
		void transform_flush(ssi_stream_t &stream_in,
			ssi_stream_t &stream_out,
			ssi_size_t xtra_stream_in_num = 0,
			ssi_stream_t xtra_stream_in[] = 0);

		ssi_size_t getSampleDimensionOut(ssi_size_t sample_dimension_in) {
			return FEATURE::NUM;
		}

		ssi_size_t getSampleBytesOut(ssi_size_t sample_bytes_in) {
			return sizeof(SSI_REAL);
		}

		ssi_type_t getSampleTypeOut(ssi_type_t sample_type_in) {
			return SSI_REAL;
		}

		struct FEATURE {
			enum Value {
				DETECTION_SUCCESS,
				DETECTION_CERTAINTY,
				POSE_X,
				POSE_Y,
				POSE_Z,
				POSE_ROT_X,
				POSE_ROT_Y,
				POSE_ROT_Z,
				FACIAL_LANDMARK_1_X,
				FACIAL_LANDMARK_1_Y,
				FACIAL_LANDMARK_2_X,
				FACIAL_LANDMARK_2_Y,
				FACIAL_LANDMARK_3_X,
				FACIAL_LANDMARK_3_Y,
				FACIAL_LANDMARK_4_X,
				FACIAL_LANDMARK_4_Y,
				FACIAL_LANDMARK_5_X,
				FACIAL_LANDMARK_5_Y,
				FACIAL_LANDMARK_6_X,
				FACIAL_LANDMARK_6_Y,
				FACIAL_LANDMARK_7_X,
				FACIAL_LANDMARK_7_Y,
				FACIAL_LANDMARK_8_X,
				FACIAL_LANDMARK_8_Y,
				FACIAL_LANDMARK_9_X,
				FACIAL_LANDMARK_9_Y,
				FACIAL_LANDMARK_10_X,
				FACIAL_LANDMARK_10_Y,
				FACIAL_LANDMARK_11_X,
				FACIAL_LANDMARK_11_Y,
				FACIAL_LANDMARK_12_X,
				FACIAL_LANDMARK_12_Y,
				FACIAL_LANDMARK_13_X,
				FACIAL_LANDMARK_13_Y,
				FACIAL_LANDMARK_14_X,
				FACIAL_LANDMARK_14_Y,
				FACIAL_LANDMARK_15_X,
				FACIAL_LANDMARK_15_Y,
				FACIAL_LANDMARK_16_X,
				FACIAL_LANDMARK_16_Y,
				FACIAL_LANDMARK_17_X,
				FACIAL_LANDMARK_17_Y,
				FACIAL_LANDMARK_18_X,
				FACIAL_LANDMARK_18_Y,
				FACIAL_LANDMARK_19_X,
				FACIAL_LANDMARK_19_Y,
				FACIAL_LANDMARK_20_X,
				FACIAL_LANDMARK_20_Y,
				FACIAL_LANDMARK_21_X,
				FACIAL_LANDMARK_21_Y,
				FACIAL_LANDMARK_22_X,
				FACIAL_LANDMARK_22_Y,
				FACIAL_LANDMARK_23_X,
				FACIAL_LANDMARK_23_Y,
				FACIAL_LANDMARK_24_X,
				FACIAL_LANDMARK_24_Y,
				FACIAL_LANDMARK_25_X,
				FACIAL_LANDMARK_25_Y,
				FACIAL_LANDMARK_26_X,
				FACIAL_LANDMARK_26_Y,
				FACIAL_LANDMARK_27_X,
				FACIAL_LANDMARK_27_Y,
				FACIAL_LANDMARK_28_X,
				FACIAL_LANDMARK_28_Y,
				FACIAL_LANDMARK_29_X,
				FACIAL_LANDMARK_29_Y,
				FACIAL_LANDMARK_30_X,
				FACIAL_LANDMARK_30_Y,
				FACIAL_LANDMARK_31_X,
				FACIAL_LANDMARK_31_Y,
				FACIAL_LANDMARK_32_X,
				FACIAL_LANDMARK_32_Y,
				FACIAL_LANDMARK_33_X,
				FACIAL_LANDMARK_33_Y,
				FACIAL_LANDMARK_34_X,
				FACIAL_LANDMARK_34_Y,
				FACIAL_LANDMARK_35_X,
				FACIAL_LANDMARK_35_Y,
				FACIAL_LANDMARK_36_X,
				FACIAL_LANDMARK_36_Y,
				FACIAL_LANDMARK_37_X,
				FACIAL_LANDMARK_37_Y,
				FACIAL_LANDMARK_38_X,
				FACIAL_LANDMARK_38_Y,
				FACIAL_LANDMARK_39_X,
				FACIAL_LANDMARK_39_Y,
				FACIAL_LANDMARK_40_X,
				FACIAL_LANDMARK_40_Y,
				FACIAL_LANDMARK_41_X,
				FACIAL_LANDMARK_41_Y,
				FACIAL_LANDMARK_42_X,
				FACIAL_LANDMARK_42_Y,
				FACIAL_LANDMARK_43_X,
				FACIAL_LANDMARK_43_Y,
				FACIAL_LANDMARK_44_X,
				FACIAL_LANDMARK_44_Y,
				FACIAL_LANDMARK_45_X,
				FACIAL_LANDMARK_45_Y,
				FACIAL_LANDMARK_46_X,
				FACIAL_LANDMARK_46_Y,
				FACIAL_LANDMARK_47_X,
				FACIAL_LANDMARK_47_Y,
				FACIAL_LANDMARK_48_X,
				FACIAL_LANDMARK_48_Y,
				FACIAL_LANDMARK_49_X,
				FACIAL_LANDMARK_49_Y,
				FACIAL_LANDMARK_50_X,
				FACIAL_LANDMARK_50_Y,
				FACIAL_LANDMARK_51_X,
				FACIAL_LANDMARK_51_Y,
				FACIAL_LANDMARK_52_X,
				FACIAL_LANDMARK_52_Y,
				FACIAL_LANDMARK_53_X,
				FACIAL_LANDMARK_53_Y,
				FACIAL_LANDMARK_54_X,
				FACIAL_LANDMARK_54_Y,
				FACIAL_LANDMARK_55_X,
				FACIAL_LANDMARK_55_Y,
				FACIAL_LANDMARK_56_X,
				FACIAL_LANDMARK_56_Y,
				FACIAL_LANDMARK_57_X,
				FACIAL_LANDMARK_57_Y,
				FACIAL_LANDMARK_58_X,
				FACIAL_LANDMARK_58_Y,
				FACIAL_LANDMARK_59_X,
				FACIAL_LANDMARK_59_Y,
				FACIAL_LANDMARK_60_X,
				FACIAL_LANDMARK_60_Y,
				FACIAL_LANDMARK_61_X,
				FACIAL_LANDMARK_61_Y,
				FACIAL_LANDMARK_62_X,
				FACIAL_LANDMARK_62_Y,
				FACIAL_LANDMARK_63_X,
				FACIAL_LANDMARK_63_Y,
				FACIAL_LANDMARK_64_X,
				FACIAL_LANDMARK_64_Y,
				FACIAL_LANDMARK_65_X,
				FACIAL_LANDMARK_65_Y,
				FACIAL_LANDMARK_66_X,
				FACIAL_LANDMARK_66_Y,
				FACIAL_LANDMARK_67_X,
				FACIAL_LANDMARK_67_Y,
				FACIAL_LANDMARK_68_X,
				FACIAL_LANDMARK_68_Y,
				FACIAL_LANDMARK_3D_1_X,
				FACIAL_LANDMARK_3D_1_Y,
				FACIAL_LANDMARK_3D_1_Z,
				FACIAL_LANDMARK_3D_2_X,
				FACIAL_LANDMARK_3D_2_Y,
				FACIAL_LANDMARK_3D_2_Z,
				FACIAL_LANDMARK_3D_3_X,
				FACIAL_LANDMARK_3D_3_Y,
				FACIAL_LANDMARK_3D_3_Z,
				FACIAL_LANDMARK_3D_4_X,
				FACIAL_LANDMARK_3D_4_Y,
				FACIAL_LANDMARK_3D_4_Z,
				FACIAL_LANDMARK_3D_5_X,
				FACIAL_LANDMARK_3D_5_Y,
				FACIAL_LANDMARK_3D_5_Z,
				FACIAL_LANDMARK_3D_6_X,
				FACIAL_LANDMARK_3D_6_Y,
				FACIAL_LANDMARK_3D_6_Z,
				FACIAL_LANDMARK_3D_7_X,
				FACIAL_LANDMARK_3D_7_Y,
				FACIAL_LANDMARK_3D_7_Z,
				FACIAL_LANDMARK_3D_8_X,
				FACIAL_LANDMARK_3D_8_Y,
				FACIAL_LANDMARK_3D_8_Z,
				FACIAL_LANDMARK_3D_9_X,
				FACIAL_LANDMARK_3D_9_Y,
				FACIAL_LANDMARK_3D_9_Z,
				FACIAL_LANDMARK_3D_10_X,
				FACIAL_LANDMARK_3D_10_Y,
				FACIAL_LANDMARK_3D_10_Z,
				FACIAL_LANDMARK_3D_11_X,
				FACIAL_LANDMARK_3D_11_Y,
				FACIAL_LANDMARK_3D_11_Z,
				FACIAL_LANDMARK_3D_12_X,
				FACIAL_LANDMARK_3D_12_Y,
				FACIAL_LANDMARK_3D_12_ZY,
				FACIAL_LANDMARK_3D_13_X,
				FACIAL_LANDMARK_3D_13_Y,
				FACIAL_LANDMARK_3D_13_Z,
				FACIAL_LANDMARK_3D_14_X,
				FACIAL_LANDMARK_3D_14_Y,
				FACIAL_LANDMARK_3D_14_Z,
				FACIAL_LANDMARK_3D_15_X,
				FACIAL_LANDMARK_3D_15_Y,
				FACIAL_LANDMARK_3D_15_Z,
				FACIAL_LANDMARK_3D_16_X,
				FACIAL_LANDMARK_3D_16_Y,
				FACIAL_LANDMARK_3D_16_Z,
				FACIAL_LANDMARK_3D_17_X,
				FACIAL_LANDMARK_3D_17_Y,
				FACIAL_LANDMARK_3D_17_Z,
				FACIAL_LANDMARK_3D_18_X,
				FACIAL_LANDMARK_3D_18_Y,
				FACIAL_LANDMARK_3D_18_Z,
				FACIAL_LANDMARK_3D_19_X,
				FACIAL_LANDMARK_3D_19_Y,
				FACIAL_LANDMARK_3D_19_Z,
				FACIAL_LANDMARK_3D_20_X,
				FACIAL_LANDMARK_3D_20_Y,
				FACIAL_LANDMARK_3D_20_Z,
				FACIAL_LANDMARK_3D_21_X,
				FACIAL_LANDMARK_3D_21_Y,
				FACIAL_LANDMARK_3D_21_Z,
				FACIAL_LANDMARK_3D_22_X,
				FACIAL_LANDMARK_3D_22_Y,
				FACIAL_LANDMARK_3D_22_Z,
				FACIAL_LANDMARK_3D_23_X,
				FACIAL_LANDMARK_3D_23_Y,
				FACIAL_LANDMARK_3D_23_Z,
				FACIAL_LANDMARK_3D_24_X,
				FACIAL_LANDMARK_3D_24_Y,
				FACIAL_LANDMARK_3D_24_Z,
				FACIAL_LANDMARK_3D_25_X,
				FACIAL_LANDMARK_3D_25_Y,
				FACIAL_LANDMARK_3D_25_Z,
				FACIAL_LANDMARK_3D_26_X,
				FACIAL_LANDMARK_3D_26_Y,
				FACIAL_LANDMARK_3D_26_Z,
				FACIAL_LANDMARK_3D_27_X,
				FACIAL_LANDMARK_3D_27_Y,
				FACIAL_LANDMARK_3D_27_Z,
				FACIAL_LANDMARK_3D_28_X,
				FACIAL_LANDMARK_3D_28_Y,
				FACIAL_LANDMARK_3D_28_Z,
				FACIAL_LANDMARK_3D_29_X,
				FACIAL_LANDMARK_3D_29_Y,
				FACIAL_LANDMARK_3D_29_Z,
				FACIAL_LANDMARK_3D_30_X,
				FACIAL_LANDMARK_3D_30_Y,
				FACIAL_LANDMARK_3D_30_Z,
				FACIAL_LANDMARK_3D_31_X,
				FACIAL_LANDMARK_3D_31_Y,
				FACIAL_LANDMARK_3D_31_Z,
				FACIAL_LANDMARK_3D_32_X,
				FACIAL_LANDMARK_3D_32_Y,
				FACIAL_LANDMARK_3D_32_Z,
				FACIAL_LANDMARK_3D_33_X,
				FACIAL_LANDMARK_3D_33_Y,
				FACIAL_LANDMARK_3D_33_Z,
				FACIAL_LANDMARK_3D_34_X,
				FACIAL_LANDMARK_3D_34_Y,
				FACIAL_LANDMARK_3D_34_Z,
				FACIAL_LANDMARK_3D_35_X,
				FACIAL_LANDMARK_3D_35_Y,
				FACIAL_LANDMARK_3D_35_Z,
				FACIAL_LANDMARK_3D_36_X,
				FACIAL_LANDMARK_3D_36_Y,
				FACIAL_LANDMARK_3D_36_Z,
				FACIAL_LANDMARK_3D_37_X,
				FACIAL_LANDMARK_3D_37_Y,
				FACIAL_LANDMARK_3D_37_Z,
				FACIAL_LANDMARK_3D_38_X,
				FACIAL_LANDMARK_3D_38_Y,
				FACIAL_LANDMARK_3D_38_Z,
				FACIAL_LANDMARK_3D_39_X,
				FACIAL_LANDMARK_3D_39_Y,
				FACIAL_LANDMARK_3D_39_Z,
				FACIAL_LANDMARK_3D_40_X,
				FACIAL_LANDMARK_3D_40_Y,
				FACIAL_LANDMARK_3D_40_Z,
				FACIAL_LANDMARK_3D_41_X,
				FACIAL_LANDMARK_3D_41_Y,
				FACIAL_LANDMARK_3D_41_Z,
				FACIAL_LANDMARK_3D_42_X,
				FACIAL_LANDMARK_3D_42_Y,
				FACIAL_LANDMARK_3D_42_Z,
				FACIAL_LANDMARK_3D_43_X,
				FACIAL_LANDMARK_3D_43_Y,
				FACIAL_LANDMARK_3D_43_Z,
				FACIAL_LANDMARK_3D_44_X,
				FACIAL_LANDMARK_3D_44_Y,
				FACIAL_LANDMARK_3D_44_Z,
				FACIAL_LANDMARK_3D_45_X,
				FACIAL_LANDMARK_3D_45_Y,
				FACIAL_LANDMARK_3D_45_Z,
				FACIAL_LANDMARK_3D_46_X,
				FACIAL_LANDMARK_3D_46_Y,
				FACIAL_LANDMARK_3D_46_Z,
				FACIAL_LANDMARK_3D_47_X,
				FACIAL_LANDMARK_3D_47_Y,
				FACIAL_LANDMARK_3D_47_Z,
				FACIAL_LANDMARK_3D_48_X,
				FACIAL_LANDMARK_3D_48_Y,
				FACIAL_LANDMARK_3D_48_Z,
				FACIAL_LANDMARK_3D_49_X,
				FACIAL_LANDMARK_3D_49_Y,
				FACIAL_LANDMARK_3D_49_Z,
				FACIAL_LANDMARK_3D_50_X,
				FACIAL_LANDMARK_3D_50_Y,
				FACIAL_LANDMARK_3D_50_Z,
				FACIAL_LANDMARK_3D_51_X,
				FACIAL_LANDMARK_3D_51_Y,
				FACIAL_LANDMARK_3D_51_Z,
				FACIAL_LANDMARK_3D_52_X,
				FACIAL_LANDMARK_3D_52_Y,
				FACIAL_LANDMARK_3D_52_Z,
				FACIAL_LANDMARK_3D_53_X,
				FACIAL_LANDMARK_3D_53_Y,
				FACIAL_LANDMARK_3D_53_Z,
				FACIAL_LANDMARK_3D_54_X,
				FACIAL_LANDMARK_3D_54_Y,
				FACIAL_LANDMARK_3D_54_Z,
				FACIAL_LANDMARK_3D_55_X,
				FACIAL_LANDMARK_3D_55_Y,
				FACIAL_LANDMARK_3D_55_Z,
				FACIAL_LANDMARK_3D_56_X,
				FACIAL_LANDMARK_3D_56_Y,
				FACIAL_LANDMARK_3D_56_Z,
				FACIAL_LANDMARK_3D_57_X,
				FACIAL_LANDMARK_3D_57_Y,
				FACIAL_LANDMARK_3D_57_Z,
				FACIAL_LANDMARK_3D_58_X,
				FACIAL_LANDMARK_3D_58_Y,
				FACIAL_LANDMARK_3D_58_Z,
				FACIAL_LANDMARK_3D_59_X,
				FACIAL_LANDMARK_3D_59_Y,
				FACIAL_LANDMARK_3D_59_Z,
				FACIAL_LANDMARK_3D_60_X,
				FACIAL_LANDMARK_3D_60_Y,
				FACIAL_LANDMARK_3D_60_Z,
				FACIAL_LANDMARK_3D_61_X,
				FACIAL_LANDMARK_3D_61_Y,
				FACIAL_LANDMARK_3D_61_Z,
				FACIAL_LANDMARK_3D_62_X,
				FACIAL_LANDMARK_3D_62_Y,
				FACIAL_LANDMARK_3D_62_Z,
				FACIAL_LANDMARK_3D_63_X,
				FACIAL_LANDMARK_3D_63_Y,
				FACIAL_LANDMARK_3D_63_Z,
				FACIAL_LANDMARK_3D_64_X,
				FACIAL_LANDMARK_3D_64_Y,
				FACIAL_LANDMARK_3D_64_Z,
				FACIAL_LANDMARK_3D_65_X,
				FACIAL_LANDMARK_3D_65_Y,
				FACIAL_LANDMARK_3D_65_Z,
				FACIAL_LANDMARK_3D_66_X,
				FACIAL_LANDMARK_3D_66_Y,
				FACIAL_LANDMARK_3D_66_Z,
				FACIAL_LANDMARK_3D_67_X,
				FACIAL_LANDMARK_3D_67_Y,
				FACIAL_LANDMARK_3D_67_Z,
				FACIAL_LANDMARK_3D_68_X,
				FACIAL_LANDMARK_3D_68_Y,
				FACIAL_LANDMARK_3D_68_Z,
				EYE_LANDMARK_1_X,
				EYE_LANDMARK_1_Y,
				EYE_LANDMARK_2_X,
				EYE_LANDMARK_2_Y,
				EYE_LANDMARK_3_X,
				EYE_LANDMARK_3_Y,
				EYE_LANDMARK_4_X,
				EYE_LANDMARK_4_Y,
				EYE_LANDMARK_5_X,
				EYE_LANDMARK_5_Y,
				EYE_LANDMARK_6_X,
				EYE_LANDMARK_6_Y,
				EYE_LANDMARK_7_X,
				EYE_LANDMARK_7_Y,
				EYE_LANDMARK_8_X,
				EYE_LANDMARK_8_Y,
				EYE_LANDMARK_9_X,
				EYE_LANDMARK_9_Y,
				EYE_LANDMARK_10_X,
				EYE_LANDMARK_10_Y,
				EYE_LANDMARK_11_X,
				EYE_LANDMARK_11_Y,
				EYE_LANDMARK_12_X,
				EYE_LANDMARK_12_Y,
				EYE_LANDMARK_13_X,
				EYE_LANDMARK_13_Y,
				EYE_LANDMARK_14_X,
				EYE_LANDMARK_14_Y,
				EYE_LANDMARK_15_X,
				EYE_LANDMARK_15_Y,
				EYE_LANDMARK_16_X,
				EYE_LANDMARK_16_Y,
				EYE_LANDMARK_17_X,
				EYE_LANDMARK_17_Y,
				EYE_LANDMARK_18_X,
				EYE_LANDMARK_18_Y,
				EYE_LANDMARK_19_X,
				EYE_LANDMARK_19_Y,
				EYE_LANDMARK_20_X,
				EYE_LANDMARK_20_Y,
				EYE_LANDMARK_21_X,
				EYE_LANDMARK_21_Y,
				EYE_LANDMARK_22_X,
				EYE_LANDMARK_22_Y,
				EYE_LANDMARK_23_X,
				EYE_LANDMARK_23_Y,
				EYE_LANDMARK_24_X,
				EYE_LANDMARK_24_Y,
				EYE_LANDMARK_25_X,
				EYE_LANDMARK_25_Y,
				EYE_LANDMARK_26_X,
				EYE_LANDMARK_26_Y,
				EYE_LANDMARK_27_X,
				EYE_LANDMARK_27_Y,
				EYE_LANDMARK_28_X,
				EYE_LANDMARK_28_Y,
				EYE_LANDMARK_29_X,
				EYE_LANDMARK_29_Y,
				EYE_LANDMARK_30_X,
				EYE_LANDMARK_30_Y,
				EYE_LANDMARK_31_X,
				EYE_LANDMARK_31_Y,
				EYE_LANDMARK_32_X,
				EYE_LANDMARK_32_Y,
				EYE_LANDMARK_33_X,
				EYE_LANDMARK_33_Y,
				EYE_LANDMARK_34_X,
				EYE_LANDMARK_34_Y,
				EYE_LANDMARK_35_X,
				EYE_LANDMARK_35_Y,
				EYE_LANDMARK_36_X,
				EYE_LANDMARK_36_Y,
				EYE_LANDMARK_37_X,
				EYE_LANDMARK_37_Y,
				EYE_LANDMARK_38_X,
				EYE_LANDMARK_38_Y,
				EYE_LANDMARK_39_X,
				EYE_LANDMARK_39_Y,
				EYE_LANDMARK_40_X,
				EYE_LANDMARK_40_Y,
				EYE_LANDMARK_41_X,
				EYE_LANDMARK_41_Y,
				EYE_LANDMARK_42_X,
				EYE_LANDMARK_42_Y,
				EYE_LANDMARK_43_X,
				EYE_LANDMARK_43_Y,
				EYE_LANDMARK_44_X,
				EYE_LANDMARK_44_Y,
				EYE_LANDMARK_45_X,
				EYE_LANDMARK_45_Y,
				EYE_LANDMARK_46_X,
				EYE_LANDMARK_46_Y,
				EYE_LANDMARK_47_X,
				EYE_LANDMARK_47_Y,
				EYE_LANDMARK_48_X,
				EYE_LANDMARK_48_Y,
				EYE_LANDMARK_49_X,
				EYE_LANDMARK_49_Y,
				EYE_LANDMARK_50_X,
				EYE_LANDMARK_50_Y,
				EYE_LANDMARK_51_X,
				EYE_LANDMARK_51_Y,
				EYE_LANDMARK_52_X,
				EYE_LANDMARK_52_Y,
				EYE_LANDMARK_53_X,
				EYE_LANDMARK_53_Y,
				EYE_LANDMARK_54_X,
				EYE_LANDMARK_54_Y,
				EYE_LANDMARK_55_X,
				EYE_LANDMARK_55_Y,
				EYE_LANDMARK_56_X,
				EYE_LANDMARK_56_Y,
				EYE_LANDMARK_3D_1_X,
				EYE_LANDMARK_3D_1_Y,
				EYE_LANDMARK_3D_1_Z,
				EYE_LANDMARK_3D_2_X,
				EYE_LANDMARK_3D_2_Y,
				EYE_LANDMARK_3D_2_Z,
				EYE_LANDMARK_3D_3_X,
				EYE_LANDMARK_3D_3_Y,
				EYE_LANDMARK_3D_3_Z,
				EYE_LANDMARK_3D_4_X,
				EYE_LANDMARK_3D_4_Y,
				EYE_LANDMARK_3D_4_Z,
				EYE_LANDMARK_3D_5_X,
				EYE_LANDMARK_3D_5_Y,
				EYE_LANDMARK_3D_5_Z,
				EYE_LANDMARK_3D_6_X,
				EYE_LANDMARK_3D_6_Y,
				EYE_LANDMARK_3D_6_Z,
				EYE_LANDMARK_3D_7_X,
				EYE_LANDMARK_3D_7_Y,
				EYE_LANDMARK_3D_7_Z,
				EYE_LANDMARK_3D_8_X,
				EYE_LANDMARK_3D_8_Y,
				EYE_LANDMARK_3D_8_Z,
				EYE_LANDMARK_3D_9_X,
				EYE_LANDMARK_3D_9_Y,
				EYE_LANDMARK_3D_9_Z,
				EYE_LANDMARK_3D_10_X,
				EYE_LANDMARK_3D_10_Y,
				EYE_LANDMARK_3D_10_Z,
				EYE_LANDMARK_3D_11_X,
				EYE_LANDMARK_3D_11_Y,
				EYE_LANDMARK_3D_11_Z,
				EYE_LANDMARK_3D_12_X,
				EYE_LANDMARK_3D_12_Y,
				EYE_LANDMARK_3D_12_ZY,
				EYE_LANDMARK_3D_13_X,
				EYE_LANDMARK_3D_13_Y,
				EYE_LANDMARK_3D_13_Z,
				EYE_LANDMARK_3D_14_X,
				EYE_LANDMARK_3D_14_Y,
				EYE_LANDMARK_3D_14_Z,
				EYE_LANDMARK_3D_15_X,
				EYE_LANDMARK_3D_15_Y,
				EYE_LANDMARK_3D_15_Z,
				EYE_LANDMARK_3D_16_X,
				EYE_LANDMARK_3D_16_Y,
				EYE_LANDMARK_3D_16_Z,
				EYE_LANDMARK_3D_17_X,
				EYE_LANDMARK_3D_17_Y,
				EYE_LANDMARK_3D_17_Z,
				EYE_LANDMARK_3D_18_X,
				EYE_LANDMARK_3D_18_Y,
				EYE_LANDMARK_3D_18_Z,
				EYE_LANDMARK_3D_19_X,
				EYE_LANDMARK_3D_19_Y,
				EYE_LANDMARK_3D_19_Z,
				EYE_LANDMARK_3D_20_X,
				EYE_LANDMARK_3D_20_Y,
				EYE_LANDMARK_3D_20_Z,
				EYE_LANDMARK_3D_21_X,
				EYE_LANDMARK_3D_21_Y,
				EYE_LANDMARK_3D_21_Z,
				EYE_LANDMARK_3D_22_X,
				EYE_LANDMARK_3D_22_Y,
				EYE_LANDMARK_3D_22_Z,
				EYE_LANDMARK_3D_23_X,
				EYE_LANDMARK_3D_23_Y,
				EYE_LANDMARK_3D_23_Z,
				EYE_LANDMARK_3D_24_X,
				EYE_LANDMARK_3D_24_Y,
				EYE_LANDMARK_3D_24_Z,
				EYE_LANDMARK_3D_25_X,
				EYE_LANDMARK_3D_25_Y,
				EYE_LANDMARK_3D_25_Z,
				EYE_LANDMARK_3D_26_X,
				EYE_LANDMARK_3D_26_Y,
				EYE_LANDMARK_3D_26_Z,
				EYE_LANDMARK_3D_27_X,
				EYE_LANDMARK_3D_27_Y,
				EYE_LANDMARK_3D_27_Z,
				EYE_LANDMARK_3D_28_X,
				EYE_LANDMARK_3D_28_Y,
				EYE_LANDMARK_3D_28_Z,
				EYE_LANDMARK_3D_29_X,
				EYE_LANDMARK_3D_29_Y,
				EYE_LANDMARK_3D_29_Z,
				EYE_LANDMARK_3D_30_X,
				EYE_LANDMARK_3D_30_Y,
				EYE_LANDMARK_3D_30_Z,
				EYE_LANDMARK_3D_31_X,
				EYE_LANDMARK_3D_31_Y,
				EYE_LANDMARK_3D_31_Z,
				EYE_LANDMARK_3D_32_X,
				EYE_LANDMARK_3D_32_Y,
				EYE_LANDMARK_3D_32_Z,
				EYE_LANDMARK_3D_33_X,
				EYE_LANDMARK_3D_33_Y,
				EYE_LANDMARK_3D_33_Z,
				EYE_LANDMARK_3D_34_X,
				EYE_LANDMARK_3D_34_Y,
				EYE_LANDMARK_3D_34_Z,
				EYE_LANDMARK_3D_35_X,
				EYE_LANDMARK_3D_35_Y,
				EYE_LANDMARK_3D_35_Z,
				EYE_LANDMARK_3D_36_X,
				EYE_LANDMARK_3D_36_Y,
				EYE_LANDMARK_3D_36_Z,
				EYE_LANDMARK_3D_37_X,
				EYE_LANDMARK_3D_37_Y,
				EYE_LANDMARK_3D_37_Z,
				EYE_LANDMARK_3D_38_X,
				EYE_LANDMARK_3D_38_Y,
				EYE_LANDMARK_3D_38_Z,
				EYE_LANDMARK_3D_39_X,
				EYE_LANDMARK_3D_39_Y,
				EYE_LANDMARK_3D_39_Z,
				EYE_LANDMARK_3D_40_X,
				EYE_LANDMARK_3D_40_Y,
				EYE_LANDMARK_3D_40_Z,
				EYE_LANDMARK_3D_41_X,
				EYE_LANDMARK_3D_41_Y,
				EYE_LANDMARK_3D_41_Z,
				EYE_LANDMARK_3D_42_X,
				EYE_LANDMARK_3D_42_Y,
				EYE_LANDMARK_3D_42_Z,
				EYE_LANDMARK_3D_43_X,
				EYE_LANDMARK_3D_43_Y,
				EYE_LANDMARK_3D_43_Z,
				EYE_LANDMARK_3D_44_X,
				EYE_LANDMARK_3D_44_Y,
				EYE_LANDMARK_3D_44_Z,
				EYE_LANDMARK_3D_45_X,
				EYE_LANDMARK_3D_45_Y,
				EYE_LANDMARK_3D_45_Z,
				EYE_LANDMARK_3D_46_X,
				EYE_LANDMARK_3D_46_Y,
				EYE_LANDMARK_3D_46_Z,
				EYE_LANDMARK_3D_47_X,
				EYE_LANDMARK_3D_47_Y,
				EYE_LANDMARK_3D_47_Z,
				EYE_LANDMARK_3D_48_X,
				EYE_LANDMARK_3D_48_Y,
				EYE_LANDMARK_3D_48_Z,
				EYE_LANDMARK_3D_49_X,
				EYE_LANDMARK_3D_49_Y,
				EYE_LANDMARK_3D_49_Z,
				EYE_LANDMARK_3D_50_X,
				EYE_LANDMARK_3D_50_Y,
				EYE_LANDMARK_3D_50_Z,
				EYE_LANDMARK_3D_51_X,
				EYE_LANDMARK_3D_51_Y,
				EYE_LANDMARK_3D_51_Z,
				EYE_LANDMARK_3D_52_X,
				EYE_LANDMARK_3D_52_Y,
				EYE_LANDMARK_3D_52_Z,
				EYE_LANDMARK_3D_53_X,
				EYE_LANDMARK_3D_53_Y,
				EYE_LANDMARK_3D_53_Z,
				EYE_LANDMARK_3D_54_X,
				EYE_LANDMARK_3D_54_Y,
				EYE_LANDMARK_3D_54_Z,
				EYE_LANDMARK_3D_55_X,
				EYE_LANDMARK_3D_55_Y,
				EYE_LANDMARK_3D_55_Z,
				EYE_LANDMARK_3D_56_X,
				EYE_LANDMARK_3D_56_Y,
				EYE_LANDMARK_3D_56_Z,
				GAZE_LEFT_EYE_X,
				GAZE_LEFT_EYE_Y,
				GAZE_LEFT_EYE_Z,
				GAZE_RIGHT_EYE_X,
				GAZE_RIGHT_EYE_Y,
				GAZE_RIGHT_EYE_Z,
				GAZE_ANGLE_X,
				GAZE_ANGLE_Y,
				AU_REG_DETECTION_SUCCESS,
				AU01_r,
				AU02_r,
				AU04_r,
				AU05_r,
				AU06_r,
				AU07_r,
				AU09_r,
				AU10_r,
				AU12_r,
				AU14_r,
				AU15_r,
				AU17_r,
				AU20_r,
				AU23_r,
				AU25_r,
				AU26_r,
				AU45_r,
				AU_CLASS_DETECTION_SUCCESS,
				AU01_c,
				AU02_c,
				AU04_c,
				AU05_c,
				AU06_c,
				AU07_c,
				AU09_c,
				AU10_c,
				AU12_c,
				AU14_c,
				AU15_c,
				AU17_c,
				AU20_c,
				AU23_c,
				AU25_c,
				AU26_c,
				AU28_c,
				AU45_c,
				NUM
			};
		};

	protected:

		Openface2(const ssi_char_t *file = 0);
		ssi_char_t *_file;
		Options _options;
		static char ssi_log_name[];

		ssi_video_params_t _video_format;
		ssi_size_t _stride;

		std::vector<std::string> arguments;
		// Some initial parameters that can be overriden from command line	
		std::vector<std::string> files, depth_directories, output_video_files, out_dummy;


		Openface2Helper *_helper;
	};

}

#endif
