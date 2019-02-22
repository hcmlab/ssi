// OpenposePainter.cpp
// author: Felix Kickstein felix.kickstein@student.uni-augsburg.de
// created: 14/05/2018
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

#include "OpenposePainter.h"
#include "OpenposeModelInfos.h"

#include <gflags/gflags.h>

// See all the available parameter options withe the `--help` flag. E.g. `build/examples/openpose/openpose.bin --help`
// Note: This command will show you flags for other unnecessary 3rdparty files. Check only the flags for the OpenPose
// executable. E.g. for `openpose.bin`, look for `Flags from examples/openpose/openpose.cpp:`.
// Debugging/Other
DECLARE_int32(logging_level);
DECLARE_bool(disable_multi_thread);
DECLARE_int32(profile_speed);
// Producer
DECLARE_string(image_dir);
DECLARE_double(camera_fps);
// OpenPose
DECLARE_string(model_folder);
DECLARE_string(output_resolution);
DECLARE_int32(num_gpu);
DECLARE_int32(num_gpu_start);
DECLARE_int32(keypoint_scale);
DECLARE_int32(number_people_max);
// OpenPose Body Pose
DECLARE_bool(body_disable);
DECLARE_string(model_pose);
DECLARE_string(net_resolution);
DECLARE_int32(scale_number);
DECLARE_double(scale_gap);
// OpenPose Body Pose Heatmaps and Part Candidates
DECLARE_bool(heatmaps_add_parts);
DECLARE_bool(heatmaps_add_bkg);
DECLARE_bool(heatmaps_add_PAFs);
DECLARE_int32(heatmaps_scale);
DECLARE_bool(part_candidates);
// OpenPose Face
DECLARE_bool(face);
DECLARE_string(face_net_resolution);
// OpenPose Hand
DECLARE_bool(hand);
DECLARE_string(hand_net_resolution);
DECLARE_int32(hand_scale_number);
DECLARE_double(hand_scale_range);
DECLARE_bool(hand_tracking);
// OpenPose 3-D Reconstruction
DECLARE_bool(3d);
DECLARE_int32(3d_min_views);
DECLARE_int32(3d_views);
// OpenPose Rendering
DECLARE_int32(part_to_show);
DECLARE_bool(disable_blending);
// OpenPose Rendering Pose
DECLARE_double(render_threshold);
DECLARE_int32(render_pose);
DECLARE_double(alpha_pose);
DECLARE_double(alpha_heatmap);
// OpenPose Rendering Face
DECLARE_double(face_render_threshold);
DECLARE_int32(face_render);
DECLARE_double(face_alpha_pose);
DECLARE_double(face_alpha_heatmap);
// OpenPose Rendering Hand
DECLARE_double(hand_render_threshold);
DECLARE_int32(hand_render);
DECLARE_double(hand_alpha_pose);
DECLARE_double(hand_alpha_heatmap);
// Result Saving
DECLARE_string(write_images);
DECLARE_string(write_images_format);
DECLARE_string(write_video);
DECLARE_string(write_json);
DECLARE_string(write_coco_json);
DECLARE_string(write_heatmaps);
DECLARE_string(write_heatmaps_format);
DECLARE_string(write_keypoint);
DECLARE_string(write_keypoint_format);
DECLARE_string(write_keypoint_json);


namespace ssi {

	char OpenposePainter::ssi_log_name[] = "OpenposePainter__";

	OpenposePainter::OpenposePainter(const ssi_char_t *file)
		: _file(0) {

		if (file) {
			if (!OptionList::LoadXML(file, &_options)) {
				OptionList::SaveXML(file, &_options);
			}
			_file = ssi_strcpy(file);
		}


	}

	OpenposePainter::~OpenposePainter() {

		if (_file) {
			OptionList::SaveXML(_file, &_options);
			delete[] _file;
		}
	}

	void OpenposePainter::transform_enter(ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {

		std::vector<std::string> arguments;

		// Applying user defined configuration - Google flags to program variables
		// outputSize
		const auto outputSize = op::flagsToPoint(FLAGS_output_resolution, "-1x-1");
		// netInputSize
		const auto netInputSize = op::flagsToPoint(FLAGS_net_resolution, "-1x368");
		// faceNetInputSize
		const auto faceNetInputSize = op::flagsToPoint(FLAGS_face_net_resolution, "368x368 (multiples of 16)");
		// handNetInputSize
		const auto handNetInputSize = op::flagsToPoint(FLAGS_hand_net_resolution, "368x368 (multiples of 16)");
		// poseModel
		const auto poseModel = op::flagsToPoseModel(FLAGS_model_pose);
		// JSON saving
		//const auto writeJson = (!FLAGS_write_json.empty() ? FLAGS_write_json : FLAGS_write_keypoint_json);
		//if (!FLAGS_write_keypoint.empty() || !FLAGS_write_keypoint_json.empty())
		//	op::log("Flags `write_keypoint` and `write_keypoint_json` are deprecated and will eventually be removed."
		//		" Please, use `write_json` instead.", op::Priority::Max);
		// keypointScale
		const auto keypointScale = op::flagsToScaleMode(FLAGS_keypoint_scale);
		// heatmaps to add
		const auto heatMapTypes = op::flagsToHeatMaps(FLAGS_heatmaps_add_parts, FLAGS_heatmaps_add_bkg,
			FLAGS_heatmaps_add_PAFs);
		const auto heatMapScale = op::flagsToHeatMapScaleMode(FLAGS_heatmaps_scale);

		//// >1 camera view?
		//const auto multipleView = (FLAGS_3d || FLAGS_3d_views > 1);
		//// Enabling Google Logging
		//const bool enableGoogleLogging = true;
		// Logging
		//op::log("", op::Priority::Low, __LINE__, __FUNCTION__, __FILE__);



		/*
		POSE renderer
		*/

		poseRenderer = new op::PoseCpuRenderer{ poseModel, (float)FLAGS_render_threshold, !FLAGS_disable_blending,
			(float)FLAGS_alpha_pose };
		// Step 4 - Initialize resources on desired thread (in this case single thread, i.e. we init resources here)
		poseRenderer->initializationOnThread();




		/*
		HAND renderer
		*/

		if (FLAGS_hand) {

			handRenderer = new op::HandCpuRenderer{ (float)FLAGS_render_threshold };
			// Step 4 - Initialize resources on desired thread (in this case single thread, i.e. we init resources here)
			handRenderer->initializationOnThread();
		}

		/*
		FACE renderer
		*/

		if (FLAGS_face) {

			faceRenderer = new op::FaceCpuRenderer{ (float)FLAGS_render_threshold };
			// Step 4 - Initialize resources on desired thread (in this case single thread, i.e. we init resources here)
			faceRenderer->initializationOnThread();
		}
	}

	void OpenposePainter::transform(ITransformer::info info,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {

		float *xtra_in = ssi_pcast(float, xtra_stream_in[0].ptr);

		cv::Mat captured_image;

		int sizeModelBodyParts = OpenposeModelInfos::SKELETON_JOINT_BODY::NUM;

		if (_video_format.numOfChannels == 3) {
			captured_image = cv::Mat(_video_format.heightInPixels, _video_format.widthInPixels, CV_8UC(_video_format.numOfChannels), stream_in.ptr, ssi_video_stride(_video_format));
		}
		else if (_video_format.numOfChannels == 4) {
			cv::Mat temp = cv::Mat(_video_format.heightInPixels, _video_format.widthInPixels, CV_8UC(_video_format.numOfChannels), stream_in.ptr, ssi_video_stride(_video_format));
			cv::cvtColor(temp, captured_image, CV_BGRA2RGB);
		}

		//calculate number of tracked People, pose is always active and cannot be disabled
		int numberOfTrackedPeople = FLAGS_number_people_max;

		op::Array<float> poseKeypoints({ numberOfTrackedPeople,sizeModelBodyParts, OpenposeModelInfos::SKELETON_JOINT_VALUE::NUM });

		op::Array<float> faceKeypoints({ numberOfTrackedPeople,OpenposeModelInfos::SKELETON_FACE_MODEL::PARTS, OpenposeModelInfos::SKELETON_FACE_MODEL::DIM });


		op::Array<float> leftHand = op::Array<float>({ numberOfTrackedPeople, OpenposeModelInfos::SKELETON_HAND_MODEL::PARTS, OpenposeModelInfos::SKELETON_HAND_MODEL::DIM });
		op::Array<float> rightHand = op::Array<float>({ numberOfTrackedPeople,OpenposeModelInfos::SKELETON_HAND_MODEL::PARTS, OpenposeModelInfos::SKELETON_HAND_MODEL::DIM });

		for (int i = 0; i < numberOfTrackedPeople * sizeModelBodyParts* OpenposeModelInfos::SKELETON_JOINT_VALUE::NUM; ++i) {
			poseKeypoints[i] = xtra_in[i];
		}

		if (FLAGS_face) {
			int baseFace = numberOfTrackedPeople * sizeModelBodyParts* OpenposeModelInfos::SKELETON_JOINT_VALUE::NUM;
			for (int i = 0; i < numberOfTrackedPeople * OpenposeModelInfos::SKELETON_FACE_MODEL::PARTS* OpenposeModelInfos::SKELETON_FACE_MODEL::DIM; ++i) {
				faceKeypoints[i] = xtra_in[i + baseFace];
			}
		}

		if (FLAGS_hand) {
			int baseHand = numberOfTrackedPeople * OpenposeModelInfos::SKELETON_JOINT_BODY::NUM * OpenposeModelInfos::SKELETON_JOINT_VALUE::NUM; //pose
			if (FLAGS_face) baseHand += numberOfTrackedPeople * OpenposeModelInfos::SKELETON_FACE_MODEL::PARTS * OpenposeModelInfos::SKELETON_FACE_MODEL::DIM; //face

			for (int i = 0; i < numberOfTrackedPeople * OpenposeModelInfos::SKELETON_HAND_MODEL::PARTS * OpenposeModelInfos::SKELETON_HAND_MODEL::DIM; ++i) {
				leftHand[i] = xtra_in[i + baseHand];
			}

			baseHand += numberOfTrackedPeople * OpenposeModelInfos::SKELETON_HAND_MODEL::PARTS * OpenposeModelInfos::SKELETON_HAND_MODEL::DIM;

			for (int i = 0; i < numberOfTrackedPeople * OpenposeModelInfos::SKELETON_HAND_MODEL::PARTS * OpenposeModelInfos::SKELETON_HAND_MODEL::DIM; ++i) {
				rightHand[i] = xtra_in[i + baseHand];
			}
		}

		std::array<op::Array<float>, 2> handKeypoints = { leftHand ,rightHand };

		// ------------------------- POSE ESTIMATION AND RENDERING -------------------------
		// Step 1 - Read and load image, error if empty (possibly wrong path)
		// Alternative: cv::imread(FLAGS_image_path, CV_LOAD_IMAGE_COLOR);
		const op::Point<int> imageSize{ captured_image.cols, captured_image.rows };
		// Step 2 - Get desired scale sizes
		std::vector<double> scaleInputToNetInputs;
		std::vector<op::Point<int>> netInputSizes;
		double scaleInputToOutput;
		op::Point<int> outputResolution;

		// Step 2 - Read Google flags (user defined configuration)
		//outputSize
		const auto outputSize = op::flagsToPoint(FLAGS_output_resolution, "-1x-1");
		// netInputSize
		const auto netInputSize = op::flagsToPoint(FLAGS_net_resolution, "-1x368");

		scaleAndSizeExtractor = new op::ScaleAndSizeExtractor(netInputSize, outputSize, FLAGS_scale_number, FLAGS_scale_gap);

		std::tie(scaleInputToNetInputs, netInputSizes, scaleInputToOutput, outputResolution)
			= scaleAndSizeExtractor->extract(imageSize);
		// Step 3 - Format input image to OpenPose input and output formats

		auto outputArray = cvMatToOpOutput->createArray(captured_image, scaleInputToOutput, outputResolution);

		//POSE RENDERING

		// Step 5 - Render poseKeypoints
		poseRenderer->renderPose(outputArray, poseKeypoints, scaleInputToOutput);
		

		//FACE RENDERING

		if (FLAGS_face) {

			// Step 5 - Render poseKeypoints
			faceRenderer->renderFace(outputArray, faceKeypoints, scaleInputToOutput);

		}


		//HAND RENDERING
		if (FLAGS_hand) {

			// Step 5 - Render poseKeypoints
			handRenderer->renderHand(outputArray, handKeypoints, scaleInputToOutput);

		}

		// Step 6 - OpenPose output format to cv::Mat
		auto outputImage = opOutputToCvMat->formatToCvMat(outputArray);


		//COPY image in output stream
		memcpy(stream_out.ptr, outputImage.data, stream_in.tot);
	}

	void OpenposePainter::transform_flush(ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {
	}

}
