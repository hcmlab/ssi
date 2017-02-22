// MainFile.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/02/07
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute itand/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#include "ssi.h"
using namespace ssi;

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

bool ex_value2string(void *arg);
bool ex_event_write (void *arg);
bool ex_stream_write(void *arg);
bool ex_stream_read(void *arg);
bool ex_sample_write(void *arg);
bool ex_sample_read(void *arg);
bool ex_wav_write(void *arg);

int main () {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	Factory::RegisterDLL ("ssievent");
	Factory::RegisterDLL ("ssiioput");
	Factory::RegisterDLL ("ssisignal");
	Factory::RegisterDLL ("ssimodel");

	ssi_random_seed ();
	
	Exsemble ex;
	ex.add(&ex_value2string, 0, "VALUE2STRING", "How to convert a value to a string.");
	ex.add(&ex_event_write, 0, "EVENT_WRITE", "How to write events to a file.");
	ex.add(&ex_stream_write, 0, "STREAM_WRITE", "How to write streams to a file.");
	ex.add(&ex_stream_read, 0, "STREAM_READ", "How to read streams from a file.");
	ex.add(&ex_sample_write, 0, "SAMPLES_WRITE", "How to write samples to a file.");
	ex.add(&ex_sample_read, 0, "SAMPLES_READ", "How to read samples from a file.");
	ex.add(&ex_wav_write, 0, "WAV_WRITE", "How to write samples to a wav file.");
	ex.show();

	Factory::Clear ();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif
	
	return 0;
}

bool ex_value2string(void *arg) {

	ssi_char_t string[SSI_MAX_CHAR];

	int precision = -1;

	char chr_value = 'a';
	ssi_val2str(SSI_CHAR, &chr_value, SSI_MAX_CHAR, string, precision);
	ssi_print("%c -> %s\n", chr_value, string);

	int int_value = 6;
	ssi_val2str(SSI_INT, &int_value, SSI_MAX_CHAR, string, precision);
	ssi_print("%d -> %s\n", int_value, string);

	float flt_value = 6.12345f;
	ssi_val2str(SSI_FLOAT, &flt_value, SSI_MAX_CHAR, string, precision);
	ssi_print("%f -> %s\n", flt_value, string);

	double dbl_value = 6.987654321;
	ssi_val2str(SSI_DOUBLE, &dbl_value, SSI_MAX_CHAR, string, precision);
	ssi_print("%lf -> %s\n", dbl_value, string);

	return true;
}

bool ex_event_write(void *arg) {

	ssi_char_t string[SSI_MAX_CHAR];

	ssi_event_t e_string;
	ssi_sprint (string, "foo");
	ssi_event_init (e_string, SSI_ETYPE_STRING, Factory::AddString ("foo"), Factory::AddString ("string"));
	ssi_event_adjust (e_string, ssi_strlen (string) + 1);
	ssi_strcpy (e_string.ptr, string);

	ssi_event_t e_floats;
	ssi_event_init (e_floats, SSI_ETYPE_TUPLE, Factory::AddString ("foo"), Factory::AddString ("floats"));
	ssi_event_adjust (e_floats, 5 * sizeof (ssi_real_t));
	ssi_real_t *ptr = ssi_pcast (ssi_real_t, e_floats.ptr);
	for (ssi_size_t i = 0; i < e_floats.tot / sizeof (ssi_real_t); i++) {
		*ptr++ = ssi_cast (ssi_real_t, i);
	}

	ssi_event_t e_tuple;
	ssi_event_init (e_tuple, SSI_ETYPE_MAP, Factory::AddString ("foo"), Factory::AddString ("tuple"));
	ssi_event_adjust (e_tuple, 3 * sizeof (ssi_event_map_t));
	ssi_event_map_t *tuple = ssi_pcast (ssi_event_map_t, e_tuple.ptr);
	for (ssi_size_t i = 0; i < e_tuple.tot / sizeof (ssi_event_map_t); i++) {
		ssi_sprint (string, "foo-%u", i);
		tuple[i].id = Factory::AddString (string);
		tuple[i].value = ssi_cast (ssi_real_t, i);		
	} 

	FileEventsOut eout;	
	eout.open ("foo", File::ASCII);
	eout.write (e_string);
	eout.write (e_floats);
	eout.write (e_tuple);
	eout.close ();

	ssi_event_destroy (e_string);
	ssi_event_destroy (e_floats);
	ssi_event_destroy (e_tuple);

	return true;
}

bool ex_stream_write(void *arg) {
		
	File::SetLogLevel (SSI_LOG_LEVEL_DEBUG);

	{
		ssi_stream_t stream;
		ssi_stream_init (stream, 20, 5, sizeof (int), SSI_INT, 1.0);
		int *ptr = ssi_pcast (int, stream.ptr);
		for (ssi_size_t i = 0; i < 100; i++) {
			*ptr++ = ssi_cast (int, i);
		}
		FileTools::WriteStreamFile (File::ASCII, "stream_i.txt", stream);
		ssi_stream_destroy (stream);
	}

	{
		ssi_stream_t stream;
		ssi_stream_init (stream, 20, 5, sizeof (int), SSI_INT, 1.0);
		int *ptr = ssi_pcast (int, stream.ptr);
		for (ssi_size_t i = 0; i < 100; i++) {
			*ptr++ = ssi_cast (int, i);
		}
		FileTools::WriteStreamFile (File::BINARY, "stream_i.bin", stream);
		ssi_stream_destroy (stream);
	}

	{		
		ssi_stream_t stream;
		ssi_stream_init (stream, 20, 5, sizeof (float), SSI_FLOAT, 1.0);
		float *ptr = ssi_pcast (float, stream.ptr);
		for (ssi_size_t i = 0; i < 100; i++) {
			*ptr++ = ssi_cast (float, i);
		}
		FileTools::WriteStreamFile (File::ASCII, "stream_f.txt", stream);
		ssi_stream_destroy (stream);
	}

	{		
		ssi_stream_t stream;
		ssi_stream_init (stream, 20, 5, sizeof (float), SSI_FLOAT, 1.0);
		float *ptr = ssi_pcast (float, stream.ptr);
		for (ssi_size_t i = 0; i < 100; i++) {
			*ptr++ = ssi_cast (float, i);
		}
		FileTools::WriteStreamFile (File::BINARY, "stream_f.bin", stream);
		ssi_stream_destroy (stream);		
	}

	return true;
}

bool ex_stream_read(void *arg) {

	File::SetLogLevel (SSI_LOG_LEVEL_DEBUG);

	{
		ssi_stream_t stream;
		FileTools::ReadStreamFile ("stream_i.txt", stream);
		FileTools::WriteStreamFile (File::ASCII, "", stream);
		ssi_stream_destroy (stream);
	}

	{
		ssi_stream_t stream;
		FileTools::ReadStreamFile ("stream_i.bin", stream);
		FileTools::WriteStreamFile (File::ASCII, "", stream);
		ssi_stream_destroy (stream);
	}

	{
		ssi_stream_t stream;
		FileTools::ReadStreamFile ("stream_f.txt", stream);
		FileTools::WriteStreamFile (File::ASCII, "", stream);
		ssi_stream_destroy (stream);
	}

	{
		ssi_stream_t stream;
		FileTools::ReadStreamFile ("stream_f.bin", stream);
		FileTools::WriteStreamFile (File::ASCII, "", stream);
		ssi_stream_destroy (stream);
	}

	return true;
}

bool ex_sample_write(void *arg) {

	File::SetLogLevel (SSI_LOG_LEVEL_DEBUG);

	ssi_size_t n_classes = 2;
	ssi_size_t n_samples = 2;
	ssi_size_t n_streams = 2;
	ssi_real_t distr[][3] = { 0.0f, 0.0f, 0.5f, 1.0f, 1.0f, 0.5f };
	SampleList s;	

	ModelTools::CreateTestSamples (s, n_classes, n_samples, n_streams, distr, "user");
	ModelTools::SaveSampleList (s, "samples.txt", File::ASCII);
	ModelTools::SaveSampleList (s, "samples.bin", File::BINARY);

	return true;
}

bool ex_sample_read(void *arg) {

	File::SetLogLevel (SSI_LOG_LEVEL_BASIC);

	{
		SampleList s;
		ModelTools::LoadSampleList (s, "samples.txt");
		ModelTools::PrintSamples (s);
	}

	{
		SampleList s;
		ModelTools::LoadSampleList (s, "samples.bin");
		ModelTools::PrintSamples (s);
	}

	return true;
}

bool ex_wav_write(void *arg) {

	// create sample
	ssi_stream_t wave;
	ssi_stream_init (wave, 0, 1, sizeof (ssi_real_t), SSI_REAL, 44100.0);
	ssi_time_t fs[] = {440.0};
	ssi_real_t as[] = {1.0f};
	SignalTools::Series (wave, 5.0);
	SignalTools::Sine (wave, fs, as);

	// write wav
	WavTools::WriteWavFile ("test.wav", wave);

	// read wav
	ssi_stream_t check;
	WavTools::ReadWavFile ("test.wav", check, false);

	ssi_stream_destroy (wave);
	ssi_stream_destroy (check);

	return true;
}
