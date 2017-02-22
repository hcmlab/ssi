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
#include "ssiehealth.h"
using namespace ssi;

#include "Serial.h"

// load libraries
#ifdef _MSC_VER 
#ifdef _DEBUG
#pragma comment(lib, "ssid.lib")
#else
#pragma comment(lib, "ssi.lib")
#endif
#endif

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

void ex_simple ();
void ex_ehealth ();

int main () {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	Factory::RegisterDLL ("ssiframe.dll");
	Factory::RegisterDLL ("ssievent.dll");
	Factory::RegisterDLL ("ssiehealth.dll");
	Factory::RegisterDLL ("ssiioput.dll");
	Factory::RegisterDLL ("ssigraphic.dll");

	ex_simple ();
	ex_ehealth ();		

	ssi_print ("\n\n\tpress a key to quit\n");
	getchar ();

	Factory::Clear ();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}

void writeCommand (Serial *serial, const char *cmd) {
	int len = strlen (cmd) + 3;
	char *buffer = new char[len];
	sprintf (buffer, "<%s>", cmd);
	serial->WriteData(buffer, len-1);
	delete[] buffer;
}

void ex_simple () {

	Serial *serial = new Serial ("COM11", CBR_115200);
	if (!serial->IsConnected()) {
		printf ("ERROR: connect failed\n");
		return;
	}

	char buffer[100];

	int n = 0;

	writeCommand (serial, "ecg");
	writeCommand (serial, "gsr");
	writeCommand (serial, "air");
	writeCommand (serial, "tmp");
	writeCommand (serial, "bpm");
	writeCommand (serial, "oxy");

	writeCommand (serial, "start");

	ssi_tic ();

	int pos = 0;
	for (int i = 0; i < 1000; i++) {
		int r = serial->ReadData(buffer + pos, 1);
		if (buffer[pos] == '\n') {	
			if (n > 0) { // skip first line
				buffer[pos] = '\0';								
				printf ("BUFFER=%s\n", buffer);	
			}
			pos = 0;
			n++;
		} else {
			++pos;
		}
	}

	ssi_size_t elapsed = ssi_toc ();
	ssi_time_t sr = n / (elapsed / 1000.0);
	printf ("\n\n%lf\n", sr);

	writeCommand (serial, "stop");

	delete serial;	

	ssi_print ("press enter to continue\n");
	getchar ();
}

void ex_ehealth () {

	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	ssi_size_t n_channels = 6;
	ssi_char_t *channels[] = {
		SSI_EHEALTH_ECG_PROVIDER_NAME,
		SSI_EHEALTH_GSR_PROVIDER_NAME,
		SSI_EHEALTH_AIR_PROVIDER_NAME,
		SSI_EHEALTH_TMP_PROVIDER_NAME,
		SSI_EHEALTH_BPM_PROVIDER_NAME,
		SSI_EHEALTH_OXY_PROVIDER_NAME
	};

	EHealth *ehealth = ssi_create (EHealth, 0, true);
	ehealth->getOptions()->port = 11;
	ITransformable **provider = new ITransformable *[n_channels];
	for (ssi_size_t i = 0; i < n_channels; i++) {
		provider[i] = frame->AddProvider(ehealth, channels[i]);	
	}
	frame->AddSensor(ehealth);
	//ehealth->setLogLevel(SSI_LOG_LEVEL_DEBUG);

	SignalPainter *plot = 0;

	for (ssi_size_t i = 0; i < n_channels; i++) {
		plot = ssi_create_id (SignalPainter, 0, "plot");
		plot->getOptions()->setTitle(channels[i]);
		plot->getOptions()->size = 10.0;		
		frame->AddConsumer(provider[i], plot, "0.25s");
	}

	FileWriter *writer = 0;

	for (ssi_size_t i = 0; i < n_channels; i++) {
		writer = ssi_create (FileWriter, 0, true);
		writer->getOptions()->type = File::ASCII;
		writer->getOptions()->setPath(channels[i]);
		frame->AddConsumer(provider[i], writer, "0.25s");
	}

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	frame->Start();	
	frame->Wait();
	frame->Stop();
	frame->Clear();
}
