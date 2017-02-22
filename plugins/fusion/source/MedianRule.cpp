// MedianRule.cpp
// author: Florian Lingenfelser <florian.lingenfelser@informatik.uni-augsburg.de>
// created: 2010/4/6
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

#include "MedianRule.h"
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

	ssi_char_t *MedianRule::ssi_log_name = "medianrule";

MedianRule::MedianRule (const ssi_char_t *file) 
	:	_file (0),
		_n_classes (0),
		_n_streams (0),
		_n_models (0),
		_n_models_actual (0),
		ssi_log_level (SSI_LOG_LEVEL_DEFAULT){
} 

MedianRule::~MedianRule () { 
	release();
}

void MedianRule::release ()
{
	_n_classes = 0;
	_n_streams = 0;
	_n_models  = 0;
	_n_models_actual = 0;
}

bool MedianRule::train (ssi_size_t n_models,
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

bool MedianRule::forward (ssi_size_t n_models,
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
	
	//calculate profile size
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

	ssi_real_t *votes = 0;
	votes = new ssi_real_t[(n_models - miss_counter)];

	if(found_data){
		
		_n_models_actual = n_models - miss_counter;

		//initialize DP
		ssi_real_t **decision_profile = new ssi_real_t*[_n_models_actual];
		for (ssi_size_t n_model = 0; n_model < _n_models_actual; n_model++) {
			decision_profile[n_model] = new ssi_real_t[_n_classes];		
		}

		for (ssi_size_t n_model = 0; n_model < _n_models_actual; n_model++) {
			model = models[models_actual[n_model]];
			stream = streams[models_actual[n_model]];
			if (stream->num > 0) {
				model->forward (*stream, n_probs, probs);
				found_data = true;
				//fill decision_profile DP
				for (ssi_size_t num_probs = 0; num_probs < n_probs; num_probs++){
					decision_profile[n_model][num_probs] = probs[num_probs];
				}
			} else {
				//fill decision_profile DP
				for (ssi_size_t num_probs = 0; num_probs < n_probs; num_probs++){
					decision_profile[n_model][num_probs] = 0.0f;
				}
			}
		}

		//clear probs
		for (ssi_size_t num_probs = 0; num_probs < n_probs; num_probs++){
			probs[num_probs] = 0.0f;
		}

		//fill probs
		ssi_size_t even = ssi_cast(ssi_size_t, (n_models - miss_counter)) % ssi_cast(ssi_size_t, 2);
		switch (even)
		{
		case 1:
			for (ssi_size_t num_probs = 0; num_probs < n_probs; num_probs++){
				for (ssi_size_t n_model = 0; n_model < (n_models - miss_counter); n_model++){
					votes[n_model] =  decision_profile[n_model][num_probs];
				}
				sort((n_models - miss_counter), votes);
				probs[num_probs] = votes[ ((_n_models_actual + 1) / 2) - 1 ];
			}
			break;
		case 0:
			for (ssi_size_t num_probs = 0; num_probs < n_probs; num_probs++){
				for (ssi_size_t n_model = 0; n_model < (n_models - miss_counter); n_model++){
					votes[n_model] =  decision_profile[n_model][num_probs];
				}
				sort((n_models - miss_counter), votes);
				probs[num_probs] = ( votes[ (_n_models_actual / 2) - 1 ] + votes[ (_n_models_actual / 2) ] ) / 2;
			}
			break;
		}

		if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
			print_DP(decision_profile, probs);
		}

		if(decision_profile){
			for (ssi_size_t n_model = 0; n_model < _n_models_actual; n_model++) {
				delete [] decision_profile[n_model];
			}
			delete[] decision_profile;
			decision_profile = 0;
		}
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

void MedianRule::sort (ssi_size_t nSize, ssi_real_t *anArray) {

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
	}
}

SSI_INLINE void MedianRule::real_swap (ssi_real_t &x, ssi_real_t &y) {
	ssi_real_t z = x;
	x = y;
	y = z;
}

void MedianRule::print_DP(ssi_real_t **decision_profile, ssi_real_t *probs){

	ssi_print("DP:\n");
	for (ssi_size_t n_model = 0; n_model < _n_models_actual; n_model++){
		for (ssi_size_t num_probs = 0; num_probs < _n_classes; num_probs++){
			ssi_print("%f ", decision_profile[n_model][num_probs]);
		}ssi_print("\n");
	}ssi_print("\n");
	ssi_print("MedianRule:\n");
	for (ssi_size_t num_probs = 0; num_probs < _n_classes; num_probs++){
		ssi_print("%f ", probs[num_probs]);
	}ssi_print("\n\n");

}

bool MedianRule::save (const ssi_char_t *filepath) {

	File *file = File::CreateAndOpen (File::BINARY, File::WRITE, filepath);

	file->write (&ssi_log_level, sizeof (ssi_log_level), 1);

	file->write (&_n_classes, sizeof (_n_classes), 1);
	file->write (&_n_streams, sizeof (_n_streams), 1);
	file->write (&_n_models, sizeof (_n_models), 1);
	
	delete file;
	
	return true;
}

bool MedianRule::load (const ssi_char_t *filepath) {

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
