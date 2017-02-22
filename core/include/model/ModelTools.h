// ModelTools.h
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

#pragma once

#ifndef SSI_MODEL_MODELTOOLS_H
#define SSI_MODEL_MODELTOOLS_H

#include "model/SampleList.h"
#include "Annotation.h"
#include "base/ITransformer.h"
#include "ioput/file/StringList.h"
#include "ioput/file/File.h"
#include "ioput/file/FileSamplesIn.h"
#include "ioput/file/FileSamplesOut.h"

namespace ssi {

class ModelTools {

public:

	static void LoadAnnotation (Annotation &anno, 
		File &file, char* tier_id = "");
	static void LoadAnnotation (Annotation &anno,
		const ssi_char_t *file_name, char* tier_id = "");
	static void SaveAnnotation (Annotation &anno, 
		File &file);
	static void SaveAnnotation (Annotation &anno, 
		const ssi_char_t *file_name);
	static void ConvertToContinuousAnnotation (Annotation &from,
		Annotation &to,
		ssi_time_t frame_len_in_sec,
		ssi_time_t delta_len_in_sec = 0,
		ssi_time_t min_overlap_with_frame = 0.5,
		const char *default_label = 0);

	static void LoadSampleList (SampleList &samples, 
		File &file, 
		Annotation &annotation, 
		ssi_char_t delim = '_');
	static void LoadSampleList (SampleList &samples, // deprecated
		File &file, 
		Annotation &annotation, 
		ssi_char_t *user_name);
	static void LoadSampleList (SampleList &samples,
		const ssi_char_t *file_name, 
		Annotation &annotation, 
		ssi_char_t *user_name);
	static void LoadSampleList (SampleList &samples,
		ssi_stream_t &stream, 
		Annotation &annotation, 
		const ssi_char_t *user_name);
	static void LoadSampleList (SampleList &samples,
		ssi_size_t num,
		ssi_stream_t *streams[], 
		Annotation &annotation, 
		const ssi_char_t *user_name);
	static void LoadSampleList (SampleList &samples,
		StringList &files);	
	static void LoadSampleList (SampleList &samples, // deprecated 
		File &file);
	static void LoadSampleList (SampleList &samples, // deprecated
		const ssi_char_t *file_name,
		File::TYPE type); 
	static bool LoadSampleList (SampleList &samples,
		const ssi_char_t *file_name);
	static bool LoadSampleList(SampleList &samples, 
		const ssi_char_t *stream_path,
		const ssi_char_t *label_path,
		const ssi_char_t *user_name);

	static void SaveSampleList (ISamples &samples, 
		const ssi_char_t *dir, 
		const ssi_char_t *type, 
		ssi_char_t delim = '_');
	static void SaveSampleList (ISamples &samples, // deprecated
		File &file);
	static bool SaveSampleList (ISamples &samples,
		const ssi_char_t *filename,
		File::TYPE type,
		File::VERSION version = FileSamplesIn::DEFAULT_VERSION);
	static void SaveSampleListArff (ISamples &samples, 
		ssi_char_t *filename,
		const ssi_char_t **feature_names = 0);

	static void SelectSampleList (ISamples &from, 
		SampleList &to, 
		ssi_size_t class_id);
	static void SelectSampleList (ISamples &from, 
		SampleList &to, 
		ssi_size_t n_label_ids,
		ssi_size_t *label_ids);
	static void SelectSampleList (ISamples &from, 
		SampleList &to_select,
		SampleList &to_remain,
		ssi_size_t n_indices,
		ssi_size_t *indices);

	static void CopySampleList (ISamples &from, 
		SampleList &to);
	static void CopySampleList (ISamples &from, 
		SampleList &to,
		ssi_size_t stream_index);

	static void MergeSampleList (SampleList &to,
		ssi_size_t n_from,
		SampleList *from[],
		bool ignoreOverfull = false);

	enum CALL_ENTER_AND_FLUSH {
		CALL_NEVER,
		CALL_ONCE,
		CALL_ALWAYS
	};
	static void TransformSampleList (ISamples &from, 
		SampleList &to, 
		ITransformer &transformer,		
		ssi_size_t frame_size = 0,
		ssi_size_t delta_size = 0,
		CALL_ENTER_AND_FLUSH call_enter_and_flush = CALL_ALWAYS);
	static void TransformSampleList (ISamples &from, 
		SampleList &to,
		ssi_size_t num,
		ITransformer *transformers[],		
		ssi_size_t frame_size = 0,
		ssi_size_t delta_size = 0,
		CALL_ENTER_AND_FLUSH call_enter_and_flush = CALL_ALWAYS);
	static void TransformSampleListWithExtraStream(ISamples &from, ISamples **from_extra,
		SampleList &to,
		ssi_size_t num, ssi_size_t num_extra,
		ITransformer *transformers[],
		ssi_size_t frame_size = 0,
		ssi_size_t delta_size = 0,
		ModelTools::CALL_ENTER_AND_FLUSH call_enter_and_flush = CALL_ALWAYS);

	static void AlignStreams (ssi_size_t num,
		ssi_stream_t *from[],
		ssi_stream_t &to);
	static void AlignSampleList (SampleList &from,
		SampleList &to);

	static void CreateTestSamples (SampleList &samples, 
		ssi_size_t n_classes, 
		ssi_size_t n_samples_per_class, 	
		ssi_size_t n_streams,
		ssi_real_t distr[][3],
		const ssi_char_t* user = "user");
	static void CreateDynamicTestSamples (SampleList &samples, 
		ssi_size_t n_classes, 
		ssi_size_t n_samples_per_class, 	
		ssi_size_t n_streams,
		ssi_real_t distr[][3],
		ssi_size_t num_min,
		ssi_size_t num_max,
		const ssi_char_t* user = "user");
	static void CreateMissingData(SampleList &samples, double prob);

	static void CreateSampleMatrix (ISamples &samples, ssi_size_t stream_index, ssi_size_t &n_samples, ssi_size_t &n_features, ssi_size_t **classes, ssi_real_t ***matrix); // matrix and classes will be allocated!
	static void ReleaseSampleMatrix (ssi_size_t n_samples, ssi_size_t *classes, ssi_real_t **matrix);
	static void FromSampleMatrix(SampleList &samples, ssi_size_t n_samples, ssi_size_t n_features, ssi_size_t *classes, ssi_real_t **matrix);

	static void PrintInfo (ISamples &samples, FILE *file = stdout);
	static void PrintSample (ISamples &samples, ssi_size_t index, FILE *file = stdout);
	static void PrintSamples (ISamples &samples, FILE *file = stdout);
	static void PrintClasses (ISamples &samples, FILE *file = stdout);
	static void PlotSamples(ISamples &samples, const ssi_char_t *name, ssi_rect_t pos);

protected:

	static ssi_char_t *ssi_log_name;
};

}

#endif
