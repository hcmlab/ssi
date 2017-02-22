// FileSampleWriter.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2014/11/20
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

#ifndef SSI_IOPUT_FILESAMPLEWRITER_H
#define SSI_IOPUT_FILESAMPLEWRITER_H

#include "base/IConsumer.h"
#include "ioput/file/FileSamplesOut.h"
#include "ioput/option/OptionList.h"
#include "base/ITheFramework.h"

namespace ssi {

class Mutex;

class FileSampleWriter : public IConsumer {

public:

	class Options : public OptionList {

	public:

		Options ()
			: mode(File::WRITE), type(File::BINARY), version(FileSamplesOut::DEFAULT_VERSION) {

			path[0] = '\0';
			classes[0] = '\0';
			user[0] = '\0';

			addOption ("path", path, SSI_MAX_CHAR, SSI_CHAR, "file path (empty for stdout)");
			addOption ("type", &type, 1, SSI_UCHAR, "file type (0=binary, 1=ascii)");																	
			addOption("version", &version, 1, SSI_UCHAR, "file version (0=V0, 1=V1, 2=V2(xml))");
			addOption ("classes", classes, SSI_MAX_CHAR, SSI_CHAR, "class names (separated by ;)");
			addOption ("user", user, SSI_MAX_CHAR, SSI_CHAR, "user name");
		};

		void setPath (const ssi_char_t *path) {
			this->path[0] = '\0';
			if (path) {
				ssi_strcpy (this->path, path);
			}
		}
		void setClasses(const ssi_char_t *classes) {
			this->classes[0] = '\0';
			if (classes) {
				ssi_strcpy(this->classes, classes);
			}
		}
		void setUser(const ssi_char_t *user) {
			this->user[0] = '\0';
			if (user) {
				ssi_strcpy(this->user, user);
			}
		}

		ssi_char_t path[SSI_MAX_CHAR];
		File::TYPE type;
		File::MODE mode;
		File::VERSION version;
		ssi_char_t classes[SSI_MAX_CHAR];
		ssi_char_t user[SSI_MAX_CHAR];
	};

public: 	

	static const ssi_char_t *GetCreateName () { return "FileSampleWriter"; };
	static IObject *Create (const ssi_char_t *file) { return new FileSampleWriter (file); };
	~FileSampleWriter ();

	FileSampleWriter::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Stores samples to a file on disk."; };

	void consume_enter (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);
	void consume (IConsumer::info consume_info,
		ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);
	void consume_flush (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);

	bool update(IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms);

	void setLogLevel (int level) {
		ssi_log_level = level;
	}

protected:

	FileSampleWriter (const ssi_char_t *file = 0);
	FileSampleWriter::Options _options;
	ssi_char_t *_file;

	static ssi_char_t *ssi_log_name;
	int ssi_log_level;
	
	bool classFromEvent(ssi_event_t *e);
	ssi_size_t _class_id;
	ssi_sample_t *_sample;
	FileSamplesOut _out;

	ssi_char_t _string[1024];
	bool _first_call;
	ITheFramework *_frame;

	Mutex *_mutex;
};

}

#endif
