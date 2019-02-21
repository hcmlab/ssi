// OpenposeHelper.h
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

#pragma once
// 3rdparty dependencies
// GFlags: DEFINE_bool, _int32, _int64, _uint64, _double, _string

#include <gflags/gflags.h>
/*
// Allow Google Flags in Ubuntu 14
#ifndef GFLAGS_GFLAGS_H_
namespace gflags = google;
#endif*/
// OpenPose dependencies
//#include <openpose/core/headers.hpp>
//#include <openpose/filestream/headers.hpp>
//#include <openpose/gui/headers.hpp>
//#include <openpose/pose/headers.hpp>
//#include <openpose/utilities/headers.hpp>

#include <openpose/headers.hpp>


namespace ssi {

	class OpenposeHelper
	{
	public:
		OpenposeHelper(std::string modelFolder, std::string net_resolution, std::string hand_net_resolution, std::string face_net_resolution, int numberOfMaxPeople, bool hand, bool face);

		OpenposeHelper();
		~OpenposeHelper();

		void initializeOpenposeHelper();

		void getKeyPoints(cv::Mat inputImage, float * out);


	private:

		op::Wrapper<std::vector<op::Datum>> opWrapper{ op::ThreadManagerMode::Asynchronous };

	};


}

