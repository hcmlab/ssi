// MicrosoftKinectPainter.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/06/23
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

#include "MicrosoftKinectPainter.h"
#include "KinectUtils.h"

namespace ssi {

ssi_char_t *MicrosoftKinectPainter::ssi_log_name = "mickinectp";

MicrosoftKinectPainter::MicrosoftKinectPainter (const ssi_char_t *file) 
	: _file (0),
	_skeleton_index (-1),
	_facepoints_index (-1),
	_n_skeletons (0),
	_n_faces (0),
	_israwdepth (false),
	_old_skeleton (false),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
}

MicrosoftKinectPainter::~MicrosoftKinectPainter () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

void MicrosoftKinectPainter::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	_skeleton_index = -1;
	for (ssi_size_t i = 0; i < xtra_stream_in_num; i++) {
		if (xtra_stream_in[i].dim % (MicrosoftKinect::SKELETON_JOINT_VALUE::NUM * MicrosoftKinect::SKELETON_JOINT::NUM) == 0)
		{
			_skeleton_index = i;
			_n_skeletons = xtra_stream_in[i].dim / (MicrosoftKinect::SKELETON_JOINT_VALUE::NUM * MicrosoftKinect::SKELETON_JOINT::NUM);
			ssi_msg (SSI_LOG_LEVEL_DETAIL, "found %u skeleton(s) in stream '%u'", _n_skeletons, _skeleton_index);
			_old_skeleton = false;
			break;
		} else if (xtra_stream_in[i].dim % (MicrosoftKinect::SKELETON_JOINT_VALUE_OLD::NUM * MicrosoftKinect::SKELETON_JOINT::NUM) == 0) {
			_skeleton_index = i;
			_n_skeletons = xtra_stream_in[i].dim / (MicrosoftKinect::SKELETON_JOINT_VALUE_OLD::NUM * MicrosoftKinect::SKELETON_JOINT::NUM);
			ssi_msg (SSI_LOG_LEVEL_DETAIL, "found %u old skeleton(s) in stream '%u'", _n_skeletons, _skeleton_index);
			_old_skeleton = true;
			break;
		}
	}

	_facepoints_index = -1;
	for (ssi_size_t i = 0; i < xtra_stream_in_num; i++) {
		if (xtra_stream_in[i].dim % (MicrosoftKinect::FACEPOINT_VALUE::NUM * MicrosoftKinect::FACEPOINT::NUM) == 0) {
			_facepoints_index = i;
			_n_faces = xtra_stream_in[i].dim / (MicrosoftKinect::FACEPOINT_VALUE::NUM * MicrosoftKinect::FACEPOINT::NUM);
			ssi_msg (SSI_LOG_LEVEL_DETAIL, "found %u face(s) in stream '%u'", _n_faces, _facepoints_index);
			break;
		}
	}
}

void MicrosoftKinectPainter::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {	

	MicrosoftKinect::FACEPOINTS *pface = 0;
	if (_facepoints_index >= 0) {
		pface = ssi_pcast (MicrosoftKinect::FACEPOINTS, xtra_stream_in[_facepoints_index].ptr);
	}

	ssi_byte_t *pimgsrc = stream_in.ptr;
	ssi_byte_t *pimgdst = stream_out.ptr;
	ssi_size_t imgsize = ssi_video_size (_video_format);

	for (ssi_size_t i = 0; i < stream_in.num; i++) {

		if (_israwdepth) { // convert to grayscale
			KinectUtils::depthRaw2Image (pimgsrc, pimgdst, _video_format, _raw_format);
		} else { // otherwise just copy
			memcpy (pimgdst, pimgsrc, imgsize);
		}
		
		IFTImage *pImage = FTCreateImage();	
		if(!pImage)
		{
			ssi_wrn ("FTCreateImage () failed");
			return;
		}
		int stride = ssi_video_stride (_video_format);

		switch (_video_format.numOfChannels) {
			case 4:
				pImage->Attach (_video_format.widthInPixels, _video_format.heightInPixels, pimgdst, FTIMAGEFORMAT_UINT8_B8G8R8X8, stride);
				break;
			case 3:
				pImage->Attach (_video_format.widthInPixels, _video_format.heightInPixels, pimgdst, FTIMAGEFORMAT_UINT8_R8G8B8, stride);
				break;
			case 1:
				pImage->Attach (_video_format.widthInPixels, _video_format.heightInPixels, pimgdst, FTIMAGEFORMAT_UINT8_GR8, stride);
		}

		if (_skeleton_index >= 0) {

			if (!_old_skeleton) {
			
				MicrosoftKinect::SKELETON *pskel = ssi_pcast (MicrosoftKinect::SKELETON, xtra_stream_in[_skeleton_index].ptr);

				// draw skeleton
				if(_options.showskeleton)
				{
					if (pskel) {
						for (ssi_size_t j = 0; j < _n_skeletons; j++) {		
							KinectUtils::paintSkeleton (pImage, *pskel, _options.scaled, j == 0 ? 0x0000FF00 : 0x00ffA500);
							pskel++;							
						}
					}
				}

			} else {

				MicrosoftKinect::SKELETON_OLD *pskel = ssi_pcast (MicrosoftKinect::SKELETON_OLD, xtra_stream_in[_skeleton_index].ptr);

				// draw skeleton
				if(_options.showskeleton)
				{
					if (pskel) {
						for (ssi_size_t j = 0; j < _n_skeletons; j++) {		
							KinectUtils::paintSkeleton (pImage, *pskel, _options.scaled, j == 0 ? 0x0000FF00 : 0x00ffA500);
							pskel++;							
						}
					}
				}
			}
		}

		// draw face points
		if(_options.showface)
		{
			if (pface) {
				for (ssi_size_t j = 0; j < _n_faces; j++) {		
					KinectUtils::paintFacePoints (pImage, *pface, _options.scaled);
					pface++;
				}
			}
		}
		
		pimgsrc += imgsize;
		pimgdst += imgsize;
	}
}

void MicrosoftKinectPainter::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {
}

}
