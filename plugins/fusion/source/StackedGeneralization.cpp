// StackedGeneralizationDiscreteDiscrete.cpp
// author: Florian Lingenfelser <florian.lingenfelser@informatik.uni-augsburg.de>
// created: 2010/07/06
// Copyright (C) 2007-12 University of Augsburg, Florian Lingenfelser
//
// *************************************************************************************************
//
// This file is part of Smart Sensor Integration (SSI) developed at the 
// Lab for Multimedia Concepts and Applications of the University of Augsburg
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

#include "StackedGeneralization.h"
#include "ISMissingData.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

	ssi_char_t *StackedGeneralization::ssi_log_name = "stacked_d_";

StackedGeneralization::StackedGeneralization (const ssi_char_t *file) 
	:	_file (0),
		_n_classes (0),
		_n_streams (0),
		_n_models (0),
		ssi_log_level (SSI_LOG_LEVEL_DEFAULT){
} 

StackedGeneralization::~StackedGeneralization () {
	release();
}

void StackedGeneralization::release ()
{
	_n_classes = 0;
	_n_streams = 0;
	_n_models  = 0;
}

bool StackedGeneralization::train (ssi_size_t n_models,
	IModel **models,
	ISamples &samples) {

	if (samples.getSize () == 0) {
		ssi_wrn ("empty sample list");
		return false;
	}

	if (isTrained ()) {
		ssi_wrn ("already trained");
		return false;
	}

	_n_streams = samples.getStreamSize ();
	_n_classes = samples.getClassSize ();
	_n_models = n_models; //including meta model

	if (samples.hasMissingData ()) {

		ISMissingData samples_h (&samples);
		for (ssi_size_t n_model = 0; n_model < (_n_models - 1); n_model++) {
			if (!models[n_model]->isTrained ()) {
				samples_h.setStream (n_model);
				models[n_model]->train (samples_h, n_model);
			}
		}

		SampleList meta_samples;

		for(ssi_size_t n_user = 0; n_user < samples.getUserSize(); n_user++){
			meta_samples.addUserName(samples.getUserName(n_user));
		}
	
		for(ssi_size_t i = 0; i < samples.getClassSize(); i++){
			meta_samples.addClassName(samples.getClassName(i));
		}
	
		ssi_sample_t *sample = 0;
	
		ssi_real_t *data = new ssi_real_t[(_n_models-1)*_n_classes];
		ssi_real_t *base_probs = new ssi_real_t[_n_classes];
	
		samples.reset ();
		while (sample = samples.next ()) {

			//check availability of all streams
			bool all_streams = true;
			for (ssi_size_t n_model = 0; n_model < (_n_models - 1); n_model++) {
				if(sample->streams[n_model]->num == 0){
					all_streams = false;
				}				
			}

			if(all_streams){
		
				ssi_stream_t **meta_streams = new ssi_stream_t*[1];
	
				meta_streams[0] = new ssi_stream_t;

				meta_streams[0]->num		= 1;
				meta_streams[0]->num_real	= 1;
				meta_streams[0]->dim		= (_n_models-1)*_n_classes;
				meta_streams[0]->byte		= 4;
				meta_streams[0]->tot		= meta_streams[0]->num * meta_streams[0]->dim * meta_streams[0]->byte;
				meta_streams[0]->tot_real	= meta_streams[0]->num_real * meta_streams[0]->dim * meta_streams[0]->byte;
				meta_streams[0]->sr			= sample->streams[0]->sr;
				meta_streams[0]->time		= sample->streams[0]->time;
				meta_streams[0]->type		= sample->streams[0]->type;
		
				ssi_size_t counter = 0;
				for (ssi_size_t n_model = 0; n_model < (_n_models - 1); n_model++) {
					models[n_model]->forward(*sample->streams[n_model], _n_classes, base_probs);

					for(ssi_size_t nprobs = 0; nprobs < _n_classes; nprobs++){
						data[counter+nprobs] = ssi_cast(ssi_real_t, base_probs[nprobs]); 
					}
					counter = counter + _n_classes;
				}
				meta_streams[0]->ptr = (ssi_byte_t*)data;
	
				ssi_sample_t *meta_sample = new ssi_sample_t;
		
				meta_sample->num = 1;
				meta_sample->streams	= meta_streams;
				meta_sample->user_id	= sample->user_id;
				meta_sample->class_id	= sample->class_id;
				meta_sample->time		= sample->time;
				meta_sample->score      = sample->score;

				meta_samples.addSample(meta_sample, true);


				if (meta_streams) {
					for(ssi_size_t meta_num = 0; meta_num < meta_sample->num; meta_num++) {
						delete[] meta_streams[meta_num];
					}
					delete[] meta_streams;
					meta_streams = 0;
				}
				if (meta_sample) {
					delete [] meta_sample;
				}

			}//if all_streams ..

		}//samples

		if (!models[_n_models-1]->isTrained ()) {
			models[_n_models-1]->train(meta_samples, 0);
		}
	
		delete [] data;
		delete [] base_probs;
	
		return true;

	}//missing data

	else{
		
		for (ssi_size_t n_model = 0; n_model < (_n_models - 1); n_model++) {
			if (!models[n_model]->isTrained ()) { models[n_model]->train (samples, n_model); }
		}

		SampleList meta_samples;

		for(ssi_size_t n_user = 0; n_user < samples.getUserSize(); n_user++){
			meta_samples.addUserName(samples.getUserName(n_user));
		}
	
		for(ssi_size_t i = 0; i < samples.getClassSize(); i++){
			meta_samples.addClassName(samples.getClassName(i));
		}
	
		ssi_sample_t *sample = 0;
	
		ssi_real_t *data = new ssi_real_t[(_n_models-1)*_n_classes];
		ssi_real_t *base_probs = new ssi_real_t[_n_classes];
	
		samples.reset ();
		while (sample = samples.next ()) {
		
			ssi_stream_t **meta_streams = new ssi_stream_t*[1];
	
			meta_streams[0] = new ssi_stream_t;

			meta_streams[0]->num		= 1;
			meta_streams[0]->num_real	= 1;
			meta_streams[0]->dim		= (_n_models-1)*_n_classes;
			meta_streams[0]->byte		= 4;
			meta_streams[0]->tot		= meta_streams[0]->num * meta_streams[0]->dim * meta_streams[0]->byte;
			meta_streams[0]->tot_real	= meta_streams[0]->num_real * meta_streams[0]->dim * meta_streams[0]->byte;
			meta_streams[0]->sr			= sample->streams[0]->sr;
			meta_streams[0]->time		= sample->streams[0]->time;
			meta_streams[0]->type		= sample->streams[0]->type;
		
			ssi_size_t counter = 0;
			for (ssi_size_t n_model = 0; n_model < (_n_models - 1); n_model++) {
				models[n_model]->forward(*sample->streams[n_model], _n_classes, base_probs);

				for(ssi_size_t nprobs = 0; nprobs < _n_classes; nprobs++){
					data[counter+nprobs] = ssi_cast(ssi_real_t, base_probs[nprobs]); 
				}
				counter = counter + _n_classes;
			}
			meta_streams[0]->ptr = (ssi_byte_t*)data;
	
			ssi_sample_t *meta_sample = new ssi_sample_t;
		
			meta_sample->num = 1;
			meta_sample->streams	= meta_streams;
			meta_sample->user_id	= sample->user_id;
			meta_sample->class_id	= sample->class_id;
			meta_sample->time		= sample->time;
			meta_sample->score      = sample->score;

			meta_samples.addSample(meta_sample, true);


			if (meta_streams) {
				for(ssi_size_t meta_num = 0; meta_num < meta_sample->num; meta_num++) {
					delete[] meta_streams[meta_num];
				}
				delete[] meta_streams;
				meta_streams = 0;
			}
			if (meta_sample) {
				delete [] meta_sample;
			}
		}

		models[_n_models-1]->train(meta_samples, 0);
	
		delete [] data;
		delete [] base_probs;
	
		return true;

	}//no missing data

	
}

bool StackedGeneralization::forward (ssi_size_t n_models,
	IModel **models,
	ssi_size_t n_streams,
	ssi_stream_t *streams[],
	ssi_size_t n_probs,
	ssi_real_t *probs) {

	if (n_streams != _n_streams) {
		ssi_wrn ("#streams (%u) differs from #streams (%u)", n_streams, _n_streams);
		return false;
	}

	if (_n_classes != n_probs) {
		ssi_wrn ("#probs (%u) differs from #classes (%u)", n_probs ,_n_classes);
		return false;
	}

	//check availability of all streams
	bool all_streams = true;
	ssi_size_t unavailable_streams = 0;
	for (ssi_size_t n_model = 0; n_model < (_n_models - 1); n_model++) {
		if(streams[n_model]->num == 0){
			all_streams = false;
			unavailable_streams++;
		}				
	}

	//break, if there just missing data in the sample
	if(unavailable_streams == n_streams){
		return false;
	}

	if(all_streams){

		ssi_real_t *data = new ssi_real_t[(_n_models-1)*_n_classes];
		ssi_real_t *base_probs = new ssi_real_t[_n_classes];

		//get hypotheses from base classifiers
		IModel *model = 0;
		ssi_size_t counter = 0;
		for (ssi_size_t n_model = 0; n_model < (_n_models-1); n_model++) {
			model = models[n_model];
			model->forward (*streams[n_model], _n_classes, base_probs);

			for(ssi_size_t nprobs = 0; nprobs < _n_classes; nprobs++){
				data[counter+nprobs] = ssi_cast(ssi_real_t, base_probs[nprobs]); 
			}
			counter = counter + _n_classes;

		}

		//create meta stream
		ssi_stream_t *meta_stream = new ssi_stream_t;

		meta_stream->num		= 1;
		meta_stream->num_real	= 1;
		meta_stream->dim		= (_n_models-1)*_n_classes;
		meta_stream->byte		= 4;
		meta_stream->tot		= meta_stream->num * meta_stream->dim * meta_stream->byte;
		meta_stream->tot_real	= meta_stream->num_real * meta_stream->dim * meta_stream->byte;
		meta_stream->sr			= streams[0]->sr;
		meta_stream->time		= streams[0]->time;
		meta_stream->type		= streams[0]->type;
	
		meta_stream->ptr		= (ssi_byte_t*)data;

		models[_n_models-1]->forward(*meta_stream, _n_classes, probs);

		delete [] data;
		delete [] base_probs;
		delete [] meta_stream;

	}else{

		//missing data
		bool classified = false;
		for (ssi_size_t n_model = 0; n_model < (_n_models - 1); n_model++) {
			if(streams[n_model]->num != 0){
				models[n_model]->forward(*streams[n_model], _n_classes, probs);
				classified = true;
			}				
			if(classified){
				break;
			}
		}

	}

	/// is there a draw ? ///
	ssi_size_t max_ind = 0;
	ssi_size_t max_ind_draw = 0;
	ssi_real_t max_val = probs[0];
	bool draw = false;

	for (ssi_size_t i = 1; i < n_probs; i++) {
		if (probs[i] >= max_val) {
			if(probs[i] == max_val){
				draw = true;
				max_ind_draw = i;
			}
			max_val = probs[i];
			max_ind = i;
		}
	}
	
	if(draw && (max_ind == max_ind_draw)){
		return false;
	}else{
		return true;
	}

}

bool StackedGeneralization::save (const ssi_char_t *filepath) {

	File *file = File::CreateAndOpen (File::BINARY, File::WRITE, filepath);

	file->write (&ssi_log_level, sizeof (ssi_log_level), 1);

	file->write (&_n_classes, sizeof (_n_classes), 1);
	file->write (&_n_streams, sizeof (_n_streams), 1);
	file->write (&_n_models, sizeof (_n_models), 1);
	
	delete file;
	
	return true;
}

bool StackedGeneralization::load (const ssi_char_t *filepath) {

	release ();

	File *file = File::CreateAndOpen (File::BINARY, File::READ, filepath);

	file->read (&ssi_log_level, sizeof (ssi_log_level), 1);

	file->read (&_n_classes, sizeof (_n_classes), 1);
	file->read (&_n_streams, sizeof (_n_streams), 1);
	file->read (&_n_models, sizeof (_n_models), 1);
	
	delete file;
	
	return true;
}

}
