// Main.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2007/11/23 
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
#include "ssimlp.h"
using namespace ssi;

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

#define DATA_DIR "data"

#define USERNAME "user"
#define N_ITERATIONS 3

#define N_GESTURES 3
ssi_char_t *GESTURES[3] = { "circle" , "rectangle" , "triangle" };

#define SAMPLESNAME "gesture.samples"
#define CLASSIFIER "dollar"
#define ANNOTATION "trigger"

MLP *create (const ssi_char_t *dir);
void record (MLP *pipe);
void train (MLP *pipe, const ssi_char_t *anno, const ssi_char_t *dir);
void classify (MLP *pipe, const ssi_char_t *dir);
void expfeat (MLP *pipe, const ssi_char_t *anno, const ssi_char_t *filename);

Event notifyEventCallback (true, true);

class EventCallback : public MLPIEventCallback {	
public:
	void call (ssi_time_t time, ssi_time_t dur, const ssi_char_t *label) {
		ssi_print ("%.2lf %.2lf %s\n", time, dur, label); 
		notifyEventCallback.release ();	
	}
};

int main () {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

#ifdef _DEBUG	
	Factory::RegisterDLL ("ssiioputd.dll");
	Factory::RegisterDLL ("ssimoused.dll");
	Factory::RegisterDLL ("ssisignald.dll");
	Factory::RegisterDLL ("ssigraphicd.dll");
	Factory::RegisterDLL ("ssimodeld.dll");
	Factory::RegisterDLL ("ssimlpd.dll");
#else
	Factory::RegisterDLL ("ssiioput.dll");
	Factory::RegisterDLL ("ssimouse.dll");
	Factory::RegisterDLL ("ssisignal.dll");
	Factory::RegisterDLL ("ssigraphic.dll");
	Factory::RegisterDLL ("ssimodel.dll");
	Factory::RegisterDLL ("ssimlp.dll");
#endif

	MLP *pipe = create (DATA_DIR);

	record (pipe);
	train (pipe, ANNOTATION, ".");
	classify (pipe, ".");
	expfeat (pipe, ANNOTATION, SAMPLESNAME);

	ssi_print ("\n\n\t\tpress enter to quit!\n");
	getchar ();

	Factory::ClearObjects ();
	Factory::Clear ();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}

MLP *create (const ssi_char_t *dir) {

	MLP *pipe = ssi_create (MLP, "pipe", true);
	pipe->setDir(dir);

	Mouse *mouse = ssi_create (Mouse, "mouse", true);
	mouse->getOptions()->mask = Mouse::LEFT;

	Dollar$1 *dollar = ssi_create (Dollar$1, "dollar", true);
	Selector *select = ssi_create (Selector, "select", true);
	ssi_size_t inds[2] = {0,1};
	select->getOptions()->set(2, inds);
	
	pipe->addSensor("mouse", mouse);
	pipe->addSignal("cursor", "mouse", SSI_MOUSE_CURSOR_PROVIDER_NAME, 0, 0.2, 0, MLP::SSI);
	pipe->addSignal("button", "mouse", SSI_MOUSE_BUTTON_PROVIDER_NAME, 0, 0.2, 0, MLP::SSI);
	pipe->addTrigger("trigger", "button", 0.2);
	pipe->addClassifier("dollar", "cursor", "trigger", dollar, select); 

	return pipe;
}

void record (MLP *pipe) {

	EventCallback callback;

	pipe->setRecord(true);		
	pipe->setPaint(true);		
	pipe->setUser(USERNAME);
	pipe->setLabel(GESTURES[0]);
	pipe->setEventCallback(&callback);
	pipe->start();

	ssi_print ("\n\n  draw gestures while pressing left mouse button\n\n\n");

	for (ssi_size_t i = 0; i < N_GESTURES; i++) {
		pipe->setLabel(GESTURES[i]);
		ssi_print ("%u times gesture '%s'\n", N_ITERATIONS, GESTURES[i]);
		for (ssi_size_t j = 0; j < N_ITERATIONS; j++) {
			notifyEventCallback.wait ();
		}
	}	
	pipe->stop();
	pipe->setEventCallback(0);
	pipe->releaseModels();
}

void train (MLP *pipe, const ssi_char_t *anno, const ssi_char_t *dir) {

	pipe->extract(CLASSIFIER, anno);
	SampleList samples;
	pipe->collect(CLASSIFIER, anno, samples);
	pipe->eval(CLASSIFIER, ssiout, samples, MLP::KFOLD, 2);
	pipe->train(CLASSIFIER, dir, samples);

}

void classify (MLP *pipe, const ssi_char_t *dir) {

	EventCallback callback;

	pipe->loadModel(dir);
	pipe->setEventCallback(CLASSIFIER, &callback);

	pipe->setRecord(false);		
	pipe->setPaint(false);		

	pipe->start();	
	getchar ();
	pipe->stop();
}

void expfeat (MLP *pipe, const ssi_char_t *anno, const ssi_char_t *filename) {

	SampleList samples;
	pipe->collect(CLASSIFIER, anno, samples);
	ModelTools::SaveSampleList (samples, filename, File::BINARY);
}
