// Main
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/12/29
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
 
#include "ssi.h"
#include "ssiev.h"
#include "model/include/ssimodel.h"
#include "ssiml.h"
using namespace ssi;

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

void split (const ssi_char_t *dir, const ssi_char_t *wavfile, const ssi_char_t *annofile);
void train (const ssi_char_t *dir, const ssi_char_t *model);
void test (const ssi_char_t *file, const ssi_char_t *model);

int main (int argc, char **argv) {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif
	
	char info[1024];
	ssi_sprint (info, "\n%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	CmdArgParser cmd;
	cmd.info (info);

	Factory::RegisterDLL ("ssigraphic.dll");
	Factory::RegisterDLL ("ssimodel.dll");
	Factory::RegisterDLL ("ssiaudio.dll");
	Factory::RegisterDLL ("ssisignal.dll");
	Factory::RegisterDLL ("ssiemovoice.dll");
	Factory::RegisterDLL ("ssiioput.dll");	

	ssi_char_t *dir = 0;
	ssi_char_t *model = 0;
	ssi_char_t *wavfile = 0;
	ssi_char_t *annofile = 0;

	cmd.addMasterSwitch ("--split");

	cmd.addText("\nArguments:");
	cmd.addSCmdArg("dir", &dir, "output directory");
	cmd.addSCmdArg("file", &wavfile, "filename of wave file");
	cmd.addSCmdArg("anno", &annofile, "filename of annoation file");

	cmd.addMasterSwitch ("--train");

	cmd.addText("\nArguments:");
	cmd.addSCmdArg("dir", &dir, "directory with training files");
	cmd.addSCmdArg("model", &model, "filename of model");

	cmd.addMasterSwitch ("--test");

	cmd.addText("\nArguments:");
	cmd.addSCmdArg("file", &wavfile, "filename of wave file");
	cmd.addSCmdArg("model", &model, "filename of model");

	if (cmd.read (argc, argv)) {		

		switch (cmd.master_switch) {

			case 1: {
				split (dir, wavfile, annofile);
				break;
			}
			case 2: {
				train (dir, model);
				break;
			}
			case 3: {
				test (wavfile, model);
				break;
			}			
		}
	}

	Factory::Clear ();

	delete[] dir;
	delete[] model;
	delete[] wavfile;
	delete[] annofile;

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}

void split (const ssi_char_t *dir, const ssi_char_t *wavfile, const ssi_char_t *annofile) {

	ssi_stream_t stream;
	WavTools::ReadWavFile (wavfile, stream);

	Annotation anno;
	ModelTools::LoadAnnotation (anno, annofile);

	Annotation::Entry *entry;
	anno.reset ();
	ssi_stream_t chunk;
	ssi_size_t count = 0;
	ssi_char_t string[SSI_MAX_CHAR];
	while (entry = anno.next ()) {
		chunk = stream;
		chunk.ptr += stream.byte * ssi_cast (ssi_size_t, entry->start * stream.sr);
		chunk.num = ssi_cast (ssi_size_t, (entry->stop - entry->start) * stream.sr);
		chunk.tot = chunk.num * chunk.byte;
		ssi_sprint (string, "%s\\%s_%03d.wav", dir, anno.getLabel (entry->label_index), count++);
		WavTools::WriteWavFile (string, chunk);
	}

	ssi_stream_destroy (stream);
}

void train (const ssi_char_t *dir, const ssi_char_t *model) {

	// load samples
	StringList files;
	FileTools::ReadFilesFromDir (files, dir, "*.wav");
	SampleList samples;	
	samples.addUserName ("user");

	for (ssi_size_t i = 0; i < files.size (); i++) {
		ssi_stream_t *stream = new ssi_stream_t;
		ssi_sample_t *sample = new ssi_sample_t;
		const ssi_char_t *filename = files.get (i);
	
		// parse class name
		FilePath fp (files.get(i));
		ssi_char_t *class_name = ssi_strcpy (fp.getName ());
		for (ssi_size_t j = 0; j < strlen (class_name); j++) {
			if (class_name[j] == '_') {
				class_name[j] = '\0';
				break;
			}
		}
		ssi_size_t class_id = samples.addClassName (class_name);
		delete[] class_name;

		// read wave file
		WavTools::ReadWavFile (filename, *stream);

		// create sample
		sample->class_id = class_id;
		sample->num = 1;
		sample->prob = 1.0f;
		sample->streams = new ssi_stream_t *[1];
		sample->streams[0] = stream;
		sample->time = 0;
		sample->user_id = 0;				

		// add sample
		samples.addSample (sample);
	}

	// extract features
	SampleList samples_t;
	EmoVoiceFeat *ev_feat = ssi_create (EmoVoiceFeat, "ev_feat", true);
	ModelTools::TransformSampleList (samples, samples_t, *ev_feat);
	
	// create model
	IModel *bayes = ssi_create (NaiveBayes, "bayes", true);
	Trainer trainer (bayes);

	// evalulation
	Evaluation eval;
	eval.evalKFold (&trainer, samples_t, 10);
	eval.print ();

	// train & save
	trainer.train (samples_t);
	trainer.save (model);
}

void test (const ssi_char_t *file, const ssi_char_t *model) {

	// load model
	Trainer trainer;
	trainer.Load (trainer, model);

	// load file
	ssi_stream_t stream;
	WavTools::ReadWavFile (file, stream);

	// extract features
	ssi_stream_t stream_t;
	EmoVoiceFeat *ev_feat = ssi_create (EmoVoiceFeat, "ev_feat", true);
	SignalTools::Transform (stream, stream_t, *ev_feat, 0u);

	// classify
	ssi_size_t class_index;
	trainer.forward (stream_t, class_index);

	ssi_print ("%s\n", trainer.getClassName (class_index));

	ssi_stream_destroy (stream);
	ssi_stream_destroy (stream_t);
	
}
