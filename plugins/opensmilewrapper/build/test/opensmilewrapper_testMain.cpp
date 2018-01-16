// Main.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/09/21 
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
#include "ssiopensmilewrapper.h"
#include "audio\include\ssiaudio.h"
using namespace ssi;

#ifdef USE_SSI_LEAK_DETECTOR
#include "SSI_LeakWatcher.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

bool ex_online(void *args);
bool ex_offline(void *args);

#define CONFIG "IS13_ComParE_Voc"
#define FRAME "0.025s"
#define DELTA "0.0s"

int main() {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

		ssi_print("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

		Factory::RegisterDLL("frame");
		Factory::RegisterDLL("opensmilewrapper");
		Factory::RegisterDLL("graphic");
		Factory::RegisterDLL("audio");
		Factory::RegisterDLL("ioput");
		Factory::RegisterDLL("event");

		Exsemble ex;
		ex.add(ex_online, 0, "ONLINE", "");
		ex.add(ex_offline, 0, "OFFLINE", "");
		ex.show();

		Factory::Clear();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}

bool ex_online(void *args) {

	char *name = CONFIG;
	char *config = ssi_strcat(CONFIG, ".conf");

	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	WavReader *audio = ssi_create(WavReader, 0, true);
	audio->getOptions()->scale = false;
	audio->getOptions()->setPath("audio.wav");
	audio->getOptions()->loop = true;
	ITransformable *audio_p = frame->AddProvider(audio, SSI_AUDIO_PROVIDER_NAME);
	frame->AddSensor(audio);

	OSWrapper *opensmile = ssi_create(OSWrapper, "oswrapper", true);
	opensmile->getOptions()->enableConsole = false;
#ifdef DEBUG
	opensmile->getOptions()->logLevel = 1;
	opensmile->getOptions()->setLogFile("opensmile.log");
#else
	opensmile->getOptions()->logLevel = 0;
#endif
	opensmile->getOptions()->setConfigPath(config);
	opensmile->getOptions()->continuous = true;
	ITransformable *opensmile_t = frame->AddTransformer(audio_p, opensmile, FRAME, DELTA);

	FileWriter *writer = ssi_create(FileWriter, 0, true);
	writer->getOptions()->setPath(CONFIG);
	writer->getOptions()->type = File::ASCII;
	writer->getOptions()->setDelim(";");
	writer->getOptions()->overwrite = false;
	frame->AddConsumer(opensmile_t, writer, "1");

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();

	delete[] config;

	return true;
}

bool ex_offline(void *args) {

	char *name = CONFIG;
	char *config = ssi_strcat(CONFIG, ".conf");

	ssi_stream_t wav;
	ssi_stream_t features;

	WavTools::ReadWavFile("audio.wav", wav);

	OSWrapper *opensmile = ssi_create(OSWrapper, 0, false);
	opensmile->getOptions()->continuous = true;
	opensmile->getOptions()->setConfigPath(config);

	SignalTools::Transform(wav, features, *opensmile, FRAME, DELTA);
	FileTools::WriteStreamFile(File::ASCII, name, features, ";");

	ssi_stream_destroy(wav);
	ssi_stream_destroy(features);

	delete[] config;

	return true;
}