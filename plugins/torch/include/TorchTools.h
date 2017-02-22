// TorchTools.h
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

#ifndef SSI_TORCH_TORCHTOOLS_H
#define SSI_TORCH_TORCHTOOLS_H

#include "base/ISamples.h"
#include "MeanVarNorm.h"
#include "MemoryDataSet.h"
#include "ISSelectClass.h"

namespace ssi {

class TorchTools {

public:
	static Torch::MemoryDataSet *CreateMemoryDataSet (ISamples &samples, 
		ssi_size_t stream_index,
		bool deep_copy) {

		samples.reset ();

		Torch::MemoryDataSet *data_set = new Torch::MemoryDataSet ();
		unsigned int sample_size = samples.getSize ();
		Torch::Sequence **inputs = reinterpret_cast<Torch::Sequence **> (data_set->allocator->alloc (sizeof (Torch::Sequence *) * sample_size));
		Torch::Sequence **targets = reinterpret_cast<Torch::Sequence **> (data_set->allocator->alloc (sizeof (Torch::Sequence *) * sample_size));

		ssi_sample_t *sample;
		if (deep_copy) {
			for (unsigned int i = 0; i < sample_size; i++) {
				sample = samples.next ();
				int num = sample->streams[stream_index]->num;
				int dim = sample->streams[stream_index]->dim;
				inputs[i] = new (data_set->allocator) Torch::Sequence (num, dim);
				targets[i] = new (data_set->allocator) Torch::Sequence (1, 1);
				memcpy (inputs[i]->frames[0], sample->streams[stream_index]->ptr, sizeof (ssi_real_t) * num * dim);
				for (int j = 1; j < num; j++) {
					inputs[i]->frames[j] = inputs[i]->frames[j-1] + dim;
				}
				targets[i]->frames[0][0] = ssi_cast (float, sample->class_id);
			}	
		} else {
			for (unsigned int i = 0; i < sample_size; i++) {
				sample = samples.next ();
				int num = sample->streams[stream_index]->num;
				int dim = sample->streams[stream_index]->dim;
				ssi_real_t **frames = (ssi_real_t **) data_set->allocator->alloc (sizeof (ssi_real_t *) * num);
				//ssi_real_t **frames = new ssi_real_t *[rows];
				ssi_real_t *dataptr = ssi_pcast (ssi_real_t, sample->streams[stream_index]->ptr);
				for (int j = 0; j < num; j++) {
					frames[j] = dataptr;
					dataptr += dim;
				}
				inputs[i] = new (data_set->allocator) Torch::Sequence (frames, num, dim);
				targets[i] = new (data_set->allocator) Torch::Sequence (1, 1);
				targets[i]->frames[0][0] = ssi_cast (float, sample->class_id);
			}
		}

		data_set->setInputs (inputs, sample_size);
		data_set->setTargets (targets, sample_size);

		return data_set;
	};

	static Torch::MemoryDataSet *CreateMemoryDataSet (ISamples &samples, 
		ssi_size_t stream_index,
		ssi_size_t label_index, 
		bool deep_copy) {
	
		Torch::MemoryDataSet *data_set = new Torch::MemoryDataSet ();
		unsigned int sample_size = samples.getSize (label_index);
		Torch::Sequence **inputs = reinterpret_cast<Torch::Sequence **> (data_set->allocator->alloc (sizeof (Torch::Sequence *) * sample_size));
		Torch::Sequence **targets = reinterpret_cast<Torch::Sequence **> (data_set->allocator->alloc (sizeof (Torch::Sequence *) * sample_size));

		ssi_sample_t *sample;
		ISSelectClass samples_s (&samples);
		samples_s.setSelection (label_index);
		samples_s.reset ();
		if (deep_copy) {
			for (unsigned int i = 0; i < sample_size; i++) {			
				sample = samples_s.next ();
				int num = sample->streams[stream_index]->num;
				int dim = sample->streams[stream_index]->dim;
				inputs[i] = new (data_set->allocator) Torch::Sequence (num, dim);
				targets[i] = new (data_set->allocator) Torch::Sequence (1, 1);
				memcpy (inputs[i]->frames[0], sample->streams[stream_index]->ptr, sizeof (ssi_real_t) * num * dim);
				for (int j = 1; j < num; j++) {
					inputs[i]->frames[j] = inputs[i]->frames[j-1] + dim;
				}
				targets[i]->frames[0][0] = ssi_cast (float, sample->class_id);
			}	
		} else {
			for (unsigned int i = 0; i < sample_size; i++) {
				sample = samples_s.next ();
				int num = sample->streams[stream_index]->num;
				int dim = sample->streams[stream_index]->dim;
				ssi_real_t **frames = (ssi_real_t **) data_set->allocator->alloc (sizeof (ssi_real_t *) * num);
				//ssi_real_t **frames = new ssi_real_t *[rows];
				ssi_real_t *dataptr = ssi_pcast (ssi_real_t, sample->streams[stream_index]->ptr);
				for (int j = 0; j < num; j++) {
					frames[j] = dataptr;
					dataptr += dim;
				}
				inputs[i] = new (data_set->allocator) Torch::Sequence (frames, num, dim);
				targets[i] = new (data_set->allocator) Torch::Sequence (1, 1);
				targets[i]->frames[0][0] = ssi_cast (float, sample->class_id);
			}
		}

		data_set->setInputs (inputs, sample_size);
		data_set->setTargets (targets, sample_size);

		return data_set;
	};

	static Torch::Sequence *CreateSequence (ssi_stream_t &stream) {
		return TorchTools::CreateSequence (stream.dim, stream.num, ssi_pcast (ssi_real_t, stream.ptr));
	};

	static Torch::Sequence *CreateSequence (ssi_size_t sample_dimension, ssi_size_t sample_number, ssi_real_t *sample_data) {

		ssi_real_t **frames = new ssi_real_t*[sample_number];
		ssi_real_t **frames_ptr = frames;
		ssi_real_t *dataptr = sample_data;
		for (ssi_size_t i = 0; i < sample_number; i++) {
			*frames_ptr++ = dataptr;
			dataptr += sample_dimension;
		}

		Torch::Sequence *sequence = new Torch::Sequence (frames, sample_number, sample_dimension);
		sequence->allocator->retain (sequence->frames);

		return sequence;
	};

	static void InitThreshold (Torch::DataSet* data, ssi_real_t* thresh, ssi_real_t threshold) {

		Torch::MeanVarNorm norm (data);
		ssi_real_t*	ptr = norm.inputs_stdv;
		ssi_real_t* p_var = thresh;
		for (int i=0; i < data->n_inputs; i++)
			*p_var++ = *ptr * *ptr++ * threshold;
	};

	static void Print (Torch::Sequence *sequence,
		FILE *file) {

		int len = sequence->n_frames;
		int dim = sequence->frame_size;

		for (int i = 0; i < len; i++) {
			for (int j = 0; j < dim; j++) {
				ssi_fprint (file, "%.2f ", sequence->frames[i][j]);
			}
			ssi_fprint (file, "\n");
		}
	};
};

}

#endif
