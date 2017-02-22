// MeanRule.cpp
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

#include "MeanRule.h"
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

	ssi_char_t *MeanRule::ssi_log_name = "meanrule__";

MeanRule::MeanRule (const ssi_char_t *file) 
	:	_file (0),
		_n_classes (0),
		_n_streams (0),
		_n_models (0),
		ssi_log_level (SSI_LOG_LEVEL_DEFAULT){
} 

MeanRule::~MeanRule () { 
	release();
}

void MeanRule::release ()
{
	_n_classes = 0;
	_n_streams = 0;
	_n_models  = 0;
}

bool MeanRule::train (ssi_size_t n_models,
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

bool MeanRule::forward (ssi_size_t n_models,
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

	ssi_size_t miss_count = 0;
	IModel *model = 0;
	ssi_real_t **decision_profile= new ssi_real_t*[n_models];
	for (ssi_size_t n_model = 0; n_model < n_models; n_model++) {
		decision_profile[n_model] = new ssi_real_t[_n_classes];		
	}
	
	ssi_stream_t *stream = 0;
	for (ssi_size_t n_model = 0; n_model < n_models; n_model++) {
		model = models[n_model];
		stream = streams[n_model];
		if (stream->num > 0) {
			model->forward (*stream, n_probs, probs);
			found_data = true;
			//fill decision_profile DP
			for (ssi_size_t num_probs = 0; num_probs < n_probs; num_probs++){
				decision_profile[n_model][num_probs] = probs[num_probs];
			}
		} else {
			miss_count++;
			//fill decision_profile DP
			for (ssi_size_t num_probs = 0; num_probs < n_probs; num_probs++){
				decision_profile[n_model][num_probs] = 0.0f;
			}
		}			
	}

	if(found_data){

		//clear probs
		for (ssi_size_t num_probs = 0; num_probs < n_probs; num_probs++){
			probs[num_probs] = 0.0f;
		}

		//fill probs
		for (ssi_size_t num_probs = 0; num_probs < n_probs; num_probs++){
			for (ssi_size_t n_model = 0; n_model < n_models; n_model++){
				probs[num_probs] = probs[num_probs]+decision_profile[n_model][num_probs];
			}
			probs[num_probs] = probs[num_probs] / ssi_cast(ssi_real_t, (n_models - miss_count));
		}

		if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
			print_DP(decision_profile, probs);
		}

	}

	if(decision_profile){
		for (ssi_size_t n_model = 0; n_model < _n_models; n_model++) {
			delete [] decision_profile[n_model];
		}
		delete[] decision_profile;
		decision_profile= 0;
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

void MeanRule::print_DP(ssi_real_t **decision_profile, ssi_real_t* probs){

	ssi_print("DP:\n");
	for (ssi_size_t n_model = 0; n_model < _n_models; n_model++){
		for (ssi_size_t num_probs = 0; num_probs < _n_classes; num_probs++){
			ssi_print("%f ", decision_profile[n_model][num_probs]);
		}ssi_print("\n");
	}ssi_print("\n");
	ssi_print("MeanRule:\n");
	for (ssi_size_t num_probs = 0; num_probs < _n_classes; num_probs++){
		ssi_print("%f ", probs[num_probs]);
	}ssi_print("\n\n");

}

bool MeanRule::save (const ssi_char_t *filepath) {

	File *file = File::CreateAndOpen (File::BINARY, File::WRITE, filepath);

	file->write (&ssi_log_level, sizeof (ssi_log_level), 1);

	file->write (&_n_classes, sizeof (_n_classes), 1);
	file->write (&_n_streams, sizeof (_n_streams), 1);
	file->write (&_n_models, sizeof (_n_models), 1);
	
	delete file;
	
	return true;
}

bool MeanRule::load (const ssi_char_t *filepath) {

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
