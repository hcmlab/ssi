// Main.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/04/24
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
#include "MyObject.h"
using namespace ssi;

bool ex_string (void *arg);
bool ex_array(void *arg);
bool ex_object(void *arg);
bool ex_dll(void *arg);
bool ex_notify(void *arg);

int main () {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	Exsemble ex;
	ex.add(&ex_string, 0, "STRING", "How to use 'String' class.");
	ex.add(&ex_array, 0, "ARRAY", "How to use 'EventBoard'.");
	ex.add(&ex_object, 0, "OBJECT", "How to create an object using 'Factory'.");
	ex.add(&ex_dll, 0, "DLL", "How create objects from a dll.");
	ex.add(&ex_notify, 0, "NOTIFY", "How to exchange information with other objects.");
	ex.show();

	Factory::Clear ();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif

	return 0;

}

bool ex_string (void *arg) {

	ssi_size_t id = 0;

	ssi_size_t n_strings = 10;
	ssi_char_t *strings[] = {"Fischers", "Fritze", "fischt", "frische", "Fische", "Frische", "Fische", "fischt", "Fischers", "Fritze"};

	for (ssi_size_t i = 0; i < n_strings; i++) {
		Factory::AddString (strings[i]);
	}
	for (ssi_size_t i = 0; i < n_strings; i++) {
		ssi_print ("%s[%u] ", Factory::GetString (Factory::GetStringId (strings[i])), Factory::GetStringId (strings[i]));
	}
	ssi_print ("\n");

	id = Factory::GetStringId ("invalid");

	Factory::Print();
	Factory::Clear();

	return true;
}

bool ex_array (void *arg) {

	ssi_random_seed ();

	Array1D<float> a;
	a.init (5);	
	for (float *ptr = a.ptr (); ptr < a.end (); ptr++) {	
		*ptr = ssi_cast (float, ssi_random ());
	}
	a[0] = 0;
	a.print (&Array1D<float>::ToString, ssiout);
	a.clear ();
	a.print (&Array1D<float>::ToString, ssiout, 2, 3);
	a.release ();
	a.print (&Array1D<float>::ToString);

	return true;
}

bool ex_object (void *arg) {

	Factory::Register(MyObject::GetCreateName(), MyObject::Create);

	MyObject *o = ssi_create (MyObject, "object", true);
	o->print();
	o->getOptions()->setString("hello world");
	o->getOptions()->toggle = true;
	o->print();

	o = ssi_create_id (MyObject, "object", "myobject");

	Factory::Print();
	Factory::Clear();

	return true;
}

bool ex_dll (void *arg) {

	Factory::RegisterDLL ("ssiframe");
	Factory::RegisterDLL ("ssievent");
	Factory::RegisterDLL ("ssiioput");
	Factory::RegisterDLL ("ssimouse");

	// add some objects
	ssi_char_t **object_names;
	ssi_size_t n_objects = Factory::GetObjectNames(&object_names);	
	for (ssi_size_t i = 0; i < n_objects; i++) {
		Factory::Create(object_names[i], 0, true);	
	}
	for (ssi_size_t i = 0; i < 3; i++) {
		Factory::Create(object_names[0], 0, true);
	}
	for (ssi_size_t i = 0; i < 3; i++) {
		Factory::Create(object_names[0], 0, true, "myid");
	}
	Factory::Create(object_names[0], 0, true, "another1");
	Factory::Create(object_names[0], 0, true, "another2");
	
	// retrieve object from id
	IObject *object = Factory::GetObjectFromId("myid");
	const ssi_char_t *id = Factory::GetObjectId(object);
	ssi_print("found '%s' with id '%s'\n", object->getName(), id);

	// try to retrieve an unkown object
	object = Factory::GetObjectFromId("unkown");

	// create an object without 'auto free' and try to retrieve id
	object = Factory::Create(object_names[0], 0, false, "ignore");
	id = Factory::GetObjectId(object);

	// retrieve all object ids that match filter
	ssi_char_t **ids;
	ssi_size_t n_ids = Factory::GetObjectIds(&ids, "my*,another1");
	ssi_print("SELECTEDS IDS:\n");
	for (ssi_size_t i = 0; i < n_ids; i++) {
		ssi_print("> %s\n", ids[i]);
		delete[] ids[i];
	}
	ssi_print("\n");
	delete[] ids;
		
	// output all dlls and objects
	Factory::Print();
	Factory::Clear();

	// clean
	for (ssi_size_t i = 0; i < n_objects; i++) {
		delete[] object_names[i];
	}
	delete[] object_names;

	return true;
}

bool ex_notify(void *arg) {

	Factory::Register(MyObject::GetCreateName(), MyObject::Create);

	MyObject *o = ssi_create_id (MyObject, "object", "myobject");

	o->notify(INotify::COMMAND::MESSAGE, "hello");

	Factory::Print();
	Factory::Clear();

	return true;
}
