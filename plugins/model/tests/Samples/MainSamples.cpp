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
#include "signal/include/ssisignal.h"
using namespace ssi;

//#include <vld.h>

#define N_COLORS 5
unsigned short COLORS[][3] = {
	128,0,0,
	0,128,0,
	0,0,128,
	128,0,128,
	0,128,128
};

bool ex_samplelist (void *args);
bool ex_samplesel(void *args);
bool ex_classsel(void *args);
bool ex_usersel(void *args);
bool ex_strmmerge(void *args);
bool ex_alignstrms(void *args);
bool ex_sampmerge(void *args);
bool ex_missingstrm(void *args);
bool ex_oversample(void *args);
bool ex_undersample(void *args);
bool ex_norm(void *args);
bool ex_dupl(void *args);
bool ex_arff(void *args);
bool ex_trigger(void *args);
bool ex_elan(void *args);

void PlotSamples (ISamples &samples, const ssi_char_t *name);
void CreateMissingData (SampleList &samples, double prob);

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

int main () {	

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	Factory::RegisterDLL ("ssimodel.dll");
	Factory::RegisterDLL ("ssigraphic.dll");
	Factory::RegisterDLL ("ssisignal.dll");

	ssi_random_seed ();

	Factory::GetPainter ()->MoveConsole(0,600,600,400);

	Exsemble exsemble;
	exsemble.add(&ex_samplelist, 0, "SSI SAMPLES", "");
	exsemble.add(&ex_arff, 0, "ARFF SAMPLES", "");
	exsemble.add(&ex_elan, 0, "ELAN SAMPLES", "");
	exsemble.add(&ex_samplesel, 0, "SELECT SAMPLES", "");
	exsemble.add(&ex_classsel, 0, "SELECT CLASSES", "");
	exsemble.add(&ex_usersel, 0, "SELECT USERS", "");
	exsemble.add(&ex_strmmerge, 0, "MERGE STREAMS", "");
	exsemble.add(&ex_alignstrms, 0, "ALIGN STREAMS", "");
	exsemble.add(&ex_sampmerge, 0, "MERGE SAMPLES", "");
	exsemble.add(&ex_missingstrm, 0, "REMOVE MISSING", "");
	exsemble.add(&ex_oversample, 0, "OVER SAMPLE", "");
	exsemble.add(&ex_undersample, 0, "UNDER SAMPLE", "");
	exsemble.add(&ex_norm, 0, "NORMALIZE SAMPLES", "");
	exsemble.add(&ex_dupl, 0, "DUPLICATE SAMPLES", "");
	exsemble.add(&ex_trigger, 0, "TRIGGER SAMPLES", "");
	exsemble.show();

	ssi_print ("\n\n\tpress enter to quit\n\n");
	getchar ();

	Factory::Clear ();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif
	
	return 0;
}

bool ex_samplelist(void *args) {

	ssi_char_t now[ssi_time_size];

	{
		ssi_size_t n_classes = 4;
		ssi_size_t n_samples = 5;
		ssi_size_t n_streams = 3;
		ssi_real_t distr[][3] = { 0.3f, 0.3f, 0.2f, 0.3f, 0.6f, 0.2f, 0.6f, 0.3f, 0.2f, 0.6f, 0.6f, 0.2f };

		SampleList samples;
		ModelTools::CreateTestSamples (samples, n_classes, n_samples, n_streams, distr, "user");
		CreateMissingData (samples, 0.9);
		samples.get (5)->class_id = SSI_ISAMPLES_GARBAGE_CLASS_ID;

		ModelTools::PrintInfo (samples);
		ModelTools::PrintSamples (samples);

		FileSamplesOut samples_out;
		samples_out.open (samples, "out\\test-static", File::ASCII);
		ssi_sample_t *sample = 0;
		samples.reset ();
		while (sample = samples.next ()) {
			samples_out.write (*sample);
		}
		samples_out.write (samples);
		samples_out.close ();

		FileSamplesIn samples_in;
		samples_in.open ("out\\test-static");
		ModelTools::PrintInfo (samples_in);
		ModelTools::PrintSamples (samples_in);		
		ModelTools::SaveSampleList (samples_in, "out\\test-static-check", File::ASCII);

		samples_in.close ();
	}

	{
		ssi_size_t n_classes = 4;
		ssi_size_t n_samples = 5;
		ssi_size_t n_streams = 3;
		ssi_real_t distr[][3] = { 0.3f, 0.3f, 0.2f, 0.3f, 0.6f, 0.2f, 0.6f, 0.3f, 0.2f, 0.6f, 0.6f, 0.2f };
		ssi_size_t num_min = 2;
		ssi_size_t num_max = 5;

		SampleList samples;
		ModelTools::CreateDynamicTestSamples (samples, n_classes, n_samples, n_streams, distr, num_min, num_max, "user");
		CreateMissingData (samples, 0.9);

		ModelTools::PrintInfo (samples);
		ModelTools::PrintSamples (samples);

		FileSamplesOut samples_out;
		samples_out.open (samples, "out\\test-dynamic", File::ASCII);
		ssi_sample_t *sample = 0;
		samples.reset ();
		while (sample = samples.next ()) {
			samples_out.write (*sample);
		}
		samples_out.write (samples);
		samples_out.close ();

		FileSamplesIn samples_in;
		samples_in.open ("out\\test-dynamic");
		ModelTools::PrintInfo (samples_in);
		ModelTools::PrintSamples (samples_in);		
		ModelTools::SaveSampleList (samples_in, "out\\test-dynamic-check", File::ASCII);

		samples_in.close ();
	}

	{
		ssi_now (now);
		ssi_print ("%s\n", now);
		time_t now_t = ssi_time_parse (now);
		ssi_time_format (now_t, now);
		ssi_print ("%s\n", now);

		ssi_char_t *base_dir = "in\\";

		ssi_print ("read files from %s\n", base_dir);

		// load
		StringList files;
		FileTools::ReadFilesFromDir (files, base_dir, "*.feat");
		files.print (stdout);
		SampleList samples;
		ModelTools::LoadSampleList (samples, files);

		ssi_print ("merged..\n");
		samples.print (stdout);		

		// clone
		SampleList samples_clone;
		ModelTools::CopySampleList (samples, samples_clone);
		ModelTools::SaveSampleList (samples, "out", ".feat");

		ssi_print ("cloned..\n");
		samples_clone.print (stdout);

		SampleList samples_merge;
		SampleList *samples_from[2] = {&samples, &samples_clone};
		ModelTools::MergeSampleList (samples_merge, 2, samples_from);

		ssi_print ("merged..\n");
		samples_merge.print (stdout);
	}

	{
		Annotation anno;
		ModelTools::LoadAnnotation (anno, "in/anno.txt");

		ssi_stream_t *streams[2];
		
		streams[0] = new ssi_stream_t;
		FileTools::ReadStreamFile (File::BINARY, "in/audio.dat", *streams[0]);

		streams[1] = new ssi_stream_t;		
		FileTools::ReadStreamFile (File::BINARY, "in/wii.dat", *streams[1]);

		SampleList samples;
		ModelTools::LoadSampleList (samples, 2, streams, anno, "user");
		samples.print (stdout);
		ModelTools::SaveSampleList (samples, "out", ".dat");
		{
			ModelTools::SaveSampleList (samples, "out/samples.samples", File::BINARY, File::V1);
		}
		SampleList samples_check, samples_check_2;
		StringList files;
		FileTools::ReadFilesFromDir (files, "out", "*.dat");
		ModelTools::LoadSampleList (samples_check, files);	
		ModelTools::LoadSampleList (samples_check_2, "out/samples.samples", File::BINARY);
		samples_check.print (stdout);
		samples_check_2.print (stdout);

		Annotation anno_c;
		ModelTools::ConvertToContinuousAnnotation (anno, anno_c, 0.075, 0.025, 0.5);
		ModelTools::SaveAnnotation (anno_c, "out/anno_c.anno");

		ssi_stream_destroy (*streams[0]);
		delete streams[0];
		ssi_stream_destroy (*streams[1]);
		delete streams[1];
	}

	{
		Functionals *functs = ssi_create (Functionals, 0, true);

		SampleList samples;
		ModelTools::LoadSampleList (samples, "out/samples.samples", File::BINARY);

		ssi_print ("samples:\n");
		ModelTools::PrintInfo (samples);

		ssi_print ("samples (trans):\n");
		ISTransform samples_trans (&samples);
		samples_trans.setTransformer (1, *functs);
		samples_trans.callEnter ();
		ModelTools::PrintInfo (samples_trans);

		ssi_print ("samples (select dim):\n");
		ISSelectDim samples_seldim (&samples_trans);
		ssi_size_t dims[] = {0,2,4};
		samples_seldim.setSelection (1, 3, dims);
		ModelTools::PrintInfo (samples_seldim);

		ssi_print ("samples (select class):\n");
		ISSelectClass samples_selclass (&samples_seldim);
		ssi_size_t classes[] = {0,2};
		samples_selclass.setSelection (2, classes);
		ModelTools::PrintInfo (samples_selclass);

		ssi_print ("samples (hot):\n");
		ISHotClass samples_hot (&samples_seldim);
		samples_hot.setHotClass (1);
		ModelTools::PrintInfo (samples_hot);

		ModelTools::PrintClasses (samples_seldim);
		ModelTools::PrintClasses (samples_hot);

		ssi_print ("samples (2xhot):\n");
		ISHotClass samples_hot2 (&samples_seldim);
		ssi_size_t hotties[2] = {0,2};
		samples_hot2.setHotClass (2, hotties, "labelAC");
		ModelTools::PrintInfo (samples_hot2);

		ModelTools::PrintClasses (samples_seldim);
		ModelTools::PrintClasses (samples_hot2);

		ssi_print ("samples (re-class):\n");
		ISReClass samples_re (&samples_trans);
		ssi_size_t n_classes = 4;
		const ssi_char_t *class_names[] = {"reA", "reB", "reC", "reD"};
		ssi_size_t n_ids = samples_trans.getSize ();
		ssi_size_t *ids = new ssi_size_t[n_ids];
		for (ssi_size_t i = 0; i < n_ids; i++) {
			ids[i] = i % n_classes;
		}
		samples_re.setReClass (n_classes, class_names, n_ids, ids);
		ModelTools::PrintInfo (samples_re);
		delete[] ids;

		ModelTools::PrintClasses (samples_trans);
		ModelTools::PrintClasses (samples_re);

		ssi_print ("samples (select samps):\n");
		ssi_size_t samps[2] = { 0, 2 };		
		ISSelectSample sample_selsamp (&samples_hot);
		sample_selsamp.setSelection (2, samps);
		ModelTools::PrintInfo (sample_selsamp);

		samples_trans.callFlush ();
	}

	return true;
}

bool ex_alignstrms(void *args){

	ssi_size_t n_classes = 4;
	ssi_size_t n_samples = 5;
	ssi_size_t n_streams = 3;
	ssi_real_t distr[][3] = { 0.3f, 0.3f, 0.2f, 0.3f, 0.6f, 0.2f, 0.6f, 0.3f, 0.2f, 0.6f, 0.6f, 0.2f };
	SampleList s;
	
	ModelTools::CreateTestSamples (s, n_classes, n_samples, n_streams, distr, "user_1");

	ssi::ModelTools::PrintInfo(s);

	ISAlignStrms aligned (&s);

	ModelTools::PrintInfo(aligned);

	ssi_sample_t* temp = 0;

	ssi_print("\nsamples selection next()");
	//next()
	ssi_print("\n");
	while( temp = aligned.next() )
	{
		ssi_print("%d", temp->num);
	}
	ssi_print("\n");

	aligned.reset();

	return true;
}

bool ex_strmmerge(void *args) {

	SampleList s1;
	SampleList s2;
	SampleList s3;

	ssi_size_t n_classes = 4;
	ssi_size_t n_samples = 5;
	ssi_real_t distr[][3] = { 0.3f, 0.3f, 0.2f, 0.3f, 0.6f, 0.2f, 0.6f, 0.3f, 0.2f, 0.6f, 0.6f, 0.2f };

	{
		ssi_size_t n_streams = 2;
		ModelTools::CreateTestSamples (s1, n_classes, n_samples, n_streams, distr);
	}

	{
		ssi_size_t n_streams = 1;
		ModelTools::CreateTestSamples (s2, n_classes, n_samples, n_streams, distr);
	}
	
	{
		ssi_size_t n_streams = 3;
		ModelTools::CreateTestSamples (s3, n_classes, n_samples, n_streams, distr);
	}

	ISamples *ss[] = { &s1, &s2, &s3 };
	ISMergeStrms merged (3, ss);

	ssi_print("samples s1:\n");
	ModelTools::PrintInfo (s1);

	ssi_print("samples s2:\n");
	ModelTools::PrintInfo (s2);

	ssi_print("samples s3:\n");
	ModelTools::PrintInfo (s3);

	ssi_print("merged:\n");
	ModelTools::PrintInfo (merged);
	PlotSamples (merged, "merged");

	return true;
}

bool ex_sampmerge(void *args) {

	SampleList s1;
	SampleList s2;
	SampleList s3;

	ssi_size_t n_streams = 2;
	ssi_real_t distr[][3] = { 0.3f, 0.3f, 0.2f, 0.3f, 0.6f, 0.2f, 0.6f, 0.3f, 0.2f, 0.6f, 0.6f, 0.2f };

	{		
		ssi_size_t n_classes = 2;
		ssi_size_t n_samples = 5;
		ModelTools::CreateTestSamples (s1, n_classes, n_samples, n_streams, distr, "adam");
		ModelTools::CreateTestSamples (s1, n_classes, n_samples, n_streams, distr, "eva");
	}

	{
		ssi_size_t n_classes = 4;
		ssi_size_t n_samples = 15;
		ModelTools::CreateTestSamples (s2, n_classes, n_samples, n_streams, distr, "eva");
		ModelTools::CreateTestSamples (s2, n_classes, n_samples, n_streams, distr, "adam");
		ModelTools::CreateTestSamples (s2, n_classes, n_samples, n_streams, distr, "abraham");
		ModelTools::CreateTestSamples (s2, n_classes, n_samples, n_streams, distr, "moses");
	}
	
	{
		ssi_size_t n_classes = 3;
		ssi_size_t n_samples = 10;
		ModelTools::CreateTestSamples (s3, n_classes, n_samples, n_streams, distr, "adam");
	}

	ISamples *ss[] = { &s1, &s2, &s3 };
	ISMergeSample merged (3, ss);

	ssi_print("samples s1:\n");
	ModelTools::PrintInfo (s1);

	ssi_print("samples s2:\n");
	ModelTools::PrintInfo (s2);

	ssi_print("samples s3:\n");
	ModelTools::PrintInfo (s3);

	ssi_print("merged:\n");
	ModelTools::PrintInfo (merged);
	merged.printDebug ();
	PlotSamples (merged, "merged");	

	return true;
}

bool ex_usersel(void *args){

	ssi_size_t n_classes = 4;
	ssi_size_t n_samples = 5;
	ssi_size_t n_streams = 1;
	ssi_real_t distr[][3] = { 0.3f, 0.3f, 0.2f, 0.3f, 0.6f, 0.2f, 0.6f, 0.3f, 0.2f, 0.6f, 0.6f, 0.2f };
	SampleList strain;
	SampleList sdevel;
	SampleList stest;
	
	ModelTools::CreateTestSamples (strain, n_classes, n_samples, n_streams, distr, "user_1");
	ModelTools::CreateTestSamples (strain, n_classes, n_samples, n_streams, distr, "user_2");
	ModelTools::CreateTestSamples (strain, n_classes, n_samples, n_streams, distr, "user_3");

	ssi_print("\nsamples train size:\t %d", strain.getSize());
	ssi_print("\nsamples user  size:\t %d", strain.getUserSize());
	for(ssi_size_t nusers = 0; nusers < strain.getUserSize(); nusers++){
		ssi_print("\nsamples user  name:\t %s", strain.getUserName(nusers));
	}
	
	//Selection	
	ssi_size_t user_set[2] = {0,2};
	ssi_size_t user_set_re[2] = {1,2};
	ISSelectUser selection (&strain);
	selection.setSelection(2, user_set);

	ssi_print("\nsamples selection size:\t %d", selection.getSize());
	ssi_print("\nsamples user  size:\t %d", selection.getUserSize());
	for(ssi_size_t nusers = 0; nusers < selection.getUserSize(); nusers++){
		ssi_print("\nsamples user  name:\t %s", selection.getUserName(nusers));
	}

	ssi_sample_t* temp = 0;

	ssi_print("\nsamples selection next()");
	//next()
	ssi_print("\n");
	while( temp = selection.next() )
	{
		ssi_print("%d", temp->user_id);
	}
	ssi_print("\n");
	selection.reset();

	//get
	ssi_print("\nsamples no-selection get(21): %d-%d-%s", strain.get(21)->class_id, strain.get(21)->user_id, strain.getUserName(strain.get(21)->user_id));
	ssi_print("\nsamples selection    get(21): %d-%d-%s", selection.get(21)->class_id, selection.get(21)->user_id, selection.getUserName(selection.get(21)->user_id));  
	
	ssi_print("\n\nsamples selection re-selection");
	//re-select
	selection.setSelection(2, user_set_re);
	ssi_print("\nsamples selection size:\t %d", selection.getSize());
	ssi_print("\nsamples user  size:\t %d", selection.getUserSize());
	for(ssi_size_t nusers = 0; nusers < selection.getUserSize(); nusers++){
		ssi_print("\nsamples user  name:\t %s", selection.getUserName(nusers));
	}

	ssi_print("\nsamples selection next()");
	//next()
	ssi_print("\n");
	while( temp = selection.next() )
	{
		ssi_print("%d", temp->user_id);
	}
	ssi_print("\n");
	selection.reset();

	//get
	ssi_print("\nsamples no-selection get(0): %d-%d-%s", strain.get(0)->class_id, strain.get(0)->user_id, strain.getUserName(strain.get(0)->user_id));
	ssi_print("\nsamples selection    get(0): %d-%d-%s", selection.get(0)->class_id, selection.get(0)->user_id, selection.getUserName(selection.get(0)->user_id)); 

	//samples_per_class
	ssi_print("\n\nsamples in class 0: %d", selection.getSize(0));
	ssi_print("\n\nsamples in class 1: %d", selection.getSize(1));
	ssi_print("\n\nsamples in class 2: %d", selection.getSize(2));
	ssi_print("\n\nsamples in class 3: %d", selection.getSize(3));

	return true;
}

bool ex_classsel(void *args) {

	ssi_size_t n_classes = 3;
	ssi_size_t n_samples = 10;
	ssi_size_t n_streams = 1;
	ssi_real_t distr[][3] = { 0.3f, 0.3f, 0.2f, 0.3f, 0.6f, 0.2f, 0.6f, 0.3f, 0.2f };
	SampleList samples;
	ModelTools::CreateTestSamples (samples, n_classes, n_samples, n_streams, distr);	

	ssi_print("\nsamples size: %d\n", samples.getSize());
	for(ssi_size_t nclass = 0; nclass < samples.getClassSize (); nclass++){
		ssi_print("samples class %d: %d\n", nclass, samples.getSize(nclass));
	}

	ssi_size_t class_set[2] = { 1, 2 };
	ISSelectClass selection (&samples);
	selection.setSelection(2, class_set);

	ssi_print("samples selection size: %d\n", selection.getSize());
	for(ssi_size_t nclass = 0; nclass < selection.getClassSize (); nclass++){
		ssi_print("samples selection class %d: %d\n", nclass, selection.getSize(nclass));
	}
	ssi_print("samples selection next()\n");
	selection.reset ();
	ssi_sample_t* temp = 0;
	while( temp = selection.next() )
	{
		ssi_print("%u ", temp->class_id);
	}
	ssi_print("\n");

	//get
	ssi_print("samples no-selection get(11): %u\n", samples.get(11)->class_id);
	ssi_print("samples selection    get(11): %u\n", selection.get(11)->class_id);  
	ssi_print("samples selection re-selection\n");

	//re-select
	selection.setSelection(2);
	ssi_print("samples selection size: %d\n", selection.getSize());
	for(ssi_size_t nclass = 0; nclass < 1; nclass++){
		ssi_print("samples selection class %d: %d\n", nclass, selection.getSize(nclass));
	}
	ssi_print("samples selection next()\n");
	selection.reset ();
	while( temp = selection.next() )
	{
		ssi_print("%u ", temp->class_id);
	}
	ssi_print("\n");

	//get
	ssi_print("samples no-selection get(1): %u\n", samples.get(1)->class_id);
	ssi_print("samples selection    get(1): %u\n", selection.get(1)->class_id);  

	return true;
}

bool ex_samplesel(void *args) {

	ssi_size_t n_classes = 4;
	ssi_size_t n_samples = 50;
	ssi_size_t n_streams = 1;
	ssi_real_t distr[][3] = { 0.3f, 0.3f, 0.2f, 0.3f, 0.6f, 0.2f, 0.6f, 0.3f, 0.2f, 0.6f, 0.6f, 0.2f };
	SampleList strain;
	SampleList sdevel;
	SampleList stest;
	ModelTools::CreateTestSamples (strain, n_classes, n_samples, n_streams, distr);	

	ssi_print("\nsamples train size:       %d", strain.getSize());

	ssi_size_t sample_set[12] = { 1, 2, 3, 51, 52, 53, 101, 102, 103, 151, 152, 153 };
	ssi_size_t sample_set_small[3] = { 151, 152, 153 };
	ISSelectSample selection (&strain);
	selection.setSelection(12, sample_set);

	ssi_print("\nsamples selection size:   %d", selection.getSize());

	ssi_sample_t* temp = 0;

	ssi_print("\nsamples selection next()");
	//next()
	while( temp = selection.next() )
	{
		ssi_print("\n%d", temp->class_id);
	}
	selection.reset();
	ssi_print("\nsamples selection re-selection");
	//re-select
	selection.setSelection(3, sample_set_small);
	ssi_print("\nsamples selection next()");
	//next()
	while( temp = selection.next() )
	{
		ssi_print("\n%d", temp->class_id);
	}
	
	//get
	ssi_print("\nsamples no-selection get(1): %d", strain.get(1)->class_id);
	ssi_print("\nsamples selection    get(1): %d", selection.get(1)->class_id);

	return true;
}

bool ex_undersample(void *args) {

	ssi_size_t n_streams = 1;
	ssi_size_t n_samples_large = 500;	
	ssi_size_t n_samples_small = 50;	

	ssi_real_t large_distr[][3] = { 0.3f, 0.3f, 0.2f };
	ssi_real_t small_distr[][3] = { 0.3f, 0.3f, 0.2f, 0.6f, 0.6f, 0.2f };

	SampleList slarge;
	SampleList ssmall;
	SampleList devel;
	ModelTools::CreateTestSamples (slarge, 1, n_samples_large, n_streams, large_distr);			
	ModelTools::CreateTestSamples (ssmall, 2, n_samples_small, n_streams, small_distr);		
	ModelTools::CreateTestSamples (devel, 2, n_samples_large, n_streams, small_distr);		

	ISamples *lists[2] = { &slarge, &ssmall };
	ISMergeSample smerge (2, lists);
	ModelTools::PrintInfo (smerge);
	PlotSamples (smerge, "original");

	ISUnderSample sunder (&smerge);	

	sunder.setUnder (0, ssmall.getSize (0) + slarge.getSize () - ssmall.getSize (1), ISUnderSample::RANDOM);
	ModelTools::PrintInfo (sunder);
	PlotSamples (sunder, "ISUnderSample::RANDOM");

	{
		NaiveBayes *model = ssi_create (NaiveBayes, 0, true);
		Trainer trainer (model);
		trainer.train (smerge);
		Evaluation eval;
		eval.eval (&trainer, devel);
		eval.print ();
	}

	{
		NaiveBayes *model = ssi_create (NaiveBayes, 0, true);
		Trainer trainer (model);
		trainer.train (sunder);
		Evaluation eval;
		eval.eval (&trainer, devel);
		eval.print ();
	}

	sunder.setUnder (0, ssmall.getSize (0) + slarge.getSize () - ssmall.getSize (1), ISUnderSample::KMEANS);	
	ModelTools::PrintInfo (sunder);
	PlotSamples (sunder, "ISUnderSample::KMEANS");

	{
		NaiveBayes *model = ssi_create (NaiveBayes, 0, true);
		Trainer trainer (model);
		trainer.train (smerge);
		Evaluation eval;
		eval.eval (&trainer, devel);
		eval.print ();
	}

	{
		NaiveBayes *model = ssi_create (NaiveBayes, 0, true);
		Trainer trainer (model);
		trainer.train (sunder);
		Evaluation eval;
		eval.eval (&trainer, devel);
		eval.print ();
	}

	return true;
}

bool ex_oversample(void *args) {
	
	ssi_size_t n_streams = 1;
	ssi_size_t n_samples_large = 500;	
	ssi_size_t n_samples_small = 100;	

	ssi_real_t large_distr[][3] = { 0.3f, 0.3f, 0.2f };
	ssi_real_t small_distr[][3] = { 0.3f, 0.3f, 0.2f, 0.6f, 0.6f, 0.2f };

	SampleList slarge;
	SampleList ssmall;
	SampleList devel;
	ModelTools::CreateTestSamples (slarge, 1, n_samples_large, n_streams, large_distr);			
	ModelTools::CreateTestSamples (ssmall, 2, n_samples_small, n_streams, small_distr);		
	ModelTools::CreateTestSamples (devel, 2, n_samples_large, n_streams, small_distr);		

	ISamples *lists[2] = { &slarge, &ssmall };
	ISMergeSample smerge (2, lists);
	ModelTools::PrintInfo (smerge);
	PlotSamples (smerge, "original");
	ISOverSample sover (&smerge);

	sover.setOver (1, slarge.getSize () - ssmall.getSize (1), ISOverSample::RANDOM);
	ModelTools::PrintInfo (sover);
	PlotSamples (sover, "ISOverSample::SIMPLE");

	{
		NaiveBayes *model = ssi_create (NaiveBayes, 0, true);
		Trainer trainer (model);
		trainer.train (smerge);
		Evaluation eval;
		eval.eval (&trainer, devel);
		eval.print ();
	}

	{
		NaiveBayes *model = ssi_create (NaiveBayes, 0, true);
		Trainer trainer (model);
		trainer.train (sover);
		Evaluation eval;
		eval.eval (&trainer, devel);
		eval.print ();
	}

	sover.setSmoteK (15);
	sover.setOver (1, slarge.getSize () - ssmall.getSize (1), ISOverSample::SMOTE);
	ModelTools::PrintInfo (sover);
	PlotSamples (sover, "ISOverSample::SMOTE");

	{
		NaiveBayes *model = ssi_create (NaiveBayes, 0, true);
		Trainer trainer (model);
		trainer.train (smerge);
		Evaluation eval;
		eval.eval (&trainer, devel);
		eval.print ();
	}

	{
		NaiveBayes *model = ssi_create (NaiveBayes, 0, true);
		Trainer trainer (model);
		trainer.train (sover);
		Evaluation eval;
		eval.eval (&trainer, devel);
		eval.print ();
	}

	return true;
}

bool ex_missingstrm(void *args) {

	ssi_size_t n_classes = 4;
	ssi_size_t n_samples = 50;
	ssi_size_t n_streams = 3;
	ssi_real_t train_distr[][3] = { 0.3f, 0.3f, 0.2f, 0.3f, 0.6f, 0.2f, 0.6f, 0.3f, 0.2f, 0.6f, 0.6f, 0.2f };
	ssi_real_t test_distr[][3] = { 0.5f, 0.5f, 0.5f };
	SampleList strain;
	SampleList sdevel;
	SampleList stest;
	ModelTools::CreateTestSamples (strain, n_classes, n_samples, n_streams, train_distr);			
	ModelTools::CreateTestSamples (sdevel, n_classes, n_samples, n_streams, train_distr);

	CreateMissingData (strain, 0.9);
	CreateMissingData (sdevel, 0.9);

	ModelTools::PrintInfo (strain);

	ssi_print ("\n");

	ISMissingData strain_miss (&strain);
	strain_miss.setStream (2);
	ModelTools::PrintInfo (strain_miss);

	ssi_print ("\n");

	ISAlignStrms strain_align (&strain);
	ModelTools::PrintInfo (strain_align);
	ISMissingData strain_align_miss (&strain_align);
	strain_align_miss.setStream (0);
	ModelTools::PrintInfo (strain_align_miss);

	// strain
	{
 		IModel **models = new IModel *[n_streams];
		for (ssi_size_t n_stream = 0; n_stream < n_streams; n_stream++) {
			models[n_stream] = ssi_create (IModel, 0, true);
		}
		SimpleFusion *fusion = ssi_create (SimpleFusion, 0, true);
		fusion->getOptions()->method = SimpleFusion::MAXIMUM;

		Trainer trainer (n_streams, models, fusion);
		Evaluation eval;
		trainer.train (strain);
		eval.eval (&trainer, sdevel);
		eval.print ();

		delete[] models;
	}

	return true;
}

bool ex_norm(void *args) {

	ssi_size_t n_classes = 3;
	ssi_size_t n_samples = 50;
	ssi_size_t n_streams = 4;
	ssi_real_t distr[][3] = { 0.5f, 0.5f, 0.01f, 0.5f, 0.5f, 0.01f, 0.5f, 0.5f, 0.01f };
	SampleList samples;	
	ModelTools::CreateTestSamples (samples, n_classes, n_samples, n_streams, distr);		
	PlotSamples (samples, "Samples");	
	ModelTools::SaveSampleList(samples, "norm_raw.samples", File::BINARY);

	ISNorm snorm (&samples);
	ISNorm::Params params[4];
	ISNorm::ZeroParams(params[0], ISNorm::Method::NONE);
	ISNorm::ZeroParams(params[1], ISNorm::Method::SCALE);
	ISNorm::ZeroParams(params[2], ISNorm::Method::SCALE);
	params[2].limits[0] = -2.0f;
	params[2].limits[1] = 0.5f;
	ISNorm::ZeroParams(params[3], ISNorm::Method::ZSCORE);

	snorm.setNorm (0, params[0]);
	snorm.setNorm (1, params[1]);
	snorm.setNorm (2, params[2]);
	snorm.setNorm (3, params[3]);

	ModelTools::SaveSampleList(snorm, "norm_1.samples", File::ASCII);
	PlotSamples (snorm, "Normalized");

	snorm.norm (samples);
	ModelTools::SaveSampleList(samples, "norm_2.samples", File::ASCII);

	ISNorm::ReleaseParams(params[0]);
	ISNorm::ReleaseParams(params[1]);
	ISNorm::ReleaseParams(params[2]);
	ISNorm::ReleaseParams(params[3]);

	return true;
}

bool ex_dupl(void *args) {

	ssi_size_t n_classes = 3;
	ssi_size_t n_samples = 5;
	ssi_size_t n_streams = 1;
	ssi_real_t distr[][3] = { 0.3f, 0.3f, 0.2f, 0.3f, 1.6f, 1.2f, 1.6f, 1.3f, 10.2f, 10.6f, 10.6f, 10.2f };	
	SampleList samples;	
	ModelTools::CreateTestSamples (samples, n_classes, n_samples, n_streams, distr);		
	
	ISDuplStrms dupl (&samples, 3);
	ModelTools::PrintSamples (dupl);

	return true;
}

bool ex_arff(void *args) {

	// create samples
	ssi_size_t n_classes = 3;
	ssi_size_t n_samples = 5;
	ssi_size_t n_streams = 1;
	ssi_real_t distr[][3] = { 0.3f, 0.3f, 0.2f, 0.3f, 1.6f, 1.2f, 1.6f, 1.3f, 10.2f, 10.6f, 10.6f, 10.2f };	
	SampleList samples;	
	ModelTools::CreateTestSamples (samples, n_classes, n_samples, n_streams, distr);
	CreateMissingData (samples, 0.9);
	
	// write arff
	ModelTools::SaveSampleListArff (samples, "out\\samples.arff");

	// read arff
	SampleArff sa ("out\\samples.arff", 0);
	
	// output samples
	ModelTools::PrintInfo (sa);
	ModelTools::PrintSamples (sa);

	return true;
}

bool ex_trigger(void *args) {

	ssi_size_t n_classes = 2;
	ssi_size_t n_samples = 4;
	ssi_size_t n_streams = 2;
	ssi_real_t distr[][3] = { 0.3f, 0.3f, 0.2f, 0.3f, 1.6f, 1.2f, 1.6f, 1.3f, 10.2f, 10.6f, 10.6f, 10.2f };	
	SampleList samples;	
	ModelTools::CreateTestSamples (samples, n_classes, n_samples, n_streams, distr);	

	//ModelTools::PrintSamples (samples);

	// add time stamps
	samples.reset ();
	ssi_sample_t *s = 0;
	ssi_time_t time = 0;
	ssi_time_t delta = 1.0;
	ssi_time_t stream_sr = 1.0;
	while (s = samples.next ()) {
		s->time = time;
		for (ssi_size_t i = 0; i < s->num; i++) {
			s->streams[i]->sr = stream_sr;
		}
		time += delta;
	}

	// create trigger stream
	ssi_stream_t trigger;
	ssi_time_t trigger_sr = 1.0;
	ssi_size_t trigger_num = ssi_cast (ssi_size_t, trigger_sr * time + 0.5);
	ssi_stream_init (trigger, trigger_num, 1, sizeof (ssi_real_t), SSI_REAL, trigger_sr);
	ssi_real_t *ptr = ssi_pcast (ssi_real_t, trigger.ptr);
	for (ssi_size_t i = 0; i < trigger_num; i++) {
		*ptr++ = ssi_cast (ssi_real_t, ssi_random (0, 1.0));
	}
	ssi_print ("1st trigger stream\n");
	ssi_stream_print (trigger, stdout);

	// apply trigger stream
	ISTrigger strigger (&samples);
	strigger.setTriggerStream (0, trigger, 0.25f);

	// fill second trigger stream	
	ptr = ssi_pcast (ssi_real_t, trigger.ptr);
	for (ssi_size_t i = 0; i < trigger_num; i++) {
		*ptr++ = ssi_cast (ssi_real_t, ssi_random (0, 1.0));
	}
	ssi_print ("2nd trigger stream\n");
	ssi_stream_print (trigger, stdout);

	// apply second trigger stream
	strigger.setTriggerStream (1, trigger, 0.75f);

	ModelTools::PrintSamples (strigger);

	return true;
}

bool ex_elan(void *args) {

	const ssi_char_t *filepath = "in\\test.eaf";

	ElanDocument *elanDoc = ElanDocument::Read(filepath);
	ElanDocument::iterator tier;
	for (tier = elanDoc->begin(); tier != elanDoc->end(); tier++) {
		ssi_print("TIER: %s\n", tier->name());
		ElanTier::iterator anno;
		for (anno = tier->begin(); anno != tier->end(); anno++) {
			ssi_print("\t%d-%d:%s\n", anno->from, anno->to, anno->value.str());
		}
	}

	elanDoc->print();
	elanDoc->write("check.eaf");

	ElanTier &tierByName = (*elanDoc)["Transcription"];
	ssi_print("TIER %s has %u entries\n", tierByName.name(), tierByName.size());

	ElanTier packedTier("PackedTranscription");
	tierByName.pack(packedTier);
	packedTier.print();
	packedTier.unify("Speech");
	packedTier.print();

	ElanTier myTier("MyTier");
	ElanAnnotation a;
	ssi_size_t last_to = 1000;
	for (int i = 0; i < 10; i++) {
		a.from = last_to + ssi_random(1000u);
		a.to = a.from + ssi_random(1000u);
		last_to = a.to;
		a.value = "label";
		myTier.push_back(a);
	}
	myTier.print();
	myTier.sort();
	myTier.print();

	ElanTier myTierSplit("MyTierSplit");
	myTier.split(myTierSplit, 1000, 500, 500);
	myTierSplit.print();

	// to create sample list use:
	// ElanTools::LoadSampleList(samples, stream, tier, "user", false); -> label names are kept
	// ElanTools::LoadSampleList(samples, stream, tier, "user", false); -> label names are overwritten with tier name

	ElanDocument myDoc;
	myDoc.media().push_back(elanDoc->media()[0]);
	myDoc.media().push_back(elanDoc->media()[1]);
	myDoc.push_back(myTierSplit);
	myDoc.write("out\\my.eaf");

	delete elanDoc;

	return true;
}

void CreateMissingData (SampleList &samples, double prob) {

	ssi_size_t n_streams = samples.getStreamSize ();
	ssi_sample_t *sample = 0;
	samples.reset ();
	while (sample = samples.next ()) {
		for (ssi_size_t nstrm = 0; nstrm < n_streams; nstrm++) {
			if (ssi_random () > prob) {							
				ssi_stream_reset (*sample->streams[nstrm]);
			}
		}
	}
	samples.setMissingData (true);
}

void PlotSamples (ISamples &samples, const ssi_char_t *name) {

	ssi_size_t n_classes = samples.getClassSize ();	
	PaintData **plots = new PaintData *[n_classes];
	
	for (ssi_size_t n_class = 0; n_class < n_classes; n_class++) {

		plots[n_class] = new PaintData;

		ssi_size_t n_samples = samples.getSize (n_class);
		ssi_size_t n_streams = samples.getStreamSize ();		

		ssi_stream_t stream;
		ssi_stream_init(stream, n_samples * n_streams, 2, sizeof(ssi_real_t), SSI_REAL, 0);

		ssi_real_t *data = ssi_pcast(ssi_real_t, stream.ptr);

		ISSelectClass samples_sel (&samples);
		samples_sel.setSelection (n_class);
		samples_sel.reset ();
		ssi_sample_t *sample = 0;
		ssi_real_t *data_ptr = data;
		while (sample = samples_sel.next ()) {
			for (ssi_size_t n_stream = 0; n_stream < n_streams; n_stream++) {				
				memcpy (data_ptr, sample->streams[n_stream]->ptr, 2 * sizeof(ssi_real_t));
				data_ptr += 2;
			}
		}

		plots[n_class]->setBackground(n_class == 0, IPainter::ITool::COLORS::BLACK);
		plots[n_class]->setPointSize(10);
		plots[n_class]->setLimits(-0.2f, 1.2f);
		plots[n_class]->setData(stream, PaintData::TYPE::SCATTER);		
		plots[n_class]->setBrush(ssi_rgb(COLORS[n_class][0],COLORS[n_class][1],COLORS[n_class][2]));
		plots[n_class]->setPen(IPainter::ITool::COLORS::BLACK);

		ssi_stream_destroy(stream);
	}

	ssi_print ("\n\n\tpress enter to close plot\n\n");
	getchar ();

	for (ssi_size_t n_class = 0; n_class < n_classes; n_class++) {
		delete plots[n_class];
	}
	delete[] plots;

}

