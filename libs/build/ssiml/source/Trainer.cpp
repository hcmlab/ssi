// Trainer.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/06/12
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

#include "Trainer.h"
#include "ModelTools.h"
#include "base/Factory.h"
#include "ISMissingData.h"
#include "ISSelectDim.h"
#include "ISUnderSample.h"
#include "ISOverSample.h"
#include "ISTransform.h"
#include "Selection.h"
#include "signal/SignalTools.h"
#include "ioput/option/OptionList.h"
#include "base/Random.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ssi_char_t *Trainer::ssi_log_name = "trainer___";
int Trainer::ssi_log_level = SSI_LOG_LEVEL_DEFAULT;

Trainer::VERSION Trainer::DEFAULT_VERSION = Trainer::V5;

Trainer::Trainer()
	: _n_models(0),
	_models(0),
	_fusion(0),
	_n_classes(0),
	_class_names(0),
	_n_users(0),
	_user_names(0),
	_n_streams(0),
	_stream_index(0),
	_stream_refs(0),
	_n_stream_select(0),
	_stream_select(0),
	_has_selection(false),
	_transformer(0),
	_transformer_frame(0),
	_transformer_delta(0),
	_has_transformer(false),
	_activity(0),
	_activity_frame(0),
	_activity_delta(0),
	_has_activity(false),
	_has_normalization(false),
	_normalization(0),
	_normalization_free(false),
	_is_trained(false),
	_preproc_mode(false),
	_n_samplepaths(0),
	_samplepaths(0),
	_balance(BALANCE::NONE),
	_registerNode(0),
	_preventWarningsSpam(false)
{
	_seed = Random::Seed();
}

Trainer::Trainer (IModel *model,
	ssi_size_t stream_index) 
	: _n_models (1),
	_models (0),	
	_fusion (0),	
	_n_classes (0),
	_class_names (0), 
	_n_users (0),
	_user_names (0),
	_n_streams (0),
	_stream_refs (0),
	_stream_index (stream_index),
	_n_stream_select (0),
	_stream_select (0),
	_has_selection (false),
	_transformer (0),
	_transformer_frame (0),
	_transformer_delta (0),
	_has_transformer (false),
	_activity(0),
	_activity_frame(0),
	_activity_delta(0),
	_has_activity(false),
	_has_normalization(false),
	_normalization(0),
	_normalization_free(false),
	_is_trained (false),
	_preproc_mode (false),
	_n_samplepaths (0),
	_samplepaths (0),
	_balance(BALANCE::NONE),
	_registerNode(0),
	_preventWarningsSpam(false)
{

	_models = new IModel*[1];
	_models[0] = model;

	_seed = Random::Seed();
};

Trainer::Trainer (ssi_size_t n_models,
	IModel **models,
	IFusion *fusion)
	: _n_models (n_models),
	_models (0),
	_fusion (fusion),
	_n_classes (0),
	_class_names (0), 
	_n_users (0),
	_user_names (0),
	_n_streams (0),
	_stream_index (0),
	_stream_refs (0),
	_n_stream_select (0),
	_stream_select (0),
	_has_selection (false),
	_transformer (0),
	_transformer_frame (0),
	_transformer_delta (0),
	_has_transformer (false),
	_activity(0),
	_activity_frame(0),
	_activity_delta(0),
	_has_activity(false),
	_has_normalization(false),
	_normalization(0),
	_normalization_free(false),
	_is_trained (false),
	_preproc_mode (false),
	_n_samplepaths (0),
	_samplepaths (0),
	_balance(BALANCE::NONE),
	_registerNode(0),
	_preventWarningsSpam(false)
{

	_models = new IModel*[_n_models];
	for (ssi_size_t n_model = 0; n_model < n_models; n_model++) {
		_models[n_model] = models[n_model];
	}

	_seed = Random::Seed();
}

Trainer::~Trainer () {
	
	release_transformer ();
	release_activity();
	release_selection ();
	release_normalization ();
	release_samples ();

	release();

	delete[] _models;
	delete[] _stream_refs;

}

void Trainer::release () {
	
	_preventWarningsSpam = false;
	if (_fusion) {
		_fusion->release ();		
	}
	for (ssi_size_t n_model = 0; n_model < _n_models; n_model++) {
		_models[n_model]->release ();
	}
	free_class_names ();
	free_user_names ();
	_n_streams = 0;
	_is_trained = false;
	delete[] _stream_refs;
	_stream_refs = 0;
	delete[] _registerNode; _registerNode = 0;
	Meta.clear();
}

void Trainer::release_samples () {

	for (ssi_size_t i = 0; i < _n_samplepaths; i++) {
		delete[] _samplepaths[i];
	}
	delete[] _samplepaths;
	_samplepaths = 0;
	_n_samplepaths = 0;
}

void Trainer::release_transformer () {
	
	if (_has_transformer) {
		delete[] _transformer;
		_transformer = 0;
		delete[] _transformer_frame;
		_transformer_frame = 0;
		delete[] _transformer_delta;
		_transformer_delta = 0;
	}
	_has_transformer = false;
}

void Trainer::release_activity() {

	if (_has_activity) {
		delete[] _activity;
		_activity = 0;
		delete[] _activity_percentage;
		_activity_percentage = 0;
		delete[] _activity_frame;
		_activity_frame = 0;
		delete[] _activity_delta;
		_activity_delta = 0;
	}
	_has_activity = false;
}

void Trainer::release_selection () {

	if (_has_selection) {
		for (ssi_size_t i = 0; i < _n_streams; i++) {
			delete[] _stream_select[i];
		}
		delete[] _stream_select;
		delete[] _n_stream_select;
	}
	_has_selection = false;
}

void Trainer::release_normalization() {

	if (_normalization_free) {
		for (ssi_size_t i = 0; i < _n_streams; i++) {
			if (_normalization[i]) {
				ISNorm::ReleaseParams(*_normalization[i]);
			}
		}
	}

	delete[] _normalization;
	_normalization = 0;
	_has_normalization = false;
	_normalization_free = false;
}

void Trainer::init_class_names (ISamples &samples) {

	free_class_names ();

	_n_classes = samples.getClassSize ();
	_class_names = new ssi_char_t *[_n_classes];
	for (ssi_size_t i = 0; i < _n_classes; i++) {
		_class_names[i] = ssi_strcpy (samples.getClassName (i));
	}
}

void Trainer::free_class_names () {

	if (_class_names) {
		for (ssi_size_t i = 0; i < _n_classes; i++) {
			delete[] _class_names[i];
		}
		delete[] _class_names;
		_class_names = 0;
		_n_classes = 0;
	}	
}

void Trainer::init_user_names (ISamples &samples) {

	free_user_names ();

	_n_users = samples.getUserSize ();
	_user_names = new ssi_char_t *[_n_users];
	for (ssi_size_t i = 0; i < _n_users; i++) {
		_user_names[i] = ssi_strcpy (samples.getUserName (i));
	}
}

void Trainer::free_user_names () {

	if (_user_names) {
		for (ssi_size_t i = 0; i < _n_users; i++) {
			delete[] _user_names[i];
		}
		delete[] _user_names;
		_user_names = 0;
		_n_users = 0;
	}	
}

void Trainer::setSeed(ssi_size_t seed)
{
	_seed = seed;
}

void Trainer::setBalance(BALANCE::Value balance)
{
	_balance = balance;
}

bool Trainer::setSelection(ssi_size_t n_select,
	ssi_size_t *stream_select) {

	return setSelection(1, &n_select, &stream_select);
}

bool Trainer::setSelection (ssi_size_t n_streams, 
	ssi_size_t *n_stream_select, 
	ssi_size_t **stream_select) {

	if (_is_trained) {
		ssi_wrn ("already trained");
		return false;
	}

	release_selection ();

	if (_n_streams > 0 && n_streams != _n_streams) {
		ssi_wrn("#streams do not match (%u != %u)", n_streams, _n_streams);
	} else {
		_n_streams = n_streams;
	}

	_n_stream_select = new ssi_size_t[_n_streams];
	_stream_select = new ssi_size_t *[_n_streams];
	for (ssi_size_t i = 0; i < _n_streams; i++) {
		_n_stream_select[i] = n_stream_select[i];
		_stream_select[i] = 0;
		if (_n_stream_select[i] > 0) {
			_stream_select[i] = new ssi_size_t[_n_stream_select[i]];
			for (ssi_size_t j = 0; j < _n_stream_select[i]; j++) {
				_stream_select[i][j] = stream_select[i][j];
			}
		}
	}

	_has_selection = true;

	return true;
}

bool Trainer::setSelection (ISamples &samples,
	ISelection *fselmethod,
	ISelection *pre_fselmethod,
	ssi_size_t n_pre_feature) {

	if (_is_trained) {
		ssi_wrn ("already trained");
		return false;
	}
	
	release_selection ();

	if (_n_streams > 0 && samples.getStreamSize () != _n_streams) {
		ssi_wrn("#streams do not match (%u != %u)", _n_streams, samples.getStreamSize());
	} else {
		_n_streams = samples.getStreamSize ();
	}

	if (_n_streams != _n_models) {
		ssi_wrn ("to apply feature selection #streams (%u) must be equal to #models (%u)", _n_streams, _n_models);
		return false;
	}

	_n_stream_select = new ssi_size_t[_n_streams];
	_stream_select = new ssi_size_t*[_n_streams];

	if (pre_fselmethod && n_pre_feature > 0) {
			
		ssi_msg (SSI_LOG_LEVEL_DETAIL, "apply feature selection '%s'->%u->'%s' using %u streams", pre_fselmethod->getName (), n_pre_feature, fselmethod->getName (), _n_streams);		

		ISSelectDim pre_samples (&samples);
		
		for (ssi_size_t nstream = 0; nstream < _n_streams; nstream++) {

			// pre

			Selection pre_selection;
		
			if (pre_fselmethod->isWrapper ()) {
				pre_fselmethod->setModel (*_models[nstream]);
			}

			if (samples.hasMissingData ()) {
				ISMissingData samples_m (&samples);
				samples_m.setStream (nstream);							
				pre_fselmethod->train (samples_m, nstream);		
			} else {
				pre_fselmethod->train (samples, nstream);		
			}

			pre_selection.set (pre_fselmethod->getSize (), pre_fselmethod->getScores (), pre_fselmethod->sortByScore ());
			pre_selection.selNFirst (n_pre_feature);

			pre_samples.setSelection (nstream, pre_selection.getSize (), pre_selection.getSelected (), false); 

			ssi_msg (SSI_LOG_LEVEL_DEBUG, "selected %u feature applying '%s' on stream %u", n_pre_feature, pre_fselmethod->getName (), nstream);
		
			// final

			Selection final_selection (&pre_selection);

			if (fselmethod->isWrapper ()) {
				fselmethod->setModel (*_models[nstream]);
			}

			if (pre_samples.hasMissingData ()) {
				ISMissingData samples_m (&pre_samples);
				samples_m.setStream (nstream);							
				fselmethod->train (samples_m, nstream);		
			} else {
				fselmethod->train (pre_samples, nstream);		
			}

			final_selection.set (fselmethod->getSize (), fselmethod->getScores (), fselmethod->sortByScore ());
			final_selection.selNBest ();

			_n_stream_select[nstream] = final_selection.getSize ();
			_stream_select[nstream] = new ssi_size_t[_n_stream_select[nstream]];
			memcpy (_stream_select[nstream], final_selection.getSelected (), _n_stream_select[nstream] * sizeof (ssi_size_t));

			ssi_msg (SSI_LOG_LEVEL_DEBUG, "selected %u feature applying '%s' on stream %u", final_selection.getSize (), fselmethod->getName (), nstream);
		}

	} else {

		ssi_msg (SSI_LOG_LEVEL_DETAIL, "apply feature selection '%s' using %u streams", fselmethod->getName (), _n_streams);		
		
		for (ssi_size_t nstream = 0; nstream < _n_streams; nstream++) {
		
			// final

			Selection final_selection;

			if (fselmethod->isWrapper ()) {
				fselmethod->setModel (*_models[nstream]);
			}

			if (samples.hasMissingData ()) {
				ISMissingData samples_m (&samples);
				samples_m.setStream (nstream);							
				fselmethod->train (samples_m, nstream);		
			} else {
				fselmethod->train (samples, nstream);		
			}

			final_selection.set (fselmethod->getSize (), fselmethod->getScores (), fselmethod->sortByScore ());
			final_selection.selNBest ();

			_n_stream_select[nstream] = final_selection.getSize ();
			_stream_select[nstream] = new ssi_size_t[_n_stream_select[nstream]];
			memcpy (_stream_select[nstream], final_selection.getSelected (), _n_stream_select[nstream] * sizeof (ssi_size_t));

			ssi_msg (SSI_LOG_LEVEL_DEBUG, "selected %u feature applying '%s' on stream %u", final_selection.getSize (), fselmethod->getName (), nstream);
		}
	}

	_has_selection = true;

	return true;
}

bool Trainer::setTransformer(ITransformer *transformer,
	ssi_size_t transformer_frame,
	ssi_size_t transformer_delta) {

	return setTransformer(1, &transformer, &transformer_frame, &transformer_delta);
}

bool Trainer::setTransformer (ssi_size_t n_streams,
	ITransformer **transformer,
	ssi_size_t *transformer_frame,
	ssi_size_t *transformer_delta) {
	
	if (_is_trained) {
		ssi_wrn ("already trained");
		return false;
	}
	
	release_transformer ();

	if (_n_streams > 0 && n_streams != _n_streams) {
		ssi_wrn ("#streams do not match (%u != %u)", _n_streams, n_streams);
	} else {
		_n_streams = n_streams;
	}

	_transformer = new ITransformer *[_n_streams];
	_transformer_frame = new ssi_size_t[n_streams];
	_transformer_delta = new ssi_size_t[n_streams];
	for (ssi_size_t i = 0; i < _n_streams; i++) {
		_transformer[i] = transformer[i];
		_transformer_frame[i] = transformer_frame ? transformer_frame[i] : 0;
		_transformer_delta[i] = transformer_delta ? transformer_delta[i] : 0;
	}	

	_has_transformer = true;

	return true;
}

bool Trainer::setActivity(ITransformer *activity,
	ssi_real_t activity_percentage,
	ssi_size_t activity_frame,
	ssi_size_t activity_delta) {

	return setActivity(1, &activity, &activity_percentage, &activity_frame, &activity_delta);
}

bool Trainer::setActivity(ssi_size_t n_streams,
	ITransformer **activity,
	ssi_real_t *activity_percentage,
	ssi_size_t *activity_frame,
	ssi_size_t *activity_delta) {

	if (_is_trained) {
		ssi_wrn("already trained");
		return false;
	}

	release_activity();

	if (_n_streams > 0 && n_streams != _n_streams) {
		ssi_wrn("#streams do not match (%u != %u)", _n_streams, n_streams);
	}
	else {
		_n_streams = n_streams;
	}

	_activity = new ITransformer *[_n_streams];
	_activity_percentage = new ssi_real_t[n_streams];
	_activity_frame = new ssi_size_t[n_streams];
	_activity_delta = new ssi_size_t[n_streams];
	for (ssi_size_t i = 0; i < _n_streams; i++) {		
		_activity[i] = activity[i];
		_activity_percentage[i] = activity_percentage[i];
		_activity_frame[i] = activity_frame ? activity_frame[i] : 0;
		_activity_delta[i] = activity_delta ? activity_delta[i] : 0;
	}

	_has_activity = true;

	return true;
}

bool Trainer::preproc (ISamples &samples, SampleList &samples_pre) {

	if (samples.getSize () == 0) {
		ssi_wrn ("empty sample list");
		return true;
	}

	if (_has_transformer) {

		ssi_size_t n_streams = samples.getStreamSize ();
		if (n_streams != _n_streams) {
			ssi_wrn ("#streams in samples (%u) must not differ from #transformer (%u)", n_streams, _n_streams);
			return false;
		}		

		ISTransform istransform (&samples);
		for (ssi_size_t i = 0; i < _n_streams; i++) {
			if (_transformer[i]) {
				ssi_msg(SSI_LOG_LEVEL_DETAIL, "apply transformer (%s) to stream#02%u", _transformer[i]->getName(), i);
				istransform.setTransformer (i, *_transformer[i], _transformer_frame[i], _transformer_delta[i]);
			}
		}
		istransform.callEnter ();
		ModelTools::CopySampleList (istransform, samples_pre);
		istransform.callFlush ();

	} else {
		ModelTools::CopySampleList (samples, samples_pre);
	}

	if (_has_activity) {

		SampleList samples_act;

		ssi_size_t n_streams = samples.getStreamSize();
		if (n_streams != _n_streams) {
			ssi_wrn("#streams in samples (%u) must not differ from #activity (%u)", n_streams, _n_streams);
			return false;
		}

		ISTransform istransform(&samples);
		for (ssi_size_t i = 0; i < _n_streams; i++) {
			if (_activity[i]) {
				ssi_msg(SSI_LOG_LEVEL_DETAIL, "apply activity (%s) to stream#02%u", _activity[i]->getName(), i);
				istransform.setTransformer(i, *_activity[i], _activity_frame[i], _activity_delta[i]);
			}
		}
		istransform.callEnter();
		
		ssi_sample_t *sample_act, *sample_pre = 0;		
		samples_pre.reset();
		while ((sample_act = istransform.next()) && (sample_pre = samples_pre.next())) {	
			for (ssi_size_t i = 0; i < _n_streams; i++) {
				if (_activity[i]) {
					ssi_real_t *ptr = ssi_pcast(ssi_real_t, sample_act->streams[i]->ptr);
					ssi_size_t num = sample_act->streams[i]->num;
					ssi_size_t dim = sample_act->streams[i]->dim;
					ssi_size_t count = 0;
					for (ssi_size_t j = 0; j < num; j++) {
						if (*ptr > 0) {
							count++;
						}
						ptr += dim;
					}
					ssi_real_t percentage = ssi_cast(ssi_real_t, count) / ssi_cast(ssi_real_t, num);
					if (percentage < _activity_percentage[i] && sample_pre->streams[i]) {
						ssi_stream_destroy(*sample_pre->streams[i]);
						sample_pre->streams[i] = 0;
					}
				}
			}
			
		}

		istransform.callFlush();
	}

	return true;
}

bool Trainer::setPreprocMode (bool toggle,
	ssi_size_t n_streams,
	ssi_stream_t stream_refs[]) {

	if (toggle) {

		if (_n_streams > 0 && _n_streams != n_streams) {
			ssi_wrn("#streams do not match (%u != %u)", _n_streams, n_streams);
			return false;
		}
		
		if (_stream_refs) {
			for (ssi_size_t i = 0; i < _n_streams; i++) {
				if (stream_refs[i].byte != _stream_refs[i].byte) {
					ssi_wrn ("#bytes '%u' in stream #%u does not match '%u'", stream_refs[i].byte, i, _stream_refs[i].byte);
					return false;
				}
				if (stream_refs[i].dim != _stream_refs[i].dim) {
					ssi_wrn ("#dim '%u' in stream #%u does not match '%u'", stream_refs[i].dim, i, _stream_refs[i].dim);
					return false;
				}				
			}
		} else {
			_n_streams = n_streams;
			_stream_refs = new ssi_stream_t[_n_streams];
			for (ssi_size_t n_stream = 0; n_stream < _n_streams; n_stream++) {
				_stream_refs[n_stream] = stream_refs[n_stream];
				_stream_refs[n_stream].ptr = 0;
				ssi_stream_reset (_stream_refs[n_stream]);
			}
		}

		_preproc_mode = true;
	} else {
		_preproc_mode = false;
	}

	return true;
}

bool Trainer::setNormalization(ISNorm::Params *norm) {
	
	setNormalization(1, &norm);

	return true;
}

bool Trainer::setNormalization(ssi_size_t n_streams,
	ISNorm::Params **norm) {

	if (_n_streams > 0 && _n_streams != n_streams) {
		ssi_wrn("#streams do not match (%u != %u)", _n_streams, _n_streams);
	} else {
		_n_streams = n_streams;
	}

	release_normalization();

	_has_normalization = true;
	_normalization = new ISNorm::Params *[n_streams];
	for (ssi_size_t i = 0; i < n_streams; i++) {
		_normalization[i] = norm[i];
	}

	return true;
}

bool Trainer::train () {

	if (_n_samplepaths == 0) {
		ssi_wrn ("no paths to samples found");
		return false;
	}

	SampleList samples;
	for (ssi_size_t i = 0; i < _n_samplepaths; i++) {
		ModelTools::LoadSampleList (samples, _samplepaths[i]);
	}

	return train (samples);
}

bool Trainer::train (ISamples &samples) {	
	
	if (_is_trained)
	{
		ssi_msg(SSI_LOG_LEVEL_BASIC, "start re-training");
	}
	else
	{
		ssi_msg(SSI_LOG_LEVEL_BASIC, "start training");
	}

	if (_n_models == 0) {
		ssi_wrn ("no models");
		return false;
	}

	if (samples.getSize () == 0) {
		ssi_wrn ("empty sample list");
		return false;
	}

	ISamples *samples_ptr = &samples;
	SampleList *samples_transf = 0;
	ISSelectDim *samples_select = 0;	
	ISNorm *samples_norm = 0;

	if (!_preproc_mode) {

		_n_streams = samples.getStreamSize ();
		_stream_refs = new ssi_stream_t[_n_streams];
		for (ssi_size_t n_stream = 0; n_stream < _n_streams; n_stream++) {
			_stream_refs[n_stream] = samples.getStream (n_stream);
			_stream_refs[n_stream].ptr = 0;
			ssi_stream_reset (_stream_refs[n_stream]);
		}

		if (_has_transformer) {
			samples_transf = new SampleList ();
			preproc (samples, *samples_transf);
			samples_ptr = samples_transf;
		}

	}
	
	if (_has_selection) {

		ssi_size_t n_streams = samples_ptr->getStreamSize ();	
		if (_n_streams != n_streams) {
			ssi_wrn ("#streams in samples (%u) must not differ from #streams in selection (%u)", n_streams, _n_streams);
			return false;
		}

		samples_select = new ISSelectDim (samples_ptr);
		for (ssi_size_t i = 0; i < _n_streams; i++) {
			if (_n_stream_select[i] > 0) {
				ssi_msg(SSI_LOG_LEVEL_DETAIL, "apply selection (%u) to stream#02%u", _n_stream_select[i], i);
				samples_select->setSelection (i, _n_stream_select[i], _stream_select[i], false);
			}
		}

		samples_ptr = samples_select;

	}

	if (_has_normalization) {

		samples_norm = new ISNorm(samples_ptr);
		for (ssi_size_t i = 0; i < _n_streams; i++) {
			samples_norm->setNorm(i, *_normalization[i]);
		}
		samples_ptr = samples_norm;
	}

	//ModelTools::SaveSampleList(*samples_ptr, "check", File::BINARY);

	bool result = train_h (*samples_ptr);
		
	delete samples_transf;
	delete samples_select;
	delete samples_norm;

	_preventWarningsSpam = false;

	return result;
}

void Trainer::eval(ISamples &strain, ISamples &sdevel, FILE *file, Evaluation::PRINT::List format) {

	train(strain);
	eval(sdevel, file, format);
}

void Trainer::eval(ISamples &samples, FILE *file, Evaluation::PRINT::List format) {

	if (!_is_trained)
	{
		ssi_wrn("not trained");
		return;
	}

	Evaluation eval;
	eval.eval(this, samples, samples.getClassSize() == 1 ? IModel::TASK::REGRESSION : IModel::TASK::CLASSIFICATION);
	eval.print(file, format);
}

void Trainer::evalSplit(ISamples &samples, ssi_real_t split, FILE *file, Evaluation::PRINT::List format) {

	Evaluation eval;
	eval.evalSplit (this, samples, split, samples.getClassSize() == 1 ? IModel::TASK::REGRESSION : IModel::TASK::CLASSIFICATION);
	eval.print(file, format);
}

void Trainer::evalKFold(ISamples &samples, ssi_size_t k, FILE *file, Evaluation::PRINT::List format) {

	Evaluation eval;
	eval.evalKFold (this, samples, k, samples.getClassSize() == 1 ? IModel::TASK::REGRESSION : IModel::TASK::CLASSIFICATION);
	eval.print(file, format);
}

void Trainer::evalLOO(ISamples &samples, FILE *file, Evaluation::PRINT::List format) {

	Evaluation eval;
	eval.evalLOO (this, samples, samples.getClassSize() == 1 ? IModel::TASK::REGRESSION : IModel::TASK::CLASSIFICATION);
	eval.print(file, format);
}

void Trainer::evalLOUO(ISamples &samples, FILE *file, Evaluation::PRINT::List format) {

	Evaluation eval;
	eval.evalLOUO (this, samples, samples.getClassSize() == 1 ? IModel::TASK::REGRESSION : IModel::TASK::CLASSIFICATION);
	eval.print(file, format);
}

void Trainer::evalSplit(ssi_real_t split, FILE *file, Evaluation::PRINT::List format) {

	if (!_is_trained)
	{
		ssi_wrn("not trained");
		return;
	}

	SampleList samples;
	for (ssi_size_t i = 0; i < _n_samplepaths; i++) {
		ModelTools::LoadSampleList (samples, _samplepaths[i]);
	}
	evalSplit(samples, split, file, format);
}

void Trainer::eval(FILE *file, Evaluation::PRINT::List format) {

	SampleList samples;
	for (ssi_size_t i = 0; i < _n_samplepaths; i++) {
		ModelTools::LoadSampleList(samples, _samplepaths[i]);
	}
	eval(samples, file, format);
}

void Trainer::evalKFold(ssi_size_t k, FILE *file, Evaluation::PRINT::List format) {

	SampleList samples;
	for (ssi_size_t i = 0; i < _n_samplepaths; i++) {
		ModelTools::LoadSampleList (samples, _samplepaths[i]);
	}
	evalKFold(samples, k, file, format);
}

void Trainer::evalLOO(FILE *file, Evaluation::PRINT::List format) {

	SampleList samples;
	for (ssi_size_t i = 0; i < _n_samplepaths; i++) {
		ModelTools::LoadSampleList (samples, _samplepaths[i]);
	}
	evalLOO(samples, file, format);
}

void Trainer::evalLOUO(FILE *file, Evaluation::PRINT::List format) {

	SampleList samples;
	for (ssi_size_t i = 0; i < _n_samplepaths; i++) {
		ModelTools::LoadSampleList (samples, _samplepaths[i]);
	}
	evalLOUO(samples, file, format);
}

bool Trainer::train_h (ISamples &samples_raw) {	

	bool result = false;

	ISamples *samples = &samples_raw;
	if (_balance != BALANCE::NONE)
	{

#if SSI_RANDOM_LEGACY_FLAG	
		ssi_random_seed(_seed);
#endif

		switch (_balance)
		{
		case BALANCE::UNDER:
		{			
			ISUnderSample *under = new ISUnderSample(&samples_raw);
			under->setSeed(_seed);
			under->setUnder(ISUnderSample::Strategy::RANDOM);			
			samples = under;
			ssi_msg(SSI_LOG_LEVEL_BASIC, "apply under sampling (%u->%u)", samples_raw.getSize(), samples->getSize());
			break;
		}
		case BALANCE::OVER:
		{
			ssi_msg(SSI_LOG_LEVEL_BASIC, "apply over sampling (%u->%u)", samples_raw.getSize(), samples->getSize());
			ISOverSample *over = new ISOverSample(&samples_raw);
			over->setSeed(_seed);
			over->setOver(ISOverSample::Strategy::RANDOM);
			samples = over;
			break;
		}
		}
	}

	if (_fusion) {
		ssi_msg (SSI_LOG_LEVEL_BASIC, "train '%s' using %u streams", _fusion->getName (), samples->getStreamSize ());
		if (ssi_log_level >= SSI_LOG_LEVEL_BASIC) {
			for (ssi_size_t nstream = 0; nstream < samples->getStreamSize (); nstream++) {				 
				ssi_print ("             stream#%02u %ux%u %s\n", nstream, samples->getSize (), samples->getStream (nstream).dim, SSI_TYPE_NAMES[samples->getStream (nstream).type]);
			}	
			for (ssi_size_t nmodel = 0; nmodel < _n_models; nmodel++) {				 
				ssi_print ("             model#%02u '%s'\n", nmodel, _models[nmodel]->getName ());
			}			
		}
		result = _fusion->train(_n_models, _models, *samples);
	} else {
		ssi_msg (SSI_LOG_LEVEL_BASIC, "train '%s' using stream#%02u %ux%u %s", _models[0]->getName (), _stream_index, samples->getSize (), samples->getStream (_stream_index).dim, SSI_TYPE_NAMES[samples->getStream (_stream_index).type]);
		if (samples->hasMissingData ()) {
			ISMissingData samples_md (samples);
			samples_md.setStream (_stream_index);
			result = _models[0]->train(samples_md, _stream_index);
		} else {
			result = _models[0]->train(*samples, _stream_index);
		}
	}
	
	init_class_names (*samples);	
	init_user_names (*samples);
	_is_trained = result;

	if (samples != &samples_raw)
	{
		delete samples;
	}

	return result;
}

// test sample
bool Trainer::forward(ssi_stream_t &stream,
	ssi_size_t &class_index,
	ssi_real_t &class_prob)
{
	ssi_real_t *probs = new ssi_real_t[_n_classes];
	if (!forward_probs(stream, _n_classes, probs))
	{
		return false;
	}

	ssi_size_t max_ind = 0;
	ssi_real_t max_val = probs[0];
	for (ssi_size_t i = 1; i < _n_classes; i++) {
		if (probs[i] > max_val) {
			max_val = probs[i];
			max_ind = i;
		}
	}

	class_index = max_ind;
	class_prob = probs[max_ind];

	delete[] probs;

	return true;
}

bool Trainer::forward (ssi_stream_t &stream, 
	ssi_size_t &class_index) {	

	ssi_stream_t *s = &stream;
	return forward (1, &s, class_index);
}

bool Trainer::forward (ssi_size_t num,
	ssi_stream_t **streams,
	ssi_size_t &class_index) {

	ssi_real_t *probs = new ssi_real_t[_n_classes];
	if (!forward_probs (num, streams, _n_classes, probs)) {
		delete[] probs;
		return false;
	}

	ssi_size_t max_ind = 0;
	ssi_real_t max_val = probs[0];
	for (ssi_size_t i = 1; i < _n_classes; i++) {
		if (probs[i] > max_val) {
			max_val = probs[i];
			max_ind = i;
		}
	}
	
	delete[] probs;
	class_index = max_ind;

	return true;
}

bool Trainer::forward_probs (ssi_stream_t &stream,
	ssi_size_t class_num,
	ssi_real_t *class_probs) {

	ssi_stream_t *s = &stream;
	return forward_probs (1, &s, class_num, class_probs);
}

bool Trainer::forward_probs (ssi_size_t n_streams,
	ssi_stream_t **streams,
	ssi_size_t n_probs,
	ssi_real_t *probs) {

	if (!_is_trained) {
		if (!_preventWarningsSpam)
		{
			ssi_wrn("prediction failed because model is not trained");
			_preventWarningsSpam = true;
		}
		return false;
	}

	if (n_streams != _n_streams) {
		if (!_preventWarningsSpam)
		{
			ssi_wrn("prediction failed because #streams not compatible (%u != %u)", n_streams, _n_streams);
			_preventWarningsSpam = true;
		}		
		return false;
	}

	if (!_fusion && streams[_stream_index]->num == 0) {
		return false;
	}

	if (!_preproc_mode) {
		for (ssi_size_t n_stream = 0; n_stream < n_streams; n_stream++) {		
			if (streams[n_stream] && !ssi_stream_compare (*streams[n_stream], _stream_refs[n_stream])) {
				if (!_preventWarningsSpam)
				{
					ssi_wrn("prediction failed because stream #%u not compatible", n_stream);
					ssi_print("received stream:\n");
					ssi_stream_info(*streams[n_stream], ssiout);
					ssi_print("expected stream:\n");
					ssi_stream_info(_stream_refs[n_stream], ssiout);
					_preventWarningsSpam = true;
				}				
				return false;			
			}
		}
	}	

	ssi_stream_t **streams_ptr = new ssi_stream_t *[n_streams];
	ssi_stream_t **streams_o = new ssi_stream_t *[n_streams];
	ssi_stream_t **streams_s = new ssi_stream_t *[n_streams];
	ssi_stream_t **streams_t = new ssi_stream_t *[n_streams];
	ssi_stream_t **streams_n = new ssi_stream_t *[n_streams];
	for (ssi_size_t i = 0; i < n_streams; i++) {
		streams_o[i] = new ssi_stream_t;
		ssi_stream_copy(*streams[i], *streams_o[i], 0, streams[i]->num);
		streams_ptr[i] = streams_o[i];
		streams_s[i] = 0;
		streams_t[i] = 0;
		streams_n[i] = 0;
	}

	if (!_preproc_mode && _has_transformer) {
		for (ssi_size_t i = 0; i < _n_streams; i++) {
			if (streams_ptr[i] && _transformer[i]) {
				streams_t[i] = new ssi_stream_t;
				SignalTools::Transform (*streams_ptr[i], *streams_t[i], *_transformer[i], _transformer_frame[i], _transformer_delta[i]);
				streams_ptr[i] = streams_t[i];
			}	
		}
	}

	if (_has_selection) {
		for (ssi_size_t i = 0; i < _n_streams; i++) {
			if (streams_ptr[i] && _stream_select[i]) {
				streams_s[i] = new ssi_stream_t;
				ssi_stream_select (*streams_ptr[i], *streams_s[i], _n_stream_select[i], _stream_select[i]);
				streams_ptr[i] = streams_s[i];
			}	
		}
	} 

	if (_has_normalization) {
		for (ssi_size_t i = 0; i < _n_streams; i++) {
			if (streams_ptr[i] && _normalization[i] && _normalization[i]->method != ISNorm::METHOD::NONE) {
				streams_n[i] = new ssi_stream_t;
				ssi_stream_clone(*streams_ptr[i], *streams_n[i]);
				ISNorm::Norm(*streams_n[i], *_normalization[i]);
				streams_ptr[i] = streams_n[i];
			}
		}
	}

	if (_has_activity) {
		for (ssi_size_t i = 0; i < _n_streams; i++) {
			if (streams_ptr[i] && _activity[i]) {

				ssi_stream_t activity;
				SignalTools::Transform(*streams[i], activity, *_activity[i], _activity_frame[i], _activity_delta[i]);
				
				ssi_real_t *ptr = ssi_pcast(ssi_real_t, activity.ptr);
				ssi_size_t num = activity.num;
				ssi_size_t dim = activity.dim;
				ssi_size_t count = 0;
				for (ssi_size_t j = 0; j < num; j++) {
					if (*ptr > 0) {
						count++;
					}
					ptr += dim;
				}
				ssi_real_t percentage = ssi_cast(ssi_real_t, count) / ssi_cast(ssi_real_t, num);
				if (percentage < _activity_percentage[i] && streams_ptr[i]->ptr) {
					ssi_stream_destroy(*streams_ptr[i]);
					streams_ptr[i] = 0;
				}

			}
		}
	}

	bool result = false;
	if (_fusion) {
		result = _fusion->forward (_n_models, _models, n_streams, streams_ptr, n_probs, probs);		
	} else {

		/*ssi_stream_t *strm = streams_ptr[_stream_index];
		static FILE *fp = 0;
		if (!fp) {
			fp = fopen("check.txt", "w");
		}
		ssi_real_t *ptr = ssi_pcast(ssi_real_t, strm->ptr);
		for (ssi_size_t i = 0; i < strm->dim; i++) {
			ssi_fprint(fp, "%.2f ", *ptr++);
		}
		ssi_fprint(fp, "\n", *ptr++);*/

		if (streams_ptr[_stream_index]) {
			result = _models[0]->forward(*streams_ptr[_stream_index], n_probs, probs);
		}
	}

	for (ssi_size_t i = 0; i < n_streams; i++) {
		if (streams_o[i]) {
			ssi_stream_destroy(*streams_o[i]);
			delete streams_o[i];
		}
		if (streams_s[i]) {
			ssi_stream_destroy (*streams_s[i]);
			delete streams_s[i];
		}
		if (streams_t[i]) {
			ssi_stream_destroy (*streams_t[i]);
			delete streams_t[i];
		}
		if (streams_n[i]) {
			ssi_stream_destroy(*streams_n[i]);
			delete streams_n[i];
		}
	}
	delete[] streams_ptr;
	delete[] streams_o;
	delete[] streams_s;
	delete[] streams_t;
	delete[] streams_n;
	
	return result;
}

bool Trainer::Load (Trainer &trainer,
	const ssi_char_t *filepath) {

	FilePath fp (filepath);
	ssi_char_t *filepath_with_ext = 0;
	if (strcmp (fp.getExtension (), SSI_FILE_TYPE_TRAINER) != 0) {
		filepath_with_ext = ssi_strcat (filepath, SSI_FILE_TYPE_TRAINER);
	} else {
		filepath_with_ext = ssi_strcpy (filepath);
	}

	ssi_msg (SSI_LOG_LEVEL_BASIC, "load trainer from file '%s'", filepath_with_ext);

	TiXmlDocument doc;
	if (!doc.LoadFile (filepath_with_ext)) {
		ssi_wrn("failed loading trainer from file '%s' (r:%d,c:%d)", filepath_with_ext, doc.ErrorRow(), doc.ErrorCol());
		return false;
	}

	TiXmlElement *body = doc.FirstChildElement();	
	if (!body || strcmp (body->Value (), "trainer") != 0) {
		ssi_wrn ("tag <trainer> missing");
		return false;	
	}

	VERSION version;
	if (body->QueryIntAttribute ("ssi-v", ssi_pcast (int, &version)) != TIXML_SUCCESS) {
		ssi_wrn ("attribute <ssi-v> in tag <trainer> missing");
		return false;	
	}

	bool result = false;

	trainer.release ();

	switch (version) {
		case V1:
		case V2:
		case V3:
			result = Load_V1to3 (trainer, fp, body, version);
			trainer._is_trained = result;
			break;
		case V4:
			result = Load_V4 (trainer, fp, body);
			trainer._is_trained = result;
			break;
		case V5:
			result = Load_V5 (trainer, fp, body);	
			if (!result)
			{
				trainer._is_trained = false;
			}
			break;
		default:
			ssi_wrn ("unkown version %d", version);
			return false;
	}

	delete[] filepath_with_ext;

	return result;

}

bool Trainer::GetChildSize (TiXmlElement *mama, const ssi_char_t *value, ssi_size_t &n_childs) {

	n_childs = 0;

	TiXmlElement *element = mama->FirstChildElement (value);
	if (!element || strcmp (element->Value (), value) != 0) {
		ssi_wrn ("tag <%s> missing", value);
		return false;
	}

	do {
		n_childs++;			
		element = element->NextSiblingElement (value);
	} while (element);

	return true;
}

void Trainer::setSamples (const char *path) {

	release_samples ();

	_n_samplepaths = 1;
	_samplepaths = new ssi_char_t *[1];
	_samplepaths[0] = ssi_strcpy (path);
}

void Trainer::setSamples (ssi_size_t n, const char **paths) {

	release_samples ();

	_n_samplepaths = n;
	_samplepaths = new ssi_char_t *[_n_samplepaths];
	for (ssi_size_t i = 0; i < _n_samplepaths; i++) {
		_samplepaths[i] = ssi_strcpy (paths[i]);
	}
}

bool Trainer::Load_V5 (Trainer &trainer,
	FilePath &fp,
	TiXmlElement *body) {

	ssi_char_t string[SSI_MAX_CHAR];

	TiXmlElement *element = 0;
	TiXmlElement *item = 0;

	element = body->FirstChildElement ("info");
	if (!element || strcmp (element->Value (), "info") != 0) {
		ssi_wrn ("tag <info> missing");
		return false;
	}
	const ssi_char_t *trained = element->Attribute ("trained");
	bool is_trained;
	if (!trained) {
		ssi_wrn ("attribute <trained> missing in <info>");
		return false;
	} else {
		is_trained = ssi_strcmp (trained, "true", false);
	}
	trainer._is_trained = is_trained;		
	int seed = 0;
	if (element->Attribute("seed", &seed))
	{
		trainer.setSeed(ssi_size_t(seed));
	}

	element = body->FirstChildElement("register");	
	if (element)
	{
		trainer._registerNode = element->Clone();
		Factory::RegisterXML(element, ssiout, ssimsg);
	}

	element = body->FirstChildElement("meta");
	if (element)
	{		
		TiXmlAttribute *meta_attribute = element->FirstAttribute();
		do {
			trainer.Meta[meta_attribute->Name()] = meta_attribute->Value();
		} while (meta_attribute = meta_attribute->Next());		
	}

	if (!is_trained) {

		element = body->FirstChildElement ("samples");
		if (element)
		{
			int n_streams;
			if (!element->Attribute("n_streams", &n_streams)) {
				ssi_wrn("attribute <n_streams> missig in <samples>");
				return false;
			}
			trainer._n_streams = ssi_cast(ssi_size_t, n_streams);

			const ssi_char_t *balance = element->Attribute("balance");
			if (balance && !ssi_strcmp(balance, "none", false)) {
				if (ssi_strcmp(balance, "over", false))
				{
					trainer.setBalance(BALANCE::OVER);
				}
				else if (ssi_strcmp(balance, "under", false))
				{
					trainer.setBalance(BALANCE::UNDER);
				}
				else
				{
					ssi_wrn("unkown balance value '%s'", balance);
				}
			}

			ssi_size_t n_samples = 0;
			if (!GetChildSize(element, "item", n_samples)) {
				return false;
			}
			ssi_char_t const **samples = new ssi_char_t const *[n_samples];
			item = element->FirstChildElement("item");
			for (ssi_size_t n_sample = 0; n_sample < n_samples; n_sample++) {
				if (!item || strcmp(item->Value(), "item") != 0) {
					ssi_wrn("could not parse tag <item> in <samples>");
					return false;
				}
				const char *path = item->Attribute("path");
				if (!path) {
					ssi_wrn("attribute <path> missing in <item>");
					return false;
				}
				samples[n_sample] = path;
				item = item->NextSiblingElement("item");
			}

			trainer.setSamples(n_samples, samples);
			delete[] samples;
		}
		else
		{
			trainer._n_streams = 1;
		}

	} else {
	
		element = body->FirstChildElement ("streams");
		if (!element || strcmp (element->Value (), "streams") != 0) {
			ssi_wrn ("tag <streams> missing");
			return false;
		}
		ssi_size_t n_streams = 0;	
		if (!GetChildSize (element, "item", n_streams)) {		
			return false;	
		}	
		item = element->FirstChildElement ("item");
		trainer._n_streams = n_streams; 
		trainer._stream_refs = new ssi_stream_t[n_streams];
		item = element->FirstChildElement ();
		for (ssi_size_t n_stream = 0; n_stream < n_streams; n_stream++) {
			if (!item || strcmp (item->Value (), "item") != 0) {
				ssi_wrn ("could not parse tag <item> in <streams>");
				return false;
			}
			int byte = 0;
			if (!item->Attribute ("byte", &byte)) {
				ssi_wrn ("attribute <byte> missing in <item>");
				return false;
			}
			int dim = 0;
			if (!item->Attribute ("dim", &dim)) {
				ssi_wrn ("attribute <dim> missing in <item>");
				return false;
			}
			double sr = 0;
			if (!item->Attribute ("sr", &sr)) {
				ssi_wrn ("attribute <sr> missing in <item>");
				return false;
			}
			const char *type_name = item->Attribute ("type");
			if (!type_name) {
				ssi_wrn ("attribute <type> missing in <item>");
				return false;
			}
			ssi_type_t type = SSI_UNDEF;
			if (!ssi_name2type (type_name, type)) {
				ssi_wrn ("found unkown <type> in <item>");
				return false;
			}
			ssi_stream_init (trainer._stream_refs[n_stream], 0, dim, byte, type, sr);		
			item = item->NextSiblingElement("item");
		}
	
		element = body->FirstChildElement ("classes");
		if (!element || strcmp (element->Value (), "classes") != 0) {
			ssi_wrn ("tag <element> missing");
			return false;
		}

		ssi_size_t n_classes = 0;	
		if (!GetChildSize (element, "item", n_classes)) {		
			return false;	
		}	
		trainer._n_classes = n_classes; 
		trainer._class_names = new ssi_char_t *[n_classes];	  
		item = element->FirstChildElement ("item");
		for (ssi_size_t n_class = 0; n_class < n_classes; n_class++) {
			if (!item || strcmp (item->Value (), "item") != 0) {
				ssi_wrn ("could not parse tag <item> in <classes>");
				return false;
			}
			const char *name = item->Attribute ("name");
			if (!name) {
				ssi_wrn ("attribute <name> missing in <classes>");
				return false;
			}
			trainer._class_names[n_class] = ssi_strcpy (name);
			item = item->NextSiblingElement("item");
		}

		element = body->FirstChildElement ("users");
		if (!element || strcmp (element->Value (), "users") != 0) {
			ssi_wrn ("tag <users> missing");
			return false;
		}
		ssi_size_t n_users = 0;	
		if (!GetChildSize (element, "item", n_users)) {		
			return false;	
		}	
		trainer._n_users = n_users; 
		trainer._user_names = new ssi_char_t *[n_users];	  
		item = element->FirstChildElement ("item");
		for (ssi_size_t n_user = 0; n_user < n_users; n_user++) {
			if (!item || strcmp (item->Value (), "item") != 0) {
				ssi_wrn ("could not parse tag <item> in <users>");
				return false;
			}
			const char *name = item->Attribute ("name");
			if (!name) {
				ssi_wrn ("attribute <name> missing in <users>");
				return false;
			}
			trainer._user_names[n_user] = ssi_strcpy (name);
			item = item->NextSiblingElement("item");
		}	
	}

	if (element = body->FirstChildElement("activity")) {

		trainer._has_activity = true;
		trainer._activity = new ITransformer *[trainer._n_streams];
		trainer._activity_frame = new ssi_size_t[trainer._n_streams];
		trainer._activity_delta = new ssi_size_t[trainer._n_streams];
		trainer._activity_percentage = new ssi_real_t[trainer._n_streams];
		for (ssi_size_t i = 0; i < trainer._n_streams; i++) {
			trainer._activity[i] = 0;
			trainer._activity_percentage[i] = 0.5f;
			trainer._activity_frame[i] = 0;
			trainer._activity_delta[i] = 0;
		}

		item = element->FirstChildElement("item");
		if (item) {
			do {

				int stream_id;
				if (!item->Attribute("stream", &stream_id)) {
					ssi_wrn("attribute <stream> missing in <item>");
					return false;
				}
				const ssi_char_t *create = item->Attribute("create");
				if (!create) {
					ssi_wrn("attribute <create> missing in <item>");
					return false;
				}
				const ssi_char_t *option = item->Attribute("option");

				IObject *activity = 0;
				if (option) {
					ssi_sprint(string, "%s%s", fp.getDir(), option);
					activity = Factory::Create(create, string, true);
				}
				else {
					activity = Factory::Create(create, 0, true);
				}

				if (!activity) {
					ssi_wrn("could not create instance '%s'", activity->getName());
					return false;
				}
				trainer._activity[stream_id] = ssi_pcast(ITransformer, activity);

				double percentage = 0;
				if (item->Attribute("percentage", &percentage)) {
					trainer._activity_percentage[stream_id] = ssi_cast(ssi_real_t, percentage);
				}

				int frame = 0, delta = 0;
				if (item->Attribute("frame", &frame)) {
					trainer._activity_frame[stream_id] = ssi_cast(ssi_size_t, frame);
				}
				if (item->Attribute("delta", &delta)) {
					trainer._activity_delta[stream_id] = ssi_cast(ssi_size_t, delta);
				}

			} while (item = item->NextSiblingElement("item"));
		}
	}

	if (element = body->FirstChildElement ("transform")) {
		
		trainer._has_transformer = true;
		trainer._transformer = new ITransformer *[trainer._n_streams];
		trainer._transformer_frame = new ssi_size_t[trainer._n_streams];
		trainer._transformer_delta = new ssi_size_t[trainer._n_streams];
		for (ssi_size_t i = 0; i < trainer._n_streams; i++) {
			trainer._transformer[i] = 0;
			trainer._transformer_frame[i] = 0;
			trainer._transformer_delta[i] = 0;
		}

		item = element->FirstChildElement ("item");
		if (item) {
			do {
				
				int stream_id;
				if (!item->Attribute ("stream", &stream_id)) {
					ssi_wrn ("attribute <stream> missing in <item>");
					return false;
				}
				const ssi_char_t *create = item->Attribute ("create");
				if (!create) {
					ssi_wrn ("attribute <create> missing in <item>");
					return false;
				}
				const ssi_char_t *option = item->Attribute ("option");

				IObject *transformer = 0;
				if (option) {
					ssi_sprint (string, "%s%s", fp.getDir (), option);
					transformer = Factory::Create(create, string, true);
				} else {
					transformer = Factory::Create(create, 0, true);
				}

				if (!transformer) {
					ssi_wrn ("could not create instance '%s'", transformer->getName());
					return false;
				}				
				trainer._transformer[stream_id] = ssi_pcast (ITransformer, transformer);
				
				int frame = 0, delta = 0;
				if (item->Attribute ("frame", &frame)) {
					trainer._transformer_frame[stream_id] = ssi_cast (ssi_size_t, frame);
				}				
				if (item->Attribute ("delta", &delta)) {
					trainer._transformer_delta[stream_id] = ssi_cast (ssi_size_t, delta);
				}				
				
			} while (item = item->NextSiblingElement ("item"));			
		}
	}

	if (element = body->FirstChildElement ("select")) {

		trainer._has_selection = true;
		trainer._n_stream_select = new ssi_size_t[trainer._n_streams];
		trainer._stream_select = new ssi_size_t *[trainer._n_streams];
		for (ssi_size_t i = 0; i < trainer._n_streams; i++) {
			trainer._n_stream_select[i] = 0;
			trainer._stream_select[i] = 0;
		}

		item = element->FirstChildElement ("item");
		if (item) {
			do {
				
				int stream_id;
				if (!item->Attribute ("stream", &stream_id)) {
					ssi_wrn ("attribute <stream> missing in <item>");
					return false;
				}
				if (ssi_cast (ssi_size_t, stream_id) >= trainer._n_streams) {
					ssi_wrn ("stream id (%d) exceeds #streams (%u)", stream_id, trainer._n_streams);
					return false;
				}
				/*int n_select;
				if (!item->Attribute ("size", &n_select)) {
					ssi_wrn ("attribute <size> missing in <item>");
					return false;
				}*/
				const ssi_char_t *select = item->Attribute ("select");
				if (!select) {
					ssi_wrn ("attribute <select> missing in <item>");
					return false;
				}
				ssi_size_t n_select;
				int *indices = ssi_parse_indices(select, n_select);
				trainer._n_stream_select[stream_id] = n_select;
				trainer._stream_select[stream_id] = new ssi_size_t[n_select];
				for (ssi_size_t i = 0; i < n_select; i++) {
					trainer._stream_select[stream_id][i] = indices[i];
				}
				delete[] indices;
				//ssi_string2array (trainer._n_stream_select[stream_id], trainer._stream_select[stream_id], select, ' ');
				

			} while (item = item->NextSiblingElement ("item"));			
		}
	}

	if (element = body->FirstChildElement("normalize")) {

		trainer._has_normalization = true;
		trainer._normalization_free = true;
		trainer._normalization = new ISNorm::Params *[trainer._n_streams];		
		for (ssi_size_t i = 0; i < trainer._n_streams; i++) {
			trainer._normalization[i] = 0;
		}

		item = element->FirstChildElement("item");
		if (item) {
			do {

				int stream_id;
				if (!item->Attribute("stream", &stream_id)) {
					ssi_wrn("attribute <stream> missing in <item>");
					return false;
				}
				if (ssi_cast(ssi_size_t, stream_id) >= trainer._n_streams) {
					ssi_wrn("stream id (%d) exceeds #streams (%u)", stream_id, trainer._n_streams);
					return false;
				}
				const ssi_char_t *method = item->Attribute("method");
				if (!method) {
					ssi_wrn("attribute <method> missing in <item>");
					return false;
				}
				bool found_method = false;
				
				for (ssi_size_t i = 0; i < ISNorm::METHOD::NUM; i++) {
					if (ssi_strcmp(method, ISNorm::METHOD_NAMES[i])) {
						trainer._normalization[stream_id] = new ISNorm::Params;
						ISNorm::ZeroParams(*trainer._normalization[stream_id], ISNorm::METHOD::List(i));
						const ssi_char_t *limits = item->Attribute("limits");
						if (limits)
						{
							ssi_string2array(2, trainer._normalization[stream_id]->limits, limits, ',');						
						}
						found_method = true;
	
						break;
					}
				}
				if (!found_method) {
					ssi_wrn("unkown normalization method '%s'", method);
					return false;
				}
				if (is_trained) {
					const ssi_char_t *path = item->Attribute("path");
					if (!path) {
						ssi_wrn("attribute <path> missing in <item>");
						return false;
					}
					ssi_sprint(string, "%s%s", fp.getDir(), path);
					ISNorm::LoadParams(string, *trainer._normalization[stream_id]);
				}

				if (trainer._normalization[stream_id]->method == ISNorm::METHOD::SCALE)
				{
					ssi_msg(SSI_LOG_LEVEL_BASIC, "apply normalization '%s[%.g,%.g]' on stream #%u",
						ISNorm::METHOD_NAMES[trainer._normalization[stream_id]->method],
						trainer._normalization[stream_id]->limits[0],
						trainer._normalization[stream_id]->limits[1],
						stream_id);
				}
				else
				{
					ssi_msg(SSI_LOG_LEVEL_BASIC, "apply normalization '%s' on stream #%u",
						ISNorm::METHOD_NAMES[trainer._normalization[stream_id]->method],
						stream_id);
				}

			} while (item = item->NextSiblingElement("item"));
		}
	}

	if (element = body->FirstChildElement ("model")) {

		bool create_model = false;
		if (!trainer._models) {
			trainer._n_models = 1; 
			trainer._models = new IModel *[1];
			create_model = true;
		} else if (trainer._n_models != 1) {
			ssi_wrn ("expected %u models but found only one", trainer._n_models);
			return false;
		}
		const char *create = element->Attribute("create");
		if (!create) {
			ssi_wrn ("attribute <create> missing in <model>");
			return false;
		}
		int stream_index;
		if (element->QueryIntAttribute ("stream", &stream_index) != TIXML_SUCCESS) {
			ssi_wrn ("attribute <stream> in tag <model> missing");
			return false;	
		}
		trainer._stream_index = stream_index;
		if (create_model) {
			const char *optpath = element->Attribute ("option");				
			if (optpath) {
				ssi_sprint (string, "%s%s", fp.getDir (), optpath);
				trainer._models[0] = ssi_pcast(IModel, Factory::Create(create, string, false));
			} else {
				trainer._models[0] = ssi_pcast(IModel, Factory::Create(create, 0, false));
			}
		}
		else if (strcmp(trainer._models[0]->getName(), create) != 0) {
			ssi_wrn("expected model '%s' but found '%s'", trainer._models[0]->getName(), create);
			return false;
		}		
		const char *path = element->Attribute ("path");
		if (path) {			
			ssi_sprint (string, "%s%s%s", fp.getDir (), path, SSI_FILE_TYPE_MODEL); 
			if (!trainer._models[0]->load (string)) {
				ssi_wrn ("failed loading model from '%s'", string);
				return false;
			}
		}

	} else if (element = body->FirstChildElement ("fusion")) {

		const char *create = element->Attribute("create");
		if (!create) {
			ssi_wrn ("attribute <create> missing in <fusion>");
			return false;
		}
		if (!trainer._fusion) {
			const char *optpath = element->Attribute ("option");				
			if (optpath) {
				ssi_sprint (string, "%s%s", fp.getDir (), optpath);
				trainer._fusion = ssi_pcast(IFusion, Factory::Create(create, string, false));
			} else {
				trainer._fusion = ssi_pcast(IFusion, Factory::Create(create, 0, false));
			}
		}
		else if (strcmp(trainer._fusion->getName(), create) != 0) {
			ssi_wrn("expected fusion '%s' but found '%s'", trainer._fusion->getName(), create);
			return false;
		}
		const char *path = element->Attribute ("path");
		if (path) {			
			ssi_sprint (string, "%s%s%s", fp.getDir (), path, SSI_FILE_TYPE_FUSION); 
			if (!trainer._fusion->load (string)) {
				ssi_wrn ("failed loading fusion from '%s'", string);
				return false;
			}
		}

		element = element->FirstChildElement ("models");
		if (!element || strcmp (element->Value (), "models") != 0) {
			ssi_wrn ("tag <models> missing in <fusion>");
			return false;
		}

		ssi_size_t n_models = 0;	
		if (!GetChildSize (element, "item", n_models)) {		
			return false;	
		}

		bool create_models = false;
		if (!trainer._models) {
			trainer._n_models = n_models; 
			trainer._models = new IModel *[n_models];
			create_models = true;
		} else if (n_models != trainer._n_models) {
			ssi_wrn ("expected %u models but found %u", trainer._n_models, n_models);			
			return false;
		}
		item = element->FirstChildElement ("item");
		for (ssi_size_t n_model = 0; n_model < n_models; n_model++) {

			const char *create = item->Attribute("create");
			if (!create) {
				ssi_wrn ("attribute <create> missing in <models>");
				return false;
			}
			if (create_models) {
				const char *optpath = item->Attribute ("option");				
				if (optpath) {
					ssi_sprint (string, "%s%s", fp.getDir (), optpath);
					trainer._models[n_model] = ssi_pcast(IModel, Factory::Create(create, string, false));
				} else {
					trainer._models[n_model] = ssi_pcast(IModel, Factory::Create(create, 0, false));
				}
			}
			else if (strcmp(trainer._models[n_model]->getName(), create) != 0) {
				ssi_wrn("expected model '%s' but found '%s'", trainer._models[n_model]->getName(), create);
				return false;
			}
			const char *path = item->Attribute ("path");
			if (path) {				
				ssi_sprint (string, "%s%s%s", fp.getDir (), path, SSI_FILE_TYPE_MODEL); 
				if (!trainer._models[n_model]->load (string)) {
					ssi_wrn ("failed loading model #%u from '%s'", n_model, string);
					return false;
				}
			}

			item = item->NextSiblingElement("item");
		}

	} else {
		ssi_wrn ("tag <fusion> or <model> missing");
		return false;
	}

	return true;
}

bool Trainer::Load_V4 (Trainer &trainer,
	FilePath &fp,
	TiXmlElement *body) {

	ssi_char_t string[SSI_MAX_CHAR];

	TiXmlElement *element = body->FirstChildElement ("classes");
	if (!element || strcmp (element->Value (), "classes") != 0) {
		ssi_wrn ("tag <element> missing");
		return false;
	}

	int n_classes;
	if (element->QueryIntAttribute ("size", &n_classes) != TIXML_SUCCESS) {
		ssi_wrn ("attribute <size> in tag <classes> missing");
		return false;	
	}	
	trainer._n_classes = n_classes; 
	trainer._class_names = new ssi_char_t *[n_classes];	  
	TiXmlElement *item = element->FirstChildElement ("item");
	for (int n_class = 0; n_class < n_classes; n_class++) {
		if (!item || strcmp (item->Value (), "item") != 0) {
			ssi_wrn ("could not parse tag <item> in <classes>");
			return false;
		}
		const char *name = item->Attribute ("name");
		if (!name) {
			ssi_wrn ("attribute <name> missing in <classes>");
			return false;
		}
		trainer._class_names[n_class] = ssi_strcpy (name);
		item = item->NextSiblingElement("item");
	}

	element = body->FirstChildElement ("users");
	if (!element || strcmp (element->Value (), "users") != 0) {
		ssi_wrn ("tag <users> missing");
		return false;
	}
	int n_users;
	if (element->QueryIntAttribute ("size", &n_users) != TIXML_SUCCESS) {
		ssi_wrn ("attribute <size> in tag <users> missing");
		return false;	
	}
	trainer._n_users = n_users; 
	trainer._user_names = new ssi_char_t *[n_users];	  
	item = element->FirstChildElement ("item");
	for (int n_user = 0; n_user < n_users; n_user++) {
		if (!item || strcmp (item->Value (), "item") != 0) {
			ssi_wrn ("could not parse tag <item> in <users>");
			return false;
		}
		const char *name = item->Attribute ("name");
		if (!name) {
			ssi_wrn ("attribute <name> missing in <users>");
			return false;
		}
		trainer._user_names[n_user] = ssi_strcpy (name);
		item = item->NextSiblingElement("item");
	}

	element = body->FirstChildElement ("streams");
	if (!element || strcmp (element->Value (), "streams") != 0) {
		ssi_wrn ("tag <streams> missing");
		return false;
	}
	int n_streams;
	if (element->QueryIntAttribute ("size", &n_streams) != TIXML_SUCCESS) {
		ssi_wrn ("attribute <size> in tag <streams> missing");
		return false;	
	}	
	item = element->FirstChildElement ("item");
	trainer._n_streams = n_streams; 
	trainer._stream_refs = new ssi_stream_t[n_streams];
	item = element->FirstChildElement ();
	for (int n_stream = 0; n_stream < n_streams; n_stream++) {
		if (!item || strcmp (item->Value (), "item") != 0) {
			ssi_wrn ("could not parse tag <item> in <streams>");
			return false;
		}
		int byte = 0;
		if (!item->Attribute ("byte", &byte)) {
			ssi_wrn ("attribute <byte> missing in <item>");
			return false;
		}
		int dim = 0;
		if (!item->Attribute ("dim", &dim)) {
			ssi_wrn ("attribute <dim> missing in <item>");
			return false;
		}
		double sr = 0;
		if (!item->Attribute ("sr", &sr)) {
			ssi_wrn ("attribute <sr> missing in <item>");
			return false;
		}
		const char *type_name = item->Attribute ("type");
		if (!type_name) {
			ssi_wrn ("attribute <type> missing in <item>");
			return false;
		}
		ssi_type_t type = SSI_UNDEF;
		if (!ssi_name2type (type_name, type)) {
			ssi_wrn ("found unkown <type> in <item>");
			return false;
		}
		ssi_stream_init (trainer._stream_refs[n_stream], 0, dim, byte, type, sr);		
		item = item->NextSiblingElement("item");
	}

	if (element = body->FirstChildElement ("transform")) {
		
		trainer._has_transformer = true;
		trainer._transformer = new ITransformer *[trainer._n_streams];
		trainer._transformer_frame = new ssi_size_t[trainer._n_streams];
		trainer._transformer_delta = new ssi_size_t[trainer._n_streams];
		for (ssi_size_t i = 0; i < trainer._n_streams; i++) {
			trainer._transformer[i] = 0;
			trainer._transformer_frame[i] = 0;
			trainer._transformer_delta[i] = 0;
		}

		item = element->FirstChildElement ("item");
		if (item) {
			do {
				
				int stream_id;
				if (!item->Attribute ("stream", &stream_id)) {
					ssi_wrn ("attribute <stream> missing in <item>");
					return false;
				}
				const ssi_char_t *name = item->Attribute ("name");
				if (!name) {
					ssi_wrn ("attribute <name> missing in <item>");
					return false;
				}
				const ssi_char_t *option = item->Attribute ("option");
				if (!option) {
					ssi_wrn ("attribute <option> missing in <item>");
					return false;
				}

				IObject *transformer = 0;
				if (option[0] != '\0') {
					ssi_sprint (string, "%s%s", fp.getDir (), option);
					transformer = Factory::Create (name, string, true);
				} else {
					transformer = Factory::Create (name, 0, true);
				}

				if (!transformer) {
					ssi_wrn ("could not create instance '%s'", transformer->getName());
					return false;
				}				
				trainer._transformer[stream_id] = ssi_pcast (ITransformer, transformer);
				
				int frame = 0, delta = 0;
				if (item->Attribute ("frame", &frame)) {
					trainer._transformer_frame[stream_id] = ssi_cast (ssi_size_t, frame);
				}				
				if (item->Attribute ("delta", &delta)) {
					trainer._transformer_frame[stream_id] = ssi_cast (ssi_size_t, delta);
				}				
				
			} while (item = item->NextSiblingElement ("item"));			
		}
	}

	if (element = body->FirstChildElement ("select")) {

		trainer._has_selection = true;
		trainer._n_stream_select = new ssi_size_t[trainer._n_streams];
		trainer._stream_select = new ssi_size_t *[trainer._n_streams];
		for (ssi_size_t i = 0; i < trainer._n_streams; i++) {
			trainer._n_stream_select[i] = 0;
			trainer._stream_select[i] = 0;
		}

		item = element->FirstChildElement ("item");
		if (item) {
			do {
				
				int stream_id;
				if (!item->Attribute ("stream", &stream_id)) {
					ssi_wrn ("attribute <stream> missing in <item>");
					return false;
				}
				if (ssi_cast (ssi_size_t, stream_id) >= trainer._n_streams) {
					ssi_wrn ("stream id (%d) exceeds #streams (%u)", stream_id, trainer._n_streams);
					return false;
				}
				int n_select;
				if (!item->Attribute ("size", &n_select)) {
					ssi_wrn ("attribute <size> missing in <item>");
					return false;
				}				
				const ssi_char_t *select = item->Attribute ("select");
				if (!select) {
					ssi_wrn ("attribute <select> missing in <item>");
					return false;
				}

				trainer._n_stream_select[stream_id] = n_select;
				trainer._stream_select[stream_id] = new ssi_size_t[n_select];
				ssi_string2array (trainer._n_stream_select[stream_id], trainer._stream_select[stream_id], select, ' ');

			} while (item = item->NextSiblingElement ("item"));			
		}
	}

	if (element = body->FirstChildElement ("model")) {

		bool create_model = false;
		if (!trainer._models) {
			trainer._n_models = 1; 
			trainer._models = new IModel *[1];
			create_model = true;
		} else if (trainer._n_models != 1) {
			ssi_wrn ("expected %u models but found only one", trainer._n_models);
			return false;
		}
		const char *name = element->Attribute ("name");
		if (!name) {
			ssi_wrn ("attribute <name> missing in <model>");
			return false;
		}
		int stream_index;
		if (element->QueryIntAttribute ("stream", &stream_index) != TIXML_SUCCESS) {
			ssi_wrn ("attribute <stream> in tag <model> missing");
			return false;	
		}
		trainer._stream_index = stream_index;
		if (create_model) {
			trainer._models[0] = ssi_pcast (IModel, Factory::Create (name, 0, false));
		} else if (strcmp (trainer._models[0]->getName (), name) != 0) {
			ssi_wrn ("expected model '%s' but found '%s'", trainer._models[0]->getName (), name);
			return false;
		}		
		const char *path = element->Attribute ("path");
		if (!path) {
			ssi_wrn ("attribute <path> missing in <model>");
			return false;
		}
		ssi_sprint (string, "%s%s", fp.getDir (), path); 
		if (!trainer._models[0]->load (string)) {
			ssi_wrn ("failed loading model from '%s'", string);
			return false;
		}

	} else if (element = body->FirstChildElement ("fusion")) {

		const char *name = element->Attribute ("name");
		if (!name) {
			ssi_wrn ("attribute <name> missing in <fusion>");
			return false;
		}
		if (!trainer._fusion) {
			trainer._fusion = ssi_pcast (IFusion, Factory::Create (name, 0, false));		
		} else if (strcmp (trainer._fusion->getName (), name) != 0) {
			ssi_wrn ("expected fusion '%s' but found '%s'", trainer._fusion->getName (), name);
			return false;
		}
		const char *path = element->Attribute ("path");
		if (!path) {
			ssi_wrn ("attribute <path> missing in <fusion>");
			return false;
		}
		ssi_sprint (string, "%s%s", fp.getDir (), path); 
		if (!trainer._fusion->load (string)) {
			ssi_wrn ("failed loading fusion from '%s'", string);
			return false;
		}

		element = element->FirstChildElement ("models");
		if (!element || strcmp (element->Value (), "models") != 0) {
			ssi_wrn ("tag <models> missing in <fusion>");
			return false;
		}

		int n_models;
		if (element->QueryIntAttribute ("size", &n_models) != TIXML_SUCCESS) {
			ssi_wrn ("attribute <size> in tag <models> missing");
			return false;	
		}
		bool create_models = false;
		if (!trainer._models) {
			trainer._n_models = n_models; 
			trainer._models = new IModel *[n_models];
			create_models = true;
		} else if (n_models != trainer._n_models) {
			ssi_wrn ("expected %u models but found %u", trainer._n_models, n_models);			
			return false;
		}
		item = element->FirstChildElement ("item");
		for (int n_model = 0; n_model < n_models; n_model++) {

			const char *name = item->Attribute ("name");
			if (!name) {
				ssi_wrn ("attribute <name> missing in <models>");
				return false;
			}
			if (create_models) {
				trainer._models[n_model] = ssi_pcast (IModel, Factory::Create (name, 0, false));		
			} else if (strcmp (trainer._models[n_model]->getName (), name) != 0) {
				ssi_wrn ("expected model '%s' but found '%s'", trainer._models[n_model]->getName (), name);
				return false;
			}
			const char *path = item->Attribute ("path");
			if (!path) {
				ssi_wrn ("attribute <path> missing in <models>");
				return false;
			}

			ssi_sprint (string, "%s%s", fp.getDir (), path); 
			if (!trainer._models[n_model]->load (string)) {
				ssi_wrn ("failed loading model #%u from '%s'", n_model, string);
				return false;
			}

			item = item->NextSiblingElement("item");
		}

	} else {
		ssi_wrn ("tag <fusion> or <model> missing");
		return false;
	}

	return true;
}

bool Trainer::Load_V1to3 (Trainer &trainer,
	FilePath &fp,
	TiXmlElement *body,
	VERSION version) {

	ssi_char_t string[SSI_MAX_CHAR];

	TiXmlElement *element = body->FirstChildElement ();
	if (!element || strcmp (element->Value (), "classes") != 0) {
		ssi_wrn ("tag <element> missing");
		return false;
	}

	int n_classes;
	if (element->QueryIntAttribute ("size", &n_classes) != TIXML_SUCCESS) {
		ssi_wrn ("attribute <size> in tag <classes> missing");
		return false;	
	}
	TiXmlElement *item = element->FirstChildElement ();
	trainer._n_classes = n_classes; 
	trainer._class_names = new ssi_char_t *[n_classes];	  
	item = element->FirstChildElement ();
	for (int n_class = 0; n_class < n_classes; n_class++) {
		if (!item || strcmp (item->Value (), "item") != 0) {
			ssi_wrn ("could not parse tag <item> in <classes>");
			return false;
		}
		const char *name = item->Attribute ("name");
		if (!name) {
			ssi_wrn ("attribute <name> missing in <classes>");
			return false;
		}
		trainer._class_names[n_class] = ssi_strcpy (name);
		item = item->NextSiblingElement();
	}

	if (version > V1) {

		element = element->NextSiblingElement ();
		if (!element || strcmp (element->Value (), "users") != 0) {
			ssi_wrn ("tag <users> missing");
			return false;
		}

		int n_users;
		if (element->QueryIntAttribute ("size", &n_users) != TIXML_SUCCESS) {
			ssi_wrn ("attribute <size> in tag <users> missing");
			return false;	
		}
		TiXmlElement *item = element->FirstChildElement ();
		trainer._n_users = n_users; 
		trainer._user_names = new ssi_char_t *[n_users];	  
		item = element->FirstChildElement ();
		for (int n_user = 0; n_user < n_users; n_user++) {
			if (!item || strcmp (item->Value (), "item") != 0) {
				ssi_wrn ("could not parse tag <item> in <users>");
				return false;
			}
			const char *name = item->Attribute ("name");
			if (!name) {
				ssi_wrn ("attribute <name> missing in <users>");
				return false;
			}
			trainer._user_names[n_user] = ssi_strcpy (name);
			item = item->NextSiblingElement();
		}
	}

	element = element->NextSiblingElement ();
	if (!element || strcmp (element->Value (), "streams") != 0) {
		ssi_wrn ("tag <streams> missing");
		return false;
	}

	int n_streams;
	if (element->QueryIntAttribute ("size", &n_streams) != TIXML_SUCCESS) {
		ssi_wrn ("attribute <size> in tag <streams> missing");
		return false;	
	}
	if (version > Trainer::V2) {
		const ssi_char_t *select = element->Attribute ("select");
		if (!select) {
			ssi_wrn ("attribute <select> missing in <streams>");
			return false;
		}
		if (strcmp (select, "true") == 0) {
			trainer._has_selection = true;
			trainer._n_stream_select = new ssi_size_t[n_streams];
			trainer._stream_select = new ssi_size_t *[n_streams];
		}
	}
	item = element->FirstChildElement ();
	trainer._n_streams = n_streams; 
	trainer._stream_refs = new ssi_stream_t[n_streams];
	item = element->FirstChildElement ();
	for (int n_stream = 0; n_stream < n_streams; n_stream++) {
		if (!item || strcmp (item->Value (), "item") != 0) {
			ssi_wrn ("could not parse tag <item> in <streams>");
			return false;
		}
		int byte = 0;
		if (!item->Attribute ("byte", &byte)) {
			ssi_wrn ("attribute <byte> missing in <item>");
			return false;
		}
		int dim = 0;
		if (!item->Attribute ("dim", &dim)) {
			ssi_wrn ("attribute <dim> missing in <item>");
			return false;
		}
		double sr = 0;
		if (!item->Attribute ("sr", &sr)) {
			ssi_wrn ("attribute <sr> missing in <item>");
			return false;
		}
		const char *type_name = item->Attribute ("type");
		if (!type_name) {
			ssi_wrn ("attribute <type> missing in <item>");
			return false;
		}
		ssi_type_t type = SSI_UNDEF;
		if (!ssi_name2type (type_name, type)) {
			ssi_wrn ("found unkown <type> in <item>");
			return false;
		}
		ssi_stream_init (trainer._stream_refs[n_stream], 0, dim, byte, type, sr);
		if (version > Trainer::V2 && trainer._has_selection) {
			const ssi_char_t *select = item->Attribute ("select");
			if (!select) {
				ssi_wrn ("attribute <select> missing in <item>");
				return false;
			}
			if (select[0] == '\0') {
				trainer._n_stream_select[n_stream] = 0;
				trainer._stream_select[n_stream] = 0;
			} else {
				trainer._n_stream_select[n_stream] = dim;
				trainer._stream_select[n_stream] = new ssi_size_t[dim];
				ssi_string2array (trainer._n_stream_select[n_stream], trainer._stream_select[n_stream], select, ' ');
			}
		}
		item = item->NextSiblingElement();
	}

	element = element->NextSiblingElement ();
	if (!element) {
		ssi_wrn ("tag <fusion> or <model> missing");
		return false;
	}

	if (strcmp (element->Value (), "fusion") == 0) {
	
		const char *name = element->Attribute ("name");
		if (!name) {
			ssi_wrn ("attribute <name> missing in <fusion>");
			return false;
		}
		if (!trainer._fusion) {
			trainer._fusion = ssi_pcast (IFusion, Factory::Create (name, 0, false));		
		} else if (strcmp (trainer._fusion->getName (), name) != 0) {
			ssi_wrn ("expected fusion '%s' but found '%s'", trainer._fusion->getName (), name);
			return false;
		}
		const char *path = element->Attribute ("path");
		if (!path) {
			ssi_wrn ("attribute <path> missing in <fusion>");
			return false;
		}
		if (version > Trainer::V2) {
			ssi_sprint (string, "%s%s", fp.getDir (), path); 
			if (!trainer._fusion->load (string)) {
				ssi_wrn ("failed loading fusion from '%s'", string);
				return false;
			}
		} else {
			if (!trainer._fusion->load (path)) {
				ssi_wrn ("failed loading fusion from '%s'", path);
				return false;
			}
		}

		element = element->FirstChildElement ();
		if (!element || strcmp (element->Value (), "models") != 0) {
			ssi_wrn ("tag <models> missing in <fusion>");
			return false;
		}

		int n_models;
		if (element->QueryIntAttribute ("size", &n_models) != TIXML_SUCCESS) {
			ssi_wrn ("attribute <size> in tag <models> missing");
			return false;	
		}
		bool create_models = false;
		if (!trainer._models) {
			trainer._n_models = n_models; 
			trainer._models = new IModel *[n_models];
			create_models = true;
		} else if (n_models != trainer._n_models) {
			ssi_wrn ("expected %u models but found %u", trainer._n_models, n_models);			
			return false;
		}
		item = element->FirstChildElement ();
		for (int n_model = 0; n_model < n_models; n_model++) {

			const char *name = item->Attribute ("name");
			if (!name) {
				ssi_wrn ("attribute <name> missing in <models>");
				return false;
			}
			if (create_models) {
				trainer._models[n_model] = ssi_pcast (IModel, Factory::Create (name, 0, false));		
			} else if (strcmp (trainer._models[n_model]->getName (), name) != 0) {
				ssi_wrn ("expected model '%s' but found '%s'", trainer._models[n_model]->getName (), name);
				return false;
			}
			const char *path = item->Attribute ("path");
			if (!path) {
				ssi_wrn ("attribute <path> missing in <models>");
				return false;
			}
			if (version > Trainer::V2) {
				ssi_sprint (string, "%s%s", fp.getDir (), path); 
				if (!trainer._models[n_model]->load (string)) {
					ssi_wrn ("failed loading model #%u from '%s'", n_model, string);
					return false;
				}
			} else {
				if (!trainer._models[n_model]->load (path)) {
					ssi_wrn ("failed loading model #%u from '%s'", n_model, path);
					return false;
				}
			}

			item = item->NextSiblingElement();
		}

	} else if (strcmp (element->Value (), "model") == 0) {

		bool create_model = false;
		if (!trainer._models) {
			trainer._n_models = 1; 
			trainer._models = new IModel *[1];
			create_model = true;
		} else if (trainer._n_models != 1) {
			ssi_wrn ("expected %u models but found only one", trainer._n_models);
			return false;
		}
		const char *name = element->Attribute ("name");
		if (!name) {
			ssi_wrn ("attribute <name> missing in <model>");
			return false;
		}
		int stream_index;
		if (element->QueryIntAttribute ("stream", &stream_index) != TIXML_SUCCESS) {
			ssi_wrn ("attribute <stream> in tag <model> missing");
			return false;	
		}
		trainer._stream_index = stream_index;
		if (create_model) {
			trainer._models[0] = ssi_pcast (IModel, Factory::Create (name, 0, false));
		} else if (strcmp (trainer._models[0]->getName (), name) != 0) {
			ssi_wrn ("expected model '%s' but found '%s'", trainer._models[0]->getName (), name);
			return false;
		}		
		const char *path = element->Attribute ("path");
		if (!path) {
			ssi_wrn ("attribute <path> missing in <model>");
			return false;
		}
		if (version > Trainer::V2) {
			ssi_sprint (string, "%s%s", fp.getDir (), path); 
			if (!trainer._models[0]->load (string)) {
				ssi_wrn ("failed loading model from '%s'", string);
				return false;
			}
		} else {
			if (!trainer._models[0]->load (path)) {
				ssi_wrn ("failed loading model from '%s'", path);
				return false;
			}
		}

	} else {
		ssi_wrn ("tag <fusion> or <model> missing");
		return false;
	}

	return true;

};

bool Trainer::save(const ssi_char_t *filepath, VERSION version, File::TYPE type) {

	if (version != V5 && !_is_trained) {
		ssi_wrn ("not trained");
		return false;
	}

	ssi_char_t *filepath_with_ext = 0;
	FilePath fp(filepath);
	if (ssi_strcmp(fp.getExtension(), SSI_FILE_TYPE_TRAINER, false)) {
		filepath_with_ext = ssi_strcpy(filepath);
	}
	else {
		filepath_with_ext = ssi_strcat(filepath, SSI_FILE_TYPE_TRAINER);
	}
	
	TiXmlDocument doc;

	TiXmlDeclaration head ("1.0", "", "");
	doc.InsertEndChild (head);
	
	TiXmlElement body ("trainer");		
	body.SetAttribute ("ssi-v", version);

	switch (version) {
		case V1:
		case V2:
		case V3:
			if (!save_V1to3 (filepath, body, version)) {
				ssi_wrn ("saving trainer to file '%s' failed", filepath_with_ext);	
				return false;
			}
			break;
		case V4:
			if (!save_V4 (filepath, body)) {
				ssi_wrn ("saving trainer to file '%s' failed", filepath_with_ext);	
				return false;
			}
			break;
		case V5:
			if (!save_V5 (filepath, body, type)) {
				ssi_wrn ("saving trainer to file '%s' failed", filepath_with_ext);	
				return false;
			}
			break;
		default:
			ssi_wrn ("unkown version %d", version);
			return false;
	}

	doc.InsertEndChild (body);
	
	if (!doc.SaveFile (filepath_with_ext)) {
		ssi_wrn ("saving trainer to file '%s' failed", filepath_with_ext);	
		return false;
	}

	ssi_msg (SSI_LOG_LEVEL_BASIC, "saved trainer to file '%s'", filepath_with_ext);
	delete[] filepath_with_ext;

	return true;
}

bool Trainer::save_V5(const ssi_char_t *filepath, TiXmlElement &body, File::TYPE type) {

	ssi_char_t string[SSI_MAX_CHAR];

	TiXmlElement info("info");
	info.SetAttribute("trained", _is_trained ? "true" : "false");
	body.InsertEndChild(info);
	
	if (Meta.size() > 0)
	{
		TiXmlElement meta("meta");
		for (std::map<String, String>::iterator it = Meta.begin(); it != Meta.end(); it++) {
			meta.SetAttribute(it->first.str(), it->second.str());
		}		
		body.InsertEndChild(meta);
	}

	if (_registerNode)
	{ 
		body.InsertEndChild(*_registerNode);
	}

	if (!_is_trained) {
		if (_n_streams == 0 && _n_samplepaths > 0) {
			SampleList samples;
			ModelTools::LoadSampleList(samples, _samplepaths[0]);
			_n_streams = samples.getStreamSize();
		}
		TiXmlElement samples("samples");
		samples.SetAttribute("n_streams", ssi_cast(int, _n_streams));
		for (ssi_size_t n_samples = 0; n_samples < _n_samplepaths; n_samples++) {
			TiXmlElement item("item");
			item.SetAttribute("path", _samplepaths[n_samples]);
			samples.InsertEndChild(item);
		}
		body.InsertEndChild(samples);
	}
	else {

		TiXmlElement streams("streams");
		for (ssi_size_t n_stream = 0; n_stream < _n_streams; n_stream++) {
			TiXmlElement item("item");
			item.SetAttribute("byte", ssi_cast(int, _stream_refs[n_stream].byte));
			item.SetAttribute("dim", ssi_cast(int, _stream_refs[n_stream].dim));
			item.SetDoubleAttribute("sr", _stream_refs[n_stream].sr);
			item.SetAttribute("type", SSI_TYPE_NAMES[_stream_refs[n_stream].type]);
			streams.InsertEndChild(item);
		}
		body.InsertEndChild(streams);

		TiXmlElement classes("classes");
		for (ssi_size_t n_class = 0; n_class < _n_classes; n_class++) {
			TiXmlElement item("item");
			item.SetAttribute("name", _class_names[n_class]);
			classes.InsertEndChild(item);
		}
		body.InsertEndChild(classes);

		TiXmlElement users("users");
		for (ssi_size_t n_user = 0; n_user < _n_users; n_user++) {
			TiXmlElement item("item");
			item.SetAttribute("name", _user_names[n_user]);
			users.InsertEndChild(item);
		}
		body.InsertEndChild(users);
	}

	if (_has_activity) {
		TiXmlElement transform("activity");
		for (ssi_size_t n = 0; n < _n_streams; n++) {
			if (_activity[n]) {
				TiXmlElement item("item");
				item.SetAttribute("create", _activity[n]->getName());
				item.SetAttribute("stream", n);
				item.SetDoubleAttribute("percentage", _activity_percentage[n]);
				item.SetDoubleAttribute("frame", _activity_frame[n]);
				item.SetDoubleAttribute("delta", _activity_delta[n]);
				if (_activity[n]->getOptions()) {

					ssi_sprint(string, "%s%s.#%u.%s", filepath, SSI_FILE_TYPE_TRAINER, n, _activity[n]->getName());
					OptionList::SaveXML(string, _activity[n]->getOptions());
					FilePath fp(filepath);
					ssi_sprint(string, "%s%s.#%u.%s", fp.getName(), SSI_FILE_TYPE_TRAINER, n, _activity[n]->getName());
					item.SetAttribute("option", string);

				}
				transform.InsertEndChild(item);
			}
		}
		body.InsertEndChild(transform);
	}


	if (_has_transformer) {
		TiXmlElement transform("transform");
		for (ssi_size_t n = 0; n < _n_streams; n++) {
			if (_transformer[n]) {
				TiXmlElement item("item");
				item.SetAttribute("create", _transformer[n]->getName());
				item.SetAttribute("stream", n);
				item.SetDoubleAttribute("frame", _transformer_frame[n]);
				item.SetDoubleAttribute("delta", _transformer_delta[n]);
				if (_transformer[n]->getOptions()) {
					ssi_sprint(string, "%s%s.#%u.%s", filepath, SSI_FILE_TYPE_TRAINER, n, _transformer[n]->getName());

					OptionList::SaveXML(string, _transformer[n]->getOptions());
					FilePath fp(filepath);
					ssi_sprint(string, "%s%s.#%u.%s", fp.getName(), SSI_FILE_TYPE_TRAINER, n, _transformer[n]->getName());
					item.SetAttribute("option", string);

				}
				transform.InsertEndChild(item);
			}
		}
		body.InsertEndChild(transform);
	}

	if (_has_selection) {
		TiXmlElement select("select");
		for (ssi_size_t n = 0; n < _n_streams; n++) {
			if (_stream_select[n]) {
				TiXmlElement item("item");
				item.SetAttribute("stream", ssi_cast(int, n));
				item.SetAttribute("size", ssi_cast(int, _n_stream_select[n]));
				ssi_array2string(_n_stream_select[n], _stream_select[n], SSI_MAX_CHAR, string, ' ');
				item.SetAttribute("select", string);
				select.InsertEndChild(item);
			}
		}
		body.InsertEndChild(select);
	}

	if (_has_normalization) {
		TiXmlElement normalize("normalize");
		for (ssi_size_t n = 0; n < _n_streams; n++) {
			if (_normalization[n] && _normalization[n]->method != ISNorm::METHOD::NONE) {
				TiXmlElement item("item");
				item.SetAttribute("method", ISNorm::METHOD_NAMES[_normalization[n]->method]);
				item.SetAttribute("stream", n);
				if (_is_trained) {
					ssi_sprint(string, "%s%s.#%u", filepath, SSI_FILE_TYPE_TRAINER, n);
					if (!ISNorm::SaveParams(string, *_normalization[n], type)) {
						ssi_wrn("failed saving normalization for stream#%u to '%s'", n, string);
						return false;
					}
					FilePath fp(filepath);
					ssi_sprint(string, "%s%s.#%u", fp.getNameFull(), SSI_FILE_TYPE_TRAINER, n);
					item.SetAttribute("path", string);
				}
				normalize.InsertEndChild(item);
			}
		}
		body.InsertEndChild(normalize);
	}

	if (_fusion) {

		TiXmlElement fusion("fusion");
		fusion.SetAttribute("create", _fusion->getName());
		ssi_sprint(string, "%s%s.%s%s", filepath, SSI_FILE_TYPE_TRAINER, _fusion->getName(), SSI_FILE_TYPE_FUSION);
		FilePath fp(string);
		fusion.SetAttribute("path", fp.getName());
		if (!_fusion->save(string)) {
			ssi_wrn("failed saving fusion to '%s'", string);
			return false;
		}
		if (_fusion->getOptions()) {

			FilePath fp(string);
			OptionList::SaveXML(string, _fusion->getOptions());
			fusion.SetAttribute("option", fp.getName());

		}

		TiXmlElement models("models");
		for (ssi_size_t n_model = 0; n_model < _n_models; n_model++) {
			TiXmlElement item("item");
			item.SetAttribute("create", _models[n_model]->getName());
			if (_models[n_model]->isTrained()) {
				ssi_sprint(string, "%s%s.#%u.%s%s", filepath, SSI_FILE_TYPE_TRAINER, n_model, _models[n_model]->getName(), SSI_FILE_TYPE_MODEL);
				FilePath fp(string);
				item.SetAttribute("path", fp.getName());
				if (!_models[n_model]->save(string)) {
					ssi_wrn("failed saving model #%u to '%s'", n_model, string);
					return false;
				}
			}
			if (_models[n_model]->getOptions()) {

				ssi_sprint(string, "%s%s.#%u.%s", filepath, SSI_FILE_TYPE_TRAINER, n_model, _models[n_model]->getName());
				OptionList::SaveXML(string, _models[n_model]->getOptions());
				FilePath fp(filepath);
				ssi_sprint(string, "%s%s.#%u.%s", fp.getName(), SSI_FILE_TYPE_TRAINER, n_model, _models[n_model]->getName());
				item.SetAttribute("option", string);

			}
			models.InsertEndChild(item);
		}
		fusion.InsertEndChild(models);

		body.InsertEndChild(fusion);

	}
	else {

		TiXmlElement model("model");
		model.SetAttribute("create", _models[0]->getName());
		model.SetAttribute("stream", _stream_index);
		if (_models[0]->isTrained()) {
			ssi_sprint(string, "%s%s.%s%s", filepath, SSI_FILE_TYPE_TRAINER, _models[0]->getName(), SSI_FILE_TYPE_MODEL);
			FilePath fp(string);
			model.SetAttribute("path", fp.getName());
			if (!_models[0]->save(string)) {
				ssi_wrn("failed saving model '%s'", string);
				return false;
			}
		}
		if (_models[0]->getOptions()) {

			ssi_sprint(string, "%s%s.%s", filepath, SSI_FILE_TYPE_TRAINER, _models[0]->getName());
			OptionList::SaveXML(string, _models[0]->getOptions());
			FilePath fp(filepath);
			ssi_sprint(string, "%s%s.%s", fp.getNameFull(), SSI_FILE_TYPE_TRAINER, _models[0]->getName());
			model.SetAttribute("option", string);

		}
		body.InsertEndChild(model);
	}

	return true;
}

bool Trainer::save_V4 (const ssi_char_t *filepath, TiXmlElement &body) {
	
	ssi_char_t string[SSI_MAX_CHAR];

	TiXmlElement classes ("classes" );	
	classes.SetAttribute ("size", ssi_cast (int, _n_classes));
	for (ssi_size_t n_class = 0; n_class < _n_classes; n_class++) {
		TiXmlElement item ("item" );
		item.SetAttribute ("name", _class_names[n_class]);								
		item.SetAttribute ("id", ssi_cast (int, n_class));				
		classes.InsertEndChild (item);
	}
	body.InsertEndChild (classes);
	
	TiXmlElement users ("users" );	
	users.SetAttribute ("size", ssi_cast (int, _n_users));
	for (ssi_size_t n_user = 0; n_user < _n_users; n_user++) {
		TiXmlElement item ("item" );
		item.SetAttribute ("name", _user_names[n_user]);								
		item.SetAttribute ("id", ssi_cast (int, n_user));				
		users.InsertEndChild (item);
	}
	body.InsertEndChild (users);

	TiXmlElement streams ("streams" );	
	streams.SetAttribute ("size", ssi_cast (int, _n_streams));	
	for (ssi_size_t n_stream = 0; n_stream < _n_streams; n_stream++) {
		TiXmlElement item ("item" );
		item.SetAttribute ("byte", ssi_cast (int, _stream_refs[n_stream].byte));							
		item.SetAttribute ("dim", ssi_cast (int, _stream_refs[n_stream].dim));
		item.SetDoubleAttribute ("sr", _stream_refs[n_stream].sr);
		item.SetAttribute ("type", SSI_TYPE_NAMES[_stream_refs[n_stream].type]);		
		streams.InsertEndChild (item);
	}
	body.InsertEndChild (streams);

	if (_has_transformer) {
		TiXmlElement transform ("transform" );
		for (ssi_size_t n = 0; n < _n_streams; n++) {
			if (_transformer[n]) {
				TiXmlElement item ("item");
				item.SetAttribute ("stream", n); 
				item.SetAttribute ("name", _transformer[n]->getName ());							
				item.SetAttribute ("info", _transformer[n]->getInfo ());	
				if (_transformer[n]->getOptions ()) {

					ssi_sprint (string, "%s.%02u.%s%s", filepath, n, _transformer[n]->getName (), SSI_FILE_TYPE_OPTION);
					OptionList::SaveXML (string, _transformer[n]->getOptions ());
					FilePath fp (filepath);
					ssi_sprint (string, "%s.%02u.%s%s", fp.getName (), n, _transformer[n]->getName (), SSI_FILE_TYPE_OPTION);
                    item.SetAttribute ("option", string);

				}
				item.SetDoubleAttribute ("frame", _transformer_frame[n]);
				item.SetDoubleAttribute ("delta", _transformer_delta[n]);
				transform.InsertEndChild (item);
			}
		}
		body.InsertEndChild (transform);
	}
	
	if (_has_selection) {
		TiXmlElement select ("select" );
		for (ssi_size_t n = 0; n < _n_streams; n++) {
			if (_stream_select[n]) {
				TiXmlElement item ("item" );				
				item.SetAttribute ("stream", ssi_cast (int, n)); 
				item.SetAttribute ("size", ssi_cast (int, _n_stream_select[n])); 
				ssi_array2string (_n_stream_select[n], _stream_select[n], SSI_MAX_CHAR, string, ' ');
				item.SetAttribute ("select", string);			
				select.InsertEndChild (item);
			}
		}
		body.InsertEndChild (select);

	}
	
	if (_fusion) {

		TiXmlElement fusion ("fusion" );
		fusion.SetAttribute ("name", _fusion->getName ());
		fusion.SetAttribute ("info", _fusion->getInfo ());

		ssi_sprint (string, "%s%s", filepath, SSI_FILE_TYPE_FUSION);				
		FilePath fp (string);			
		fusion.SetAttribute ("path", fp.getNameFull ());
		if (!_fusion->save (string)) {
			ssi_wrn ("failed saving fusion to '%s'", string);
			return false;
		}

		TiXmlElement models ("models" );
		models.SetAttribute ("size", ssi_cast (int, _n_models));	
		for (ssi_size_t n_model = 0; n_model < _n_models; n_model++) {
			TiXmlElement item ("item" );
			item.SetAttribute ("name", _models[n_model]->getName ());								
			item.SetAttribute ("info", _models[n_model]->getInfo ());
			ssi_sprint (string, "%s.%02d%s%s", filepath, n_model, _models[n_model]->getName (), SSI_FILE_TYPE_MODEL);				
			FilePath fp (string);
			item.SetAttribute ("path", fp.getNameFull ());			
			if (!_models[n_model]->save (string)) {
				ssi_wrn ("failed saving model #%u to '%s'", n_model, string);
				return false;
			}
			models.InsertEndChild (item);
		}
		fusion.InsertEndChild (models);

		body.InsertEndChild (fusion);

	} else {

		TiXmlElement model ("model" );		
		model.SetAttribute ("name", _models[0]->getName ());								
		model.SetAttribute ("info", _models[0]->getInfo ());
		model.SetAttribute ("stream", _stream_index);
		ssi_sprint (string, "%s.%s%s", filepath, _models[0]->getName (), SSI_FILE_TYPE_MODEL);				
		FilePath fp (string);
		model.SetAttribute ("path", fp.getNameFull ());		
		if (!_models[0]->save (string)) {
			ssi_wrn ("failed saving model '%s'", string);
			return false;
		}
		body.InsertEndChild (model);
	}

	return true;
}

bool Trainer::save_V1to3 (const ssi_char_t *filepath, TiXmlElement &body, VERSION version) {	
	
	ssi_char_t string[SSI_MAX_CHAR];

	TiXmlElement classes ("classes" );
	classes.SetAttribute ("size", ssi_cast (int, _n_classes));
	for (ssi_size_t n_class = 0; n_class < _n_classes; n_class++) {
		TiXmlElement item ("item" );
		item.SetAttribute ("name", _class_names[n_class]);								
		item.SetAttribute ("id", ssi_cast (int, n_class));				
		classes.InsertEndChild (item);
	}
	body.InsertEndChild (classes);

	if (version > Trainer::V1) {
		TiXmlElement users ("users" );
		users.SetAttribute ("size", ssi_cast (int, _n_users));
		for (ssi_size_t n_user = 0; n_user < _n_users; n_user++) {
			TiXmlElement item ("item" );
			item.SetAttribute ("name", _user_names[n_user]);								
			item.SetAttribute ("id", ssi_cast (int, n_user));				
			users.InsertEndChild (item);
		}
		body.InsertEndChild (users);
	}

	TiXmlElement streams ("streams" );
	streams.SetAttribute ("size", ssi_cast (int, _n_streams));	
	if (version > Trainer::V2) {		
		streams.SetAttribute ("select", _has_selection ? "true" : "false");	
	}
	for (ssi_size_t n_stream = 0; n_stream < _n_streams; n_stream++) {
		TiXmlElement item ("item" );
		item.SetAttribute ("byte", ssi_cast (int, _stream_refs[n_stream].byte));							
		item.SetAttribute ("dim", ssi_cast (int, _stream_refs[n_stream].dim));
		item.SetDoubleAttribute ("sr", _stream_refs[n_stream].sr);
		item.SetAttribute ("type", SSI_TYPE_NAMES[_stream_refs[n_stream].type]);
		if (version > Trainer::V2 && _has_selection) {				
			ssi_array2string (_n_stream_select[n_stream], _stream_select[n_stream], SSI_MAX_CHAR, string, ' ');
			item.SetAttribute ("select", string);			
		}
		streams.InsertEndChild (item);
	}
	body.InsertEndChild (streams);
	
	if (_fusion) {
		TiXmlElement fusion ("fusion" );
		fusion.SetAttribute ("name", _fusion->getName ());
		fusion.SetAttribute ("info", _fusion->getInfo ());

		ssi_sprint (string, "%s%s", filepath, SSI_FILE_TYPE_FUSION);		
		if (version > Trainer::V2) {
			FilePath fp (string);			
			fusion.SetAttribute ("path", fp.getNameFull ());
		} else {
			fusion.SetAttribute ("path", string);
		}
		if (!_fusion->save (string)) {
			ssi_wrn ("failed saving fusion to '%s'", string);
			return false;
		}

		TiXmlElement models ("models" );
		models.SetAttribute ("size", ssi_cast (int, _n_models));	
		for (ssi_size_t n_model = 0; n_model < _n_models; n_model++) {
			TiXmlElement item ("item" );
			item.SetAttribute ("name", _models[n_model]->getName ());								
			item.SetAttribute ("info", _models[n_model]->getInfo ());
			ssi_sprint (string, "%s.%02d%s", filepath, n_model, SSI_FILE_TYPE_MODEL);	
			if (version > Trainer::V2) {
				FilePath fp (string);
				item.SetAttribute ("path", fp.getNameFull ());
			} else {
				item.SetAttribute ("path", string);
			}
			if (!_models[n_model]->save (string)) {
				ssi_wrn ("failed saving model #%u to '%s'", n_model, string);
				return false;
			}
			models.InsertEndChild (item);
		}
		fusion.InsertEndChild (models);

		body.InsertEndChild (fusion);

	} else {

		TiXmlElement model ("model" );		
		model.SetAttribute ("name", _models[0]->getName ());								
		model.SetAttribute ("info", _models[0]->getInfo ());
		model.SetAttribute ("stream", _stream_index);
		ssi_sprint (string, "%s%s", filepath, SSI_FILE_TYPE_MODEL);		
		if (version > Trainer::V2) {
			FilePath fp (string);
			model.SetAttribute ("path", fp.getNameFull ());
		} else {
			model.SetAttribute ("path", string);
		}
		if (!_models[0]->save (string)) {
			ssi_wrn ("failed saving model '%s'", string);
			return false;
		}
		body.InsertEndChild (model);
	}

	return true;

};

bool Trainer::cluster (ISamples &samples) {

	if (!_is_trained) {
		ssi_wrn ("not trained");
		return false;
	}

	ssi_size_t n_classes = _models[0]->getClassSize();
	if (samples.getClassSize () != n_classes) {
		ssi_wrn ("#classes differs from #cluster");
		return false;
	}

	ssi_sample_t *sample;
	samples.reset ();
	while (sample = samples.next ()) {		
		if (n_classes == 1) // REGRESSION
		{			
			forward_probs(sample->num, sample->streams, 1, &sample->score);
		}
		else // MULTICLASS
		{
			forward(sample->num, sample->streams, sample->class_id);			 
		}
	}

	return true;
}

ssi_size_t Trainer::getModelSize() {
	return _n_models;
}

IModel *Trainer::getModel (ssi_size_t index) {

	if (index < _n_models) {
		return _models[index];
	}
	ssi_wrn ("invalid index");
	return 0;
}

IFusion *Trainer::getFusion() {
	return _fusion;
}

const ssi_char_t *Trainer::getName () {

	if (_fusion) {
		return _fusion->getName ();
	} else {
		return _models[0]->getName ();
	}
}

const ssi_char_t *Trainer::getInfo () {

	if (_fusion) {
		return _fusion->getInfo ();
	} else {
		return _models[0]->getInfo ();
	}
}

ssi_size_t Trainer::getMetaSize () { 

	if (_fusion) {
		return _fusion->getMetaSize ();
	} else {
		return _models[0]->getMetaSize ();
	}
};

bool Trainer::getMetaData (ssi_size_t n_metas,
	ssi_real_t *metas) { 

	if (_fusion) {
		return _fusion->getMetaData (n_metas, metas);
	} else {
		return _models[0]->getMetaData (n_metas, metas);
	}
};

}
