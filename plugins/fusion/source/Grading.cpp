// Grading.cpp
// author: Florian Lingenfelser <florian.lingenfelser@informatik.uni-augsburg.de>
// created: 2010/12/08
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

#include "Grading.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

	ssi_char_t *Grading::ssi_log_name = "grading___";

Grading::Grading (const ssi_char_t *file) 
	:	_file (0),
		_n_classes (0),
		_n_streams (0),
		_n_models (0),
		ssi_log_level (SSI_LOG_LEVEL_DEFAULT){
} 

Grading::~Grading () {
	release();
}

void Grading::release ()
{
	_n_classes = 0;
	_n_streams = 0;
	_n_models  = 0;
}

bool Grading::train (ssi_size_t n_models,
	IModel **models,
	ISamples &samples) {

	if (samples.getSize () == 0) {
		ssi_wrn ("empty sample list");
		return false;
	}

	if ((samples.getStreamSize() * 2) != n_models) {
		ssi_wrn ("#models (%u) differs from default", n_models);
		return false;
	}

	if (isTrained ()) {
		ssi_wrn ("already trained");
		return false;
	}

	_n_streams = samples.getStreamSize ();
	_n_classes = samples.getClassSize ();
	_n_models = n_models;

	for (ssi_size_t n_model = 0; n_model < (_n_models / 2); n_model++) {
		if (!models[n_model]->isTrained ()) { models[n_model]->train (samples, n_model); }
	}

	for(ssi_size_t nstrm = 0; nstrm < samples.getStreamSize(); nstrm++){
	
		SampleList meta_samples;

		for(ssi_size_t n_user = 0; n_user < samples.getUserSize(); n_user++){
			meta_samples.addUserName(samples.getUserName(n_user));
		}
	
		meta_samples.addClassName("false");
		meta_samples.addClassName("true");
				
		ssi_sample_t *sample = 0;
	
		ssi_real_t *base_probs = new ssi_real_t[_n_classes];
	
		samples.reset ();
		while (sample = samples.next ()) {
	
			ssi_size_t meta_label = -1;

			ssi_stream_t **meta_streams = new ssi_stream_t*[1];
				
			meta_streams[0] = new ssi_stream_t();

			meta_streams[0]->num		= 1;
			meta_streams[0]->num_real	= 1;
			meta_streams[0]->dim		= sample->streams[nstrm]->dim;
			meta_streams[0]->byte		= sample->streams[nstrm]->byte;
			meta_streams[0]->tot		= meta_streams[0]->num * meta_streams[0]->dim * meta_streams[0]->byte;
			meta_streams[0]->tot_real	= meta_streams[0]->num_real * meta_streams[0]->dim * meta_streams[0]->byte;
			meta_streams[0]->sr			= sample->streams[nstrm]->sr;
			meta_streams[0]->time		= sample->streams[nstrm]->time;
			meta_streams[0]->type		= sample->streams[nstrm]->type;
			meta_streams[0]->ptr		= sample->streams[nstrm]->ptr;

			models[nstrm]->forward(*sample->streams[nstrm], _n_classes, base_probs);

			ssi_size_t max_ind = 0;
			ssi_real_t max_val = base_probs[0];
			for (ssi_size_t i = 1; i < _n_classes; i++) {
				if (base_probs[i] > max_val) {
					max_val = base_probs[i];
					max_ind = i;
				}
			}

			if(max_ind == sample->class_id){
				meta_label = 1;
			}else{
				meta_label = 0;
			}
			
			ssi_sample_t *meta_sample = new ssi_sample_t();
		
			meta_sample->num		= 1;
			meta_sample->streams	= meta_streams;
			meta_sample->user_id	= sample->user_id;
			meta_sample->class_id	= meta_label;
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
		//meta_samples.printInfo();getchar();
		models[(_n_models / 2) + nstrm]->train(meta_samples, 0);

		delete [] base_probs;
	}

	return true;
}

bool Grading::forward (ssi_size_t n_models,
	IModel **models,
	ssi_size_t n_streams,
	ssi_stream_t *streams[],
	ssi_size_t n_probs,
	ssi_real_t *probs) {

	if (n_streams != _n_streams) {
		ssi_wrn ("#streams (%u) differs from #streams (%u)", n_streams, _n_streams);
		return false;
	}

	if ((n_streams * 2) != n_models) {
		ssi_wrn ("#models (%u) differs from default", n_models);
		return false;
	}

	if (_n_classes != n_probs) {
		ssi_wrn ("#probs (%u) differs from #classes (%u)", n_probs ,_n_classes);
		return false;
	}

	IModel *model = 0;
	IModel *meta_model = 0;
	ssi_stream_t *stream = 0;
	ssi_real_t *meta_probs = new ssi_real_t[2];
	ssi_real_t **tmp_probs = new ssi_real_t *[(n_models / 2)];
	
	//clear probs
	for (ssi_size_t num_probs = 0; num_probs < n_probs; num_probs++){
		probs[num_probs] = 0.0f;
	}

	for (ssi_size_t n_model = 0; n_model < (n_models / 2); n_model++) {
		model = models[n_model];
		meta_model = models[(n_models / 2) + n_model];
		stream = streams[n_model];
		tmp_probs[n_model] = new ssi_real_t[n_probs];
		model->forward (*stream, n_probs, tmp_probs[n_model]);
		meta_model->forward (*stream, 2, meta_probs);

		ssi_size_t max_ind = 0;
		ssi_real_t max_val = tmp_probs[n_model][0];
		for (ssi_size_t i = 1; i < n_probs; i++) {
			if (tmp_probs[n_model][i] > max_val) {
				max_val = tmp_probs[n_model][i];
				max_ind = i;
			}
		}
		
		//fill probs
		probs[max_ind] = probs[max_ind] + meta_probs[1];
	}

	delete [] meta_probs;
	meta_probs = 0;
	
	if(tmp_probs){
		for (ssi_size_t n_model = 0; n_model < (n_models/2); n_model++) {
		delete[] tmp_probs[n_model];
		}
		delete[] tmp_probs;
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

bool Grading::save (const ssi_char_t *filepath) {

	File *file = File::CreateAndOpen (File::BINARY, File::WRITE, filepath);

	file->write (&ssi_log_level, sizeof (ssi_log_level), 1);

	file->write (&_n_classes, sizeof (_n_classes), 1);
	file->write (&_n_streams, sizeof (_n_streams), 1);
	file->write (&_n_models, sizeof (_n_models), 1);
	
	delete file;
	
	return true;
}

bool Grading::load (const ssi_char_t *filepath) {

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
