// Openface.h
// author: Johannes Wagner <wagner@hcm-lab.de>
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

#ifndef SSI_OPENFACE_OPENFACE_H
#define SSI_OPENFACE_OPENFACE_H

#include "base/IFilter.h"
#include "ioput/option/OptionList.h"


namespace ssi {

	class OpenfaceHelper;

	class Openface : public IFilter {

	public:

		class Options : public OptionList {

		public:

			Options() {

				setModelPath(".\\");
				setAuPath(".\\");
				setTriPath(".\\");
				poscam = true;
				posworld = true;
				corrposcam = true;
				corrposworld = true;
				landmarks = true;
				gaze = true;
				actionunits = true;

				addOption("modelPath", modelPath, SSI_MAX_CHAR, SSI_CHAR, "Path to model");
				addOption("auPath", AuPath, SSI_MAX_CHAR, SSI_CHAR, "Path to action units");
				addOption("triPath", TriPath, SSI_MAX_CHAR, SSI_CHAR, "Path to triangulation");
				addOption("poscam", &poscam,1, SSI_BOOL, "Calculate camera position");
				addOption("posworld", &posworld, 1, SSI_BOOL, "Calculate world position");
				addOption("corrposcam", &corrposcam, 1, SSI_BOOL, "Calculate corrected camera position");
				addOption("corrposworld", &corrposworld, 1, SSI_BOOL, "Calculate correted world position");
				addOption("landmarks", &landmarks, 1, SSI_BOOL, "Calculate landmarks");
				addOption("gaze", &gaze, 1, SSI_BOOL, "Calculate gaze direction and pupil position");				
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

			void setTriPath(const ssi_char_t *string) {
				this->TriPath[0] = '\0';
				if (string) {
					ssi_strcpy(this->TriPath, string);
				}
			}

			ssi_char_t modelPath[SSI_MAX_CHAR], AuPath[SSI_MAX_CHAR], TriPath[SSI_MAX_CHAR];
			bool poscam, posworld, corrposcam, corrposworld, landmarks, gaze, actionunits;
		};

	public:

		static const ssi_char_t *GetCreateName() { return "Openface"; };
		static IObject *Create(const ssi_char_t *file) { return new Openface(file); };
		~Openface();

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
				POSE_CAMERA_X,
				POSE_CAMERA_Y,
				POSE_CAMERA_Z,
				POSE_CAMERA_ROT_X,
				POSE_CAMERA_ROT_Y,
				POSE_CAMERA_ROT_Z,
				POSE_WORLD_X,
				POSE_WORLD_Y,
				POSE_WORLD_Z,
				POSE_WORLD_ROT_X,
				POSE_WORLD_ROT_Y,
				POSE_WORLD_ROT_Z,
				CORRECTED_POSE_CAMERA_X,
				CORRECTED_POSE_CAMERA_Y,
				CORRECTED_POSE_CAMERA_Z,
				CORRECTED_POSE_CAMERA_ROT_X,
				CORRECTED_POSE_CAMERA_ROT_Y,
				CORRECTED_POSE_CAMERA_ROT_Z,
				CORRECTED_POSE_WORLD_X,
				CORRECTED_POSE_WORLD_Y,
				CORRECTED_POSE_WORLD_Z,
				CORRECTED_POSE_WORLD_ROT_X,
				CORRECTED_POSE_WORLD_ROT_Y,
				CORRECTED_POSE_WORLD_ROT_Z,
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
				GAZE_LEFT_EYE_X,
				GAZE_LEFT_EYE_Y,
				GAZE_LEFT_EYE_Z,
				GAZE_RIGHT_EYE_X,
				GAZE_RIGHT_EYE_Y,
				GAZE_RIGHT_EYE_Z,
				PUPIL_LEFT_EYE_X,
				PUPIL_LEFT_EYE_Y,
				PUPIL_LEFT_EYE_Z,
				PUPIL_RIGHT_EYE_X,
				PUPIL_RIGHT_EYE_Y,
				PUPIL_RIGHT_EYE_Z,
				AU_REG_DETECTION_SUCCESS,
				AU01_r,
				AU02_r,
				AU04_r,
				AU05_r,
				AU06_r,
				AU09_r,
				AU10_r,
				AU12_r,
				AU14_r,
				AU15_r,
				AU17_r,
				AU20_r,
				AU25_r,
				AU26_r,
				AU_CLASS_DETECTION_SUCCESS,
				AU04_c,
				AU12_c,
				AU15_c,
				AU23_c,
				AU28_c,
				AU45_c,
				NUM
			};
		};

	protected:

		Openface(const ssi_char_t *file = 0);
		ssi_char_t *_file;
		Options _options;
		static char ssi_log_name[];

		ssi_video_params_t _video_format;
		ssi_size_t _stride;

		std::vector<std::string> arguments;
		// Some initial parameters that can be overriden from command line	
		std::vector<std::string> files, depth_directories, output_video_files, out_dummy;


		OpenfaceHelper *_helper;

	};

}

#endif
