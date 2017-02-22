// MlpXml.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/04/27
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

#ifndef SSI_MLPXML_H
#define SSI_MLPXML_H

#include "MlpXmlDef.h"
#include "MlpXmlICallback.h"
#include "BufferWriter.h"

#include "base/IConsumer.h"
#include "Trainer.h"
#include "ioput/option/OptionList.h"
#include "ioput/include/FileWriter.h"
#include "ioput/file/FileAnnotationWriter.h"

namespace ssi {

class IMlpXml : public IConsumer {

public:

	virtual void setCallback (MlpXmlICallback *callback) = 0;
	virtual void setLabel (const ssi_char_t *label) = 0;
	virtual const ssi_char_t *getLastRecordDir () = 0;
};

class MlpXml : IMlpXml {

public:

static ssi_char_t *DATA_DIR;
static ssi_char_t *MODEL_DIR;
static ssi_char_t *OPTS_DIR;
static ssi_char_t *LOG_DIR;

public: 

	class Options : public OptionList {

	public:

		Options ()
			: type (MlpXmlDef::STREAM), plot (true), continuous (false) {

			setName ("mlp");
			setBase (".");
			setUser ("user");
			signal[0] = '\0';
			anno[0] = '\0';
			trainer[0] = '\0';

			addOption ("type", &type, 1, SSI_INT, "input signal type (0=STREAM,1=AUDIO,2=VIDEO)");
			addOption ("base", base, SSI_MAX_CHAR, SSI_CHAR, "base directory");			
			addOption ("name", name, SSI_MAX_CHAR, SSI_CHAR, "mlp name");			
			addOption ("user", user, SSI_MAX_CHAR, SSI_CHAR, "user name");
			addOption ("signal", signal, SSI_MAX_CHAR, SSI_CHAR, "signal name");	
			addOption ("anno", anno, SSI_MAX_CHAR, SSI_CHAR, "anno name");	
			addOption ("trainer", trainer, SSI_MAX_CHAR, SSI_CHAR, "filepath to trainer");
			addOption ("continuous", &continuous, 1, SSI_BOOL, "apply continuous classification, i.e. classify every frame instead of whole segments");
		};		

		void setTrainer (const ssi_char_t *filepath) {
			if (filepath) {
				ssi_strcpy (trainer, filepath);
			} else {
				trainer[0] = '\0';
			}
		}
		void setBase (const ssi_char_t *dirpath) {
			ssi_strcpy (base, dirpath);
		}
		void setUser (const ssi_char_t *filepath) {
			ssi_strcpy (user, filepath);
		}
		void setName (const ssi_char_t *n) {
			ssi_strcpy (name, n);
		}		
		void setSignal (const ssi_char_t *n) {
			if (n) {
				ssi_strcpy (signal, n);
			} else {
				signal[0] = '\0';
			}
		}		
		void setAnno (const ssi_char_t *n) {
			if (n) {
				ssi_strcpy (anno, n);
			} else {
				anno[0] = '\0';
			}
		}		

		MlpXmlDef::signal_t type;
		ssi_char_t name[SSI_MAX_CHAR];
		ssi_char_t base[SSI_MAX_CHAR];
		ssi_char_t trainer[SSI_MAX_CHAR];
		ssi_char_t user[SSI_MAX_CHAR];
		ssi_char_t signal[SSI_MAX_CHAR];
		ssi_char_t anno[SSI_MAX_CHAR];
		bool plot;
		bool continuous;
	};

public:	

	static const ssi_char_t *GetCreateName () { return "MlpXml"; };
	static IObject *Create (const ssi_char_t *file) { return new MlpXml (file); };
	virtual ~MlpXml ();

	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Machine learning pipeline"; };

	void consume_enter (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);	
	void consume (IConsumer::info consume_info,
		ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);
	void consume_flush (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);		

	void setCallback (MlpXmlICallback *callback) {
		_callback = callback;
	}
	void setLabel (const ssi_char_t *label);
	const ssi_char_t *getLastRecordDir () {
		return _last_record_dir;
	}

	bool update (ssi_event_t &e);
	ssi_size_t getStringId (const ssi_char_t *str) {
		return 0;
	}
	ssi_size_t getGlueId () {
		return SSI_FACTORY_UNIQUE_INVALID_ID;
	}

	void setLogLevel (int level) {
		ssi_log_level = level;
	}		

protected:

	MlpXml (const ssi_char_t *file);
	Options _options;
	ssi_char_t *_file;

	int ssi_log_level;
	static ssi_char_t *ssi_log_name;

	IConsumer *_filewrite;
	IBufferWriter *_bufwrite;

	IConsumer *_trigger;
	FileAnnotationWriter *_annowrite;
	ssi_stream_t _stream;

	Trainer *_trainer;
	MlpXmlICallback *_callback;

	void mkdirs ();

	ssi_char_t _last_record_dir[SSI_MAX_CHAR];
};

}

#endif
