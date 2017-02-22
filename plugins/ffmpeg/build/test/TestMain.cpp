// TestMain.cpp
// author: Florian Lingenfelser <lingenfelser@hcm-lab.de>
// created: 2013/08/06
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
#include "ssiffmpeg.h"
#include "audio\include\ssiaudio.h"
#include "camera\include\ssicamera.h"
using namespace ssi;

//#include <vld.h>

bool ex_output_file (void *args);
bool ex_input_file (void *args);
bool ex_output_stream (void *args);
bool ex_input_stream (void *args);
bool ex_offline(void *args);

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

	Factory::RegisterDLL ("ssiframe");
	Factory::RegisterDLL ("ssiffmpeg");
	Factory::RegisterDLL ("ssigraphic");
	Factory::RegisterDLL ("ssiioput");
	Factory::RegisterDLL ("ssisignal");
	Factory::RegisterDLL ("ssievent");
	Factory::RegisterDLL ("ssicamera");
	Factory::RegisterDLL ("ssiaudio");

	Exsemble ex;
	ex.add(ex_output_file, 0, "FFMPEG", "Output file");
	ex.add(ex_input_file, 0, "FFMPEG", "Input file");
	ex.add(ex_output_stream, 0, "FFMPEG", "Output stream");
	ex.add(ex_input_stream, 0, "FFMPEG", "Input stream");
	ex.add(ex_offline, 0, "FFMPEG", "Offline");
	ex.show();
	
	Factory::Clear ();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}

bool ex_output_file (void *args) {

	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	// audio	
	Audio *microphone = ssi_create (Audio, "audio", true);
	microphone->getOptions()->scale = true;
	ITransformable *audio = frame->AddProvider(microphone, SSI_AUDIO_PROVIDER_NAME);	
	frame->AddSensor(microphone);

	// camera	
	Camera *camera = ssi_create (Camera, "camera", true);
	camera->getOptions()->flip = true;
	ITransformable *video = frame->AddProvider(camera, SSI_CAMERA_PROVIDER_NAME, 0, "2.0s");	
	frame->AddSensor(camera);

	// writer	

	FFMPEGWriter *ffmpeg = 0;
	
	ffmpeg = ssi_create(FFMPEGWriter, "writer-mp4", true);
	ffmpeg->getOptions()->setUrl("out.mp4");
	frame->AddConsumer(video, ffmpeg, "1");

	ffmpeg = ssi_create(FFMPEGWriter, "writer+a-mp4", true);
	ffmpeg->getOptions()->setUrl("out+a.mp4");
	ITransformable *ts[2] = { video, audio };
	frame->AddConsumer(2, ts, ffmpeg, "1");

	ffmpeg = ssi_create(FFMPEGWriter, "writer-mp3", true);
	ffmpeg->getOptions()->setUrl("out.mp3");
	frame->AddConsumer(audio, ffmpeg, "0.01s");

	// check
	WavWriter *wavwrite = ssi_create(WavWriter, 0, true);
	wavwrite->getOptions()->setPath("out.wav");
	frame->AddConsumer(audio, wavwrite, "0.01s");

	// plot	
	VideoPainter *cam_plot = ssi_create_id (VideoPainter, 0, "plot");
	cam_plot->getOptions()->setTitle("video");
	cam_plot->getOptions()->flip = false;
	frame->AddConsumer(video, cam_plot, "1");

	SignalPainter *aplot = ssi_create_id (SignalPainter, 0, "plot");
	aplot->getOptions()->setTitle("audio");
	aplot->getOptions()->size = 2.0;		
	aplot->getOptions()->type = PaintSignalType::AUDIO;
	frame->AddConsumer(audio, aplot, "0.01s");

	// run framework
	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 800);

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();

	return true;
}

bool ex_input_file (void *args) {

	if (! (ssi_exists ("out.mp4") && ssi_exists("out+a.mp4") && ssi_exists ("out.mp3"))) {
		ex_output_file (args);
	}

	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	ssi_time_t offset = 0.0;

	FFMPEGReader *ffmpeg = 0;
	
	ffmpeg = ssi_create(FFMPEGReader, "read-mp3", true);
	//ffmpeg->setLogLevel(SSI_LOG_LEVEL_DEBUG);
	ffmpeg->getOptions()->setUrl("out.mp3");
	ffmpeg->getOptions()->stream = false;
	ffmpeg->getOptions()->buffer = 1.0;
	ffmpeg->getOptions()->ablock = 0.01;
	ffmpeg->getOptions()->offset = offset;
	ITransformable *audio = frame->AddProvider(ffmpeg, SSI_FFMPEGREADER_AUDIO_PROVIDER_NAME);
	frame->AddSensor(ffmpeg);

	ffmpeg = ssi_create(FFMPEGReader, "read-mp4", true);
	//ffmpeg->setLogLevel(SSI_LOG_LEVEL_DEBUG);
	ffmpeg->getOptions()->setUrl("out.mp4");
	ffmpeg->getOptions()->stream = false;
	ffmpeg->getOptions()->buffer = 1.0;
	ffmpeg->getOptions()->offset = offset;
	ITransformable *video = frame->AddProvider(ffmpeg, SSI_FFMPEGREADER_VIDEO_PROVIDER_NAME);
	frame->AddSensor(ffmpeg);

	ffmpeg = ssi_create(FFMPEGReader, "read+a-mp4", true);
	//ffmpeg->setLogLevel(SSI_LOG_LEVEL_DEBUG);
	ffmpeg->getOptions()->setUrl("out+a.mp4");
	ffmpeg->getOptions()->stream = false;
	ffmpeg->getOptions()->buffer = 1.0;
	ffmpeg->getOptions()->offset = offset;
	ITransformable *video2 = frame->AddProvider(ffmpeg, SSI_FFMPEGREADER_VIDEO_PROVIDER_NAME);
	ITransformable *audio2 = frame->AddProvider(ffmpeg, SSI_FFMPEGREADER_AUDIO_PROVIDER_NAME);
	frame->AddSensor(ffmpeg);
	
	VideoPainter *vpaint = 0;
	
	vpaint = ssi_create_id(VideoPainter, 0, "plot");
	vpaint->getOptions()->setTitle("video");
	vpaint->getOptions()->flip = false;
	frame->AddConsumer(video, vpaint, "1");

	vpaint = ssi_create_id(VideoPainter, 0, "plot");
	vpaint->getOptions()->setTitle("video");
	vpaint->getOptions()->flip = false;
	frame->AddConsumer(video2, vpaint, "1");

	SignalPainter *apaint;
	
	apaint = ssi_create_id (SignalPainter, 0, "plot");
	apaint->getOptions()->setTitle("audio");
	apaint->getOptions()->size = 3.0;		
	apaint->getOptions()->type = PaintSignalType::AUDIO;
	frame->AddConsumer(audio, apaint, "0.1s");

	apaint = ssi_create_id (SignalPainter, 0, "plot");
	apaint->getOptions()->setTitle("audio");
	apaint->getOptions()->size = 3.0;		
	apaint->getOptions()->type = PaintSignalType::AUDIO;
	frame->AddConsumer(audio2, apaint, "0.1s");

	WavWriter *wavwrite = ssi_create (WavWriter, 0, true);
	wavwrite->getOptions()->setPath("check.wav");
	frame->AddConsumer(audio, wavwrite, "0.1s");

	AudioPlayer *aplay = ssi_create (AudioPlayer, "aplayer", true);
	aplay->getOptions()->numBuffers = 64;
	aplay->getOptions()->bufferSizeSamples = 265;
	frame->AddConsumer(audio, aplay, "0.01s");
	
	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 1, 0, 650, 0, 400, 800);

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();

	return true;
}

bool ex_output_stream (void *args) {

	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	// audio	
	Audio *microphone = ssi_create (Audio, "audio", true);
	microphone->getOptions()->scale = true;
	microphone->getOptions()->sr = 16000;
	ITransformable *audio = frame->AddProvider(microphone, SSI_AUDIO_PROVIDER_NAME);	
	frame->AddSensor(microphone);

	// camera	
	Camera *camera = ssi_create (Camera, "camera", true);
	camera->getOptions()->params.widthInPixels = 320;
	camera->getOptions()->params.heightInPixels = 240;
	camera->getOptions()->params.framesPerSecond = 25.0;
	ITransformable *video = frame->AddProvider(camera, SSI_CAMERA_PROVIDER_NAME, 0, "2.0s");
	frame->AddSensor(camera);	

	// video writer		
	FFMPEGWriter *vwrite = ssi_create  (FFMPEGWriter, "vstreamout", true);	
	vwrite->setLogLevel(SSI_LOG_LEVEL_DEBUG);
	vwrite->getOptions()->setUrl("udp://127.0.0.1:1111");	
	vwrite->getOptions()->setFormat("mpegts");
	vwrite->getOptions()->video_bit_rate_kb = 256;	
	vwrite->getOptions()->stream = true;
	frame->AddConsumer(video, vwrite, "1");

	// audio writer		
	FFMPEGWriter *awrite = ssi_create  (FFMPEGWriter, "astreamout", true);	
	//awrite->setLogLevel(SSI_LOG_LEVEL_DEBUG);
	awrite->getOptions()->setUrl("udp://127.0.0.1:2222");
	awrite->getOptions()->setFormat("mpegts");
	awrite->getOptions()->audio_bit_rate_kb = 64;	
	awrite->getOptions()->stream = true;
	frame->AddConsumer(audio, awrite, "0.04s");

	// plot
	SignalPainter *audio_plot = ssi_create_id (SignalPainter, 0, "plot");
	audio_plot->getOptions()->setTitle("audio");
	audio_plot->getOptions()->size = 2.0;		
	audio_plot->getOptions()->type = PaintSignalType::AUDIO;
	frame->AddConsumer(audio, audio_plot, "0.01s");
	
	VideoPainter *cam_plot = ssi_create_id (VideoPainter, 0, "plot");
	cam_plot->getOptions()->setTitle("video");
	cam_plot->getOptions()->flip = false;
	frame->AddConsumer(video, cam_plot, "1");

	// run framework
	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 800);

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();

	return true;
}

bool ex_input_stream (void *args) {

	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	FFMPEGReader *vread = ssi_create (FFMPEGReader, "vstreamin", true);
	//avread->SetLogLevel(SSI_LOG_LEVEL_DEBUG);
	//vread->getOptions()->setUrl("my-video.sdp");
	vread->getOptions()->setUrl("udp://127.0.0.1:1111");
	vread->getOptions()->stream = true;
	vread->getOptions()->buffer = 1.0;
	vread->getOptions()->fps = 25.0;
	vread->getOptions()->width = 320;
	vread->getOptions()->height = 240;
	ITransformable *video = frame->AddProvider(vread, SSI_FFMPEGREADER_VIDEO_PROVIDER_NAME);	
	frame->AddSensor(vread);

	FFMPEGReader *aread = ssi_create (FFMPEGReader, "astreamin", true);
	//avread->SetLogLevel(SSI_LOG_LEVEL_DEBUG);
	//aread->getOptions()->setUrl("my-audio.sdp");
	aread->getOptions()->setUrl("udp://127.0.0.1:2222");
	aread->getOptions()->stream = true;
	aread->getOptions()->buffer = 1.0;
	aread->getOptions()->asr = 16000;
	ITransformable *audio = frame->AddProvider(aread, SSI_FFMPEGREADER_AUDIO_PROVIDER_NAME);	
	frame->AddSensor(aread);
	
	// plot
	SignalPainter *audio_plot = ssi_create_id (SignalPainter, 0, "plot");
	audio_plot->getOptions()->setTitle("audio");
	audio_plot->getOptions()->size = 2.0;		
	audio_plot->getOptions()->type = PaintSignalType::AUDIO;
	frame->AddConsumer(audio, audio_plot, "0.01s");
	
	VideoPainter *cam_plot = ssi_create_id (VideoPainter, 0, "plot");
	cam_plot->getOptions()->setTitle("video");
	cam_plot->getOptions()->flip = false;
	frame->AddConsumer(video, cam_plot, "1");

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 800);

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();
	
	return true;
}

bool ex_offline(void *args) {

	const ssi_char_t *filename = "out.mp4";
	if (!ssi_exists(filename)) {
		ex_output_file (args);
	}

	FFMPEGReader *reader = ssi_create(FFMPEGReader, 0, true);
	reader->getOptions()->setUrl(filename);
	reader->getOptions()->bestEffort = true;

	FFMPEGWriter *writer = ssi_create(FFMPEGWriter, 0, true);
	writer->getOptions()->setUrl("out_copy.mp4");

	FileProvider provider(writer, 0);
	reader->setProvider(SSI_FFMPEGREADER_VIDEO_PROVIDER_NAME, &provider);

	reader->connect();
	reader->start();
	reader->wait();
	Sleep(1000);
	reader->stop();
	reader->disconnect();

	return true;
}
