// MainOption.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/07/21
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

bool ex_xml (void *arg);
bool ex_indices(void *arg);

int main () {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	Exsemble ex;
	ex.add(&ex_xml, 0, "XML", "How to save/load options to an xml file.");
	ex.add(&ex_indices, 0, "INDICES", "How to parse indices.");
	ex.show();

	Factory::Clear();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}

bool ex_xml (void *arg) {

	OptionList opts;

	bool mybools[] = {false,true,false,true,false};
	opts.addOption ("mybools", &mybools, 5, SSI_BOOL, "this is my bool array");

	int myints[] = {1,2,3,4,5};
	opts.addOption("myints", &myints, 5, SSI_INT, "this is my int array");

	unsigned int myuints[] = {1,2,3,4,5};
	opts.addOption("myuints", &myuints, 5, SSI_UINT, "this is my uint array");

	float myfloats[] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
	opts.addOption("myfloats", myfloats, 5, SSI_FLOAT, "this is my float array");

	double mydoubles[] = {1.0, 2.0, 3.0, 4.0, 5.0};
	opts.addOption ("mydoubles", mydoubles, 5, SSI_DOUBLE, "this is my double array");

	char mychar = 'c';
	opts.addOption ("mychar", &mychar, 1, SSI_CHAR, "this is my char");

	char unsigned myuchar = 123;
	opts.addOption ("myuchar", &myuchar, 1, SSI_UCHAR, "this is my unsigned char");

	char mystring[256] = "mystring";
	opts.addOption("mystring", mystring, 256, SSI_CHAR, "this is my string", false);

	opts.print (ssiout);

	// save and load
	OptionList::SaveXML ("test.xml", opts);
	ssi_print("\n\nLOAD FROM FILE\n\n");
	OptionList::LoadXML ("test.xml", opts);

	opts.print(ssiout);	

	return true;
}

bool ex_indices(void *arg) {

	char *string = "10-15; 5 ;2-3";
	ssi_size_t n_indices;
	int *indices = ssi_parse_indices(string, n_indices, true, ";");

	for (ssi_size_t i = 0; i < n_indices; i++) {
		ssi_print("%u ", indices[i]);
	}
	ssi_print("\n");

	delete[] indices;

	return true;
}
