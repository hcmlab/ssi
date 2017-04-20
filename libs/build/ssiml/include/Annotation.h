// Annotation.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/11/04
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

#ifndef SSI_MODEL_ANNOTATION_H
#define SSI_MODEL_ANNOTATION_H

#include "SSI_Cons.h"
#include "base/IAnnotation.h"
#include "base/StringList.h"
#include "ioput/file/FileCSV.h"
#include "ioput/file/File.h"

namespace ssi {

class SampleList;

class Annotation : public IAnnotation, public std::vector<ssi_label_t>
{

public:

	struct FILTER_PROPERTY
	{
		enum List
		{
			CONFIDENCE = 0,
			DURATION,
			FROM,
			TO,
			NUM
		};
	};

	struct FILTER_OPERATOR
	{
		enum List
		{
			GREATER = 0,
			GREATER_EQUAL,
			LESSER,
			LESSER_EQUAL,
			EQUAL,
			NOT_EQUAL,
			NUM
		};
	};

public:

	Annotation();
	Annotation(Annotation &self); 
	virtual ~Annotation();

	// clean up

	void empty();	
	void release();

	// scheme

	bool setSchemeName(const ssi_char_t *name);
	bool setScheme(const ssi_scheme_t scheme);
	bool setFreeScheme(const ssi_char_t *scheme_name);
	bool setDiscreteScheme(const ssi_char_t *scheme_name);
	bool setDiscreteScheme(const ssi_char_t *scheme_name,
		StringList &classes,
		const ssi_size_t *class_ids = 0);
	bool setDiscreteScheme(const ssi_char_t *scheme_name,
		ssi_size_t n_classes,
		const ssi_char_t **classes,
		const ssi_size_t *class_ids = 0);
	bool setDiscreteScheme(const ssi_char_t *scheme_name,
		std::map<String, ssi_size_t> &classes);
	bool setContinuousScheme(const ssi_char_t *scheme_name,
		ssi_time_t sr,
		ssi_real_t min_score,
		ssi_real_t max_score);
	bool hasScheme();
	const ssi_scheme_t *getScheme();	

	// labels

	ssi_size_t getSize();
	ssi_label_t getLabel(ssi_size_t index);
	bool add(const ssi_label_t &label);
	bool add(ssi_time_t from, ssi_time_t to, ssi_int_t class_id, ssi_real_t conf);
	bool add(ssi_time_t from, ssi_time_t to, const ssi_char_t *name, ssi_real_t conf);
	bool add(const ssi_char_t *from, const ssi_char_t *to, const ssi_char_t *name, const ssi_char_t *conf);
	bool add(const ssi_char_t *from, const ssi_char_t *to, const ssi_char_t *name, ssi_real_t conf);
	bool add(ssi_real_t score, ssi_real_t conf);
	bool add(const ssi_char_t *score, const ssi_char_t *conf);
	bool add(const ssi_char_t *score, ssi_real_t conf);
	void sort();

	bool addStream(ssi_stream_t scores, ssi_size_t score_dim, ssi_real_t conf);
	bool addStream(ssi_stream_t scores, ssi_size_t score_dim, ssi_size_t conf_dim);

	bool addCSV(FileCSV &file, ssi_size_t column_from, ssi_size_t column_to, ssi_size_t column_class, ssi_size_t column_conf);
	bool addCSV(FileCSV &file, ssi_size_t column_score, ssi_size_t column_conf);
	bool addCSV(FileCSV &file, const ssi_char_t *column_from, const ssi_char_t *column_to, const ssi_char_t *column_class, const ssi_char_t *column_conf);
	bool addCSV(FileCSV &file, const ssi_char_t *column_score, const ssi_char_t *column_conf);
	bool addCSV(FileCSV &file, ssi_size_t column_from, ssi_size_t column_to, ssi_size_t column_class, ssi_real_t conf);
	bool addCSV(FileCSV &file, ssi_size_t column_score, ssi_real_t conf);
	bool addCSV(FileCSV &file, const ssi_char_t *column_from, const ssi_char_t *column_to, const ssi_char_t *column_class, ssi_real_t conf);
	bool addCSV(FileCSV &file, const ssi_char_t *column_score, ssi_real_t conf);

	bool getIndexAt(ssi_size_t &index, ssi_time_t time); // return the index of the first segment that overlaps
	bool getIndexAt(ssi_size_t &index, 
		ssi_time_t from, 
		ssi_time_t to, 
		ssi_real_t min_percent = 0.5); // returns the index of the segment with the maximum overlap [from..to] and an overlap of at least 'min_percent'

	// meta

	ssi_size_t getMetaSize();
	void setMeta(const ssi_char_t *key, const ssi_char_t *value);
	const ssi_char_t *getMeta(const ssi_char_t *key);
	const ssi_char_t *getMetaKey(ssi_size_t index);

	// save/load

	bool save(const ssi_char_t *path, File::TYPE type);
	bool loadScheme(const ssi_char_t *path);
	bool load(const ssi_char_t *path);
	void print(FILE *file = stdout, ssi_size_t n_max = 0);

	// discrete scheme

	bool hasClassId(ssi_int_t class_id);
	bool hasClassName(const ssi_char_t *class_name);
	bool getClassId(const ssi_char_t *class_name, ssi_int_t &class_id);
	bool getClassIndex(ssi_int_t class_id, ssi_size_t &class_index);
	bool getClassIndex(const ssi_char_t *class_name, ssi_size_t &class_index);
	const ssi_char_t *getClassName(ssi_int_t class_id);	
	ssi_size_t getClassSize(ssi_int_t class_id);	
	bool removeClass(const ssi_char_t *class_name);
	bool removeClass(StringList class_names);
	bool keepClass(const ssi_char_t *class_name);
	bool keepClass(StringList class_names);
	bool renameClass(const ssi_char_t *from, const ssi_char_t *to);
	bool mapClass(const ssi_char_t *from, const ssi_char_t *to, bool keep = false);	
	bool addClass(const ssi_char_t *class_name);
	bool addClass(const ssi_char_t *class_name, ssi_int_t &class_id);
	bool addClass(ssi_int_t class_id, const ssi_char_t *class_name);
	bool addClass(StringList class_names);
	bool packClass(ssi_time_t max_time_gap = 0, const ssi_char_t *class_name = 0); // combine successive labels of same class if gap is smaller than 'max_time_gap'	
	bool convertToFrames(ssi_time_t frame_s, // frame size in seconds		
		const ssi_char_t *empty_class_name, // class name of frames that do not overlap with a label (if 0 they will not be added)
		ssi_time_t duration_s = 0.0,         // duration in seconds (if <= 0 defined by last label)
		ssi_real_t empty_percent = 0.5f // if the % overlap of a frame with a segment falls below this value it will be classified as empty		
	); 		
	bool addOffset(ssi_time_t offset_left, ssi_time_t offset_right);	
	bool removeOverlap();

	// confidence

	void normConfidence();
	void setConfidence(ssi_real_t confidence);

	// filter	
	bool filter(double threshold, FILTER_PROPERTY::List prop, FILTER_OPERATOR::List op, const ssi_char_t *class_name = 0);

	// stream

	bool extractStream(const ssi_stream_t &from, ssi_stream_t &to);
	bool extractSamples(const ssi_stream_t &stream, 
		SampleList *samples,
		const ssi_char_t *user = SSI_SAMPLE_GARBAGE_USER_NAME);
	bool extractSamplesFromDiscreteScheme(ssi_size_t n_streams,
		const ssi_stream_t *streams, 
		SampleList *samples,
		const ssi_char_t *user = SSI_SAMPLE_GARBAGE_USER_NAME);
	bool extractSamplesFromContinuousScheme(const ssi_stream_t &stream,
		SampleList *samples,
		ssi_size_t context_left,
		ssi_size_t context_right,
		const ssi_char_t *user = SSI_SAMPLE_GARBAGE_USER_NAME);
	

protected:

	static ssi_char_t *ssi_log_name;

	bool check_type(SSI_SCHEME_TYPE::List type);
	bool check_not_type(SSI_SCHEME_TYPE::List type);
	void release_scheme();
	ssi_size_t max_class_id();
	
	static bool compare_label(const ssi_label_t lhs, const ssi_label_t rhs);

	ssi_scheme_t *_scheme;
	static ssi_size_t *default_ids(ssi_size_t n);
	static void copy_ids(ssi_size_t n, const ssi_size_t *from, ssi_size_t *to);
	std::map<String, String> _meta;

};





















// old annotation class begins here (deprecated)

namespace old {

	class Annotation {

	public:

		Annotation();
		~Annotation();

		struct Entry {
			ssi_time_t start;
			ssi_time_t stop;
			ssi_size_t label_index;
		};

		void add(ssi_time_t start,
			ssi_time_t stop,
			const ssi_char_t *label);
		void add(Annotation::Entry &entry);

		void reset();
		Annotation::Entry *next();
		Annotation::Entry *next(ssi_size_t label_index);
		Annotation::Entry *last();

		ssi_size_t size();
		void clear();

		ssi_size_t addLabel(const ssi_char_t *label);
		const ssi_char_t *getLabel(ssi_size_t index);
		ssi_size_t labelSize();

		void print(FILE *file = stdout);

		/**
			* Returns the entry corresponding to the given time
			* @param time the timestamp at which we want to look (in seconds)
			* @param buffer a time buffer zone (in seconds). This zone causes timestamps after an entries stop-value to be still matched if they are within this buffer zone.
			* @return pointer to the matched entry
			*/
		Annotation::Entry *getEntryAt(ssi_time_t time, ssi_time_t max_delay);

		void trim(ssi_size_t offset, ssi_size_t size);

	protected:

		struct entry_compare {
			bool operator() (const Annotation::Entry *lhs, const Annotation::Entry *rhs) const {
				if (lhs->start == rhs->start) {
					return lhs->stop < rhs->stop;
				}
				else {
					return lhs->start < rhs->start;
				}
			}
		};

		typedef std::set<Annotation::Entry *, entry_compare> entries_set_t;
		entries_set_t entries;
		entries_set_t::iterator entries_iter;
		std::vector<ssi_char_t *> labels;
		ssi_size_t counter;
	};

}

}

#endif
