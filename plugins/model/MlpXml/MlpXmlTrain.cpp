// MlpXmlTrain.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/04/27
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

#include "MlpXmlTrain.h"
#include "model/ModelTools.h"
#include "Evaluation.h"
#include "EvaluationCont.h"
#include "ioput/wav/WavTools.h"
#include "ioput/xml/tinyxml.h"
#include "base/Factory.h"
#include "ISTransform.h"
#include "signal/SignalTools.h"

namespace ssi {

ssi_char_t *MlpXmlTrain::ssi_log_name = "mlptrain__";

MlpXmlTrain::MlpXmlTrain (const ssi_char_t *def_file) 
	: _trainer (0),
	_stream_ref (0),
	_n_def_names (0),
	_def_names (0),
	_def_name (0),
	_signal_name (0),
	_signal_type (MlpXmlDef::STREAM),
	_anno_name (0),
	_file (0),
	_transf (0),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

	_def_file = ssi_strcpy (def_file);
}

MlpXmlTrain::~MlpXmlTrain () {

	release ();

	for (ssi_size_t i = 0; i < _n_def_names; i++) {
		delete[] _def_names[i];
	}
	delete[] _def_names;
	delete[] _def_file;
}

void MlpXmlTrain::release () {

	_samples.clear ();
	_paths.clear ();
	delete _trainer;
	_trainer = 0;
	delete _stream_ref;
	_stream_ref = 0;
	delete[] _def_name;
	_def_name = 0;
	delete[] _signal_name;
	_signal_name = 0;
	delete[] _anno_name;
	_anno_name = 0;

	std::vector<ssi_stream_t*>::iterator i = _streams.begin();
	while(i != _streams.end())
	{
		ssi_stream_destroy(**i);
		delete *i;
		*i = 0;
		i = _streams.erase(i);
	} 
	std::vector<Annotation*>::iterator j = _annos.begin();
	while(j != _annos.end())
	{
		delete *j;
		*j = 0;
		j = _annos.erase(j);
	}
}

ssi_char_t *const*MlpXmlTrain::getDefNames (ssi_size_t &n_def_names) {

	if (!_def_names) {
		_def_names = parseDefNames (_def_file, _n_def_names);
	}

	n_def_names = _n_def_names;
	return _def_names;
}

bool MlpXmlTrain::init (const ssi_char_t *def_name,
	const ssi_char_t *signal_name,
	MlpXmlDef::signal_t signal_type,
	const ssi_char_t *anno_name) {

	release ();

	_def_name = ssi_strcpy (def_name);
	_signal_name = ssi_strcpy (signal_name);
	_signal_type = signal_type;
	_anno_name = ssi_strcpy (anno_name);

	_trainer = parseTrainer (_def_file, _def_name);	
	
	return _trainer != 0;
}

bool MlpXmlTrain::collect (const ssi_char_t *dir,
	const ssi_char_t *userfilt, 
	const ssi_char_t *datefilt,
	bool re_extract,
	int mode) {

	if (!_trainer) {
		ssi_wrn ("empty trainer");
		return false;
	}

	StringList dates;
	loadDates (dates, dir, userfilt, datefilt);

	ssi_msg (SSI_LOG_LEVEL_DEFAULT, "loading samples..");
	for (ssi_size_t i = 0; i < dates.size (); i++) {
		if (!collect (dates.get (i), re_extract, mode)) {
			ssi_wrn ("no samples found '%s'", dates.get(i));
		}		
	}
	if (ssi_log_level > SSI_LOG_LEVEL_DEFAULT) {
		_samples.print (stdout);
	}

	return true;
}

bool MlpXmlTrain::collect (const ssi_char_t *dir,		
	bool re_extract,
	int mode) {

	if (!_trainer) {
		ssi_wrn ("empty trainer");
		return false;
	}

	ssi_char_t sample_path[SSI_MAX_CHAR];
	ssi_sprint (sample_path, "%s\\%s.%s.%s%s", dir, _signal_name, _anno_name, _def_name, SSI_FILE_TYPE_SAMPLES);

	if (!extract (dir, re_extract, mode)) {
		ssi_wrn ("feature extraction failed '%s'", sample_path);
		return false;
	};	
	if (ssi_exists (sample_path)) {
		if (!ModelTools::LoadSampleList (_samples, sample_path)) {
			ssi_msg (SSI_LOG_LEVEL_BASIC, "try old format..");
			ModelTools::LoadSampleList (_samples, sample_path, File::BINARY);
		}		
		_paths.add (dir);
	} else {
		ssi_wrn ("samples not found '%s'", sample_path);
		return false;
	}

	return true;
}

bool MlpXmlTrain::loadSignal (const ssi_char_t *dir,
	const ssi_char_t *_signal_name,
	ssi_stream_t &stream) {

	ssi_char_t signal_path[SSI_MAX_CHAR];

	switch (_signal_type) {
		case MlpXmlDef::STREAM:
			ssi_sprint (signal_path, "%s\\%s%s", dir, _signal_name, SSI_FILE_TYPE_STREAM);	
			ssi_msg (SSI_LOG_LEVEL_DEFAULT, "load stream from file '%s'..", signal_path);
			if (!ssi_exists (signal_path)) {
				// try old format
				ssi_sprint (signal_path, "%s\\%s%s", dir, _signal_name, ".data");	
				ssi_msg (SSI_LOG_LEVEL_DEFAULT, "try old format: '%s'..", signal_path);
				if (!ssi_exists (signal_path)) {
					ssi_wrn ("file not found '%s'", signal_path);
					return false;
				}
			} else {
				if (!FileTools::ReadStreamFile (signal_path, stream)) {
					ssi_wrn ("loading stream from file '%s' failed", signal_path);
					return false;
				}
			}			
			break;
		case MlpXmlDef::AUDIO:
			ssi_sprint (signal_path, "%s\\%s%s", dir, _signal_name, SSI_FILE_TYPE_WAV);
			ssi_msg (SSI_LOG_LEVEL_DEFAULT, "load stream from file '%s'..", signal_path);
			if (!ssi_exists (signal_path)) {
				ssi_wrn ("file not found '%s'", signal_path);
				return false;
			}
			WavTools::ReadWavFile (signal_path, stream, true);
			break;
		default:
			ssi_wrn ("stream type not supported '%d'", _signal_type);
			return false;
	}

	return true;
}

bool MlpXmlTrain::extract (const ssi_char_t *dir,
	bool re_extract,
	int mode) {

	if (!_trainer) {
		ssi_wrn ("empty trainer");
		return false;
	}

	ssi_char_t anno_path[SSI_MAX_CHAR];
	ssi_char_t sample_path[SSI_MAX_CHAR];
	ssi_char_t user_name[SSI_MAX_CHAR];

	date2user (dir, user_name);

	// make sure to load _signal_name once to determine stream properties
	if (!_stream_ref) {
		ssi_stream_t stream;
		if (!loadSignal (dir, _signal_name, stream)) {
			return false;
		}
		_stream_ref = new ssi_stream_t;
		*_stream_ref = stream;
		_stream_ref->ptr = 0;
		ssi_stream_reset (*_stream_ref);
		ssi_stream_destroy (stream);
	}	

	// load annotation
	ssi_sprint (anno_path, "%s\\%s%s", dir, _anno_name, SSI_FILE_TYPE_ANNOTATION);
	ssi_msg (SSI_LOG_LEVEL_DEFAULT, "reading annotation '%s'..", anno_path);
	if (!ssi_exists (anno_path)) {
		ssi_wrn ("annotation not found '%s'", anno_path);
		return false;
	}
	Annotation *anno = new Annotation();
	ModelTools::LoadAnnotation (*anno, anno_path);
	if (ssi_log_level > SSI_LOG_LEVEL_DEFAULT) {
		anno->print (stdout);
	}
	
	// load stream
	ssi_stream_t* stream = new ssi_stream_t;
	if (!loadSignal (dir, _signal_name, *stream)) {
		delete stream;
		return false;
	}

	ssi_sprint (sample_path, "%s\\%s.%s.%s%s", dir, _signal_name, _anno_name, _def_name, SSI_FILE_TYPE_SAMPLES);
	if (!ssi_exists (sample_path) || re_extract) {

		SampleList samples;
		SampleList samples_t;
	
		// extract samples
		ssi_msg (SSI_LOG_LEVEL_DEFAULT, "extract samples..");
		ModelTools::LoadSampleList (samples, *stream, *anno, user_name);
		if (ssi_log_level > SSI_LOG_LEVEL_DEFAULT) {
			samples.print (stdout);
		}

		// start feature extraction	
		ssi_msg (SSI_LOG_LEVEL_DEFAULT, "transform samples..");
		_trainer->preproc (samples, samples_t);

		// output samples
		ssi_msg (SSI_LOG_LEVEL_DEFAULT, "save samples '%s'..", sample_path);
		ModelTools::SaveSampleList (samples_t, sample_path, File::BINARY);
		if (ssi_log_level > SSI_LOG_LEVEL_DEFAULT) {
			samples_t.print (stdout);
		}
	}
	
	// continuous evaluation requires some extra information to be stored locally
	if(mode >= MlpXmlDef::KFOLD_CONT)
	{
		// store annotation
		_annos.push_back(anno);

		// store stream
		if(_trainer->_has_transformer)
		{
			ssi_stream_t* stream_proc = new ssi_stream_t;

			ITransformer* t = (*(_trainer->_transformer));
            SignalTools::Transform(*stream, *stream_proc, *t, (ssi_size_t)0 );
			
			_streams.push_back(stream_proc);

			ssi_stream_destroy(*stream);
			delete stream;
		}
		else
		{
			_streams.push_back(stream);
		}
	}
	else
	{
		delete anno;
		ssi_stream_destroy(*stream);
		delete stream;
	}
	
	return true;
}

bool MlpXmlTrain::train (const ssi_char_t *filepath) {
	
	if (!_trainer) {
		ssi_wrn ("empty trainer");
		return false;
	}

	if (!_stream_ref) {
		ssi_wrn ("empty stream reference");
		return false;
	}

	ssi_msg (SSI_LOG_LEVEL_DEFAULT, "start training..");

	_trainer->setPreprocMode (true, 1, _stream_ref);	
	if (!_trainer->train (_samples)) {
		ssi_wrn ("training failed '%s'", filepath);
		return false;
	}

	ssi_msg (SSI_LOG_LEVEL_DEFAULT, "save trainer '%s'..", filepath);
	if (!_trainer->save (filepath)) {
		ssi_wrn ("saving failed '%s'", filepath);
		return false;
	}
	
	return true;
}

bool MlpXmlTrain::eval (const ssi_char_t *filepath,
	int mode,
	ssi_size_t k_folds, 
	ssi_size_t reps,
	ssi_real_t fps) {

	FILE *fp = fopen (filepath, "w");
	bool result = false;
	if (fp) {
		result = eval (fp, mode, k_folds, reps, fps);
	} else {
		ssi_wrn ("could not open '%s'", filepath);
	}
	fclose (fp);

	return result;
}

bool MlpXmlTrain::eval (FILE *file,
	int mode,
	ssi_size_t k_folds, 
	ssi_size_t reps,
	ssi_real_t fps) {

	if (!_trainer) {
		ssi_wrn ("empty trainer");
		return false;
	}

	if (!_stream_ref) {
		ssi_wrn ("empty stream reference");
		return false;
	}

	ssi_msg (SSI_LOG_LEVEL_DEFAULT, "starting evaluation..");		

	switch (ssi_cast (MlpXmlDef::eval_t, mode)) {

		case MlpXmlDef::KFOLD:
		{
			Evaluation evaluation;
			//evaluation.setFselMethod (mlp_classifier->_fselmethod, mlp_classifier->_pre_fselmethod, mlp_classifier->_n_pre_feature); 

			evaluation.setPreprocMode (true, 1, _stream_ref);
			evaluation.evalKFold (_trainer, _samples, k_folds);
			
			ssi_msg (SSI_LOG_LEVEL_DEFAULT, "output evaluation..");
			evaluation.print (file);			
			ssi_fprint (file, "\n");
			ssi_fprint (file, "Evaluation Method: kfold (k=%u)\n", k_folds);		
			ssi_fprint (file, "\n== Options == \n");
			((OptionList*)_trainer->getModel(0)->getOptions())->print(file);
		} break;
			

		case MlpXmlDef::LOO:
		{
			Evaluation evaluation;
			//evaluation.setFselMethod (mlp_classifier->_fselmethod, mlp_classifier->_pre_fselmethod, mlp_classifier->_n_pre_feature); 

			evaluation.setPreprocMode (true, 1, _stream_ref);
			evaluation.evalLOO (_trainer, _samples);
			
			ssi_msg (SSI_LOG_LEVEL_DEFAULT, "output evaluation..");
			evaluation.print (file);			
			ssi_fprint (file, "\n");
			ssi_fprint (file, "Evaluation Method: leave one out\n");			
			ssi_fprint (file, "\n== Options == \n");
			((OptionList*)_trainer->getModel(0)->getOptions())->print(file);
		} break;

		case MlpXmlDef::LOUO:
		{
			Evaluation evaluation;
			//evaluation.setFselMethod (mlp_classifier->_fselmethod, mlp_classifier->_pre_fselmethod, mlp_classifier->_n_pre_feature); 

			evaluation.setPreprocMode (true, 1, _stream_ref);
			evaluation.evalLOUO (_trainer, _samples);
			
			ssi_msg (SSI_LOG_LEVEL_DEFAULT, "output evaluation..");
			evaluation.print (file);			
			ssi_fprint (file, "\n");
			ssi_fprint (file, "Evaluation Method: leave one user out\n");			
			ssi_fprint (file, "\n== Options == \n");
			((OptionList*)_trainer->getModel(0)->getOptions())->print(file);
		} break;

		case MlpXmlDef::FULL:
		{
			Evaluation evaluation;
			//evaluation.setFselMethod (mlp_classifier->_fselmethod, mlp_classifier->_pre_fselmethod, mlp_classifier->_n_pre_feature); 

			_trainer->setPreprocMode (true, 1, _stream_ref);
			_trainer->train (_samples);
			evaluation.eval (_trainer, _samples);
			
			ssi_msg (SSI_LOG_LEVEL_DEFAULT, "output evaluation..");
			evaluation.print (file);			
			ssi_fprint (file, "\n");
			ssi_fprint (file, "Evaluation Method: full\n");	
			ssi_fprint (file, "\n== Options == \n");
			((OptionList*)_trainer->getModel(0)->getOptions())->print(file);
		} break;

		case MlpXmlDef::LOUO_CONT:
		{
			EvaluationCont evaluation;

			evaluation.setPreprocMode (true, 1, _stream_ref);
			evaluation.evalLOUO (*_trainer, _samples, &_streams, &_annos, fps, reps);
			
			ssi_msg (SSI_LOG_LEVEL_DEFAULT, "output evaluation..");
			evaluation.print (file);			
			ssi_fprint (file, "\n");
			ssi_fprint (file, "Evaluation Method: leave one user out (cont) - %d repetitions, %.2f fps\n", reps, fps);			
			ssi_fprint (file, "\n== Options == \n");
			((OptionList*)_trainer->getModel(0)->getOptions())->print(file);
		} break;

		case MlpXmlDef::FULL_CONT:
		{
			EvaluationCont evaluation; 
			
			evaluation.setPreprocMode (true, 1, _stream_ref);
			evaluation.evalFull (*_trainer, _samples, &_streams, &_annos, fps, reps);
			
			ssi_msg (SSI_LOG_LEVEL_DEFAULT, "output evaluation..");
			evaluation.print (file);			
			ssi_fprint (file, "\n");
			ssi_fprint (file, "Evaluation Method: full (cont) - %d repetitions, %.2f fps\n", reps, fps);	
			ssi_fprint (file, "\n== Options == \n");
			((OptionList*)_trainer->getModel(0)->getOptions())->print(file);
		} break;
	}

	return true;
}

bool MlpXmlTrain::test (FILE *file,
	ssi_stream_t &stream) {

	if (!_trainer) {
		ssi_wrn ("empty trainer");
		return false;
	}

	if (!_trainer->isTrained ()) {
		ssi_fprint (file, "not trained\n");
		return false;
	}

	ssi_size_t class_id;
	if (!_trainer->forward (stream, class_id)) {
		ssi_fprint (file, "forward failed\n");
		return false;
	}	

	ssi_fprint (file, "%s\n", _trainer->getClassName (class_id));
	return true;
};

void MlpXmlTrain::loadDates (StringList &dates,
	const ssi_char_t *dir,
	const ssi_char_t *userfilt,
	const ssi_char_t *datefilt) {

	ssi_msg (SSI_LOG_LEVEL_DEFAULT, "load users..");
	StringList dirs;
	FileTools::ReadDirsFromDir (dirs, dir, userfilt, true);
	if (ssi_log_level > SSI_LOG_LEVEL_DEFAULT) {
		dirs.print (stdout);
	}	

	ssi_msg (SSI_LOG_LEVEL_DEFAULT, "load dates..");
	for (ssi_size_t i = 0; i < dirs.size (); i++) {
		FileTools::ReadDirsFromDir (dates, dirs.get (i), datefilt);
	}
	if (ssi_log_level > SSI_LOG_LEVEL_DEFAULT) {
		dates.print (stdout);
	}
}

void MlpXmlTrain::date2user (const ssi_char_t *date, ssi_char_t *user) {
	
	size_t from, to;

	to = strlen (date) - 2;
	ssi_char_t *data_ptr = ssi_ccast (ssi_char_t *, date) + to;
	while (*data_ptr-- != '\\')
		to--;
	from = to;
	while (*data_ptr-- != '\\')
		from--;
	memcpy (user, date + from, to - from);
	user[to-from] = '\0';
}

ssi_char_t **MlpXmlTrain::parseDefNames (const ssi_char_t *filepath, ssi_size_t &_n_def_names) {

	FilePath fp (filepath);
	ssi_char_t *filepath_with_ext = 0;
	if (strcmp (fp.getExtension (), SSI_FILE_TYPE_TRAINDEF) != 0) {
		filepath_with_ext = ssi_strcat (filepath, SSI_FILE_TYPE_TRAINDEF);
	} else {
		filepath_with_ext = ssi_strcpy (filepath);
	}

	ssi_msg (SSI_LOG_LEVEL_BASIC, "load '%s'", filepath_with_ext);

	TiXmlDocument doc;
	if (!doc.LoadFile (filepath_with_ext)) {
		ssi_wrn ("failed loading traindef from file '%s'", filepath_with_ext);
		delete[] filepath_with_ext;
		return 0;
	}

	TiXmlElement *body = doc.FirstChildElement();	
	if (!body || strcmp (body->Value (), "traindef") != 0) {
		ssi_wrn ("tag <traindef> missing");
		delete[] filepath_with_ext;
		return 0;	
	}

	TiXmlElement *item = body->FirstChildElement ("item");
	if (!item) {
		ssi_wrn ("element <item> missing");
		return 0;
	}

	const ssi_char_t *name = 0;
	_n_def_names = 0;
	do {
		name = item->Attribute ("name");
		if (!name) {
			ssi_wrn ("traindef->item: attribute 'name' is missing");
			return 0;
		}
		SSI_DBG (SSI_LOG_LEVEL_DEBUG, "traindef->item: <name='%s'>", name);		
		_n_def_names++;
	
	} while (item = item->NextSiblingElement ("item"));

	item = body->FirstChildElement ("item");
	ssi_char_t **def_names = new ssi_char_t *[_n_def_names];
	for (ssi_size_t i = 0;  i < _n_def_names; i++) {
		def_names[i] = ssi_strcpy (item->Attribute ("name"));
		item = item->NextSiblingElement ("item");
	}
	
	delete[] filepath_with_ext;

	return def_names;
}

Trainer *MlpXmlTrain::parseTrainer (const ssi_char_t *filepath, const ssi_char_t *defname) {

	Trainer *trainer = 0;

	FilePath fp (filepath);
	ssi_char_t *filepath_with_ext = 0;
	if (strcmp (fp.getExtension (), SSI_FILE_TYPE_TRAINDEF) != 0) {
		filepath_with_ext = ssi_strcat (filepath, SSI_FILE_TYPE_TRAINDEF);
	} else {
		filepath_with_ext = ssi_strcpy (filepath);
	}

	ssi_msg (SSI_LOG_LEVEL_BASIC, "load '%s'", filepath_with_ext);

	TiXmlDocument doc;
	if (!doc.LoadFile (filepath_with_ext)) {
		ssi_wrn ("failed loading traindef from file '%s'", filepath_with_ext);
		delete[] filepath_with_ext;
		return 0;
	}

	TiXmlElement *body = doc.FirstChildElement();	
	if (!body || strcmp (body->Value (), "traindef") != 0) {
		ssi_wrn ("tag <traindef> missing");
		delete[] filepath_with_ext;
		return 0;	
	}

	TiXmlElement *item = body->FirstChildElement ("item");
	if (!item) {
		ssi_wrn ("element <item> missing");
		return 0;
	}

	const ssi_char_t *name = 0;
	bool found = false;
	do {
		name = item->Attribute ("name");
		if (!name) {
			ssi_wrn ("traindef->item: attribute 'name' is missing");
			return 0;
		}

		SSI_DBG (SSI_LOG_LEVEL_DEBUG, "traindef->item: <name='%s'>", name);
		
		if (strcmp (name, defname) == 0) {
			trainer = parseTrainerItem (item);
			if (!trainer) {
				ssi_wrn ("failed loading traindef '%s' from '%s'", defname, filepath_with_ext);
				return false;
			}
			found = true;
		}
		
	} while (!found && (item = item->NextSiblingElement ("item")));
	
	delete[] filepath_with_ext;

	return trainer;

}

Trainer *MlpXmlTrain::parseTrainerItem (TiXmlElement *element) {

	ITransformer *transformer = 0;
	IModel *model = 0;
	Trainer *trainer = 0;

	TiXmlElement *t = element->FirstChildElement ("transform");
	if (t) {
		IObject *object = parseObject (t, true);
		if (object) {
			switch (object->getType ()) {
				case SSI_TRANSFORMER:
				case SSI_FILTER:
				case SSI_FEATURE:
					transformer = ssi_pcast (ITransformer, object);
					break;
				default:
					ssi_wrn ("traindef->item->transform: object '%s' is not a transformer", object->getName ());						
			}
		}
	}

	TiXmlElement *m = element->FirstChildElement ("model");
	if (m) {
		IObject *object = parseObject (m, true);
		if (object) {
			switch (object->getType ()) {
				case SSI_MODEL:
				case SSI_MODEL_CONTINUOUS:								
					model = ssi_pcast (IModel, object);
					break;
				default:
					ssi_wrn ("traindef->item->model: object '%s' is not a model", object->getName ());						
			}
		}
	}

	if (model) {
		trainer = new Trainer (model);
		if (transformer) {
			ITransformer *t[] = { transformer };
			trainer->setTransformer (1, t);			
		}
	}

	return trainer;
}

IObject *MlpXmlTrain::parseObject (TiXmlElement *element, bool auto_free) {

	const ssi_char_t *create = element->Attribute ("create");
	if (!create) {
		ssi_wrn ("%s: attribute 'create' is missing", element->Value ());
		return 0;
	}
	const ssi_char_t *option = element->Attribute ("option");
	if (!option || option[0] == '\0') {
		option = 0;
	}

	SSI_DBG (SSI_LOG_LEVEL_DEBUG, "%s: <create='%s' option='%s'>", element->Value (), create, option);

	IObject *object = Factory::Create (create, option, auto_free);
	if (!object) {
		ssi_wrn ("%s: could not create object '%s'", element->Value (), create);
		return 0;
	}

	return object;	
}

bool MlpXmlTrain::save (const ssi_char_t *filepath,
	MlpXmlTrain::VERSION version) {

	if (!_trainer) {
		ssi_wrn ("empty trainer");
		return false;
	}

	ssi_char_t string[SSI_MAX_CHAR];

	FilePath fp (filepath);
	ssi_char_t *filepath_with_ext = 0;
	if (strcmp (fp.getExtension (), SSI_FILE_TYPE_TRAINING) != 0) {
		filepath_with_ext = ssi_strcat (filepath, SSI_FILE_TYPE_TRAINING);
	} else {
		filepath_with_ext = ssi_strcpy (filepath);
	}

	ssi_msg (SSI_LOG_LEVEL_BASIC, "save training to file '%s'", filepath_with_ext);
	
	TiXmlDocument doc;

	TiXmlDeclaration head ("1.0", "", "");
	doc.InsertEndChild (head);
	
	TiXmlElement body ("training");		
	body.SetAttribute ("ssi-v", version);
	
	TiXmlElement defname ("def");
	defname.InsertEndChild (TiXmlText (_def_name));
	body.InsertEndChild (defname);

	TiXmlElement signal ("signal");			
	signal.InsertEndChild (TiXmlText (_signal_name));
	body.InsertEndChild (signal);

	TiXmlElement type ("type");	
	ssi_sprint (string, "%d", _signal_type);
	type.InsertEndChild (TiXmlText (string));
	body.InsertEndChild (type);
	
	TiXmlElement annotation ("anno");		 
	annotation.InsertEndChild (TiXmlText (_anno_name));
	body.InsertEndChild (annotation);

	TiXmlElement paths ("paths");	
	for (ssi_size_t i = 0; i < _paths.size (); i++) {
		TiXmlElement path ("item" );
		TiXmlText path_text (_paths.get (i));
		path.InsertEndChild (path_text);
		paths.InsertEndChild (path);
	}
	body.InsertEndChild (paths);
	

	doc.InsertEndChild (body);
	if (!doc.SaveFile (filepath_with_ext)) {
		ssi_wrn ("failed saving trainer to file '%s'", filepath_with_ext);	
		return false;
	}

	delete[] filepath_with_ext;

	return true;
}

bool MlpXmlTrain::load (const ssi_char_t *filepath, bool re_extract, int mode) {

	release ();

	FilePath fp (filepath);
	ssi_char_t *filepath_with_ext = 0;
	if (strcmp (fp.getExtension (), SSI_FILE_TYPE_TRAINING) != 0) {
		filepath_with_ext = ssi_strcat (filepath, SSI_FILE_TYPE_TRAINING);
	} else {
		filepath_with_ext = ssi_strcpy (filepath);
	}
	
	TiXmlDocument doc;
	if (!doc.LoadFile (filepath_with_ext)) {
		ssi_wrn ("failed loading info from file '%s'", filepath_with_ext);
		return false;
	}

	TiXmlElement *body = doc.FirstChildElement();	
	if (!body || strcmp (body->Value (), "training") != 0) {
		ssi_wrn ("tag <training> missing");
		return false;	
	}

	VERSION version = MlpXmlTrain::V1;
	if (body->QueryIntAttribute ("ssi-v", ssi_pcast (int, &version)) != TIXML_SUCCESS) {
		ssi_wrn ("attribute <ssi-v> in tag <training> missing, setting to V1");				
	}

	TiXmlElement *element = body->FirstChildElement ("def");
	if (!element) {
		ssi_wrn ("element <def> is missing");
		return false;
	}
	_def_name = ssi_strcpy (element->GetText ());
	_trainer = parseTrainer (_def_file, _def_name);
	if (!_trainer) {
		return false;
	}

	if(mode >= MlpXmlDef::KFOLD_CONT && _trainer->getModel(0)->getType() != SSI_MODEL_CONTINUOUS)
	{
		ssi_err ("model incompatible. Continuous model required.");
		return false;
	}

	element = body->FirstChildElement ("signal");
	if (!element) {
		ssi_wrn ("element <signal> is missing");
		return false;
	}
	_signal_name = ssi_strcpy (element->GetText ());

	element = body->FirstChildElement ("type");
	if (!element) {
		ssi_wrn ("element <type> is missing");
		return false;
	}
	sscanf (element->GetText (), "%d", &_signal_type);

	element = body->FirstChildElement ("anno");
	if (!element) {
		ssi_wrn ("element <anno> is missing");
		return false;
	}
	_anno_name = ssi_strcpy (element->GetText ());
	
	element = element->NextSiblingElement ("paths");
	StringList paths;
	if (element) {		
		TiXmlElement *item = element->FirstChildElement ("item");
		if (item) {
			do {
				paths.add (item->GetText ());
			} while (item = item->NextSiblingElement ("item"));
		}
	}

	for (ssi_size_t i = 0; i < paths.size (); i++) {
		collect (paths.get (i), re_extract, mode);
	}

	delete[] filepath_with_ext;

	return true;
}

};
