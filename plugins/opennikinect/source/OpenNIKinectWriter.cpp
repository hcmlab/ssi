// OpenNIKinectWriter.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/08/20
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

#include "OpenNIKinectWriter.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

OpenNIKinectWriter::OpenNIKinectWriter (const ssi_char_t *file)
	: _fptr (0),
	_file (0) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
}

OpenNIKinectWriter::~OpenNIKinectWriter () {

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}

void OpenNIKinectWriter::consume_enter (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	_fptr = File::CreateAndOpen (File::ASCII, File::WRITE, _options.path);
}

void OpenNIKinectWriter::consume (IConsumer::info consume_info,
	ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	OpenNIKinect::SKELETON *ss = ssi_pcast (OpenNIKinect::SKELETON, stream_in[0].ptr);
	OpenNIKinect::PrintSkeleton (_fptr->getFile (), *ss);
}

void OpenNIKinectWriter::consume_flush (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	delete _fptr; _fptr = 0;
}

}
