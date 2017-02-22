// Main.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2007/10/11
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
#include "ssiml.h"
#include "ssimodel.h"
using namespace ssi;

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

#define N_COLORS 5
unsigned short COLORS[][3] = {
	128,0,0,
	0,128,0,
	0,0,128,
	128,0,128,
	0,128,128
};

void ex_pca (SampleList &samples); // test pca as feature reduction technique
void ex_fisher (SampleList &samples); // test fisher as feature reduction technique

void BuildSamples (SampleList &samples);

ssi_size_t n_classes = 4;
ssi_size_t n_samples = 100;
ssi_size_t n_streams = 1;
ssi_real_t distr[][3] = { 0.3f, 0.3f, 0.1f, 0.3f, 0.6f, 0.1f, 0.6f, 0.3f, 0.1f, 0.6f, 0.6f, 0.1f };
SampleList *train_samples = 0;
SampleList *test_samples = 0;

int main () {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	Factory::RegisterDLL ("ssigraphic.dll");
	Factory::RegisterDLL ("ssimodel.dll");

	ssi_random_seed ();

	Console *console = ssi_create(Console, 0, true);
	console->setPosition(ssi_rect(0, 0, 650, 800));

	SampleList samples;
	BuildSamples (samples);

    ex_pca (samples);
	ex_fisher (samples);

	ssi_print ("\n\n\tpress a key to quit\n");
	getchar ();

	Factory::Clear ();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif
	
	return 0;
}

void BuildSamples (SampleList &samples) {

   // build samples
	{	 		
		ssi_size_t n = 50;
		ssi_real_t data1[50][4];
		ssi_real_t data2[50][4];
		for (ssi_size_t i = 0; i < n; i++) {
			data1[i][0] = (float) ssi_random (0.1, 0.3);			
			data1[i][1] = (float) ssi_random (0, 1.0);
			data1[i][2] = (float) ssi_random (0, 1.0);
			data1[i][3] = (float) ssi_random (0.1, 0.3);
			data2[i][0] = (float) ssi_random (0, 1.0);			
			data2[i][1] = (float) ssi_random (0.7, 0.9);
			data2[i][2] = (float) ssi_random (0.7, 0.9);
			data2[i][3] = (float) ssi_random (0, 1.0);
		}				

		ssi_size_t index1=samples.addClassName ("X1");
		ssi_size_t index2=samples.addClassName ("X2");
		samples.addUserName ("none");
		for (ssi_size_t i = 0; i < n; i++) {
			ssi_sample_t *sample = new ssi_sample_t;
			ssi_sample_create (*sample, 1, 0, 0, 0, 0);
			ssi_stream_t *stream = new ssi_stream_t;
			ssi_stream_init (*stream, 1, 4, sizeof (ssi_real_t), SSI_REAL, 0);
			memcpy (stream->ptr, data1[i], 4 * sizeof(ssi_real_t));
			sample->streams[0] = stream;
			sample->class_id=index1;
			samples.addSample (sample);
		} 

		for (ssi_size_t i = 0; i < n; i++) {
			ssi_sample_t *sample = new ssi_sample_t;
			ssi_sample_create (*sample, 1, 0, 0, 0, 0);
			ssi_stream_t *stream = new ssi_stream_t;
			ssi_stream_init (*stream, 1, 4, sizeof (ssi_real_t), SSI_REAL, 0);
			memcpy (stream->ptr, data2[i], 4 * sizeof(ssi_real_t));
			sample->streams[0] = stream;
			sample->class_id=index2;
			samples.addSample (sample);
		} 			
	}
}

void ex_pca (SampleList &samples) {

	ssi_size_t n_dimensions;
	
	{ // build
		PCA *model = ssi_create (PCA, "pca", false);	
		model->getOptions()->percentage = 60;
		model->build(samples, 0, n_dimensions);

		model->print();

		model->save("pca.model");
		delete model;
	}

	{ // test
		PCA *model = ssi_create (PCA, "pca", false);	
		model->load("pca.model");
		ssi_stream_t reduced;
		ssi_stream_init (reduced, 1, 0, sizeof (ssi_real_t), SSI_REAL, 0);
		model->transform(*samples.get(0)->streams[0], reduced);
		
		ssi_print ("input:\n");
		ssi_stream_print (*samples.get (0)->streams[0], ssiout);
		ssi_print ("output:\n");
		ssi_stream_print (reduced, ssiout);

		if (n_dimensions == 2) {
			SampleList sreduced;
			sreduced.addClassName ("X1");
			sreduced.addClassName ("X2");
			sreduced.addUserName ("none");
			samples.reset ();
			ssi_sample_t *sample = 0;
			while (sample = samples.next ()) {
				ssi_sample_t *s = new ssi_sample_t;
				ssi_sample_create (*s, sample->num, sample->user_id, sample->class_id, sample->time, sample->score);
				s->streams[0] = new ssi_stream_t;
				ssi_stream_init (*(s->streams[0]), 1, 2, sizeof(ssi_real_t), SSI_REAL, 0);
				model->transform(*(sample->streams[0]), *(s->streams[0]));
				sreduced.addSample (s);
			}

			ISNorm sreduced_norm (&sreduced);
			ISNorm::Params params;
			ISNorm::ZeroParams(params, ISNorm::METHOD::SCALE);
			sreduced_norm.setNorm (0, params);
			ModelTools::PrintInfo (sreduced_norm);
			ModelTools::PlotSamples(sreduced_norm, "pca", ssi_rect(650, 0, 400, 400));
			ISNorm::ReleaseParams(params);
		}

		ssi_stream_destroy (reduced);
		delete model;
	}

	ssi_print ("\n\n\tpress a key to continue\n");
	getchar ();
}

void ex_fisher (SampleList &samples) {

	{ // build
		Fisher *model = ssi_create (Fisher, "LDA", false);	
		model->getOptions()->n = 2;
		model->build(samples, 0);

		model->print();

		model->save("fisher");
		delete model;
	}

	{ // test

		Fisher *model = ssi_create (Fisher, "fisher", false);	
		model->load("fisher");
		ssi_stream_t reduced;
		ssi_stream_init (reduced, 1, 2, sizeof (ssi_real_t), SSI_REAL, 0);
		model->transform(*samples.get(0)->streams[0], reduced);

		ssi_print ("input:\n");
		ssi_stream_print (*samples.get (0)->streams[0], ssiout);
		ssi_print ("output:\n");
		ssi_stream_print (reduced, ssiout);

		SampleList sreduced;
		sreduced.addClassName ("X1");
		sreduced.addClassName ("X2");
		sreduced.addUserName ("none");
		samples.reset ();
		ssi_sample_t *sample = 0;
		while (sample = samples.next ()) {
			ssi_sample_t *s = new ssi_sample_t;
			ssi_sample_create (*s, sample->num, sample->user_id, sample->class_id, sample->time, sample->score);
			s->streams[0] = new ssi_stream_t;
			ssi_stream_init (*(s->streams[0]), 1, 2, sizeof(ssi_real_t), SSI_REAL, 0);
			model->transform(*(sample->streams[0]), *(s->streams[0]));
			sreduced.addSample (s);
		}

		ISNorm sreduced_norm (&sreduced);
		ISNorm::Params params;
		ISNorm::ZeroParams(params, ISNorm::METHOD::SCALE);
		sreduced_norm.setNorm (0, params);
		ModelTools::PrintInfo (sreduced_norm);
		ModelTools::PlotSamples(sreduced_norm, "fisher", ssi_rect(650, 0, 400, 400));
		ISNorm::ReleaseParams(params);

		ssi_stream_destroy (reduced);
		delete model;
	}

	ssi_print ("\n\n\tpress a key to continue\n");
	getchar ();
}
