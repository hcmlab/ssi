// OpenNIKinectWriter.h
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

#pragma once

#ifndef SSI_OPENNIKINECT_WRITER_H
#define SSI_OPENNIKINECT_WRITER_H

#include "base/IConsumer.h"
#include "OpenNIKinect.h"
#include "ioput/option/OptionList.h"
#include "ioput/file/File.h"

namespace ssi {

class OpenNIKinectWriter : public IConsumer {


public:

	class Options : public OptionList {

	public:

		Options () {

			path[0] = '\0';			
			addOption ("path", path, SSI_MAX_CHAR, SSI_CHAR, "file path (empty for stdout)");
		};

		void setPath (const ssi_char_t *path) {
			this->path[0] = '\0';			
			if (path) { ssi_strcpy (this->path, path); }			
		}

		ssi_char_t path[SSI_MAX_CHAR];
	};

public: 

	static const ssi_char_t *GetCreateName () { return "OpenNIKinectWriter"; };
	static IObject *Create (const ssi_char_t *file) { return new OpenNIKinectWriter (file); };
	~OpenNIKinectWriter ();
	OpenNIKinectWriter::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "writes kinect skeleton joints to a file"; };

	void consume_enter (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);
	void consume (IConsumer::info consume_info,
		ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);
	void consume_flush (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);

private:

	OpenNIKinectWriter (const ssi_char_t *file = 0);
	OpenNIKinectWriter::Options _options;
	ssi_char_t *_file;

	File *_fptr;
	
};

}

#endif
