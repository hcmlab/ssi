// FeatureFusion.cpp
// author: Florian Lingenfelser <florian.lingenfelser@informatik.uni-augsburg.de>
// created: 2010/12/09
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

#include "FeatureFusion.h"
#include "ISMissingData.h"
#include "ISAlignStrms.h"
#include "Evaluation.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

	ssi_char_t *FeatureFusion::ssi_log_name = "featfuse__";

FeatureFusion::FeatureFusion (const ssi_char_t *file) 
	:	_file (0),
		_n_classes (0),
		_n_streams (0),
		_n_models (0),
		_filler (0),
		_handle_md (false){
} 

FeatureFusion::~FeatureFusion () { 
	release();
}

void FeatureFusion::release ()
{
	if(_filler){
		delete [] _filler;
		_filler = 0;
	}
	_n_classes = 0;
	_n_streams = 0;
	_n_models  = 0;

	_handle_md = false;
}

bool FeatureFusion::train (ssi_size_t n_models,
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
	_n_models  = n_models;

	//initialize weights
	ssi_real_t **weights = new ssi_real_t*[n_models];
	for (ssi_size_t n_model = 0; n_model < n_models; n_model++) {
		weights[n_model] = new ssi_real_t[_n_classes+1];		
	}

	if (samples.hasMissingData ()) {

		_handle_md = true;

		ISMissingData samples_h (&samples);
		Evaluation eval;
		
		if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
			ssi_print("\nMissing data detected.\n");
		}
		
		//models[0] is featfuse_model, followed by singlechannel_models
		ISAlignStrms ffusionSamples (&samples);
		ISMissingData ffusionSamples_h (&ffusionSamples);
		ffusionSamples_h.setStream(0);
		if (!models[0]->isTrained ()) { models[0]->train (ffusionSamples_h, 0); }

		if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
			eval.eval (*models[0], ffusionSamples_h, 0);
			eval.print();
		}
		//dummy weights for fused model
		for (ssi_size_t n_class = 0; n_class < _n_classes; n_class++) {
			weights[0][n_class] = 0.0f;
		}		
		weights[0][_n_classes] = 0.0f;	
		
		for (ssi_size_t n_model = 1; n_model < n_models; n_model++) {
			
			if (!models[n_model]->isTrained ()) {
				samples_h.setStream (n_model - 1);
				models[n_model]->train (samples_h, n_model - 1);
			}

			eval.eval (*models[n_model], samples_h, n_model - 1);

			if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
				eval.print();
			}

			for (ssi_size_t n_class = 0; n_class < _n_classes; n_class++) {
				weights[n_model][n_class] = eval.get_class_prob (n_class);
			}		
			weights[n_model][_n_classes] = eval.get_classwise_prob ();	
		}

		//calculate fillers
		_filler = new ssi_size_t[_n_streams];
		for (ssi_size_t n_fill = 0; n_fill < _n_streams; n_fill++) {
			_filler[n_fill] = 1;
			ssi_real_t filler_weight = weights[1][_n_classes];
			for (ssi_size_t n_model = 2; n_model < n_models; n_model++) {
				if (filler_weight < weights[n_model][_n_classes]) {
					_filler[n_fill] = n_model;
					filler_weight = weights[n_model][_n_classes];
				}
			}
			weights[_filler[n_fill]][_n_classes] = 0.0f;
		}
		if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
			ssi_print("\nfiller:\n");
			for (ssi_size_t n_model = 0; n_model < _n_streams; n_model++) {
				ssi_print("%d ", _filler[n_model]);
			}ssi_print("\n");
		}
	
	}
	else{

		_handle_md = false;

		if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
			ssi_print("\nNo missing data detected.\n");
		}
		ISAlignStrms ffusionSamples (&samples);
		if (!models[0]->isTrained ()) { models[0]->train (ffusionSamples, 0); }
		//dummy
		_filler = new ssi_size_t[_n_streams];
		for (ssi_size_t n_fill = 0; n_fill < _n_streams; n_fill++) {
			_filler[n_fill] = 0;
		}
	}

	if (weights) {
		for (ssi_size_t n_model = 0; n_model < _n_models; n_model++) {
			delete[] weights[n_model];
		}
		delete[] weights;
		weights = 0;
	}

	return true;
}

bool FeatureFusion::forward (ssi_size_t n_models,
	IModel **models,
	ssi_size_t n_streams,
	ssi_stream_t *streams[],
	ssi_size_t n_probs,
	ssi_real_t *probs) {

	if (!isTrained ()) {
		ssi_wrn ("not trained");
		return false;
	}

	if (n_streams != _n_streams) {
		ssi_wrn ("#streams (%u) differs from #streams (%u)", n_streams, _n_streams);
		return false;
	}

	if (_n_models != n_models) {
		ssi_wrn ("#models (%u) differs from #models (%u)", n_models, _n_models);
		return false;
	}

	if (_n_classes != n_probs) {
		ssi_wrn ("#probs (%u) differs from #classes (%u)", n_probs ,_n_classes);
		return false;
	}

	//No Missing Data:
	if(!_handle_md){

		IModel *model = 0;
		ssi_stream_t *stream = 0;

		model = models[0];

		ssi_stream_t *fusion_stream = new ssi_stream_t;

		ssi_size_t fusion_stream_dim = 0;
		for(ssi_size_t nstrm = 0; nstrm < _n_streams; nstrm++){
			fusion_stream_dim += streams[nstrm]->dim;
		}

		//create aligned streams
		ssi_stream_init (*fusion_stream, 1, fusion_stream_dim, streams[0]->byte, streams[0]->type, streams[0]->sr);
		
		ssi_byte_t *ptr = fusion_stream->ptr;
		for(ssi_size_t i = 0; i < _n_streams; i++){
			memcpy(ptr, streams[i]->ptr, ( streams[i]->byte * streams[i]->dim ) );
			ptr += ( streams[i]->byte * streams[i]->dim );
		}

		//clear probs
		for (ssi_size_t num_probs = 0; num_probs < n_probs; num_probs++){
			probs[num_probs] = 0.0f;
		}

		model->forward (*fusion_stream, n_probs, probs);

		ssi_stream_destroy(*fusion_stream);
		delete fusion_stream;
		fusion_stream = 0;

		///// is there a draw ? ///
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

	}//No Missing Data


	//Missing Data:
	bool found_data = false;

	IModel *model = 0;
	ssi_stream_t *stream = 0;

	//calculate actual models
	ssi_size_t miss_counter = 0;
	ssi_size_t *available = new ssi_size_t[n_models];
	available[0] = 1;
	for (ssi_size_t n_model = 1; n_model < _n_models; n_model++) {
		stream = streams[n_model - 1];
		if (stream->num > 0) {
			found_data = true;
			available[n_model] = 1;
		}
		else{
			miss_counter++;
			available[n_model] = 0;
			if(available[0] == 1){
				available[0] = 0;
				miss_counter++;
			}
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

	if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
		ssi_print("\n\n-----------------------------\navailable models:\n");
		for(ssi_size_t i = 0; i < (n_models - miss_counter); i++){
			ssi_print("%d ", models_actual[i]);
		}ssi_print("\n");
	}

	if(found_data){

		if(available[0] == 1){
			//feature fusion possible
			if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
				ssi_print("\nfeature fusion possible\n");
			}

			model = models[0];
			stream = 0;
			ssi_stream_t *fusion_stream = new ssi_stream_t;

			ssi_size_t fusion_stream_dim = 0;
			for(ssi_size_t nstrm = 0; nstrm < _n_streams; nstrm++){
				fusion_stream_dim += streams[nstrm]->dim;
			}

			//create aligned streams
			ssi_stream_init (*fusion_stream, 1, fusion_stream_dim, streams[0]->byte, streams[0]->type, streams[0]->sr);
			
			ssi_byte_t *ptr = fusion_stream->ptr;
			for(ssi_size_t i = 0; i < _n_streams; i++){
				memcpy(ptr, streams[i]->ptr, ( streams[i]->byte * streams[i]->dim ) );
				ptr += ( streams[i]->byte * streams[i]->dim );
			}

			//clear probs
			for (ssi_size_t num_probs = 0; num_probs < n_probs; num_probs++){
				probs[num_probs] = 0.0f;
			}

			model->forward (*fusion_stream, n_probs, probs);

			ssi_stream_destroy(*fusion_stream);
			delete fusion_stream;
			fusion_stream = 0;

			if(available){
				delete [] available;
				available = 0;
			}
			if(models_actual){
				delete [] models_actual;
				models_actual = 0;
			}

			return true;

		}else{
			//feature fusion not possible, choose filler ...
			if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
				ssi_print("\nfeature fusion not possible: filler needed\n");
				ssi_print("\nfiller:\n");
				for (ssi_size_t n_model = 0; n_model < _n_streams; n_model++) {
					ssi_print("%d ", _filler[n_model]);
				}ssi_print("\n");
			}

			bool model_available = false;
			ssi_size_t model_id = 0;
			for(ssi_size_t h = 0; h < _n_streams; h++){
				model_id = _filler[h];
				for(ssi_size_t i = 0; i < (n_models - miss_counter); i++){
					if(model_id == models_actual[i]){
						model_available = true;
						break;
					}
				}
				if(model_available == true){
					model = models[model_id];
					if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
						ssi_print("\nSelected Model: %d", model_id);
					}
					break;
				}
			}

			model->forward(*streams[model_id - 1], n_probs, probs);

		}

	}

	if(available){
		delete [] available;
		available = 0;
	}
	if(models_actual){
		delete [] models_actual;
		models_actual = 0;
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

bool FeatureFusion::save (const ssi_char_t *filepath) {

	File *file = File::CreateAndOpen (File::BINARY, File::WRITE, filepath);

	file->write (&ssi_log_level, sizeof (ssi_log_level), 1);

	file->write (&_n_classes, sizeof (_n_classes), 1);
	file->write (&_n_streams, sizeof (_n_streams), 1);
	file->write (&_n_models, sizeof (_n_models), 1);
	file->write (_filler, sizeof (ssi_size_t), _n_streams);

	file->write (&_handle_md, sizeof (_handle_md), 1);
	
	delete file;
	
	return true;
}

bool FeatureFusion::load (const ssi_char_t *filepath) {

	release ();

	File *file = File::CreateAndOpen (File::BINARY, File::READ, filepath);

	file->read (&ssi_log_level, sizeof (ssi_log_level), 1);

	file->read (&_n_classes, sizeof (_n_classes), 1);
	file->read (&_n_streams, sizeof (_n_streams), 1);
	file->read (&_n_models, sizeof (_n_models), 1);
	_filler = new ssi_size_t[_n_streams];
	file->read (_filler, sizeof (ssi_size_t), _n_streams);

	file->read (&_handle_md, sizeof (_handle_md), 1);
	
	delete file;
	
	return true;
}

}
