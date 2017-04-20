// SkeletonPainter.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/11/22
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

#include "SkeletonPainter.h"
#include "ssiocv.h"

namespace ssi {

ssi_char_t *SkeletonPainter::ssi_log_name = "skeletonp_";

SkeletonPainter::SkeletonPainter(const ssi_char_t *file)
	: _file(0),
	_image(0),
	_min_value_y (0),
	_max_value_y (0),
	_min_value_x (0),
	_max_value_x (0),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
}

SkeletonPainter::~SkeletonPainter () {

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}

void SkeletonPainter::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {
	
	ssi_video_params (_format, _options.width, _options.height, 0, 8, 3);
	_image = cvCreateImage (cvSize (_format.widthInPixels, _format.heightInPixels), _format.depthInBitsPerChannel, _format.numOfChannels);

	_n_skeletons = stream_in.dim / (SSI_SKELETON_JOINT_VALUE::NUM * SSI_SKELETON_JOINT::NUM);

	_min_value_y = new SSI_SKELETON_VALUE_TYPE[_n_skeletons];
	_max_value_y = new SSI_SKELETON_VALUE_TYPE[_n_skeletons];
	_min_value_x = new SSI_SKELETON_VALUE_TYPE[_n_skeletons];
	_max_value_x = new SSI_SKELETON_VALUE_TYPE[_n_skeletons];
	for (ssi_size_t i = 0; i < _n_skeletons; i++) {
		_min_value_x[i] = _min_value_y[i] = FLT_MAX;
		_max_value_x[i] = _max_value_y[i] = -FLT_MAX;
	}
}

void SkeletonPainter::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {
		
	memset (_image->imageData, 0, _image->imageSize);

	SSI_SKELETON *pskel =  ssi_pcast (SSI_SKELETON, stream_in.ptr);
	for (ssi_size_t i = 0; i < _n_skeletons; i++) {
		paintSkeleton (_image, *pskel++, i);
	}

	memcpy (stream_out.ptr, _image->imageData, stream_out.tot);	
}

void SkeletonPainter::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	cvReleaseImage (&_image);

	delete[] _min_value_y; _min_value_y = 0;
	delete[] _max_value_y; _max_value_y = 0;
	delete[] _min_value_x; _min_value_x = 0;
	delete[] _max_value_x; _max_value_x = 0;
}

void SkeletonPainter::paintBone (IplImage *image, 
	SSI_SKELETON &skel,
	CvPoint *points,  
	SSI_SKELETON_JOINT::List joint0, 
	SSI_SKELETON_JOINT::List joint1,
	bool ishead) {

    float joint0State = skel[joint0][SSI_SKELETON_JOINT_VALUE::POS_CONF];
	float joint1State = skel[joint1][SSI_SKELETON_JOINT_VALUE::POS_CONF];

	float POSITION_NOT_TRACKED = 0.25f;
	float POSITION_INFERRED = 0.75f;
	float POSITION_TRACKED = 1.25f;

    // If we can't find either of these joints, exit
    if (joint0State < POSITION_NOT_TRACKED || joint1State < POSITION_NOT_TRACKED)
    {
        return;
    }
    
    // Don't draw if both points are inferred
    if (joint0State < POSITION_INFERRED && joint1State < POSITION_INFERRED)
    {
        return;
    } 	

    // We assume all drawn bones are inferred unless BOTH joints are tracked
    if (joint0State > POSITION_INFERRED && joint1State > POSITION_INFERRED)
    {
		if(!ishead)
		cvDrawLine (image, points[joint0], points[joint1], CV_RGB (0, 255, 0), 2);
		else 
		cvDrawLine (image, points[joint0], points[joint1], CV_RGB (255, 255, 0), 1);
    }
    else
    {
		cvDrawLine (image, points[joint0], points[joint1], CV_RGB (255, 255, 255), 1);
    }
}

void SkeletonPainter::paintSkeleton (IplImage *image,
	SSI_SKELETON &skel,
	ssi_size_t index) {

	float POSITION_NOT_TRACKED = 0.25f;
	float POSITION_INFERRED = 0.75f;
	float POSITION_TRACKED = 1.25f;

	CvPoint points[SSI_SKELETON_JOINT::NUM];

	for (int i = 0; i < SSI_SKELETON_JOINT::NUM; i++) {
		if (skel[i][SSI_SKELETON_JOINT_VALUE::POS_X] != SSI_SKELETON_INVALID_JOINT_VALUE && _min_value_x[index] > skel[i][SSI_SKELETON_JOINT_VALUE::POS_X] && skel[i][SSI_SKELETON_JOINT_VALUE::POS_X] > MIN_ALLOWED_VALUE) {
			_min_value_x[index] = skel[i][SSI_SKELETON_JOINT_VALUE::POS_X];
		}
		if (skel[i][SSI_SKELETON_JOINT_VALUE::POS_X] != SSI_SKELETON_INVALID_JOINT_VALUE && _max_value_x[index] < skel[i][SSI_SKELETON_JOINT_VALUE::POS_X] && skel[i][SSI_SKELETON_JOINT_VALUE::POS_X] < MAX_ALLOWED_VALUE) {
			_max_value_x[index] = skel[i][SSI_SKELETON_JOINT_VALUE::POS_X];
		}
		if (skel[i][SSI_SKELETON_JOINT_VALUE::POS_Y] != SSI_SKELETON_INVALID_JOINT_VALUE && _min_value_y[index] > skel[i][SSI_SKELETON_JOINT_VALUE::POS_Y] && skel[i][SSI_SKELETON_JOINT_VALUE::POS_X] > MIN_ALLOWED_VALUE) {
			_min_value_y[index] = skel[i][SSI_SKELETON_JOINT_VALUE::POS_Y];
		}
		if (skel[i][SSI_SKELETON_JOINT_VALUE::POS_Y] != SSI_SKELETON_INVALID_JOINT_VALUE && _max_value_y[index] < skel[i][SSI_SKELETON_JOINT_VALUE::POS_Y] && skel[i][SSI_SKELETON_JOINT_VALUE::POS_X] < MAX_ALLOWED_VALUE) {
			_max_value_y[index] = skel[i][SSI_SKELETON_JOINT_VALUE::POS_Y];
		}
	}

	if (_min_value_x[index] == SSI_SKELETON_INVALID_JOINT_VALUE || _max_value_x[index] == SSI_SKELETON_INVALID_JOINT_VALUE || _min_value_x[index] == _max_value_x[index] ||
		_min_value_y[index] == SSI_SKELETON_INVALID_JOINT_VALUE || _max_value_y[index] == SSI_SKELETON_INVALID_JOINT_VALUE || _min_value_y[index] == _max_value_y[index]) {
		return;
	}


	int width = _format.widthInPixels;
	int height = _format.heightInPixels;

	for (int i = 0; i < SSI_SKELETON_JOINT::NUM; i++)
	{
		points[i].x = ssi_cast (int, ((skel[i][SSI_SKELETON_JOINT_VALUE::POS_X] - _min_value_x[index]) / (_max_value_x[index] - _min_value_x[index])) * width + 0.5f);
		points[i].y = height - ssi_cast (int, ((skel[i][SSI_SKELETON_JOINT_VALUE::POS_Y] - _min_value_y[index]) / (_max_value_y[index] - _min_value_y[index])) * height + 0.5f);
	}
	


	

	



	paintBone (image, skel, points, SSI_SKELETON_JOINT::HEAD,  SSI_SKELETON_JOINT::NECK);

    // Render Torso
   // DrawBone(image, skel, m_Points, POSITION_HEAD, POSITION_SHOULDER_CENTER);  //Dont Draw, cause we have the face
	
	paintBone (image, skel, points, SSI_SKELETON_JOINT::NECK,  SSI_SKELETON_JOINT::LEFT_SHOULDER);
    paintBone (image, skel, points, SSI_SKELETON_JOINT::NECK, SSI_SKELETON_JOINT::RIGHT_SHOULDER);
    paintBone (image, skel, points, SSI_SKELETON_JOINT::NECK, SSI_SKELETON_JOINT::TORSO);
    paintBone (image, skel, points, SSI_SKELETON_JOINT::TORSO, SSI_SKELETON_JOINT::WAIST);
    paintBone (image, skel, points, SSI_SKELETON_JOINT::WAIST, SSI_SKELETON_JOINT::LEFT_HIP);
    paintBone (image, skel, points, SSI_SKELETON_JOINT::WAIST, SSI_SKELETON_JOINT::RIGHT_HIP);

    // Left Arm
    paintBone (image, skel, points, SSI_SKELETON_JOINT::LEFT_SHOULDER, SSI_SKELETON_JOINT::LEFT_ELBOW);
    paintBone (image, skel, points, SSI_SKELETON_JOINT::LEFT_ELBOW, SSI_SKELETON_JOINT::LEFT_WRIST);
    paintBone (image, skel, points, SSI_SKELETON_JOINT::LEFT_WRIST, SSI_SKELETON_JOINT::LEFT_HAND);

    // Right Arm
    paintBone (image, skel, points, SSI_SKELETON_JOINT::RIGHT_SHOULDER, SSI_SKELETON_JOINT::RIGHT_ELBOW);
    paintBone (image, skel, points, SSI_SKELETON_JOINT::RIGHT_ELBOW, SSI_SKELETON_JOINT::RIGHT_WRIST);
    paintBone (image, skel, points, SSI_SKELETON_JOINT::RIGHT_WRIST, SSI_SKELETON_JOINT::RIGHT_HAND);

    // Left Leg
    paintBone (image, skel, points, SSI_SKELETON_JOINT::LEFT_HIP, SSI_SKELETON_JOINT::LEFT_KNEE);
    paintBone (image, skel, points, SSI_SKELETON_JOINT::LEFT_KNEE, SSI_SKELETON_JOINT::LEFT_ANKLE);
    paintBone (image, skel, points, SSI_SKELETON_JOINT::LEFT_ANKLE, SSI_SKELETON_JOINT::LEFT_FOOT);

    // Right Leg
    paintBone (image, skel, points, SSI_SKELETON_JOINT::RIGHT_HIP, SSI_SKELETON_JOINT::RIGHT_KNEE);
    paintBone (image, skel, points, SSI_SKELETON_JOINT::RIGHT_KNEE, SSI_SKELETON_JOINT::RIGHT_ANKLE);
    paintBone (image, skel, points, SSI_SKELETON_JOINT::RIGHT_ANKLE, SSI_SKELETON_JOINT::RIGHT_FOOT);


	//Face (if available)
	//paintBone (image, skel, points, SSI_SKELETON_JOINT::HEAD,  SSI_SKELETON_JOINT::FACE_FOREHEAD,true);
	paintBone (image, skel, points, SSI_SKELETON_JOINT::FACE_FOREHEAD,  SSI_SKELETON_JOINT::FACE_RIGHT_EAR,true);
	paintBone (image, skel, points, SSI_SKELETON_JOINT::FACE_FOREHEAD,  SSI_SKELETON_JOINT::FACE_LEFT_EAR,true);
	paintBone (image, skel, points, SSI_SKELETON_JOINT::FACE_CHIN,  SSI_SKELETON_JOINT::FACE_RIGHT_EAR,true);
	paintBone (image, skel, points, SSI_SKELETON_JOINT::FACE_CHIN,  SSI_SKELETON_JOINT::FACE_LEFT_EAR,true);
	paintBone (image, skel, points, SSI_SKELETON_JOINT::FACE_CHIN,  SSI_SKELETON_JOINT::FACE_NOSE,true);
	paintBone (image, skel, points, SSI_SKELETON_JOINT::FACE_FOREHEAD,  SSI_SKELETON_JOINT::FACE_NOSE,true);

    
    // Draw the joints in a different color
	CvScalar color = CV_RGB (255, (index+1) * (255/_n_skeletons), 0);
    for (int i = 0; i < SSI_SKELETON_JOINT::NUM-5; ++i) {       
		if (skel[i][SSI_SKELETON_JOINT_VALUE::POS_CONF] > POSITION_NOT_TRACKED) {
			if (skel[i][SSI_SKELETON_JOINT_VALUE::POS_CONF] > POSITION_INFERRED) {
				cvDrawCircle (image, points[i], 2, color, 2);
			} else {
				cvDrawCircle (image, points[i], 2, CV_RGB (255,255,255), 2);
			}
		}
	}

	//Draw Face Joints a little smaller
	 for (int i = SSI_SKELETON_JOINT::NUM-5; i <SSI_SKELETON_JOINT::NUM ; ++i) {       
		if (skel[i][SSI_SKELETON_JOINT_VALUE::POS_CONF] > POSITION_NOT_TRACKED) {
			if (skel[i][SSI_SKELETON_JOINT_VALUE::POS_CONF] > POSITION_INFERRED) {
				cvDrawCircle (image, points[i], 1, color, 1);
			} else {
				cvDrawCircle (image, points[i], 1, CV_RGB (255,255,255), 1);
			}
		}
	}
}

}
 
