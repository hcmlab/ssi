// ModelTools.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/03/04
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

#include "model/ModelTools.h"
#include "signal/SignalTools.h"
#include "ioput/file/FileAscii.h"
#include "ioput/file/FileBinary.h"
#include "ioput/file/FileTools.h"
#include "ioput/file/FilePath.h"
#include "base/Factory.h"
#include "graphic/PaintData.h"
#if _WIN32||_WIN64
#include "graphic/Window.h"
#include "graphic/Canvas.h"
#else
#ifdef SSI_USE_SDL
#include "graphic/Window.h"
#include "graphic/Canvas.h"
#else
#include "graphic/WindowFallback.h"
#endif
#endif

#ifdef USE_SSI_LEAK_DETECTOR
#include "SSI_LeakWatcher.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

namespace ssi {
	
	ssi_char_t *ModelTools::ssi_log_name = "modeltools";

	int model_tools_compare(const void * a, const void * b) {
		return (*(int*)a - *(int*)b);
	}

	void ModelTools::LoadAnnotation(Annotation &annotation, File &file, char* tier_id) {

		// create temporary variables
		ssi_time_t start_time, stop_time;
		ssi_char_t line[SSI_MAX_CHAR];
		ssi_char_t label_name[SSI_MAX_CHAR] = "";
		ssi_char_t tier_name[SSI_MAX_CHAR] = "";

		// walk through lines
		while (file.ready()) {
			// read next line
			file.readLine(SSI_MAX_CHAR, line);

			// parse line
			char* split = " ;,\t";
			char* token = strtok(line, split);
			sscanf(token, "%lf", &start_time);
			token = strtok(NULL, split);
			sscanf(token, "%lf", &stop_time);
			
			token = strtok(NULL, split);
			if (token != NULL) strcpy(label_name, token);
			token = strtok(NULL, split);
			if (token != NULL) strcpy(tier_name, token);

			SSI_DBG(SSI_LOG_LEVEL_DEBUG, "found new entry: %f %f %s %s\n", start_time, stop_time, label_name, tier_name);

			// add to annotation (if no tier id is set add all, else only those that are defined)
			if (strcmp("", tier_id) == 0 || strcmp(tier_name, tier_id) == 0)
			{
				annotation.add(start_time, stop_time, label_name);
			}
		}
	}

	void ModelTools::LoadAnnotation(Annotation &anno,
		const ssi_char_t *file_name, char* tier_id) {
		File *file = File::CreateAndOpen(File::ASCII, File::READ, file_name);
		LoadAnnotation(anno, *file, tier_id);
		delete file;
	}

	void ModelTools::SaveAnnotation(Annotation &anno,
		File &file) {
		ssi_char_t string[SSI_MAX_CHAR];

		anno.reset();
		Annotation::Entry *entry;
		while (entry = anno.next()) {
			ssi_sprint(string, "%lf;%lf;%s", entry->start, entry->stop, anno.getLabel(entry->label_index));
			file.writeLine(string);
		}
	}

	void ModelTools::SaveAnnotation(Annotation &anno,
		const ssi_char_t *file_name) {
		File *file = File::CreateAndOpen(File::ASCII, File::WRITE, file_name);
		SaveAnnotation(anno, *file);
		delete file;
	}

	void ModelTools::ConvertToContinuousAnnotation(Annotation &from,
		Annotation &to,
		ssi_time_t frame_len,
		ssi_time_t delta_len,
		ssi_time_t min_percent_per_frame,
		const char *default_label) {
		ssi_size_t n_labels = from.labelSize();
		ssi_size_t *label_ids = new ssi_size_t[n_labels + (default_label ? 1 : 0)];
		for (ssi_size_t i = 0; i < n_labels; i++) {
			label_ids[i] = to.addLabel(from.getLabel(i));
		}
		if (default_label) {
			label_ids[n_labels] = to.addLabel(default_label);
		}

		ssi_time_t len = from.last()->stop;
		ssi_size_t n_frames = ssi_cast(ssi_size_t, (len - (frame_len + delta_len)) / frame_len);

		ssi_time_t frame_tot = frame_len + delta_len;
		ssi_time_t frame_start = 0;
		ssi_time_t frame_stop = frame_tot;

		Annotation::Entry *entry = 0;
		Annotation::Entry new_entry;
		ssi_time_t *percent_per_label = new ssi_time_t[n_labels];
		for (ssi_size_t i = 0; i < n_frames; i++) {
			from.reset();

			new_entry.start = frame_start;
			new_entry.stop = frame_stop;
			new_entry.label_index = label_ids[n_labels];

			for (ssi_size_t i = 0; i < n_labels; i++) {
				percent_per_label[i] = 0;
			}

			while ((entry = from.next()) && entry->stop < frame_start) {
			}

			if (entry) {
				bool found_at_least_one = false;

				while (entry && entry->start < frame_stop) {
					ssi_time_t dur = (min(frame_stop, entry->stop) - max(frame_start, entry->start)) / frame_tot;
					percent_per_label[entry->label_index] += dur;
					entry = from.next();
					found_at_least_one = true;
				}

				if (found_at_least_one) {
					ssi_time_t max_percent = percent_per_label[0];
					ssi_size_t max_label_id = 0;
					for (ssi_size_t i = 1; i < n_labels; i++) {
						if (max_percent < percent_per_label[i]) {
							max_label_id = i;
							max_percent = percent_per_label[i];
						}
					}

					if (max_percent > min_percent_per_frame) {
						new_entry.start = frame_start;
						new_entry.stop = frame_stop;
						new_entry.label_index = label_ids[max_label_id];
						to.add(new_entry);
					}
					else if (default_label) {
						to.add(new_entry);
					}
				}
				else if (default_label) {
					to.add(new_entry);
				}
			}
			else if (default_label) {
				to.add(new_entry);
			}

			frame_start += frame_len;
			frame_stop += frame_len;
		}

		delete[] label_ids;
		delete[] percent_per_label;
	}

	void ModelTools::LoadSampleList(SampleList &samples,
		const ssi_char_t *file_name,
		Annotation &annotation,
		ssi_char_t *user_name) {
		File *file = File::CreateAndOpen(File::BINARY, File::READ, file_name);
		LoadSampleList(samples, *file, annotation, user_name);
		delete file;
	}

	void ModelTools::LoadSampleList(SampleList &sample_list,
		File &file,
		Annotation &annotation,
		ssi_char_t delim) {
		// get filename
		const ssi_char_t *filepath = file.getPath();
		ssi_char_t filename[SSI_MAX_CHAR];
		ssi_char_t timestamp_s[SSI_MAX_CHAR];
		ssi_char_t user_name[SSI_MAX_CHAR];

		// remove directory
		ssi_strcpy(filename, filepath);
		for (size_t i = strlen(filepath) - 1; i >= 0; i--) {
#if _WIN32|_WIN64
			if (filepath[i] == '\\') {
#else
			if (filepath[i] == '/') {
#endif
				ssi_strcpy(filename, filepath + i + 1);
				break;
			}
			}

		// replace delimiter with white space
		for (size_t i = 0; i < strlen(filename); i++)
			if (filename[i] == delim)
				filename[i] = ' ';

		// parse filename
		int result = sscanf(filename, "%s %s", user_name, &timestamp_s);
		time_t timestamp;
		if (result != 2) {
			ssi_wrn("could not parse filename %s", filename);
			ssi_sprint(user_name, "unkown");
			timestamp = 0;
		} /*else {
			timestamp_s[ssi_time_size] = '\0';
			timestamp = ssi_time_parse (timestamp_s);
		}*/

		// extract samples
		ModelTools::LoadSampleList(sample_list, file, annotation, user_name);
		}

	void ModelTools::LoadSampleList(SampleList &sample_list,
		File &file,
		Annotation &annotation,
		ssi_char_t *user_name) {
		ssi_msg(SSI_LOG_LEVEL_DETAIL, "load samples from file '%s'", file.getPath());

		// load data
		ssi_stream_t data;
		ssi_time_t start_time;
		FileTools::ReadStreamHeader(file, data);
		FileTools::ReadStreamData(file, &start_time, data);

		// extract samples
		ModelTools::LoadSampleList(sample_list, data, annotation, user_name);
		ssi_stream_destroy(data);
	}

	void ModelTools::LoadSampleList(SampleList &sample_list,
		ssi_stream_t &stream,
		Annotation &annotation,
		const ssi_char_t *user_name) {
		ssi_stream_t *s = &stream;
		ModelTools::LoadSampleList(sample_list, 1, &s, annotation, user_name);
	}

	void ModelTools::LoadSampleList(SampleList &sample_list,
		ssi_size_t num,
		ssi_stream_t *streams[],
		Annotation &annotation,
		const ssi_char_t *user_name) {
		// add user name
		ssi_size_t user_id = sample_list.addUserName(user_name);

		// add labels
		ssi_size_t *label_map = new ssi_size_t[annotation.labelSize()];
		for (ssi_size_t i = 0; i < annotation.labelSize(); i++) {
			label_map[i] = sample_list.addClassName(annotation.getLabel(i));
		}

		// walk through annoation list
		Annotation::Entry *entry;
		annotation.reset();
		while (entry = annotation.next()) {
			ssi_sample_t *sample = new ssi_sample_t;
			ssi_stream_t **chops = new ssi_stream_t *[num];

			bool success = false;
			for (ssi_size_t j = 0; j < num; j++) {
				// calculate start and stop index
				ssi_size_t start_index = ssi_cast(ssi_size_t, entry->start * streams[j]->sr + 0.5);
				ssi_size_t stop_index = ssi_cast(ssi_size_t, entry->stop * streams[j]->sr + 0.5);

				// check if samples start index is smaller than the sto index and smaller than streams number of samples
				if (!(start_index <= stop_index && stop_index < streams[j]->num)) {
					ssi_wrn("invalid interval [%lf..%lf]s (%s)", entry->start, entry->stop, user_name);
					continue;
				}

				// extract sample
				chops[j] = new ssi_stream_t;
				ssi_stream_copy(*streams[j], *chops[j], start_index, stop_index);

				success = true;
			}

			if (success) {
				// create and add new sample
				sample->class_id = label_map[entry->label_index];
				sample->num = num;
				sample->score = 0.0f;
				sample->streams = chops;
				sample->time = entry->start;
				sample->user_id = user_id;
				sample_list.addSample(sample);
			}
			else {
				delete sample;
				delete[] chops;
			}
		}

		delete[] label_map;
	}

	void ModelTools::LoadSampleList(SampleList &sample_list, StringList &files) {
		ssi_char_t user_name[SSI_MAX_CHAR], label_name[SSI_MAX_CHAR], timestamp_s[SSI_MAX_CHAR];
		ssi_char_t filename[SSI_MAX_CHAR], filepath[SSI_MAX_CHAR];

		for (ssi_size_t i = 0; i < files.size(); i++) {
			if (files.get(i)[0] == '\0') {
				continue;
			}

			// copy filename
			ssi_strcpy(filepath, files.get(i));

			// remove directory
			ssi_strcpy(filename, filepath);
			for (size_t j = strlen(filepath) - 1; j >= 0; j--) {
#if _WIN32|_WIN64
				if (filepath[j] == '\\') {
#else
				if (filepath[j] == '/') {
#endif
					ssi_strcpy(filename, filepath + j + 1);
					break;
				}
				}

			// replace delimiter with white space
			for (unsigned int j = 0; j < strlen(filename); j++)
				if (filename[j] == SAMPLELIST_DELIM)
					filename[j] = ' ';

			// parse
			int result = sscanf(filename, "%s %s %s", user_name, label_name, timestamp_s);
			if (result != 3) {
				ssi_wrn("ignored %s", filename);
				continue;
			}
			int c = 0;
			while (timestamp_s[c] != '.')
				c++;
			timestamp_s[c] = '\0';
			time_t timestamp = ssi_time_parse(timestamp_s);

			// add class_id
			ssi_size_t class_id = sample_list.addClassName(label_name);
			ssi_size_t user_id = sample_list.addUserName(user_name);

			// load sample
			File *file = File::CreateAndOpen(File::BINARY, File::READ, files.get(i));
			ssi_size_t num = FileTools::CountStreamHeader(*file);
			ssi_stream_t **streams = new ssi_stream_t *[num];
			for (ssi_size_t j = 0; j < num; j++) {
				streams[j] = new ssi_stream_t;
				File::VERSION version;
				FileTools::ReadStreamHeader(*file, *streams[j], version);
				FileTools::ReadStreamData(*file, *streams[j], version);
				if (version == File::V0) {
					ssi_wrn("found old file format -> setting type to SSI_REAL");
					streams[j]->type = SSI_REAL;
				}
			}
			delete file;

			// create and add sample
			ssi_sample_t *sample = new ssi_sample_t;
			sample->class_id = class_id;
			sample->num = num;
			sample->score = 0.0f;
			sample->streams = streams;
			sample->time = streams[0]->time;
			sample->user_id = user_id;
			sample_list.addSample(sample);
			}
		}

	void ModelTools::LoadSampleList(SampleList &samples,
		const ssi_char_t *filepath,
		File::TYPE type) {
		FilePath fp(filepath);
		ssi_char_t *filepath_with_ext = 0;
		if (strcmp(fp.getExtension(), SSI_FILE_TYPE_SAMPLES) != 0) {
			filepath_with_ext = ssi_strcat(filepath, SSI_FILE_TYPE_SAMPLES);
		}
		else {
			filepath_with_ext = ssi_strcpy(filepath);
		}

		File *file = File::CreateAndOpen(type, File::READ, filepath_with_ext);
		LoadSampleList(samples, *file);
		delete file;

		ssi_msg(SSI_LOG_LEVEL_BASIC, "loaded sample list from file '%s'", filepath_with_ext);
		delete[] filepath_with_ext;
	}

	bool ModelTools::LoadSampleList(SampleList &samples,
		const ssi_char_t *file_name) {
		FileSamplesIn sin;
		if (!sin.open(file_name)) {
			return false;
		}

		ssi_size_t n_users = sin.getUserSize();
		ssi_size_t *user_map = new ssi_size_t[n_users];
		for (ssi_size_t i = 0; i < n_users; i++) {
			user_map[i] = samples.addUserName(sin.getUserName(i));
		}

		ssi_size_t n_classes = sin.getClassSize();
		ssi_size_t *class_map = new ssi_size_t[n_classes];
		for (ssi_size_t i = 0; i < n_classes; i++) {
			class_map[i] = samples.addClassName(sin.getClassName(i));
		}

		sin.reset();
		ssi_sample_t *sample;
		while (sample = sin.next()) {
			sample->class_id = sample->class_id == SSI_ISAMPLES_GARBAGE_CLASS_ID ? SSI_ISAMPLES_GARBAGE_CLASS_ID : class_map[sample->class_id];
			sample->user_id = user_map[sample->user_id];
			samples.addSample(sample, true);
		}

		delete[] user_map;
		delete[] class_map;

		if (!sin.close()) {
			return false;
		}

		return true;
	}

	void ModelTools::LoadSampleList(SampleList &samples,
		File &file) {
		ssi_char_t string[1024];

		ssi_size_t user_size = 0;
		ssi_size_t label_size = 0;

		// load user names
		if (file.getType() == File::ASCII) {
			if (!file.readLine(1024, string)) {
				ssi_err("could not read <user_size>");
			}
			sscanf(string, "%u", &user_size);
		}
		else {
			file.read(&user_size, sizeof(user_size), 1);
		}
		ssi_size_t *user_map = new ssi_size_t[user_size];
		for (ssi_size_t i = 0; i < user_size; i++) {
			if (!file.readLine(1024, string)) {
				ssi_err("could not read <user_name>");
			}
			user_map[i] = samples.addUserName(string);
		}

		// load label names
		if (file.getType() == File::ASCII) {
			if (!file.readLine(1024, string)) {
				ssi_err("could not read <label_size>");
			}
			sscanf(string, "%u", &label_size);
		}
		else {
			file.read(&label_size, sizeof(label_size), 1);
		}
		ssi_size_t *label_map = new ssi_size_t[label_size];
		for (ssi_size_t i = 0; i < label_size; i++) {
			if (!file.readLine(1024, string)) {
				ssi_err("could not read <label_name>");
			}
			label_map[i] = samples.addClassName(string);
		}

		// load samples
		ssi_sample_t *sample = new ssi_sample_t;
		while (FileTools::ReadSampleHeader(file, *sample)) {
			FileTools::ReadSampleData(file, *sample);
			sample->class_id = label_map[sample->class_id];
			sample->user_id = user_map[sample->user_id];
			samples.addSample(sample);
			sample = new ssi_sample_t;
		}
		delete sample;

		delete[] user_map;
		delete[] label_map;
	}

	bool ModelTools::LoadSampleList(SampleList &samples,
		const ssi_char_t *stream_path,
		const ssi_char_t *label_path,
		const ssi_char_t *user_name)
	{
		ssi_stream_t stream;
		FileTools::ReadStreamFile(stream_path, stream);
		ssi_size_t n_samples = stream.num;
		ssi_size_t n_features = stream.dim;
		
		ssi_size_t n_string;
		ssi_char_t *string = FileTools::ReadAsciiFile(label_path, n_string);
		ssi_size_t n_labels = ssi_split_string_count(string, '\n');		
		if (n_samples != n_labels)
		{
			ssi_wrn("number of samples '%u' != number of labels '%u'", n_samples, n_labels);
			return false;
		}
		ssi_char_t **labels = new ssi_char_t*[n_labels];
		ssi_split_string(n_labels, labels, string, '\n');

		ssi_size_t user_id = samples.addUserName(user_name);
				
		ssi_stream_t features;
		ssi_stream_init(features, 0, n_features, stream.byte, stream.type, stream.sr, 0);
		features.num = features.num_real = 1;
		features.tot = features.tot_real = n_features * stream.byte;
		features.ptr = stream.ptr;
		ssi_sample_t sample;
		ssi_sample_create(sample, 1, user_id, 0, 0, 1.0f);
		sample.streams[0] = &features;
		for (ssi_size_t i = 0; i < n_samples; i++)
		{
			sample.class_id = samples.addClassName(labels[i]);
			samples.addSample(&sample, true);
			features.ptr += features.tot;
		}

		for (ssi_size_t i = 0; i < n_labels; i++)
		{
			delete[] labels[i];
		}
		delete[] labels;
		delete[] string;

		return true;
	}

	void ModelTools::SaveSampleList(ISamples &sample_list,
		const ssi_char_t *dir,
		const ssi_char_t *type,
		ssi_char_t delim) {
		ssi_msg(SSI_LOG_LEVEL_DETAIL, "save samples to directory '%s'", dir);

		if (sample_list.getSize() == 0) {
			return;
		}

		char filename[SSI_MAX_CHAR];

		sample_list.reset();
		const ssi_sample_t *sample;
		const ssi_char_t *class_id;
		const ssi_char_t *user_id;
		while (sample = sample_list.next()) {
			class_id = sample_list.getClassName(sample->class_id);
			user_id = sample_list.getUserName(sample->user_id);
			ssi_size_t timestamp = ssi_cast(ssi_size_t, 1000 * sample->time + 0.5);
			//ssi_char_t timestamp_s[ssi_time_size];
			//ssi_time_format (timestamp, timestamp_s);
			//ssi_sprint (filename, "%s\\%s%c%s%c%s%s", dir, user_id, delim, class_id, delim, timestamp_s, type);
#if _WIN32|_WIN64
			ssi_sprint(filename, "%s\\%s%c%s%c%010ld%s", dir, user_id, delim, class_id, delim, timestamp, type);
#else
			ssi_sprint(filename, "%s/%s%c%s%c%010ld%s", dir, user_id, delim, class_id, delim, timestamp, type);

#endif
			File *file = File::CreateAndOpen(File::BINARY, File::WRITE, filename);
			for (ssi_size_t i = 0; i < sample->num; i++) {
				FileTools::WriteStreamHeader(*file, *sample->streams[i], File::V1);
				FileTools::WriteStreamData(*file, *sample->streams[i], File::V1);
			}
			delete file;
		}
	}

	bool ModelTools::SaveSampleList(ISamples &samples,
		const ssi_char_t *filepath,
		File::TYPE type,
		File::VERSION version) {
		if (version < File::V2) {
			FilePath fp(filepath);
			ssi_char_t *filepath_with_ext = 0;
			if (strcmp(fp.getExtension(), SSI_FILE_TYPE_SAMPLES) != 0) {
				filepath_with_ext = ssi_strcat(filepath, SSI_FILE_TYPE_SAMPLES);
			}
			else {
				filepath_with_ext = ssi_strcpy(filepath);
			}

			File *file = File::CreateAndOpen(type, File::WRITE, filepath_with_ext);
			SaveSampleList(samples, *file);
			delete file;

			ssi_msg(SSI_LOG_LEVEL_BASIC, "saved sample list to file '%s'", filepath_with_ext);
			delete[] filepath_with_ext;
		}
		else {
			FileSamplesOut sout;
			if (!sout.open(samples, filepath, type, version)) {
				return false;
			}
			sout.write(samples);
			if (!sout.close()) {
				return false;
			}
		}

		return true;
	}

	void ModelTools::SaveSampleList(ISamples &samples,
		File &file) {
		ssi_msg(SSI_LOG_LEVEL_DETAIL, "save samples to file '%s'", file.getPath());

		ssi_char_t string[1024];

		ssi_size_t user_size = samples.getUserSize();
		ssi_size_t label_size = samples.getClassSize();

		// store user names
		if (file.getType() == File::ASCII) {
			sprintf(string, "%u", user_size);
			file.writeLine(string);
		}
		else {
			file.write(&user_size, sizeof(user_size), 1);
		}
		for (ssi_size_t i = 0; i < user_size; i++) {
			file.writeLine(samples.getUserName(i));
		}

		// store label names
		if (file.getType() == File::ASCII) {
			sprintf(string, "%u", label_size);
			file.writeLine(string);
		}
		else {
			file.write(&label_size, sizeof(label_size), 1);
		}
		for (ssi_size_t i = 0; i < label_size; i++) {
			file.writeLine(samples.getClassName(i));
		}

		// now store samples
		samples.reset();
		ssi_sample_t *sample;
		while (sample = samples.next()) {
			FileTools::WriteSampleHeader(file, *sample, File::V1);
			FileTools::WriteSampleData(file, *sample, File::V1);
		}
	}

	void ModelTools::SaveSampleListArff(ISamples &sample_list,
		ssi_char_t *filename,
		const ssi_char_t **feature_names) {
		FILE *file = fopen(filename, "w");
		if (!file) {
			ssi_err_static("fopen() failed: '%s'", filename);
		}

		ssi_size_t sample_dimension = 0;
		for (ssi_size_t i = 0; i < sample_list.getStreamSize(); i++) {
			sample_dimension += sample_list.getStream(i).dim;
		}

		fprintf(file, "@RELATION ssi\n\n");

		if (feature_names) {
			for (ssi_size_t i = 0; i < sample_dimension; i++) {
				fprintf(file, "@ATTRIBUTE %s NUMERIC\n", feature_names[i]);
			}
		}
		else {
			for (ssi_size_t i = 0; i < sample_dimension; i++) {
				fprintf(file, "@ATTRIBUTE feature_%u NUMERIC\n", i);
			}
		}

		fprintf(file, "@ATTRIBUTE Class {");
		bool first_label_name = true;
		for (ssi_size_t i = 0; i < sample_list.getClassSize(); i++) {
			if (!first_label_name) {
				fprintf(file, ",");
			}
			fprintf(file, sample_list.getClassName(i));
			first_label_name = false;
		}
		fprintf(file, "}\n\n");

		fprintf(file, "@DATA\n");
		sample_list.reset();
		const ssi_sample_t *sample = 0;
		ssi_real_t *sample_ptr = 0;
		ssi_size_t sample_elems = 0;
		while (sample = sample_list.next()) {
			const char *class_id = sample_list.getClassName(sample->class_id);
			// we print only the first stream
			if (sample->streams[0]->num != 0) {
				sample_ptr = ssi_pcast(ssi_real_t, sample->streams[0]->ptr);
				sample_elems = sample->streams[0]->dim;
				// we print only the first sample
				for (ssi_size_t i = 0; i < sample_elems; i++) {
#if __MINGW32__||__gnu_linux__
					if ((*sample_ptr) != (*sample_ptr) || std::isfinite(*sample_ptr))
#else
					if (_isnan(*sample_ptr) || !_finite(*sample_ptr))
#endif // __MINGW32__
					{
						fprintf(file, "0.0,");
					}
					else {
						fprintf(file, "%f,", *sample_ptr);
					}
					sample_ptr++;
				}
			}
			else {
				ssi_wrn("skip sample because of missing data");
				continue;
				}
			fprintf(file, "%s\n", class_id);
			}

		fclose(file);
		}

	void ModelTools::CopySampleList(ISamples &from, SampleList &to)
	{
		// copy labels
		ssi_size_t label_size = from.getClassSize();
		ssi_size_t *new_label = new ssi_size_t[label_size];
		for (unsigned int i = 0; i < label_size; i++) {
			new_label[i] = to.addClassName(from.getClassName(i));
		}

		// copy ids
		ssi_size_t id_size = from.getUserSize();
		ssi_size_t *new_id = new ssi_size_t[id_size];
		for (unsigned int i = 0; i < id_size; i++) {
			new_id[i] = to.addUserName(from.getUserName(i));
		}

		// copy samples
		from.reset();
		const ssi_sample_t *sample;
		ssi_sample_t *clone;
		while (sample = from.next())
		{
			clone = new ssi_sample_t;
			ssi_sample_clone(*sample, *clone);
			clone->class_id = clone->class_id == SSI_ISAMPLES_GARBAGE_CLASS_ID ? SSI_ISAMPLES_GARBAGE_CLASS_ID : new_label[sample->class_id];
			clone->user_id = new_id[sample->user_id];
			to.addSample(clone);
		}

		// clean up
		delete[] new_id;
		delete[] new_label;
	}

	void ModelTools::CopySampleList(ISamples &from,
		SampleList &to,
		ssi_size_t stream_index)
	{
		if (from.getSize() == 0) {
			return;
		}

		if (stream_index >= from.getStreamSize()) {
			ssi_err("invalid stream index");
		}

		// copy labels
		ssi_size_t label_size = from.getClassSize();
		ssi_size_t *new_label = new ssi_size_t[label_size];
		for (unsigned int i = 0; i < label_size; i++) {
			new_label[i] = to.addClassName(from.getClassName(i));
		}

		// copy ids
		ssi_size_t id_size = from.getUserSize();
		ssi_size_t *new_id = new ssi_size_t[id_size];
		for (unsigned int i = 0; i < id_size; i++) {
			new_id[i] = to.addUserName(from.getUserName(i));
		}

		// copy samples
		from.reset();
		const ssi_sample_t *sample;
		ssi_sample_t *clone;
		while (sample = from.next())
		{
			clone = new ssi_sample_t;
			clone->class_id = new_label[sample->class_id];
			clone->user_id = new_id[sample->user_id];
			clone->num = 1;
			clone->score = sample->score;
			clone->time = sample->time;
			clone->streams = new ssi_stream_t *[1];
			clone->streams[0] = new ssi_stream_t;
			ssi_stream_clone(*sample->streams[stream_index], *clone->streams[0]);
			to.addSample(clone);
		}

		// clean up
		delete[] new_id;
		delete[] new_label;
	}

	void ModelTools::SelectSampleList(ISamples &from,
		SampleList &to_select,
		SampleList &to_remain,
		ssi_size_t n_indices,
		ssi_size_t *indices)
	{
		// copy user_id
		ssi_size_t id_size = from.getUserSize();
		ssi_size_t *new_id_select = new ssi_size_t[id_size];
		ssi_size_t *new_id_remain = new ssi_size_t[id_size];
		for (unsigned int i = 0; i < id_size; i++) {
			new_id_select[i] = to_select.addUserName(from.getUserName(i));
			new_id_remain[i] = to_remain.addUserName(from.getUserName(i));
		}

		// copy class_id
		ssi_size_t label_size = from.getClassSize();
		ssi_size_t *new_label_select = new ssi_size_t[label_size];
		ssi_size_t *new_label_remain = new ssi_size_t[label_size];
		for (unsigned int i = 0; i < label_size; i++) {
			new_label_select[i] = to_select.addClassName(from.getClassName(i));
			new_label_remain[i] = to_remain.addClassName(from.getClassName(i));
		}

		// copy samples

		qsort(indices, n_indices, sizeof(int), model_tools_compare);

		from.reset();
		const ssi_sample_t *sample = 0;
		ssi_size_t i = 0;
		while (sample = from.next())
		{
			ssi_sample_t *clone = new ssi_sample_t;
			ssi_sample_clone(*sample, *clone);
			if (n_indices > 0 && i == *indices)
			{
				++indices;
				--n_indices;
				clone->class_id = new_label_select[sample->class_id];
				clone->user_id = new_id_select[sample->user_id];
				to_select.addSample(clone, false);
			}
			else
			{
				clone->class_id = new_label_remain[sample->class_id];
				clone->user_id = new_id_remain[sample->user_id];
				to_remain.addSample(clone, false);
			}
			++i;
		}

		delete[] new_id_select;
		delete[] new_id_remain;
		delete[] new_label_select;
		delete[] new_label_remain;
	};

	void ModelTools::SelectSampleList(ISamples &from,
		SampleList &to,
		ssi_size_t n_label_ids,
		ssi_size_t *label_ids) {
		// copy ids
		ssi_size_t id_size = from.getUserSize();
		ssi_size_t *new_id = new ssi_size_t[id_size];
		for (ssi_size_t i = 0; i < id_size; i++) {
			new_id[i] = to.addUserName(from.getUserName(i));
		}

		// copy label id
		ssi_size_t *new_label = new ssi_size_t[n_label_ids];
		for (ssi_size_t i = 0; i < n_label_ids; i++) {
			new_label[i] = to.addClassName(from.getClassName(label_ids[i]));
		}

		// copy samples
		from.reset();
		const ssi_sample_t *sample;
		ssi_sample_t *clone;
		while (sample = from.next()) {
			for (ssi_size_t i = 0; i < n_label_ids; i++) {
				if (sample->class_id == label_ids[i]) {
					clone = new ssi_sample_t;
					ssi_sample_clone(*sample, *clone);
					clone->class_id = new_label[i];
					clone->user_id = new_id[sample->user_id];
					to.addSample(clone);
					break;
				}
			}
		}
	}

	void ModelTools::SelectSampleList(ISamples &from,
		SampleList &to,
		ssi_size_t class_id) {
		// copy ids
		ssi_size_t id_size = from.getUserSize();
		ssi_size_t *new_id = new ssi_size_t[id_size];
		for (unsigned int i = 0; i < id_size; i++) {
			new_id[i] = to.addUserName(from.getUserName(i));
		}

		// copy class_id
		ssi_size_t new_label = to.addClassName(from.getClassName(class_id));

		// copy samples
		from.reset();
		const ssi_sample_t *sample;
		ssi_sample_t *clone;
		while (sample = from.next()) {
			if (sample->class_id == class_id) {
				clone = new ssi_sample_t;
				ssi_sample_clone(*sample, *clone);
				clone->class_id = new_label;
				clone->user_id = new_id[sample->user_id];
				to.addSample(clone);
			}
		}
	}

	void ModelTools::MergeSampleList(SampleList &to,
		ssi_size_t n_from,
		SampleList *from[],
		bool ignoreOverfull) {

		ssi_size_t n_samples = from[0]->getSize();
		ssi_size_t n_labels = from[0]->getClassSize();
		ssi_size_t n_users = from[0]->getUserSize();
		ssi_size_t n_streams = from[0]->get(0)->num;

		for (ssi_size_t i = 1; i < n_from; i++) {

			if ((n_samples != from[i]->getSize() && !ignoreOverfull) ||
				n_labels != from[i]->getClassSize() ||
				n_users != from[i]->getUserSize() ||
				n_streams != from[i]->get(0)->num) {
				ssi_err_static("sample lists are not compatible");
			}

			//find min num samples
			if (ignoreOverfull && from[i]->getSize() < n_samples)
				n_samples = from[i]->getSize();
		}
		
		for (ssi_size_t i = 0; i < n_streams; i++) {
			ssi_size_t n_bytes = from[0]->get(0)->streams[i]->byte;
			for (ssi_size_t j = 1; j < n_from; j++) {
				if (n_bytes != from[0]->get(0)->streams[i]->byte) {
					ssi_err_static("sample lists are not compatible");
				}
			}
		}

		if (n_samples == 0) {
			return;
		}

		// copy labels
		ssi_size_t *label_map = new ssi_size_t[n_labels];
		for (ssi_size_t i = 0; i < n_labels; i++) {
			label_map[i] = to.addClassName(from[0]->getClassName(i));
		}

		// copy ids
		ssi_size_t *user_map = new ssi_size_t[n_users];
		for (ssi_size_t i = 0; i < n_users; i++) {
			user_map[i] = to.addUserName(from[0]->getUserName(i));
		}

		// determine merged dimension
		ssi_size_t *n_dimension = new ssi_size_t[n_streams];
		for (ssi_size_t j = 0; j < n_streams; j++) {
			n_dimension[j] = 0;
		}
		for (ssi_size_t i = 0; i < n_from; i++) {
			for (ssi_size_t j = 0; j < n_streams; j++) {
				n_dimension[j] += from[i]->get(0)->streams[j]->dim;
			}
		}

		// merge samples
		for (ssi_size_t i = 0; i < n_from; i++) {
			from[i]->reset();
		}
		ssi_sample_t const**samples = new ssi_sample_t const*[n_from];
		ssi_sample_t *new_sample = 0;
		ssi_size_t n_num = 0;
		ssi_type_t sample_type = SSI_UNDEF;
		ssi_time_t sample_rate = 0;
		ssi_size_t n_bytes = 0;
		ssi_byte_t *stream_ptr = 0;
		ssi_stream_t **streams = 0;
		for (ssi_size_t n = 0; n < n_samples; n++) {
			// get input samples
			samples[0] = from[0]->next();
			streams = new ssi_stream_t *[n_streams];
			for (ssi_size_t m = 0; m < n_streams; m++) {
				// check if compatible
				n_num = samples[0]->streams[m]->num;
				n_bytes = samples[0]->streams[m]->byte;
				sample_rate = samples[0]->streams[m]->sr;
				sample_type = samples[0]->streams[m]->type;
				for (ssi_size_t i = 1; i < n_from; i++) {
					samples[i] = from[i]->next();
					if (samples[i]->streams[m]->num != n_num || samples[i]->streams[m]->type != sample_type || samples[i]->streams[m]->sr != sample_rate) {
						ssi_err_static("sample lists are not compatible");
					}
				}
				// merge stream
				streams[m] = new ssi_stream_t;
				ssi_stream_init(*streams[m], n_num, n_dimension[m], n_bytes, sample_type, sample_rate);
				stream_ptr = streams[m]->ptr;
				for (ssi_size_t i = 0; i < n_from; i++) {
					memcpy(stream_ptr, samples[i]->streams[m]->ptr, samples[i]->streams[m]->tot);
					stream_ptr += samples[i]->streams[m]->tot;
				}
			}

			// create and add sample
			new_sample = new ssi_sample_t;
			new_sample->class_id = label_map[samples[0]->class_id];
			new_sample->num = n_streams;
			new_sample->score = 0.0f;
			new_sample->streams = streams;
			new_sample->time = samples[0]->time;
			new_sample->user_id = user_map[samples[0]->user_id];
			to.addSample(new_sample);
		}

		// clean up
		delete[] samples;
		delete[] label_map;
		delete[] user_map;
		delete[] n_dimension;
	}

	void ModelTools::TransformSampleList(ISamples &from,
		SampleList &to,
		ITransformer &transformer,
		ssi_size_t frame_size,
		ssi_size_t delta_size,
		ModelTools::CALL_ENTER_AND_FLUSH call_enter_and_flush) {
		ITransformer *t = &transformer;
		ModelTools::TransformSampleList(from, to, 1, &t, frame_size, delta_size, call_enter_and_flush);
	}

	void ModelTools::TransformSampleList(ISamples &from,
		SampleList &to,
		ssi_size_t num,
		ITransformer *transformers[],
		ssi_size_t frame_size,
		ssi_size_t delta_size,
		ModelTools::CALL_ENTER_AND_FLUSH call_enter_and_flush) {
		if (from.getSize() == 0) {
			return;
		}

		ssi_size_t n_streams = from.getStreamSize();

		if (num != 1 && num != n_streams) {
			ssi_err("number of streams in sample list not compatible with number of transformer");
		}

		// copy labels
		ssi_size_t *label_map = new ssi_size_t[from.getClassSize()];
		for (unsigned int i = 0; i < from.getClassSize(); i++) {
			label_map[i] = to.addClassName(from.getClassName(i));
		}

		// copy ids
		ssi_size_t *user_map = new ssi_size_t[from.getUserSize()];
		for (unsigned int i = 0; i < from.getUserSize(); i++) {
			user_map[i] = to.addUserName(from.getUserName(i));
		}

		// help array with transformer
		ITransformer **transformers_h = transformers;
		if (num == 1) {
			transformers_h = new ITransformer *[n_streams];
			for (ssi_size_t i = 0; i < n_streams; i++) {
				transformers_h[i] = transformers[0];
			}
		}

		// transform samples
		from.reset();
		const ssi_sample_t *sample = 0;
		ssi_sample_t *new_sample = 0;
		ssi_size_t counter = 0;
		ssi_size_t counter_max = from.getSize() - 1;
		bool call_enter = false;
		bool call_flush = false;
		while (sample = from.next()) {
			call_enter = call_enter_and_flush == ModelTools::CALL_ALWAYS;
			if (counter == 0 && call_enter_and_flush == ModelTools::CALL_ONCE) {
				call_enter = true;
			}
			call_flush = call_enter_and_flush == ModelTools::CALL_ALWAYS;
			if (counter == counter_max && call_enter_and_flush == ModelTools::CALL_ONCE) {
				call_flush = true;
			}
			++counter;

			ssi_stream_t **result = new ssi_stream_t *[num];
			for (ssi_size_t i = 0; i < num; i++) {
				result[i] = new ssi_stream_t;
				if (transformers_h[i]) {
					SignalTools::Transform(*sample->streams[i], *result[i], *transformers_h[i], frame_size, delta_size, call_enter, call_flush);
				}
				else {
					ssi_stream_clone(*sample->streams[i], *result[i]);
				}
			}

			// create and add sample
			new_sample = new ssi_sample_t;
			new_sample->class_id = sample->class_id == SSI_ISAMPLES_GARBAGE_CLASS_ID ? SSI_ISAMPLES_GARBAGE_CLASS_ID : label_map[sample->class_id];
			new_sample->num = num;
			new_sample->score = 0.0f;
			new_sample->streams = result;
			new_sample->time = sample->time;
			new_sample->user_id = sample->user_id == SSI_ISAMPLES_GARBAGE_USER_ID ? SSI_ISAMPLES_GARBAGE_USER_ID : user_map[sample->user_id];
			to.addSample(new_sample);
		}

		// clean up
		if (num == 1) {
			delete[] transformers_h;
		}

		delete[] label_map;
		delete[] user_map;
	}

	void ModelTools::TransformSampleListWithExtraStream(ISamples &from, ISamples **from_extra,
		SampleList &to,
		ssi_size_t num, ssi_size_t num_extra,
		ITransformer *transformers[],
		ssi_size_t frame_size,
		ssi_size_t delta_size,
		ModelTools::CALL_ENTER_AND_FLUSH call_enter_and_flush) {
		if (from.getSize() == 0) {
			return;
		}
		ssi_size_t n_streams = from.getStream(0).dim;

		if (num != 1 && num != n_streams) {
			ssi_err("number of streams in sample list not compatible with number of transformer");
		}

		// copy labels
		ssi_size_t *label_map = new ssi_size_t[from.getClassSize()];
		for (unsigned int i = 0; i < from.getClassSize(); i++) {
			label_map[i] = to.addClassName(from.getClassName(i));
		}

		// copy ids
		ssi_size_t *user_map = new ssi_size_t[from.getUserSize()];
		for (unsigned int i = 0; i < from.getUserSize(); i++) {
			user_map[i] = to.addUserName(from.getUserName(i));
		}

		// help array with transformer
		ITransformer **transformers_h = transformers;
		if (num == 1) {
			transformers_h = new ITransformer *[n_streams];
			for (ssi_size_t i = 0; i < n_streams; i++) {
				transformers_h[i] = transformers[0];
			}
		}

		// transform samples
		from.reset();

		for (ssi_size_t i = 0; i < num_extra; i++) {
			from_extra[i]->reset();
		}

		const ssi_sample_t *sample = 0;
		const ssi_sample_t *sample_extra = 0;
		ssi_sample_t *new_sample = 0;
		ssi_size_t counter = 0;
		ssi_size_t counter_max = from.getSize() - 1;
		bool call_enter = false;
		bool call_flush = false;
		while (sample = from.next()) {
			ssi_stream_t* extra_stream = new ssi_stream_t[num_extra];
			for (ssi_size_t i = 0; i < num_extra; i++) {
				extra_stream[i] = *(from_extra[i]->next()->streams[0]);
			}

			call_enter = call_enter_and_flush == ModelTools::CALL_ALWAYS;
			if (counter == 0 && call_enter_and_flush == ModelTools::CALL_ONCE) {
				call_enter = true;
			}
			call_flush = call_enter_and_flush == ModelTools::CALL_ALWAYS;
			if (counter == counter_max && call_enter_and_flush == ModelTools::CALL_ONCE) {
				call_flush = true;
			}
			++counter;

			ssi_stream_t **result = new ssi_stream_t *[num];
			for (ssi_size_t i = 0; i < num; i++) {
				result[i] = new ssi_stream_t;
				if (transformers_h[i]) {
					SignalTools::Transform_Xtra(*sample->streams[i], *result[i], *transformers_h[i], frame_size, delta_size, call_enter, call_flush, num_extra, extra_stream);
				}
				else {
					ssi_stream_clone(*sample->streams[i], *result[i]);
				}
			}

			// create and add sample
			new_sample = new ssi_sample_t;
			new_sample->class_id = label_map[sample->class_id];
			new_sample->num = num;
			new_sample->score = 0.0f;
			new_sample->streams = result;
			new_sample->time = sample->time;
			new_sample->user_id = user_map[sample->user_id];
			to.addSample(new_sample);
		}

		// clean up
		if (num == 1) {
			delete[] transformers_h;
		}

		delete[] label_map;
		delete[] user_map;
	}

	void ModelTools::AlignStreams(ssi_size_t num,
		ssi_stream_t *from[],
		ssi_stream_t &to) {
		// determine aligned dimension
		ssi_size_t dim = 0;
		for (ssi_size_t i = 0; i < num; i++) {
			dim += from[i]->dim;
		}

		// determine aligned bytes
		ssi_size_t byte = from[0]->byte;
		for (ssi_size_t i = 1; i < num; i++) {
			if (byte != from[i]->byte) {
				ssi_err("number of bytes must be equal in all streams");
			}
		}

		// create aligned stream
		ssi_stream_init(to, from[0]->num, dim, byte, from[0]->type, from[0]->sr);
		ssi_byte_t *ptr = to.ptr;
		for (ssi_size_t i = 0; i < num; i++) {
			memcpy(ptr, from[i]->ptr, from[i]->tot);
			ptr += from[i]->tot;
		}
	}

	void ModelTools::AlignSampleList(SampleList &from,
		SampleList &to) {
		if (from.getSize() == 0) {
			return;
		}

		ssi_size_t n_streams = from.get(0)->num;

		// copy labels
		ssi_size_t n_labels = from.getClassSize();
		ssi_size_t *label_map = new ssi_size_t[n_labels];
		for (ssi_size_t i = 0; i < n_labels; i++) {
			label_map[i] = to.addClassName(from.getClassName(i));
		}

		// copy ids
		ssi_size_t n_user = from.getUserSize();
		ssi_size_t *user_map = new ssi_size_t[n_user];
		for (ssi_size_t i = 0; i < n_user; i++) {
			user_map[i] = to.addUserName(from.getUserName(i));
		}

		// determine aligned dimension
		ssi_size_t n_dim = 0;
		for (ssi_size_t i = 0; i < n_streams; i++) {
			n_dim += from.getStream(i).dim;
		}

		// determine aligned bytes
		ssi_size_t n_bytes = from.getStream(0).byte;
		for (ssi_size_t i = 1; i < n_streams; i++) {
			if (n_bytes != from.getStream(i).byte) {
				ssi_err("number of bytes must be equal in all streams");
			}
		}

		// align samples
		const ssi_sample_t *sample = 0;
		ssi_sample_t *new_sample = 0;
		ssi_stream_t **stream = 0;
		ssi_byte_t *stream_ptr = 0;
		ssi_size_t n_num = 0;
		ssi_time_t sample_rate = 0;
		from.reset();
		while (sample = from.next()) {
			// create aligned stream
			stream = new ssi_stream_t *[1];
			stream[0] = new ssi_stream_t;
			AlignStreams(sample->num, sample->streams, *stream[0]);

			// create and add new sample
			new_sample = new ssi_sample_t;
			new_sample->class_id = label_map[sample->class_id];
			new_sample->user_id = user_map[sample->user_id];
			new_sample->num = 1;
			new_sample->streams = stream;
			new_sample->score = sample->score;
			new_sample->time = sample->time;
			to.addSample(new_sample);
		}

		// clean up
		delete[] user_map;
		delete[] label_map;
	}

	void ModelTools::CreateTestSamples(SampleList &samples,
		ssi_size_t n_classes,
		ssi_size_t n_samples_per_class,
		ssi_size_t n_streams,
		ssi_real_t distr[][3],
		const ssi_char_t* user) {
		ssi_char_t string[SSI_MAX_CHAR];

		ssi_size_t user_id = samples.addUserName(user);
		ssi_size_t *class_id = new ssi_size_t[n_classes];
		for (ssi_size_t n_class = 0; n_class < n_classes; n_class++) {
			ssi_sprint(string, "%c", ssi_cast(char, 'A' + n_class));
			class_id[n_class] = samples.addClassName(string);
		}

		ssi_sample_t *sample = 0;
		ssi_real_t *data = 0;
		ssi_time_t time = ssi_random(60.0);
		for (ssi_size_t n_class = 0; n_class < n_classes; n_class++) {
			ssi_real_t center_x = distr[n_class][0];
			ssi_real_t center_y = distr[n_class][1];
			ssi_real_t spread = distr[n_class][2];
			for (ssi_size_t n_sample = 0; n_sample < n_samples_per_class; n_sample++) {
				ssi_stream_t **streams = new ssi_stream_t *[n_streams];
				for (ssi_size_t n_stream = 0; n_stream < n_streams; n_stream++) {
					streams[n_stream] = new ssi_stream_t;
					ssi_stream_init(*streams[n_stream], 1, 2, sizeof(ssi_real_t), SSI_REAL, 10.0);
					data = ssi_pcast(ssi_real_t, streams[n_stream]->ptr);
					//data[0] = center_x + (spread * (rand () - RAND_MAX/2)) / (RAND_MAX/2);
					data[0] = ssi_cast(ssi_real_t, ssi_random_distr(center_x, spread));
					//data[1] = center_y + (spread * (rand () - RAND_MAX/2)) / (RAND_MAX/2);
					data[1] = ssi_cast(ssi_real_t, ssi_random_distr(center_y, spread));
					//ssi_print ("%f %f %f %f\n", center_x, center_y, data[0], data[1]);
				}
				sample = new ssi_sample_t;
				sample->class_id = class_id[n_class];
				sample->user_id = user_id;
				sample->num = n_streams;
				sample->score = 0.0f;
				sample->time = time;
				time += ssi_random(60.0);
				sample->streams = streams;
				samples.addSample(sample);
			}
		}

		delete[] class_id;
	}

	void ModelTools::CreateDynamicTestSamples(SampleList &samples,
		ssi_size_t n_classes,
		ssi_size_t n_samples_per_class,
		ssi_size_t n_streams,
		ssi_real_t distr[][3],
		ssi_size_t num_min,
		ssi_size_t num_max,
		const ssi_char_t* user) {
		ssi_char_t string[SSI_MAX_CHAR];

		ssi_size_t user_id = samples.addUserName(user);
		ssi_size_t *class_id = new ssi_size_t[n_classes];
		for (ssi_size_t n_class = 0; n_class < n_classes; n_class++) {
			ssi_sprint(string, "%c", ssi_cast(char, 'A' + n_class));
			class_id[n_class] = samples.addClassName(string);
		}

		ssi_sample_t *sample = 0;
		ssi_real_t *data = 0;
		ssi_time_t time = ssi_random(1.0);
		for (ssi_size_t n_class = 0; n_class < n_classes; n_class++) {
			ssi_real_t center_x = distr[n_class][0];
			ssi_real_t center_y = distr[n_class][1];
			ssi_real_t spread = distr[n_class][2];
			for (ssi_size_t n_sample = 0; n_sample < n_samples_per_class; n_sample++) {
				ssi_stream_t **streams = new ssi_stream_t *[n_streams];
				ssi_size_t num = num_min + ssi_random(num_max - num_min);
				for (ssi_size_t n_stream = 0; n_stream < n_streams; n_stream++) {
					streams[n_stream] = new ssi_stream_t;
					ssi_stream_init(*streams[n_stream], num, 2, sizeof(ssi_real_t), SSI_REAL, 10.0);
					data = ssi_pcast(ssi_real_t, streams[n_stream]->ptr);
					for (ssi_size_t n = 0; n < num; n++) {
						*data++ = ssi_cast(ssi_real_t, ssi_random_distr(center_x, spread));
						*data++ = ssi_cast(ssi_real_t, ssi_random_distr(center_y, spread));
					}
				}
				sample = new ssi_sample_t;
				sample->class_id = class_id[n_class];
				sample->user_id = user_id;
				sample->num = n_streams;
				sample->score = 0.0f;
				sample->time = time;
				time += ssi_random(1.0);
				sample->streams = streams;
				samples.addSample(sample);
			}
		}

		delete[] class_id;
	}

	void ModelTools::CreateMissingData(SampleList &samples, double prob) {
		ssi_size_t n_streams = samples.getStreamSize();
		ssi_sample_t *sample = 0;
		samples.reset();
		while (sample = samples.next()) {
			for (ssi_size_t nstrm = 0; nstrm < n_streams; nstrm++) {
				if (ssi_random() > prob) {
					ssi_stream_reset(*sample->streams[nstrm]);
				}
			}
		}
		samples.setMissingData(true);
	}

	void ModelTools::PrintInfo(ISamples &samples, FILE *file) {
		ssi_fprint_off(file, "samples:   %ux%u\n", samples.getSize(), samples.getStreamSize());
		ssi_fprint_off(file, "per class: ");
		for (ssi_size_t nclass = 0; nclass < samples.getClassSize(); nclass++) {
			ssi_fprint(file, "%u ", samples.getSize(nclass));
		}
		ssi_fprint(file, "\n");
		ssi_fprint_off(file, "names:     %u ( ", samples.getClassSize());
		for (ssi_size_t nclass = 0; nclass < samples.getClassSize(); nclass++) {
			ssi_fprint(file, "%s ", samples.getClassName(nclass));
		}
		ssi_fprint(file, ")\n");
		ssi_fprint_off(file, "users:     %u ( ", samples.getUserSize());
		for (ssi_size_t nuser = 0; nuser < samples.getUserSize(); nuser++) {
			ssi_fprint(file, "%s ", samples.getUserName(nuser));
		}
		ssi_fprint(file, ")\n");
		ssi_fprint_off(file, "streams:   %u ( ", samples.getStreamSize());
		for (ssi_size_t nstream = 0; nstream < samples.getStreamSize(); nstream++) {
			ssi_fprint(file, "%u:%s ", samples.getStream(nstream).dim, SSI_TYPE_NAMES[samples.getStream(nstream).type]);
		}
		ssi_fprint(file, ")\n");
	}

	void ModelTools::PrintSample(ISamples &samples, ssi_size_t index, FILE *file) {
		ssi_sample_t *sample = samples.get(index);
		ssi_fprint(file, "class:   %s\n", samples.getClassName(sample->class_id));
		ssi_fprint(file, "user:    %s\n", samples.getUserName(sample->user_id));
		ssi_fprint(file, "streams: %u\n", sample->num);
		File *file_h = File::Create(File::ASCII, File::WRITE, 0, file);
		ssi_stream_t *stream = 0;
		for (ssi_size_t i = 0; i < sample->num; i++) {
			ssi_fprint(file, "#%02u:\n", i);
			stream = sample->streams[i];
			if (stream->num > 0) {
				FileTools::WriteStreamData(*file_h, *stream, File::DEFAULT_VERSION, false);
			}
			else {
				ssi_fprint(file, "MISSING\n");
			}
		}
		delete file_h;
	}

	void ModelTools::PrintSamples(ISamples &samples, FILE *file) {
		ssi_size_t n_samples = samples.getSize();
		ssi_fprint(file, "samples: %u\n", n_samples);
		for (ssi_size_t i = 0; i < n_samples; i++) {
			ssi_fprint(file, "#%02u:\n", i);
			PrintSample(samples, i, file);
		}
	}

	void ModelTools::PrintClasses(ISamples &samples, FILE *file) {
		ssi_size_t n_samples = samples.getSize();
		ssi_fprint(file, "samples: %u\n", n_samples);
		for (ssi_size_t i = 0; i < n_samples; i++) {
			ssi_fprint(file, "#%02u: %s\n", i, samples.getClassName(samples.get(i)->class_id));
		}
	}

	void ModelTools::CreateSampleMatrix(ISamples &samples, ssi_size_t stream_index, ssi_size_t &n_samples, ssi_size_t &n_features, ssi_size_t **classes, ssi_real_t ***matrix) {
		samples.reset();

		n_features = samples.getStream(stream_index).dim;
		n_samples = samples.getSize();
		ssi_real_t *matrix_data = new ssi_real_t[n_samples * n_features];
		*matrix = new ssi_real_t *[n_samples];
		*classes = new ssi_size_t[n_samples];

		ssi_sample_t *sample = 0;
		ssi_size_t bytes = n_features * sizeof(ssi_real_t);
		for (ssi_size_t i = 0; i < n_samples; i++) {
			sample = samples.next();
			(*classes)[i] = sample->class_id;
			(*matrix)[i] = matrix_data + i * n_features;
			memcpy((*matrix)[i], sample->streams[stream_index]->ptr, bytes);
		}
	}

	void ModelTools::ReleaseSampleMatrix(ssi_size_t n_samples, ssi_size_t *classes, ssi_real_t **matrix) {
		delete[] classes;
		delete[] matrix[0];
		delete[] matrix;
	}

	void ModelTools::FromSampleMatrix(SampleList &samples, ssi_size_t n_samples, ssi_size_t n_features, ssi_size_t *classes, ssi_real_t **matrix) {
		ssi_size_t user_id = samples.addUserName("dummy");

		ssi_sample_t sample;
		for (ssi_size_t i = 0; i < n_samples; i++) {
			sample.class_id = classes[i];
			sample.user_id = 0;
			sample.time = 0;
			sample.score = 0.0f;
			sample.num = 1;
			sample.streams = new ssi_stream_t *[1];
			ssi_stream_t stream;
			ssi_stream_init(stream, 0, n_features, sizeof(ssi_real_t), SSI_REAL, 0, 0);
			stream.ptr = ssi_pcast(ssi_byte_t, matrix[i]);
			stream.num = stream.num_real = 1;
			stream.tot = stream.tot_real = sizeof(ssi_real_t) * n_features;
			sample.streams[0] = &stream;
			samples.addSample(&sample, true);
			delete[] sample.streams;
		}
	}
#ifndef HEADLESS
	void ModelTools::PlotSamples(ISamples &samples, const ssi_char_t *name, ssi_rect_t pos) {
		uint16_t COLORS[][3] = {
			128, 0, 0,
			0, 128, 0,
			0, 0, 128,
			128, 0, 128,
			0, 128, 128,
			255, 128, 0,
			0, 128, 255
		};

		ssi_size_t n_classes = samples.getClassSize();
		PaintData **plots = new PaintData *[n_classes];

		Canvas canvas;

		for (ssi_size_t n_class = 0; n_class < n_classes; n_class++) {
			plots[n_class] = new PaintData;

			ssi_size_t n_samples = samples.getSize(n_class);
			ssi_size_t n_streams = samples.getStreamSize();

			ssi_stream_t stream;
			ssi_stream_init(stream, n_samples * n_streams, 2, sizeof(ssi_real_t), SSI_REAL, 0);

			ssi_real_t *data = ssi_pcast(ssi_real_t, stream.ptr);

			ssi_sample_t *sample = 0;
			ssi_real_t *data_ptr = data;
			samples.reset();
			while (sample = samples.next()) {
				if (sample->class_id == n_class) {
					for (ssi_size_t n_stream = 0; n_stream < n_streams; n_stream++) {
						memcpy(data_ptr, sample->streams[n_stream]->ptr, 2 * sizeof(ssi_real_t));
						data_ptr += 2;
					}
				}
			}

			plots[n_class]->setBackground(n_class == 0, IPainter::ITool::COLORS::BLACK);
			plots[n_class]->setPointSize(10);
			plots[n_class]->setLimits(-0.2f, 1.2f);
			plots[n_class]->setData(stream, PaintData::TYPE::SCATTER);
			plots[n_class]->setBrush(ssi_rgb(COLORS[n_class][0], COLORS[n_class][1], COLORS[n_class][2]));
			plots[n_class]->setPen(IPainter::ITool::COLORS::BLACK);
			canvas.addClient(plots[n_class]);

			ssi_stream_destroy(stream);
		}

		Window window;
		window.setTitle(name);
		window.setClient(&canvas);
		window.setPosition(pos);

		window.create();
		window.show();

		ssi_print("\n\n\tpress enter to continue\n\n");
		getchar();

		window.close();

		for (ssi_size_t n_class = 0; n_class < n_classes; n_class++) {
			delete plots[n_class];
		}
		delete[] plots;
	}
#endif //HEADLESS
	}