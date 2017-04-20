// OvRSpecialist.cpp
// author: Florian Lingenfelser <florian.lingenfelser@informatik.uni-augsburg.de>
// created: 2010/12/15
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
#include "OvRSpecialist.h"
#include "ISHotClass.h"
#include "ISMissingData.h"
#include "ISSelectSample.h"
#include "Evaluation.h"
#include "ISMergeDim.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

#define HYBRID
//#define PRODUCT
#define SUM

namespace ssi {

	ssi_char_t *OvRSpecialist::ssi_log_name = "ovr_spec__";

OvRSpecialist::OvRSpecialist (const ssi_char_t *file) 
	: _file (0),
	_n_classes (0),
	_n_streams (0),
	_n_samples (0),
	_n_models (0),
	_winner(0),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

} 

OvRSpecialist::~OvRSpecialist () {
	release();
}

void OvRSpecialist::release ()
{
	if (_winner) {
		for (ssi_size_t n_class = 0; n_class < _n_classes; n_class++) {
			delete[] _winner[n_class];
		}
		delete[] _winner;
		_winner = 0;
	}

	_n_classes = 0;
	_n_streams = 0;
	_n_models  = 0;
	_n_samples = 0;
}

ssi_size_t OvRSpecialist::getModelNumber(ISamples &samples){

#ifdef HYBRID
	return ( (samples.getClassSize() * samples.getStreamSize()) + 1 );
#else
	return ( (samples.getClassSize() * samples.getStreamSize()) );
#endif

}

bool OvRSpecialist::train (ssi_size_t n_models,
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
	_n_samples = samples.getSize();
	_n_models  = n_models;

	ssi_real_t** weights = new ssi_real_t*[_n_classes];
	for(ssi_size_t n_class = 0; n_class < _n_classes; n_class++){
		weights[n_class] = new ssi_real_t[_n_streams];
		for(ssi_size_t n_stream = 0; n_stream < _n_streams; n_stream++){
			weights[n_class][n_stream] = 0.0f;
		}
	}

	_winner = new ssi_size_t*[_n_classes];
	for (ssi_size_t n_class = 0; n_class < _n_classes; n_class++) {
		_winner[n_class] = new ssi_size_t[_n_streams];
		for (ssi_size_t n_win = 0; n_win < _n_streams; n_win++) {
			_winner[n_class][n_win] = 0;
		}
	}

	if (samples.hasMissingData ()) {

		ISMissingData samples_h (&samples);
		ISHotClass samples_hot (&samples_h);

		/// models: [Stream0Class0, Stream0Class1, Stream0Class2, ..., Stream1Class0, ..., Filler] ///
		ssi_size_t position = 0;
		for(ssi_size_t n_stream = 0; n_stream < _n_streams; n_stream++){
			for(ssi_size_t n_class = 0; n_class < _n_classes; n_class++){
				if (!models[position]->isTrained ()) { 
					samples_h.setStream(n_stream);
					samples_hot.setHotClass (n_class);
					models[position]->train (samples_hot, n_stream); 
				}
				
				Evaluation eval;
				
				eval.eval (*models[position], samples_hot, n_stream);
				if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
					eval.print (stdout);
				}
				ssi_size_t *const*conf_mat = eval.get_conf_mat();
				weights[n_class][n_stream] = eval.get_class_prob(0); //hot class

				position++;
			}
		}

		// choose specs
		if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
			ssi_print("\nClassifier Weights: \n");
			for (ssi_size_t n_class = 0; n_class < _n_classes; n_class++) {
				for (ssi_size_t n_model = 0; n_model < _n_streams; n_model++) {
					ssi_print ("%f ", weights[n_class][n_model]);
				}
				ssi_print ("\n");		
			}ssi_print("\n");
		}

		/*_winner = new ssi_size_t*[_n_classes];
		for (ssi_size_t n_class = 0; n_class < _n_classes; n_class++) {
			_winner[n_class] = new ssi_size_t[_n_streams];
			for (ssi_size_t n_win = 0; n_win < _n_streams; n_win++) {
				_winner[n_class][n_win] = 0;
			}
		}*/

		
		for (ssi_size_t n_class = 0; n_class < _n_classes; n_class++) {
			for (ssi_size_t n_win = 0; n_win < _n_streams; n_win++) {
				_winner[n_class][n_win] = n_class;
				ssi_size_t pos = 0;
				ssi_real_t winner_weight = weights[n_class][0];
				for (ssi_size_t n_model = 1; n_model < _n_streams; n_model++) {
					if (winner_weight < weights[n_class][n_model]) {
						_winner[n_class][n_win] = n_class + (n_model * _n_classes);
						pos = n_model;
						winner_weight = weights[n_class][n_model];
					}
				}
				weights[n_class][pos] = 0.0f;
			}
		}

		if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
			ssi_print("\nWinners:\n");
			for (ssi_size_t n_class = 0; n_class < _n_classes; n_class++) {
				for(ssi_size_t n_winners = 0; n_winners < _n_streams; n_winners++){
					ssi_print("%d ", _winner[n_class][n_winners]);
				}
				ssi_print("\n");
			}ssi_print("\n");
		}

#ifdef HYBRID

		ISMergeDim ffusionSamples (&samples);
		ISMissingData ffusionSamples_h (&ffusionSamples);
		ffusionSamples_h.setStream(0);
		if (!models[_n_models - 1]->isTrained ()) { models[_n_models - 1]->train (ffusionSamples_h, 0); }

		if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
			ssi_print("\nFeatureFusion for %d classes\n\n", samples.getClassSize());
			Evaluation eval;
			eval.eval (*models[_n_models - 1], ffusionSamples_h, 0);
			eval.print();
		}

#endif

	}else{

  		ISHotClass samples_hot (&samples);

		/// models: [Stream0Class0, Stream0Class1, Stream0Class2, ..., Stream1Class0, ..., Filler] ///
		ssi_size_t position = 0;
		for(ssi_size_t n_stream = 0; n_stream < _n_streams; n_stream++){
			for(ssi_size_t n_class = 0; n_class < _n_classes; n_class++){
				if (!models[position]->isTrained ()) { 
					samples_hot.setHotClass (n_class);
					models[position]->train (samples_hot, n_stream); 
				}
				
				Evaluation eval;
				eval.eval (*models[position], samples_hot, n_stream);
				if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
					eval.print (stdout);
				}
				ssi_size_t *const*conf_mat = eval.get_conf_mat();
				weights[n_class][n_stream] = eval.get_class_prob(0); //hot class

				position++;
			}
		}

		// choose specs
		if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
			ssi_print("\nClassifier Weights: \n");
			for (ssi_size_t n_class = 0; n_class < _n_classes; n_class++) {
				for (ssi_size_t n_model = 0; n_model < _n_streams; n_model++) {
					ssi_print ("%f ", weights[n_class][n_model]);
				}
				ssi_print ("\n");		
			}ssi_print("\n");
		}

		/*_winner = new ssi_size_t*[_n_classes];
		for (ssi_size_t n_class = 0; n_class < _n_classes; n_class++) {
			_winner[n_class] = new ssi_size_t[_n_streams];
			for (ssi_size_t n_win = 0; n_win < _n_streams; n_win++) {
				_winner[n_class][n_win] = 0;
			}
		}*/

		
		for (ssi_size_t n_class = 0; n_class < _n_classes; n_class++) {
			for (ssi_size_t n_win = 0; n_win < _n_streams; n_win++) {
				_winner[n_class][n_win] = n_class;
				ssi_size_t pos = 0;
				ssi_real_t winner_weight = weights[n_class][0];
				for (ssi_size_t n_model = 1; n_model < _n_streams; n_model++) {
					if (winner_weight < weights[n_class][n_model]) {
						_winner[n_class][n_win] = n_class + (n_model * _n_classes);
						pos = n_model;
						winner_weight = weights[n_class][n_model];
					}
				}
				weights[n_class][pos] = 0.0f;
			}
		}

		if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
			ssi_print("\nWinners:\n");
			for (ssi_size_t n_class = 0; n_class < _n_classes; n_class++) {
				for(ssi_size_t n_winners = 0; n_winners < _n_streams; n_winners++){
					ssi_print("%d ", _winner[n_class][n_winners]);
				}
				ssi_print("\n");
			}ssi_print("\n");
		}

#ifdef HYBRID

		ISMergeDim ffusionSamples (&samples);

		if (!models[_n_models - 1]->isTrained ()){
			models[_n_models - 1]->train (ffusionSamples, 0);
			if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
				ssi_print("\nFeatureFusion for %d classes\n\n", samples.getClassSize());
				Evaluation eval;
				eval.eval (*models[_n_models - 1], ffusionSamples, 0);
				eval.print();
			}
		}

#endif

	}

	if(weights){
		for(ssi_size_t n_class = 0; n_class < _n_classes; n_class++){
			delete [] weights[n_class];
		}
		delete [] weights;
		weights = 0;
	}

	return true;
}

bool OvRSpecialist::forward (ssi_size_t n_models,
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

	bool found_data = false;

	IModel *model = 0;
	ssi_stream_t *stream = 0;

#ifdef PRODUCT
	//clear probs
	for (ssi_size_t num_probs = 0; num_probs < n_probs; num_probs++){
		probs[num_probs] = 1.0f;
	}
#endif

#ifdef SUM
	//clear probs
	for (ssi_size_t num_probs = 0; num_probs < n_probs; num_probs++){
		probs[num_probs] = 0.0f;
	}
#endif

	if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
			ssi_print("\n\n-----------------------------\n\n");
	}

	/// models: [Stream0Class0, Stream0Class1, Stream0Class2, ..., Stream1Class0, ..., Filler] ///
	for (ssi_size_t n_class = 0; n_class < _n_classes; n_class++) {

		for(ssi_size_t n_winners = 0; n_winners < _n_streams; n_winners++){
			if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
				ssi_print ("\nTesting Model %d for Class %d with Stream %d ... \n", _winner[n_class][n_winners], n_class, ssi_cast(ssi_size_t, (_winner[n_class][n_winners] / _n_classes)));
			}
			stream = streams[ssi_cast(ssi_size_t, (_winner[n_class][n_winners] / _n_classes))];
			if(stream->num > 0){
				if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
					ssi_print ("Data available ... \n\n");
				}
				found_data = true;
				model = models[ _winner[n_class][n_winners] ];

				ssi_real_t* hot_probs = new ssi_real_t[2];

				model->forward (*stream, 2, hot_probs);

				if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
					ssi_print("\nHotProbs:\t");
					ssi_print("%f\t", hot_probs[0]);
					ssi_print("%f\n", hot_probs[1]);
					ssi_print("\n");
				}

#ifdef PRODUCT
				probs[n_class] *= hot_probs[0];
#endif

#ifdef SUM
				probs[n_class] += hot_probs[0];
#endif

				delete [] hot_probs;

				if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
					ssi_print("\nProbs:\t");
					for(ssi_size_t i = 0; i < _n_classes; i++){
						ssi_print("%f\t", probs[i]);
					}
					ssi_print("\n");
				}

				break;
			}
		}
	}
	
#ifdef HYBRID

	bool ffusion = true;
	for(ssi_size_t nstrm = 0; nstrm < _n_streams; nstrm++){
		stream = streams[nstrm];
		if(stream->num == 0){
			ffusion = false;
		}
	}

	if (ffusion){
		model = models[_n_models - 1];
		if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
			ssi_print("\nEvaluating with FeatureFusion Classifier\n");
		}

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

		ssi_real_t* ffusion_probs = new ssi_real_t [_n_classes];
		model->forward (*fusion_stream, _n_classes, ffusion_probs);

		if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
			ssi_print("\nHybridProbs:\t");
			for(ssi_size_t i = 0; i < _n_classes; i++){
			ssi_print("%f ", ffusion_probs[i]);
			}ssi_print("\n\n");
		}

#ifdef PRODUCT

		for (ssi_size_t n_class = 0; n_class < _n_classes; n_class++) {
			probs[n_class] *= ffusion_probs[n_class];
		}

#endif

#ifdef SUM

		for (ssi_size_t n_class = 0; n_class < _n_classes; n_class++) {
			probs[n_class] += ffusion_probs[n_class];
		}

#endif

		if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
			ssi_print("\nProbs:\t\t");
			for(ssi_size_t i = 0; i < _n_classes; i++){
				ssi_print("%f ", probs[i]);
			}
			ssi_print("\n");
		}

		delete ffusion_probs;
		ssi_stream_destroy(*fusion_stream);
		delete fusion_stream;
		fusion_stream = 0;
	}
	else{
		if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
			ssi_print("\nCould not Evaluate with FeatureFusion Classifier\n");
		}
		if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
			ssi_print("\nProbs:\t\t");
			for(ssi_size_t i = 0; i < _n_classes; i++){
				ssi_print("%f ", probs[i]);
			}
			ssi_print("\n");
		}
	}

#endif
	
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

bool OvRSpecialist::save (const ssi_char_t *filepath) {

	File *file = File::CreateAndOpen (File::BINARY, File::WRITE, filepath);

	file->write (&ssi_log_level, sizeof (ssi_log_level), 1);

	file->write (&_n_classes, sizeof (_n_classes), 1);
	file->write (&_n_streams, sizeof (_n_streams), 1);
	file->write (&_n_models, sizeof (_n_models), 1);
	file->write (&_n_samples, sizeof (_n_samples), 1);
	for (ssi_size_t n_class = 0; n_class < _n_classes; n_class++) {
		file->write (_winner[n_class], sizeof (ssi_size_t), (_n_streams));
	}
			
	delete file;

	return true;
}

bool OvRSpecialist::load (const ssi_char_t *filepath) {

	release ();

	File *file = File::CreateAndOpen (File::BINARY, File::READ, filepath);

	file->read (&ssi_log_level, sizeof (ssi_log_level), 1);

	file->read (&_n_classes, sizeof (_n_classes), 1);
	file->read (&_n_streams, sizeof (_n_streams), 1);
	file->read (&_n_models, sizeof (_n_models), 1);
	file->read (&_n_samples, sizeof (_n_samples), 1);
	_winner = new ssi_size_t *[_n_classes];
	for (ssi_size_t n_class = 0; n_class < _n_classes; n_class++) {
		_winner[n_class] = new ssi_size_t[_n_streams];
		file->read (_winner[n_class], sizeof (ssi_size_t), (_n_streams));
	}
		
	delete file;

	return true;
}

}
