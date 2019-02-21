// Openpose.h
// author: Felix Kickstein felix.kickstein@student.uni-augsburg.de
// created: 14/05/2018
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

#ifndef SSI_SSI_OPENPOSE_OPENPOSE_H
#define SSI_SSI_OPENPOSE_OPENPOSE_H

#include "base/IFilter.h"
#include "ioput/option/OptionList.h"
#include "OpenposeModelInfos.h"
#include <chrono>

namespace ssi {

	class OpenposeHelper;

	class Openpose : public IFilter {

	public:

		class Options : public OptionList {

		public:

			Options() {
				addOption("modelFolder", &modelFolder, SSI_MAX_CHAR, SSI_CHAR, "Folder path (absolute or relative) where the models (pose, face, ...) are located.");
				addOption("numberOfMaxPeople", &numberOfMaxPeople, 1, SSI_INT, "´Set number of Max People");
				addOption("netResolution", &netResolution, SSI_MAX_CHAR, SSI_CHAR, "Multiples of 16. If it is increased, the accuracy potentially increases. If it is"
																					" decreased, the speed increases. For maximum speed-accuracy balance, it should keep the"
																					" closest aspect ratio possible to the images or videos to be processed. Using `-1` in"
																					" any of the dimensions, OP will choose the optimal aspect ratio depending on the user's"
																					" input value. E.g. the default `-1x368` is equivalent to `656x368` in 16:9 resolutions,"
																					" e.g. full HD (1980x1080) and HD (1280x720) resolutions.");
				addOption("netFaceResolution", &netFaceResolution, SSI_MAX_CHAR, SSI_CHAR, "Like netResolution but for faceNet");
				addOption("netHandResolution", &netHandResolution, SSI_MAX_CHAR, SSI_CHAR, "Like netResolution but for handNet");
				addOption("face", &face, 1, SSI_BOOL, "Enable this for face data");
				addOption("hand", &hand, 1, SSI_BOOL, "Enable this for hand data");
			}

			void setModelFolder(const ssi_char_t *path)
			{
				ssi_strcpy(modelFolder, path);
			}

			void setNetResolution(const ssi_char_t *res)
			{
				ssi_strcpy(netResolution, res);
			}
			void setNetFaceResolution(const ssi_char_t *res)
			{
				ssi_strcpy(netFaceResolution, res);
			}
			void setNetHandResolution(const ssi_char_t *res)
			{
				ssi_strcpy(netHandResolution, res);
			}

			int numberOfMaxPeople = 2;
			bool face = false;
			bool hand = false;
			ssi_char_t netResolution[SSI_MAX_CHAR] = "-1x176";
			ssi_char_t netFaceResolution[SSI_MAX_CHAR] = "320x320";
			ssi_char_t netHandResolution[SSI_MAX_CHAR] = "320x320";

			//ssi_char_t netResolution[SSI_MAX_CHAR] = "320x176";
			ssi_char_t modelFolder[SSI_MAX_CHAR] = "models/";
		};

	public:

		static const ssi_char_t *GetCreateName() { return "Openpose"; };
		static IObject *Create(const ssi_char_t *file) { return new Openpose(file); };
		~Openpose();

		Options *getOptions() { return &_options; };
		const ssi_char_t *getName() { return GetCreateName(); };
		
		const ssi_char_t *getInfo() { return "Real-time multi-person keypoint detection library for body, face, and hands estimation."; };
		
		void setMetaData(ssi_size_t size, const void *meta) {
			if (sizeof(_video_format) != size) {
				ssi_err("invalid meta size");
				return;
			}
			memcpy(&_video_format, meta, size);
			_stride = ssi_video_stride(_video_format);
		};

		const void *getMetaData(ssi_size_t &size) {
			//theoretisch müsste noch ein eigenes SSI_SKELETON_TYPE für das BODY_25 Openpose erstellt werden und dazu hinzugefügt werden, ersatzweise steht hier einfch TYPE::SSI,
			//wichtig ist hier nur dass die Anzahl der MaxTrackedPeople also der getrackten Skelette in den Meta Informationen drinnensteht
			_meta_out = ssi_skeleton_meta(SSI_SKELETON_TYPE::SSI, _options.numberOfMaxPeople);
			size = sizeof(_meta_out);
			return &_meta_out;
		}

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
		

			//POSE:  Size: #people x #body parts (e.g. 25 for BODY or 15 for MPI) x 3 ((x,y) coordinates + score)
			ssi_size_t outputdim = (_options.numberOfMaxPeople * OpenposeModelInfos::SKELETON_JOINT_BODY::NUM * OpenposeModelInfos::SKELETON_JOINT_VALUE::NUM);


			//FACE : Size: #people x #face parts (70) x 3 ((x,y) coordinates + score)
			if (_options.face)
				outputdim += (_options.numberOfMaxPeople * OpenposeModelInfos::SKELETON_FACE_MODEL::PARTS *  OpenposeModelInfos::SKELETON_FACE_MODEL::DIM);

			//HAND :  Size each Array: #people x #hand parts (21) x 3 ((x,y) coordinates + score)
			if (_options.hand)
				outputdim += 2 * (_options.numberOfMaxPeople * OpenposeModelInfos::SKELETON_HAND_MODEL::PARTS * OpenposeModelInfos::SKELETON_HAND_MODEL::DIM);

			return outputdim;
		}

		ssi_size_t getSampleBytesOut(ssi_size_t sample_bytes_in) {
			return sizeof(SSI_REAL);
		}

		ssi_type_t getSampleTypeOut(ssi_type_t sample_type_in) {
			return SSI_REAL;
		}

	



	protected:

		Openpose(const ssi_char_t *file = 0);
		ssi_char_t *_file;
		Options _options;
		static char ssi_log_name[];

		ssi_video_params_t _video_format;
		ssi_size_t _stride;


		SSI_SKELETON_META _meta_out;
		//ssi_byte_t _meta_in[256];
		//ssi_byte_t *_meta_in_ptr;
		//ssi_size_t _meta_received;

		std::vector<std::string> arguments;
		// Some initial parameters that can be overriden from command line	
		std::vector<std::string> files, depth_directories, output_video_files, out_dummy;

		OpenposeHelper *_helper;



		//// for calulating fps
		//std::chrono::high_resolution_clock::time_point timerBegin ;
		//const int maxFrame = 20;
		//int frameCounter = 0;

	};

}

#endif
