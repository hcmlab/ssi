// FileAnnotationWriter.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/11/12
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute itand/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#pragma once

#ifndef SSI_IOPUT_FILEANNOTATIONWRITER_H
#define SSI_IOPUT_FILEANNOTATIONWRITER_H

#include "base/IConsumer.h"
#include "base/IAnnotation.h"
#include "thread/Mutex.h"
#include "thread/Lock.h"
#include "ioput/option/OptionList.h"
#include "base/Factory.h"

namespace ssi {

	class Annotation;

	class FileAnnotationWriter : public IConsumer {
	public:

		class Options : public OptionList {
		public:

			Options() : addUnkownLabel(false),
				forceDefaultLabel(false), 
				defaultConfidence(1.0f),
				forceDefaultConfidence(false),
				eventNameAsLabel(false), 
				senderNameAsLabel(false), 
				mapKeyIndex(0),
				mapKeySelectMax(false),
				streamScoreIndex(0), 
				streamConfidenceIndex(1) {

				setAnnoPath("");
				setSchemePath("");
				setDefaultLabel("");
				setMeta("");

				addOption("annotationPath", &annotationPath, SSI_MAX_CHAR, SSI_CHAR, "path of annotation file (will be created)");
				addOption("schemePath", &schemePath, SSI_MAX_CHAR, SSI_CHAR, "path of scheme file (labels that do not fit the scheme will be ignored)");
				
				// discrete				
				addOption("defaultLabel", &defaultLabel, SSI_MAX_CHAR, SSI_CHAR, "default label e.g. to label empty events (applies to discrete annotations only)");
				addOption("forceDefaultLabel", &forceDefaultLabel, 1, SSI_BOOL, "force use of default label (applies to discrete annotations only)");
				addOption("addUnkownLabel", &addUnkownLabel, 1, SSI_BOOL, "automatically add unkown labels to annotation scheme");
				addOption("eventNameAsLabel", &eventNameAsLabel, 1, SSI_BOOL, "use the event name as label (applies to discrete annotations only)");
				addOption("senderNameAsLabel", &senderNameAsLabel, 1, SSI_BOOL, "use the sender name as label (applies to discrete annotations only)");
				addOption("mapKeyIndex", &mapKeyIndex, 1, SSI_SIZE, "index of the key that will be selected from map events (applies to discrete annotations only)");
				addOption("mapKeySelectMax", &mapKeySelectMax, 1, SSI_BOOL, "select the key with the maximum value (applies to discrete annotations only)");

				// continuous
				addOption("streamScoreIndex", &streamScoreIndex, 1, SSI_SIZE, "stream dimension of score value (applies to continuous annotations only)");
				addOption("streamConfidenceIndex", &streamConfidenceIndex, 1, SSI_SIZE, "stream dimension of confidence value (applies to continuous annotations only)");
				
				addOption("defaultConfidence", &defaultConfidence, 1, SSI_REAL, "default confidence");
				addOption("forceDefaultConfidence", &forceDefaultConfidence, 1, SSI_BOOL, "force use of default confidence");

				addOption("meta", &meta, SSI_MAX_CHAR, SSI_CHAR, "list of 'key=value' pairs separated by ; to add as meta information");
										
			};

			void setAnnoPath(const ssi_char_t *string)
			{
				ssi_strcpy(annotationPath, string);
			}

			void setSchemePath(const ssi_char_t *string)
			{
				ssi_strcpy(schemePath, string);
			}

			void setDefaultLabel(const ssi_char_t *string)
			{
				ssi_strcpy(defaultLabel, string);
			}

			void setMeta(const ssi_char_t *string)
			{
				ssi_strcpy(meta, string);
			}

			ssi_char_t annotationPath[SSI_MAX_CHAR];
			ssi_char_t schemePath[SSI_MAX_CHAR];
			ssi_char_t defaultLabel[SSI_MAX_CHAR];			
			bool forceDefaultLabel;
			bool addUnkownLabel;
			ssi_char_t meta[SSI_MAX_CHAR];
			ssi_size_t mapKeyIndex;
			bool mapKeySelectMax;
			ssi_real_t defaultConfidence;
			bool forceDefaultConfidence;
			ssi_size_t streamScoreIndex, streamConfidenceIndex;

			bool eventNameAsLabel;
			bool senderNameAsLabel;
		};

	public:

		static const ssi_char_t *GetCreateName() { return "FileAnnotationWriter"; };
		static IObject *Create(const ssi_char_t *file) { return new FileAnnotationWriter(file); };
		~FileAnnotationWriter();

		Options *getOptions() { return &_options; };
		const ssi_char_t *getName() { return GetCreateName(); };
		const ssi_char_t *getInfo() { return "Converts events to labels and stores them to an annotation file."; };
		
		void listen_enter();
		bool update(IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms);
		bool update(ssi_event_t &e);
		void listen_flush();

		void consume_enter(ssi_size_t stream_in_num,
			ssi_stream_t stream_in[]);
		void consume(IConsumer::info consume_info,
			ssi_size_t stream_in_num,
			ssi_stream_t stream_in[]);
		void consume_flush(ssi_size_t stream_in_num,
			ssi_stream_t stream_in[]);
		
		virtual bool setAnnotation(Annotation *annotation);		
		
		ssi_size_t getStringId(const ssi_char_t *str) {
			return 0;
		}
		ssi_size_t getGlueId() {
			return SSI_FACTORY_UNIQUE_INVALID_ID;
		}

		void setLabel(const ssi_char_t *label, ssi_real_t confidence = 1.0f);		

	protected:

		FileAnnotationWriter(const ssi_char_t *file);
		ssi_char_t *_file;
		Options _options;
		static ssi_char_t *ssi_log_name;

		bool load_scheme();
		void save_annotation();
		void parse_meta(Annotation *annotation, const ssi_char_t *string, char delim);
		bool get_class_id(Annotation *annotation, const ssi_char_t *string, ssi_int_t &id);
		
		Annotation *_annotation;
		bool _borrowed;

		ssi_char_t *_label;
		ssi_real_t _confidence;

		Mutex _mutex;
	};

}

#endif
