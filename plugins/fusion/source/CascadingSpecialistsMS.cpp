// CascadingSpecialistsMS.cpp
// author: Florian Lingenfelser <florian.lingenfelser@informatik.uni-augsburg.de>
// created: 2010/4/12
// Copyright (C) 2007-12 University of Augsburg, Florian Lingenfelser
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
#include "CascadingSpecialistsMS.h"
#include "ISMissingData.h"
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

	ssi_char_t *CascadingSpecialistsMS::ssi_log_name = "cascspecms";

CascadingSpecialistsMS::CascadingSpecialistsMS (const ssi_char_t *file) 
	:_weights (0),
	_order (0),
	_winner (0),
	_filler (0),
	_file (0),
	_n_classes (0),
	_n_streams (0),
	_n_models (0),
	_n_winners (0),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {
} 

CascadingSpecialistsMS::~CascadingSpecialistsMS () { 
	release();
}

void CascadingSpecialistsMS::release ()
{
	if (_weights) {
		for (ssi_size_t n_model = 0; n_model < _n_models; n_model++) {
			delete[] _weights[n_model];
		}
		delete[] _weights;
		_weights = 0;
	}	
	if(_order){
		delete[] _order;
		_order = 0;
	}
	if (_winner) {
		for (ssi_size_t n_class = 0; n_class < _n_classes; n_class++) {
			delete[] _winner[n_class];
		}
		delete[] _winner;
		_winner = 0;
	}	
	if(_filler){
		delete [] _filler;
		_filler = 0;
	}

	_n_classes = 0;
	_n_streams = 0;
	_n_models  = 0;
	_n_winners = 0;
}

bool CascadingSpecialistsMS::train (ssi_size_t n_models,
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

		//initialize weights
		_weights = new ssi_real_t*[n_models];
		for (ssi_size_t n_model = 0; n_model < n_models; n_model++) {
			_weights[n_model] = new ssi_real_t[_n_classes+1];		
		}

		//train models an calculate weights
		ISMissingData samples_h (&samples);
		Evaluation eval;
		for (ssi_size_t n_model = 0; n_model < n_models; n_model++) {
			if (!models[n_model]->isTrained ()) {
				samples_h.setStream (n_model);
				models[n_model]->train (samples_h, n_model);
			}
			eval.eval (*models[n_model], samples_h, n_model);

			if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
				eval.print();
			}

			for (ssi_size_t n_class = 0; n_class < _n_classes; n_class++) {
				_weights[n_model][n_class] = eval.get_class_prob (n_class);
			}		
			_weights[n_model][_n_classes] = eval.get_classwise_prob ();			
		}

		//calculate rating of classes
		ssi_real_t *rating = new ssi_real_t[_n_classes];
		for (ssi_size_t n_class = 0; n_class < _n_classes; n_class++) {
			rating[n_class] = _weights[0][n_class];
			for (ssi_size_t n_model = 1; n_model < n_models; n_model++) {
				rating[n_class] += _weights[n_model][n_class];
			}
		}
		
		//calculate order of classes
		_order = new ssi_size_t[_n_classes];
		sort (_n_classes, rating, _order);

		//calculate list of winning classifiers for each class
		_n_winners = _n_models;

		ssi_real_t **win_weights = new ssi_real_t*[n_models];
		for (ssi_size_t n_model = 0; n_model < n_models; n_model++) {
			win_weights[n_model] = new ssi_real_t[_n_classes+1];		
		}
		
		for (ssi_size_t n_model = 0; n_model < n_models; n_model++) {
			for (ssi_size_t n_class = 0; n_class < _n_classes+1; n_class++) {
				win_weights[n_model][n_class] = _weights[n_model][n_class];
			}
			win_weights[n_model][_n_classes] = _weights[n_model][_n_classes];
		}

		_winner = new ssi_size_t*[_n_classes];
		for (ssi_size_t n_class = 0; n_class < _n_classes; n_class++) {
			_winner[n_class] = new ssi_size_t[_n_winners];
			for (ssi_size_t n_win = 0; n_win < _n_winners; n_win++) {
				_winner[n_class][n_win] = 0;
			}
		}

		for (ssi_size_t n_class = 0; n_class < _n_classes; n_class++) {
			for (ssi_size_t n_win = 0; n_win < _n_winners; n_win++) {
				_winner[n_class][n_win] = 0;
				ssi_real_t winner_weight = win_weights[0][n_class];
				for (ssi_size_t n_model = 1; n_model < n_models; n_model++) {
					if (winner_weight < win_weights[n_model][n_class]) {
						_winner[n_class][n_win] = n_model;
						winner_weight = win_weights[n_model][n_class];
					}
				}
				win_weights[_winner[n_class][n_win]][n_class] = 0.0f;
			}
		}

		//calculate filler
		_filler = new ssi_size_t[_n_winners];
		for (ssi_size_t n_win = 0; n_win < _n_winners; n_win++) {
			_filler[n_win] = 0;
			ssi_real_t filler_weight = win_weights[0][_n_classes];
			for (ssi_size_t n_model = 1; n_model < n_models; n_model++) {
				if (filler_weight < win_weights[n_model][_n_classes]) {
					_filler[n_win] = n_model;
					filler_weight = win_weights[n_model][_n_classes];
				}
			}
			win_weights[_filler[n_win]][_n_classes] = 0.0f;
		}
		
		//PRINT
		if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
			ssi_print("\nClassifier Weights: \n");
			for (ssi_size_t n_model = 0; n_model < n_models; n_model++) {
				for (ssi_size_t n_class = 0; n_class < _n_classes; n_class++) {
					ssi_print ("%f ", _weights[n_model][n_class]);
				}
				ssi_print ("%f\n", _weights[n_model][_n_classes]);		
			}ssi_print("\n");
			ssi_print("\nrating:\n");
			for (ssi_size_t n_class = 0; n_class < _n_classes; n_class++) {
				ssi_print("%f ", rating[n_class]);
			}ssi_print("\n");
			ssi_print("\norder:\n");
			for (ssi_size_t n_class = 0; n_class < _n_classes; n_class++) {
				ssi_print("%d ", _order[n_class]);
			}ssi_print("\n");
			ssi_print("\nwinners:\n");
			for (ssi_size_t n_class = 0; n_class < _n_classes; n_class++) {
				for(ssi_size_t n_winners = 0; n_winners < _n_winners; n_winners++){
					ssi_print("%d ", _winner[n_class][n_winners]);
				}
				ssi_print("\n");
			}ssi_print("\n");
			ssi_print("\nfiller:\n");
			for (ssi_size_t n_model = 0; n_model < n_models; n_model++) {
				ssi_print("%d ", _filler[n_model]);
			}ssi_print("\n");
		}

		//CLEAN UP
		if(rating){
			delete[] rating;
			rating = 0;
		}
		if(win_weights) {
			for (ssi_size_t n_model = 0; n_model < _n_models; n_model++) {
				delete[] win_weights[n_model];
			}
			delete[] win_weights;
			win_weights = 0;
		}	
		
	}
	else{

		//initialize weights
		_weights = new ssi_real_t*[n_models];
		for (ssi_size_t n_model = 0; n_model < n_models; n_model++) {
			_weights[n_model] = new ssi_real_t[_n_classes+1];		
		}

		//train models and calculate weights
		Evaluation eval;
		for (ssi_size_t n_model = 0; n_model < n_models; n_model++) {
			if (!models[n_model]->isTrained ()) { models[n_model]->train (samples, n_model); }
			eval.eval (*models[n_model], samples, n_model);

			if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
				eval.print();
			}

			for (ssi_size_t n_class = 0; n_class < _n_classes; n_class++) {
				_weights[n_model][n_class] = eval.get_class_prob (n_class);
			}		
			_weights[n_model][_n_classes] = eval.get_classwise_prob ();
		}

		//calculate rating of classes
		ssi_real_t *rating = new ssi_real_t[_n_classes];
		for (ssi_size_t n_class = 0; n_class < _n_classes; n_class++) {
			rating[n_class] = _weights[0][n_class];
			for (ssi_size_t n_model = 1; n_model < n_models; n_model++) {
				rating[n_class] += _weights[n_model][n_class];
			}
		}
		
		//calculate order of classes
		_order = new ssi_size_t[_n_classes];
		sort (_n_classes, rating, _order);

		//calculate list of winning classifiers for each class
		_n_winners = _n_models;

		ssi_real_t **win_weights = new ssi_real_t*[n_models];
		for (ssi_size_t n_model = 0; n_model < n_models; n_model++) {
			win_weights[n_model] = new ssi_real_t[_n_classes+1];		
		}
		
		for (ssi_size_t n_model = 0; n_model < n_models; n_model++) {
			for (ssi_size_t n_class = 0; n_class < _n_classes+1; n_class++) {
				win_weights[n_model][n_class] = _weights[n_model][n_class];
			}
			win_weights[n_model][_n_classes] = _weights[n_model][_n_classes];
		}

		_winner = new ssi_size_t*[_n_classes];
		for (ssi_size_t n_class = 0; n_class < _n_classes; n_class++) {
			_winner[n_class] = new ssi_size_t[_n_winners];
			for (ssi_size_t n_win = 0; n_win < _n_winners; n_win++) {
				_winner[n_class][n_win] = 0;
			}
		}

		for (ssi_size_t n_class = 0; n_class < _n_classes; n_class++) {
			for (ssi_size_t n_win = 0; n_win < _n_winners; n_win++) {
				_winner[n_class][n_win] = 0;
				ssi_real_t winner_weight = win_weights[0][n_class];
				for (ssi_size_t n_model = 1; n_model < n_models; n_model++) {
					if (winner_weight < win_weights[n_model][n_class]) {
						_winner[n_class][n_win] = n_model;
						winner_weight = win_weights[n_model][n_class];
					}
				}
				win_weights[_winner[n_class][n_win]][n_class] = 0.0f;
			}
		}

		//calculate filler
		_filler = new ssi_size_t[_n_winners];
		for (ssi_size_t n_win = 0; n_win < _n_winners; n_win++) {
			_filler[n_win] = 0;
			ssi_real_t filler_weight = win_weights[0][_n_classes];
			for (ssi_size_t n_model = 1; n_model < n_models; n_model++) {
				if (filler_weight < win_weights[n_model][_n_classes]) {
					_filler[n_win] = n_model;
					filler_weight = win_weights[n_model][_n_classes];
				}
			}
			win_weights[_filler[n_win]][_n_classes] = 0.0f;
		}
		
		if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
			ssi_print("\nClassifier Weights: \n");
			for (ssi_size_t n_model = 0; n_model < n_models; n_model++) {
				for (ssi_size_t n_class = 0; n_class < _n_classes; n_class++) {
					ssi_print ("%f ", _weights[n_model][n_class]);
				}
				ssi_print ("%f\n", _weights[n_model][_n_classes]);		
			}ssi_print("\n");
			ssi_print("\nrating:\n");
			for (ssi_size_t n_class = 0; n_class < _n_classes; n_class++) {
				ssi_print("%f ", rating[n_class]);
			}ssi_print("\n");
			ssi_print("\norder:\n");
			for (ssi_size_t n_class = 0; n_class < _n_classes; n_class++) {
				ssi_print("%d ", _order[n_class]);
			}ssi_print("\n");
			ssi_print("\nwinners:\n");
			for (ssi_size_t n_class = 0; n_class < _n_classes; n_class++) {
				for(ssi_size_t n_winners = 0; n_winners < _n_winners; n_winners++){
					ssi_print("%d ", _winner[n_class][n_winners]);
				}
				ssi_print("\n");
			}ssi_print("\n");
			ssi_print("\nfiller:\n");
			for (ssi_size_t n_model = 0; n_model < n_models; n_model++) {
				ssi_print("%d ", _filler[n_model]);
			}ssi_print("\n");
		}

		//CLEAN UP
		if(rating){
			delete[] rating;
			rating = 0;
		}
		if(win_weights) {
			for (ssi_size_t n_model = 0; n_model < _n_models; n_model++) {
				delete[] win_weights[n_model];
			}
			delete[] win_weights;
			win_weights = 0;
		}		
	}

	return true;
}

void CascadingSpecialistsMS::sort (ssi_size_t nSize, ssi_real_t *anArray, ssi_size_t *order) {

	for (ssi_size_t nStartIndex = 0; nStartIndex < nSize; nStartIndex++) {
		order[nStartIndex] = nStartIndex;
	}

	// Step through each element of the array
	for (ssi_size_t nStartIndex = 0; nStartIndex < nSize; nStartIndex++)
	{
		// nSmallestIndex is the index of the smallest element
		// we've encountered so far.
		int nSmallestIndex = nStartIndex;

		// Search through every element starting at nStartIndex+1
		for (ssi_size_t nCurrentIndex = nStartIndex + 1; nCurrentIndex < nSize; nCurrentIndex++)
		{
			// If the current element is smaller than our previously found smallest
			if (anArray[nCurrentIndex] < anArray[nSmallestIndex])
				// Store the index in nSmallestIndex
				nSmallestIndex = nCurrentIndex;
		}

		// Swap our start element with our smallest element
		real_swap(anArray[nStartIndex], anArray[nSmallestIndex]);
		size_swap(order[nStartIndex], order[nSmallestIndex]);
	}
}

SSI_INLINE void CascadingSpecialistsMS::real_swap (ssi_real_t &x, ssi_real_t &y) {
	ssi_real_t z = x;
	x = y;
	y = z;
}

SSI_INLINE void CascadingSpecialistsMS::size_swap (ssi_size_t &x, ssi_size_t &y) {
	ssi_size_t z = x;
	x = y;
	y = z;
}

bool CascadingSpecialistsMS::forward (ssi_size_t n_models,
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

		ssi_size_t n_winning_models = 0;

		if( ((_n_streams / 2) + 1) <= (n_models - miss_counter) ){
			n_winning_models = (_n_streams / 2) + 1;
		}else{
			n_winning_models = n_models - miss_counter;
		}

		if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
			ssi_print("\n\n-----------------------------\nactual models:\n");
			for(ssi_size_t i = 0; i < (n_models - miss_counter); i++){
				ssi_print("%d ", models_actual[i]);
			}ssi_print("\n");
			ssi_print("\norder:\n");
			for (ssi_size_t n_class = 0; n_class < _n_classes; n_class++) {
				ssi_print("%d ", _order[n_class]);
			}ssi_print("\n");
			ssi_print("\n#winners:\n%d\n\n", n_winning_models);
			ssi_print("\nwinners:\n");
			for (ssi_size_t n_class = 0; n_class < _n_classes; n_class++) {
				for(ssi_size_t n_winners = 0; n_winners < _n_winners; n_winners++){
					ssi_print("%d ", _winner[n_class][n_winners]);
				}
				ssi_print("\n");
			}ssi_print("\n");
		}

		

		for (ssi_size_t n_class = 0; n_class < _n_classes; n_class++) {

			//initialize DP
			ssi_real_t **decision_profile = new ssi_real_t*[n_winning_models];
			for(ssi_size_t n_win = 0; n_win < n_winning_models; n_win++){
				decision_profile[n_win] = new ssi_real_t[_n_classes];		
			}
			
			ssi_size_t next = _order[n_class];

			ssi_size_t model_id = 0;
			ssi_size_t win_counter = 0;
			if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
				ssi_print("\nSelected Models: ");
			}
			for(ssi_size_t h = 0; h < _n_models; h++){
				model_id = _winner[next][h];
				for(ssi_size_t i = 0; i < (n_models - miss_counter); i++){
					if( (model_id == models_actual[i]) && (win_counter < n_winning_models) ){
						model = models[model_id];
						model->forward(*streams[model_id], n_probs, probs);
						for(ssi_size_t j = 0; j < _n_classes; j++){
							decision_profile[win_counter][j] = probs[j];
						}
						win_counter++;
						if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
							ssi_print("%d ", model_id);
						}
					}
				}
			}
						
			//prepare probs
			for (ssi_size_t c = 0; c < _n_classes; c++) {
				probs[c] = 1.0f;
			}

			//product rule
			for (ssi_size_t num_probs = 0; num_probs < n_probs; num_probs++){
				for (ssi_size_t n_model = 0; n_model < n_winning_models; n_model++){
					probs[num_probs] = probs[num_probs]* decision_profile[n_model][num_probs];
				}
			}
			if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
				ssi_print("\n");
				ssi_print("DP:\n");
				for(ssi_size_t w = 0; w < n_winning_models; w++){
					for (ssi_size_t p = 0; p < _n_classes; p++){
						ssi_print("%f ", decision_profile[w][p]);
					}ssi_print("\n");
				}ssi_print("\n");
				ssi_print("ProductRule:\n");
				for (ssi_size_t num_probs = 0; num_probs < _n_classes; num_probs++){
					ssi_print("%f ", probs[num_probs]);
				}ssi_print("\n\n");
			}

			ssi_size_t max_ind = 0;
			ssi_real_t max_val = probs[0];
			for (ssi_size_t i = 1; i < n_probs; i++) {
				if (probs[i] > max_val) {
					max_val = probs[i];
					max_ind = i;
				}
			}

			if (max_ind == next) {

				if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
					ssi_print("\nDecision: %d\n\n", max_ind);
				}

				if(decision_profile){
					for (ssi_size_t n_model = 0; n_model < n_winning_models; n_model++) {
						delete [] decision_profile[n_model];
					}
					delete[] decision_profile;
					decision_profile = 0;
				}
				if(available){
					delete [] available;
					available = 0;
				}
				if(models_actual){
					delete [] models_actual;
					models_actual = 0;
				}
				
				return found_data;
			}

			if(decision_profile){
				for (ssi_size_t n_model = 0; n_model < n_winning_models; n_model++) {
					delete [] decision_profile[n_model];
				}
				delete[] decision_profile;
				decision_profile = 0;
			}

		}

		if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
			ssi_print("\nDecision: Filler needed!\n");
			ssi_print("\nfiller:\n");
			for (ssi_size_t n_model = 0; n_model < n_models; n_model++) {
				ssi_print("%d ", _filler[n_model]);
			}ssi_print("\n");
		}

		bool model_available = false;
		ssi_size_t model_id = 0;
		for(ssi_size_t h = 0; h < _n_models; h++){
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

		model->forward(*streams[model_id], n_probs, probs);
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

bool CascadingSpecialistsMS::save (const ssi_char_t *filepath) {

	File *file = File::CreateAndOpen (File::BINARY, File::WRITE, filepath);

	file->write (&ssi_log_level, sizeof (ssi_log_level), 1);

	file->write (&_n_classes, sizeof (_n_classes), 1);
	file->write (&_n_streams, sizeof (_n_streams), 1);
	file->write (&_n_models, sizeof (_n_models), 1);
	file->write (&_n_winners, sizeof (_n_winners), 1);
	for (ssi_size_t n_model = 0; n_model < _n_models; n_model++) {
		file->write (_weights[n_model], sizeof (ssi_real_t), (_n_classes+1));
	}
	for (ssi_size_t n_class = 0; n_class < _n_classes; n_class++) {
		file->write (_winner[n_class], sizeof (ssi_size_t), (_n_winners));
	}
	file->write (_order, sizeof (ssi_size_t), _n_classes);
	file->write (_filler, sizeof (ssi_size_t), _n_winners);
	
	delete file;

	return true;
}

bool CascadingSpecialistsMS::load (const ssi_char_t *filepath) {

	release ();

	File *file = File::CreateAndOpen (File::BINARY, File::READ, filepath);

	file->read (&ssi_log_level, sizeof (ssi_log_level), 1);

	file->read (&_n_classes, sizeof (_n_classes), 1);
	file->read (&_n_streams, sizeof (_n_streams), 1);
	file->read (&_n_models, sizeof (_n_models), 1);
	file->read (&_n_winners, sizeof (_n_winners), 1);
	_weights = new ssi_real_t *[_n_models];
	for (ssi_size_t n_model = 0; n_model < _n_models; n_model++) {
		_weights[n_model] = new ssi_real_t[_n_classes+1];
		file->read (_weights[n_model], sizeof (ssi_real_t), (_n_classes+1));
	}
	_winner = new ssi_size_t *[_n_classes];
	for (ssi_size_t n_class = 0; n_class < _n_classes; n_class++) {
		_winner[n_class] = new ssi_size_t[_n_winners];
		file->read (_winner[n_class], sizeof (ssi_size_t), (_n_winners));
	}
	_order = new ssi_size_t[_n_classes];
	file->read (_order, sizeof (ssi_size_t), _n_classes);
	_filler = new ssi_size_t[_n_winners];
	file->read (_filler, sizeof (ssi_size_t), _n_winners);
	
	delete file;

	return true;
}

}
