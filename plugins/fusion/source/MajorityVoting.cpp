// MajorityVoting.cpp
// author: Florian Lingenfelser <florian.lingenfelser@informatik.uni-augsburg.de>
// created: 2010/4/6
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

#include "MajorityVoting.h"
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

	ssi_char_t *MajorityVoting::ssi_log_name = "vote______";

MajorityVoting::MajorityVoting (const ssi_char_t *file) 
	:	_file (0),
		_n_classes (0),
		_n_streams (0),
		_n_models (0),
		ssi_log_level (SSI_LOG_LEVEL_DEFAULT){
} 

MajorityVoting::~MajorityVoting () { 
	release();
}

void MajorityVoting::release ()
{
	_n_classes = 0;
	_n_streams = 0;
	_n_models  = 0;
}

bool MajorityVoting::train (ssi_size_t n_models,
	IModel **models,
	ISamples &samples) {

	if (samples.getSize () == 0) {
		ssi_wrn ("empty sample list");
		return false;
	}

	if (samples.getStreamSize () != n_models) {
		ssi_wrn ("#models (%u) differs from #streams (%u)", n_models, samples.getStreamSize ());
		return false;
	}

	if (isTrained ()) {
		ssi_wrn ("already trained");
		return false;
	} 

	_n_streams = samples.getStreamSize ();
	_n_classes = samples.getClassSize ();
	_n_models  = n_models;

	if (samples.hasMissingData ()) {
		ISMissingData samples_h (&samples);
		for (ssi_size_t n_model = 0; n_model < n_models; n_model++) {
			if (!models[n_model]->isTrained ()) {
				samples_h.setStream (n_model);
				models[n_model]->train (samples_h, n_model);
			}
		}
	}
	else{
		for (ssi_size_t n_model = 0; n_model < n_models; n_model++) {
			if (!models[n_model]->isTrained ()) { models[n_model]->train (samples, n_model); }
		}		
	}
	
	return true;
}

bool MajorityVoting::forward (ssi_size_t n_models,
	IModel **models,
	ssi_size_t n_streams,
	ssi_stream_t *streams[],
	ssi_size_t n_probs,
	ssi_real_t *probs) {

	if (n_streams != _n_streams) {
		ssi_wrn ("#streams (%u) differs from #streams (%u)", n_streams, _n_streams);
		return false;
	}

	if (_n_streams != n_models) {
		ssi_wrn ("#models (%u) differs from #streams (%u)", n_models, _n_streams);
		return false;
	}

	if (_n_classes != n_probs) {
		ssi_wrn ("#probs (%u) differs from #classes (%u)", n_probs ,_n_classes);
		return false;
	}

	bool found_data = false;

	IModel *model = 0;
	ssi_stream_t *stream = 0;

	//calculate actual models
	ssi_size_t miss_counter = 0;
	ssi_size_t *available = new ssi_size_t[n_models];
	for (ssi_size_t n_model = 0; n_model < n_models; n_model++) {
		stream = streams[n_model];
		if (stream->num > 0) {
			found_data = true;
			available[n_model] = 1;
		}
		else{
			miss_counter++;
			available[n_model] = 0;
		}
	}
	ssi_size_t counter = 0;
	ssi_size_t *models_actual = new ssi_size_t[(n_models - miss_counter)];
	for (ssi_size_t n_model = 0; n_model < n_models; n_model++) {
		if(available[n_model] == 1){
			models_actual[counter] = n_model;
			counter++;
		}
	}

	if(found_data){

		ssi_size_t *votes = new ssi_size_t[(n_models - miss_counter)];

		for (ssi_size_t n_model = 0; n_model < (n_models - miss_counter); n_model++) {
			model = models[models_actual[n_model]];
			stream = streams[models_actual[n_model]];
			model->forward (*stream, n_probs, probs);

			ssi_size_t max_ind = 0;
			ssi_real_t max_val = probs[0];
			
			for (ssi_size_t i = 1; i < n_probs; i++) {
				if (probs[i] > max_val) {
					max_val = probs[i];
					max_ind = i;
				}
			}
			votes[n_model] = max_ind;

			if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
				for (ssi_size_t num_probs = 0; num_probs < n_probs; num_probs++){
					ssi_print("%f ", probs[num_probs]);
				}ssi_print("- vote: %d\n", max_ind);
			}

		}
		if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
			ssi_print("\n");
		}

		//clear probs
		for (ssi_size_t num_probs = 0; num_probs < n_probs; num_probs++){
			probs[num_probs] = 0;
		}

		//fill probs with votes
		for(ssi_size_t n_model = 0; n_model < (n_models - miss_counter); n_model++){
			probs[votes[n_model]]++;
		}
		
		if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
			for (ssi_size_t num_probs = 0; num_probs < n_probs; num_probs++){
				ssi_print("%f ", probs[num_probs]);
			}ssi_print("\n\n");
		}


		if(votes){
			delete[] votes;
			votes = 0;
		}
		if(available){
			delete [] available;
			available = 0;
		}
		if(models_actual){
			delete [] models_actual;
			models_actual = 0;
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
		return found_data;
	}
	
}

bool MajorityVoting::save (const ssi_char_t *filepath) {

	File *file = File::CreateAndOpen (File::BINARY, File::WRITE, filepath);

	file->write (&ssi_log_level, sizeof (ssi_log_level), 1);

	file->write (&_n_classes, sizeof (_n_classes), 1);
	file->write (&_n_streams, sizeof (_n_streams), 1);
	file->write (&_n_models, sizeof (_n_models), 1);

	delete file;
	
	return true;
}

bool MajorityVoting::load (const ssi_char_t *filepath) {

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
