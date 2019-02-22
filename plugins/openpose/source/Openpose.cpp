// Openpose.cpp
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
//*************************************************************************************************

#include "Openpose.h"
#include "OpenposeHelper.h"
#include <iostream>


namespace ssi {

	char Openpose::ssi_log_name[] = "Openpose__";

	Openpose::Openpose(const ssi_char_t *file)
		: _file(0),
		_helper(0) {
				
		if (file) {
			if (!OptionList::LoadXML(file, &_options)) {
				OptionList::SaveXML(file, &_options);
			}
			_file = ssi_strcpy(file);
		}
		


	}

	Openpose::~Openpose() {

		if (_file) {
			OptionList::SaveXML(_file, &_options);
			delete[] _file;
		}
	}

	void Openpose::transform_enter(ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {

		std::vector<std::string> arguments;
		//arguments.push_back(std::string(_options.modelPath));
		_helper = new OpenposeHelper(_options.modelFolder, _options.netResolution, _options.netHandResolution ,_options.netFaceResolution, _options.numberOfMaxPeople, _options.hand, _options.face);
		

	}

	void Openpose::transform(ITransformer::info info,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {
   
			

		//FOR CALCULATING FPS
		////timer start, calculating seconds
		//if(frameCounter == 0)
		//	timerBegin = std::chrono::high_resolution_clock::now();
		

		// prepare output
		float *out = ssi_pcast(float, stream_out.ptr);
		for (int i = 0; i < stream_out.num*stream_out.dim; i++)
			out[i] = 0.0;

		
		//convert img to Mat so Openpose can do its Magic 
		cv::Mat captured_image;
		cv::Mat keyPointData();
		if (_video_format.numOfChannels == 3) {
			captured_image = cv::Mat(_video_format.heightInPixels, _video_format.widthInPixels, CV_8UC(_video_format.numOfChannels), stream_in.ptr, ssi_video_stride(_video_format));
		}
		else if (_video_format.numOfChannels == 4) {
			cv::Mat temp = cv::Mat(_video_format.heightInPixels, _video_format.widthInPixels, CV_8UC(_video_format.numOfChannels), stream_in.ptr, ssi_video_stride(_video_format));
			cv::cvtColor(temp, captured_image, CV_BGRA2RGB);
		}

		_helper->getKeyPoints(captured_image, out);




		//	//FOR CALCULATING FPS
		//frameCounter++;
		//if (frameCounter >= maxFrame) {
		//	//timer end
		//	const auto now = std::chrono::high_resolution_clock::now();
		//	const auto totalTimeSec = (double)std::chrono::duration_cast<std::chrono::nanoseconds>(now - timerBegin).count()
		//		* 1e-9;
		//	std::cout << "frame " << frameCounter <<", time:  " << totalTimeSec << ", fps: " << (frameCounter / totalTimeSec) << std::endl;
		//	//ssi_print("time: %d \n", totalTimeSec);

		//	//ssi_print("fps: %d \n", totalTimeSec / frameCounter);
		//	frameCounter = 0;
		//}
		

	}

	void Openpose::transform_flush(ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {
		delete _helper; _helper = 0;
	}

}
