// CVSave.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/05/27
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

#ifndef SSI_IMAGE_CVSAVE_H
#define SSI_IMAGE_CVSAVE_H

#include "ICVConsumer.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class CVSave : public ICVConsumer {

public:

	enum FORMAT {
		BMP = 0, // Windows bitmaps
		TIF, // TIF files
		RAS, // Sun rasters		
		PBM, // Portable image format		
		//JPG, // JPEG files
		//PNG, // Portable Network Graphics		
		//EXR, // OpenEXR HDR images
		//JP2 // JPEG 2000 images 
	};
	const static ssi_char_t *FORMAT_NAMES[];

class Options : public OptionList {

	public:

		Options () : format (CVSave::BMP), zeros (3), number (true), remember (true), start (0), flip (true) {

			path[0] = '\0';
			addOption ("path", &path, SSI_MAX_CHAR, SSI_CHAR, "stores image to the specified path (file extension will be automatically added)");					
			addOption ("format", &format, 1, SSI_INT, "image format (0=BMP, 1=TIF, 2=RAS, 3=PBM");					
			addOption ("number", &number, 1, SSI_BOOL, "add incremental number to image");
			addOption ("start", &start, 1, SSI_INT, "starting number");
			addOption ("remember", &remember, 1, SSI_BOOL, "update starting number");
			addOption ("zeros", &zeros, 1, SSI_SIZE, "leading zeros (e.g. 3 generates 001.jpg)");	
			addOption ("flip", &flip, 1, SSI_BOOL, "flip image");
		};

		void set (const ssi_char_t *path_, FORMAT format_) {
			ssi_strcpy (path, path_);
			format = format_;
		}

		ssi_char_t path[SSI_MAX_CHAR];
		CVSave::FORMAT format;
		ssi_size_t zeros;
		bool number;
		bool remember;
		int start;
		bool flip;
	};

public:

	static const ssi_char_t *GetCreateName () { return "CVSave"; };
	static IObject *Create (const ssi_char_t *file) { return new CVSave (file); };
	~CVSave ();

	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "stores images in different formats"; };

	void setFormat (ssi_video_params_t format_in);
	void consume_enter (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);
	void consume (ssi_time_t frame_rate,
		const IplImage *_image_in);
	void consume_flush (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);

protected:

	CVSave (const ssi_char_t *file);
	Options _options;
	ssi_char_t *_file;

	static ssi_char_t *ssi_log_name;

	ssi_char_t *_path;
	ssi_char_t _string[SSI_MAX_CHAR];
	int _counter;
	int _zeros;
	FORMAT _format;
	bool _flip;
};

}

#endif
