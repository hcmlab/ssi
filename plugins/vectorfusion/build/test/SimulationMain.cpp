// TestMain.cpp (TEST)
// author: Florian Lingenfelser <florian.lingenfelser@informatik.uni-augsburg.de>
// created: 2011/10/20
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
#include "ssivectorfusion.h"
using namespace ssi;

#include "EventList.h"
#include "TheFakedFramework.h"
//#include <vld.h>

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

ssi_char_t string[SSI_MAX_CHAR];

void ex_Simulation ();

#define CONSOLE_WIDTH 650
#define CONSOLE_HEIGHT 600

int main (int argc, char *argv[]) {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	ssi_random_seed ();

	Factory::RegisterDLL ("ssievent");
	Factory::RegisterDLL ("ssivectorfusion");
	Factory::RegisterDLL ("ssigraphic");
	Factory::RegisterDLL ("ssiioput");

	ssi::Factory::Register(TheFakedFramework::GetCreateName(), TheFakedFramework::Create);

	ex_Simulation ();

	ssi_print ("\n\n\tpress a key to quit\n");
	getchar ();

	Factory::Clear ();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}

void ex_Simulation () {

	TheFakedFramework *frame = ssi_pcast (TheFakedFramework, Factory::GetFramework ());

	VectorFusion *vf = ssi_create (VectorFusion, 0, true);
	vf->getOptions()->paint = true;
	vf->getOptions()->dimension = 1;
	vf->getOptions()->setPosition(400, 0, 400, 800);

	ssi_event_t e;
	ssi_event_init (e, SSI_ETYPE_MAP, Factory::AddString ("sender"), Factory::AddString ("event"));
	ssi_event_adjust (e, sizeof (ssi_event_map_t));
	ssi_event_map_t *e_tuple = ssi_pcast (ssi_event_map_t, e.ptr);
	e_tuple->id = Factory::AddString("value");

	EventList list (1);
	list.push (e);

	ssi_size_t time_ms = 0;
	ssi_size_t delta_ms = 500;
	
	vf->listen_enter();
	for (int i = 0; i < 100; i++) {

		ssi_print ("%d\n", i);

		ssi_event_t *e = list.get (0);
		e->time = time_ms;
		ssi_event_map_t *e_tuple = ssi_pcast (ssi_event_map_t, e->ptr);
		e_tuple->value =(float) ssi_random(0, 1);

		time_ms += delta_ms;
		frame->SetElapsedTimeMs(time_ms);

		vf->update(list, 1, time_ms);

		Sleep (100);
	}
	vf->listen_flush();
}
