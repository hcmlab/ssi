// VectorFusionWriter.h
// author: Florian Lingenfelser <lingenfelser@hcm-lab.de>
// created: 2013/02/20
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

#ifndef SSI_LISTENER_VECTORFUSIONWRITER_H
#define SSI_LISTENER_VECTORFUSIONWRITER_H

#include "base/IObject.h"
#include "ioput/option/OptionList.h"
#include "ioput/file/FileStreamOut.h"
#include "base/ITheFramework.h"

namespace ssi {

class VectorFusionWriter : public IObject {

public:

	class Options : public OptionList {

	public:

		Options () : type (File::ASCII), stream (true), version (File::DEFAULT_VERSION), f_update_ms (0), dim(1) {

			path[0] = '\0';
			delim[0] = ' '; delim[1] = '\0';
			flags[0] = '\0';

			addOption ("path", path, SSI_MAX_CHAR, SSI_CHAR, "file path (empty for stdout)");
			addOption ("type", &type, 1, SSI_UCHAR, "file type (0=binary, 1=ascii)");
			addOption ("version", &version, 1, SSI_UCHAR, "file version (0=V0, 1=V1, 2=V2(xml))");
			addOption ("stream", &stream, 1, SSI_BOOL, "continuous stream mode");
			addOption ("delim", delim, SSI_MAX_CHAR, SSI_CHAR, "delimiter string");
			addOption ("flags", flags, SSI_MAX_CHAR, SSI_CHAR, "format flags");
			addOption("fusion update", &f_update_ms, 1, SSI_TIME, "set to update rate (ms) of vector fusion to calculate samplerate for generated stream");
			addOption("fusion dim", &dim, 1, SSI_INT, "set to dimension of vector fusion to calculate dimension for generated stream");

		};

		void setPath (const ssi_char_t *path) {
			if (path) {
				ssi_strcpy (this->path, path);
			}
		}

		void setFormat (const ssi_char_t *delim, const ssi_char_t *flags) {
			this->delim[0] = ' '; this->delim[1] = '\0';
			this->flags[0] = '\0';
			if (delim) {
				ssi_strcpy (this->delim, delim);
			}
			if (flags) {
				ssi_strcpy (this->flags, flags);
			}
		}

		ssi_char_t path[SSI_MAX_CHAR];
		File::TYPE type;
		File::VERSION version;
		ssi_char_t delim[20];
		ssi_char_t flags[20];
		bool stream;
		ssi_time_t f_update_ms;
		ssi_size_t dim;

	};

public:

	static const ssi_char_t *GetCreateName () { return "VectorFusionWriter"; };
	static IObject *Create (const ssi_char_t *file) { return new VectorFusionWriter (file); };
	~VectorFusionWriter ();

	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Listens to output from VectorFusion and writes them in stream format."; };

	void listen_enter ();
	bool update (ssi_event_t &e);
	bool update (IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms);
	void listen_flush ();

protected:

	VectorFusionWriter (const ssi_char_t *file = 0);
	VectorFusionWriter::Options _options;
	ssi_char_t *_file;

	static ssi_char_t *ssi_log_name;
	int ssi_log_level;

	ssi_char_t _string[1024];

	FileStreamOut _out;

	ssi_stream_t _stream;

};

}

#endif
