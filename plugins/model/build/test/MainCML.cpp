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
#include "ssimongo/include/ssimongo.h"
#include "ssicml/include/ssicml.h"
using namespace ssi;

bool ex_read(void *args);
bool ex_write(void *args);
bool ex_remove(void *args);

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

	ssi_random_seed ();

	MongoURI uri("localhost", 27017, "admin", "mongo");
	MongoClient client;
	client.connect(uri, "MyDB", false, 1000);

	if (client.is_connected())
	{
		Exsemble exsemble;
		exsemble.console(0, 0, 650, 800);
		exsemble.add(&ex_write, &client, "WRITE ANNO", "Writes a continuous/discrete/free annotation");
		exsemble.add(&ex_read, &client, "READ ANNO", "Reads a continuous/discrete/free annotation");			
		exsemble.add(&ex_remove, &client, "REMOVE ANNO", "Remove an annotation");
		exsemble.show();

		client.close();
	}
	else
	{		
		getchar();
	}

	Factory::Clear ();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif
	
	return 0;
}

bool ex_read(void *args)
{
	MongoClient *client = ssi_pcast(MongoClient, args);
	Annotation anno;
	
	if (!CMLAnnotation::Load(&anno, client, "Session", "User", "Discrete", "system"))
	{
		return false;
	}
	anno.print();

	if (!CMLAnnotation::Load(&anno, client, "Session", "User", "Continuous", "system"))
	{
		return false;
	}
	anno.print();

	if (!CMLAnnotation::Load(&anno, client, "Session", "User", "Transcription", "system"))
	{
		return false;
	}
	anno.print();

	return true;
}

bool ex_write(void *args)
{
	MongoClient *client = ssi_pcast(MongoClient, args);
	Annotation anno;

 	if (!CMLAnnotation::SetScheme(&anno, client, "Discrete"))
	{
		return false;
	}
	
	anno.empty();
	anno.add(1.0, 2.0, 0, 1.0f);
	anno.add(2.0, 3.0, 0, 1.0f);
	anno.add(3.0, 4.0, 1, 1.0f);
	anno.add(4.0, 5.0, -1, 1.0f);
	anno.add(5.0, 6.0, 1, 1.0f);
	if (!CMLAnnotation::Save(&anno, client, "Session", "User", "Discrete", "system"))
	{
		return false;
	}
	anno.print();

	if (!CMLAnnotation::SetScheme(&anno, client, "Continuous"))
	{
		return false;
	}

	anno.empty();
	anno.add(0.0f, 1.0f);
	anno.add(0.25f, 1.0f);
	anno.add(0.5f, 1.0f);
	anno.add(1.0f, 1.0f);
	if (!CMLAnnotation::Save(&anno, client, "Session", "User", "Continuous", "system"))
	{
		return false;
	}
	anno.print();

	if (!CMLAnnotation::SetScheme(&anno, client, "Transcription"))
	{
		return false;
	}

	anno.empty();
	anno.add(1.0, 2.0, "We", 1.0f);
	anno.add(2.0, 3.0, "Lovè", 1.0f);
	anno.add(3.0, 4.0, "Tobi", 1.0f);
	anno.add(4.0, 5.0, "!", 1.0f);
	if (!CMLAnnotation::Save(&anno, client, "Session", "User", "Transcription", "system"))
	{
		return false;
	}
	anno.print();

	return true;
}

bool ex_remove(void *args)
{
	MongoClient *client = ssi_pcast(MongoClient, args);

	if (!CMLAnnotation::Remove(client, "Session", "User", "Discrete", "system"))
	{
		return false;
	}

	if (!CMLAnnotation::Remove(client, "Session", "User", "Continuous", "system"))
	{
		return false;
	}

	if (!CMLAnnotation::Remove(client, "Session", "User", "Transcription", "system"))
	{
		return false;
	}

	return true;
}

