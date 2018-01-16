// Main.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2016/10/19
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
#include "ssimongo.h"
using namespace ssi;

#define USER "admin"
#define PASSWORD "mongo"
#define IP "localhost"
#define PORT 27017u
#define DATABASE "MyDB"
#define COLLECTION "MyCollection"
#define TIMEOUT 1000

bool ex_write(void *arg);
bool ex_read(void *arg);
bool ex_array(void *arg);
bool ex_update(void *arg);
bool ex_remove(void *arg);
bool ex_unicode(void *args);

class MyVisitor : public MongoDocument::DocumentVisitor
{
	void visit(MongoDocument &document, void *data)
	{
		const ssi_char_t *key = ssi_pcast(ssi_char_t, data);
		int32_t value = 0;
		document.getInt32(key, value);
		ssi_print("%d\n", value);
	}
};

int main () {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	MongoURI uri(IP, PORT, USER, PASSWORD);
	MongoClient client;
	if (client.connect(uri, DATABASE, true, TIMEOUT))
	{
		Exsemble ex;
		ex.add(&ex_write, &client, "WRITE", "Write to a collection.");
		ex.add(&ex_read, &client, "READ", "Read from a collection.");
		ex.add(&ex_array, &client, "ARRAY", "Read an array from a document.");
		ex.add(&ex_update, &client, "UPDATE", "Update a collection.");
		ex.add(&ex_remove, &client, "REMOVE", "Remove a collection.");
		ex.add(&ex_unicode, &client, "UNICODE", "Create unicode table.");
		ex.show();

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

bool ex_write(void *arg) {

	MongoClient *client = (MongoClient*)arg;

	MongoCollection collection(COLLECTION);	

	MongoDocuments array;
	for (int32_t i = 0; i < 5; i++)
	{
		array.push_back(MongoDocument("value", i));
	}

	MongoDocument document;
	MongoOID oid;
	document.setOid(&oid),
	document.setBool("bool", true);
	document.setString("string", "hello world");
	document.setInt32("int32", 32);	
	document.setInt64("int64", 64);
	document.setDouble("real", 1.0);
	document.setArray("array", array);
	MongoDate date;
	document.setDate("date", &date);
	document.print();

	client->add(collection);
	collection.insert(document);

	return true;
}

bool ex_read(void *arg) {

	MongoClient *client = (MongoClient*)arg;

	MongoCollection collection(COLLECTION);	
	client->get(collection);

	MongoDocument query("string", "hello world");
	MongoDocuments documents;
	collection.find(query, documents);
	documents.print();	

	return true;
}

bool ex_array(void *arg) {

	MongoClient *client = (MongoClient*)arg;

	MongoCollection collection(COLLECTION);
	client->get(collection);

	MongoDocument query("string", "hello world");
	MongoDocument document;
	collection.findFirst(query, document);
	
	document.getArray("array", MyVisitor(), "value");

	return true;
}

bool ex_update(void *arg) {

	MongoClient *client = (MongoClient*)arg;

	MongoCollection collection(COLLECTION);
	client->get(collection);

	MongoDocument query("string", "hello world");	
	MongoDocument update;
	update.setBool("bool", false);
	update.setString("new", "string");
	collection.update(query, update);

	return true;
}

bool ex_remove(void *arg) {

	MongoClient *client = (MongoClient*)arg;

	MongoCollection collection(COLLECTION);
	client->get(collection);

	MongoDocument query("string", "hello world");
	collection.remove(query);

	return true;
}

bool ex_unicode(void *args)
{
	MongoClient *client = (MongoClient*)args;

	MongoCollection collection("Umlaute");
	client->get(collection);

	MongoOID oid("5822064f2ccb9c196615e8b3");
	MongoDocument query(&oid);

	MongoDocument document;
	collection.findFirst(query, document);

	document.print();

	const ssi_char_t *list = document.getString("list", false);
	ssi_size_t n_umlaute = ssi_split_string_count(list, ';');
	ssi_char_t **umlaute = new ssi_char_t*[n_umlaute];
	ssi_split_string(n_umlaute, umlaute, list, ';');

	ssi_char_t *reference = "ï;î;ö;ô;Ü;£;é;Ä;à;ä;ê;ù;Ö;ã;û;ë;â;ü;è;É;ç;ß";
	ssi_size_t n_umlaute_ref = ssi_split_string_count(reference, ';');
	ssi_char_t **umlaute_ref = new ssi_char_t*[n_umlaute_ref];
	ssi_split_string(n_umlaute_ref, umlaute_ref, reference, ';');

	if (n_umlaute_ref != n_umlaute)
	{
		return false;
	}

	FILE *fp = fopen("unicode.txt", "w");	
	for (ssi_size_t i = 0; i < n_umlaute; i++)
	{
		ssi_fprint(fp, "%s;%s\n", umlaute_ref[i], umlaute[i]);
	}
	fclose(fp);

	MongoDocument update;
	update.setString("test", reference);
	collection.update(query, update);

	return true;
}