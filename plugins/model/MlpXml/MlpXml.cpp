// MlpXml.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2010/05/24
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

#include "MlpXml.h"
#include "base/Factory.h"
#include "ioput/include/FileWriter.h"
#include "audio/include/WavWriter.h"
#include "graphic/include/SignalPainter.h"
#include "event/include/ZeroEventSender.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ssi_char_t *MlpXml::ssi_log_name = "mlpxml____";
ssi_char_t *MlpXml::DATA_DIR = "data";
ssi_char_t *MlpXml::OPTS_DIR = "opts";

MlpXml::MlpXml (const ssi_char_t *file)
	: _filewrite (0),
	_bufwrite (0),
	_trigger (0),
	_annowrite (0),
	_trainer (0),
	_callback (0),
	_file (0),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}

	_last_record_dir[0] = '\0';
}

MlpXml::~MlpXml () {

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}

void MlpXml::mkdirs () {
	ssi_char_t string[SSI_MAX_CHAR];
	ssi_sprint (string, "%s\\%s", _options.base, OPTS_DIR);
	ssi_mkdir (string);
	ssi_sprint (string, "%s\\%s", _options.base, DATA_DIR);	
	ssi_mkdir (string);
	ssi_sprint (string, "%s\\%s\\%s", _options.base, DATA_DIR, _options.user);
	ssi_mkdir (string);
}

void MlpXml::consume_enter (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {	

	if (stream_in_num != 2) {
		ssi_err ("MlpXml requires two streams: a signal and trigger stream");
	}

	mkdirs ();

	ssi_char_t now[ssi_time_size_friendly];
	ssi_now_friendly (now);	
	ssi_char_t path[SSI_MAX_CHAR]; 
	ssi_char_t string[SSI_MAX_CHAR];		

	ssi_sprint (string, "%s\\%s\\%s.trigger", _options.base, OPTS_DIR, _options.name);		
	_trigger = ssi_create (ZeroEventSender, string, false);
	_trigger->consume_enter (1, stream_in + 1);	

	if (_options.signal[0] != '\0' || _options.anno[0] != '\0') {
		ssi_sprint (_last_record_dir, "%s\\%s\\%s\\%s", _options.base, DATA_DIR, _options.user, now);
		ssi_mkdir (_last_record_dir);
	}

	if (_options.signal[0] != '\0') {
		switch (_options.type) {
			case MlpXmlDef::STREAM: {
				ssi_sprint (string, "%s\\%s\\%s\\%s", _options.base, DATA_DIR, _options.user, now);
				ssi_sprint (path, "%s\\%s%s", string, _options.signal, SSI_FILE_TYPE_STREAM);
				ssi_sprint (string, "%s\\%s\\%s.recorder", _options.base, OPTS_DIR, _options.name);
				_filewrite = ssi_create (FileWriter, string, false);							
				_filewrite->getOptions ()->setOptionValue ("path", ssi_ccast (ssi_char_t *, path));	
				_filewrite->consume_enter (1, stream_in);				
				break;
			}
			case MlpXmlDef::AUDIO: {
				ssi_sprint (string, "%s\\%s\\%s\\%s", _options.base, DATA_DIR, _options.user, now);
				ssi_sprint (path, "%s\\%s%s", string, _options.signal, SSI_FILE_TYPE_WAV);
				ssi_sprint (string, "%s\\%s\\%s.recorder", _options.base, OPTS_DIR, _options.name);
				_filewrite = ssi_create (WavWriter, string, false);							
				_filewrite->getOptions ()->setOptionValue ("path", ssi_ccast (ssi_char_t *, path));	
				_filewrite->consume_enter (1, stream_in);				
				break;
			}
			default:
				ssi_wrn ("unkown signal type (%d)", _options.type);
		}			
	}

	if (_options.trainer[0] != '\0') {

		ssi_char_t string[SSI_MAX_CHAR];									
		ssi_sprint (string, "%s\\%s\\%s.buffer", _options.base, OPTS_DIR, _options.name);

		_trainer = new Trainer ();
		if (!Trainer::Load (*_trainer, _options.trainer)) {
			ssi_wrn ("could not load trainer '%s'", _options.trainer);
			delete _trainer; _trainer = 0;
		} else {
			_bufwrite = ssi_pcast (BufferWriter, BufferWriter::Create (string));
			_bufwrite->consume_enter (1, stream_in);
		}
	}

	if (_options.anno[0] != '\0') {
		ssi_sprint (string, "%s\\%s\\%s\\%s", _options.base, DATA_DIR, _options.user, now);
		ssi_sprint (path, "%s\\%s%s", string, _options.anno, SSI_FILE_TYPE_ANNOTATION);
		_annowrite = new FileAnnotationWriter (path);
	}

	_stream = stream_in[0];	

	_trigger->setEventListener (this);

	ssi_msg (SSI_LOG_LEVEL_BASIC, "continuous toggle = %s", _options.continuous ? "on" : "off");
}

void MlpXml::consume (IConsumer::info consume_info,
	ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	if (_filewrite) {
		_filewrite->consume (consume_info, 1, stream_in);
	}
	if (_bufwrite) {
		_bufwrite->consume (consume_info, 1, stream_in);
	}
	_trigger->consume (consume_info, 1, stream_in + 1);	

	if (_options.continuous) {

		if (_annowrite) {		
			ssi_event_t e;
			ssi_event_init (e, SSI_ETYPE_EMPTY, SSI_FACTORY_STRINGS_INVALID_ID, SSI_FACTORY_STRINGS_INVALID_ID, ssi_cast (ssi_size_t, consume_info.time * 1000 + 0.5), ssi_cast (ssi_size_t, consume_info.dur * 1000 + 0.5));
			_annowrite->update (e);
		}

		const ssi_char_t *label = 0;

		if (_trainer) {

			ssi_size_t class_index;
			if (_trainer->forward (stream_in[0], class_index)) {
				label = _trainer->getClassName (class_index);
			} else {
				ssi_wrn ("could not receive valid answer from trainer");
			}
		}

		if (_callback) {
			_callback->call (consume_info.time, consume_info.dur, label, true, false);
		}
	}

}

void MlpXml::setLabel (const ssi_char_t *label) {
	
	if (_annowrite) {
		_annowrite->setLabel (label);
	}
}

bool MlpXml::update (ssi_event_t &e) {

	IConsumer::info info;
	info.time = e.time / 1000.0;
	info.dur = e.dur / 1000.0;
	info.status = e.state == SSI_ESTATE_COMPLETED ? IConsumer::COMPLETED : IConsumer::CONTINUED;

	SSI_DBG (SSI_LOG_LEVEL_DEBUG, "update() %.2lf@%.2lf", info.dur, info.time);

	const ssi_char_t *label = 0;
	if (!_options.continuous) {

		if (_annowrite) {		
			_annowrite->update (e);
		}

		if (_trainer) {

			if (!_bufwrite->get (_stream, info)) {				
				ssi_wrn ("could not receive data from buffer");
				return false;
			}

			ssi_size_t class_index;
			if (_trainer->forward (_stream, class_index)) {
				label = _trainer->getClassName (class_index);
				ssi_print ("%s\n", label);
			} else {
				ssi_wrn ("could not receive valid answer from trainer");
				return false;
			}
		}
	}

	if (_callback) {
		_callback->call (info.time, info.dur, label, !_options.continuous, true);
	}

	return true;
}

void MlpXml::consume_flush (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {
		
	_trigger->consume_flush (1, stream_in + 1);

	if (_filewrite) {
		_filewrite->consume_flush (1, stream_in);
		delete _filewrite;
		_filewrite = 0;
	}

	if (_bufwrite) {
		_bufwrite->consume_flush (1, stream_in);
		delete _bufwrite;
		_bufwrite = 0;
	}

	if (_trainer) {
		delete _trainer;
		_trainer = 0;
	}

	if (_annowrite) {
		delete _annowrite;	
		_annowrite = 0;
	}

	ssi_stream_destroy (_stream);				

	delete _trigger; _trigger = 0;
}

}
