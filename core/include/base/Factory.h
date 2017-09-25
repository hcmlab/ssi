// Factory.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2010/02/28
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

#pragma once

#ifndef SSI_BASE_FACTORY_H
#define SSI_BASE_FACTORY_H

#include "base/IObject.h"
#include "base/IMessage.h"
#include "base/String.h"
#include "base/ITheEventBoard.h"
#include "base/ITheFramework.h"
#include "base/IWindowManager.h"
#include "base/Array1D.h"
#include "thread/Mutex.h"

#include "ioput/xml/tinyxml.h"


#if __ANDROID__
#define HEADLESS 1
#include "base/AndroidApp.h"
#endif


#if __gnu_linux__
#define HMODULE void*
#endif

namespace ssi {

#define SSI_FACTORY_DEFAULT_ID "noname"
#define SSI_FACTORY_CONSOLE_ID "console"
#define SSI_FACTORY_REGISTER_FUNCNAME "Register"
#define SSI_FACTORY_UNREGISTER_FUNCNAME "UnRegister"

#define ssi_create_id(NAME,FILE,ID) (ssi_pcast (NAME, Factory::Create (NAME::GetCreateName (), FILE, true, ID)))
#define ssi_create(NAME,FILE,FREE) (ssi_pcast (NAME, Factory::Create (NAME::GetCreateName (), FILE, FREE)))
#define ssi_factory_create(NAME,FILE,FREE) (ssi_pcast (NAME, Factory::Create (NAME::GetCreateName (), FILE, FREE)))

class Factory {

public:

	typedef bool (register_xml_fptr_t)(TiXmlElement *element, FILE *logfile, IMessage *message);
	typedef bool (register_dll_fptr_t)(const ssi_char_t *name, FILE *logfile, IMessage *message);
	typedef IObject *(create_fptr_t)(const ssi_char_t *name, const ssi_char_t *file, bool auto_free);
	typedef bool (*register_fptr_from_dll_t)(Factory *factory, FILE *logfile, IMessage *message);
	typedef void (*unregister_fptr_from_dll_t)();
	
	#if __ANDROID__
	static bool isFactoryNull()
	{
		if(_factory == 0)return true;
		else return false;
	}
	static ssiAndroidApp* GetApp()
	{
		return GetFactory ()->getApp();
	}
	#endif

public:

	static Factory *GetFactory();
	static void SetFactory (Factory *factory) {
		_factory = factory;
	}

	static bool Register (const ssi_char_t *name,
		IObject::create_fptr_t create_fptr) {
		return GetFactory ()->register_h (name, create_fptr);
	}
	static bool RegisterDLL (const ssi_char_t *dllpath,
		FILE *logfile = 0,
		IMessage *message = 0) {
		return GetFactory ()->registerDLL (dllpath, logfile, message);
	}
	static bool RegisterXML(TiXmlElement *element,
		FILE *logfile = 0,
		IMessage *message = 0) {
		return GetFactory()->registerXML(element, logfile, message);
	}
	static bool ExportDlls (const ssi_char_t *target_dir) {
		return GetFactory ()->exportDlls (target_dir);
	}
/*
        bool registerXML(TiXmlElement *element,
                         FILE *logfile,
                         IMessage *message);

        IObject* CreateXML(TiXmlElement *element,
                           bool auto_free);

        ssi_char_t* fullDLLPath(const ssi_char_t *filepath);

        void setDownloadDirs(const ssi_char_t *srcdir, const ssi_char_t *dstdir);
*/
        static ssi_size_t GetDllNames (ssi_char_t ***names);
	static ssi_size_t GetObjectNames(ssi_char_t ***names); 
	// filter: name1,name2,obj*
	static ssi_size_t GetObjectIds(ssi_char_t ***names, const ssi_char_t *filter = 0);

	static IObject *Create(const ssi_char_t *name,
		const ssi_char_t *file = 0,
		bool auto_free = true,
		const ssi_char_t *id = 0);
		
	static IObject *CreateXML(TiXmlElement *element,
		bool auto_free = true);

	static void SetDownloadDirs(const ssi_char_t *srcdir, const ssi_char_t *dstdir)
	{
		GetFactory()->setDownloadDirs(srcdir, dstdir);
	}


	static IObject *GetObjectFromId(const ssi_char_t *id);
	static const ssi_char_t *GetObjectId(IObject *object);
	static void ClearObjects () {
		GetFactory ()->clearObjects ();
	}
	static void Clear () {
		GetFactory ()->clear ();
	}

	static ITheEventBoard *GetEventBoard (const ssi_char_t *file = 0) {
		return GetFactory ()->getEventBoard (file);
	}
	static ITheFramework *GetFramework (const ssi_char_t *file = 0) {
		return GetFactory ()->getFramework (file);
	}
	static IWindowManager *GetWindowManager() {
		return GetFactory()->getWindowManager();
	}

	static ssi_size_t AddString (const ssi_char_t *str) {
		return GetFactory ()->addString (str);
	}
	static const ssi_char_t *GetString (ssi_size_t id) {
		return GetFactory ()->getString (id);
	}
	static ssi_size_t GetStringId (const ssi_char_t *str) {
		return GetFactory ()->getStringId (str);
	}
	static ssi_size_t GetUniqueId () {
		return GetFactory ()->getUniqueId ();
	}

	static void Print(FILE *file = ssiout);

	static void SetLogLevel (int level) {
		ssi_log_level = level;
	}

private:

		// C++ 11
		// =======
		// We can use the better technique of deleting the methods
		// we don't want.
#if __GNUC__
		Factory(Factory const&)               = delete;
		void operator=(Factory const&)  = delete;
#else
		Factory(Factory const&);              // Don't Implement
		void operator=(Factory const&); 
#endif
	
	
protected:

	Factory();
	virtual ~Factory ();

	static ssi_char_t *ssi_log_name;
	static int ssi_log_level;
  

	static Factory *_factory;
	
	#if __ANDROID__
	ssiAndroidApp* android;
	ssiAndroidApp* getApp()
	{
		if(android==NULL)
		{
			android=new ssiAndroidApp();
		}
		return android;
	}
	#endif

	// register

	bool register_h (const ssi_char_t *name,
		IObject::create_fptr_t create_fptr);
	bool registerDLL (const ssi_char_t *name,
		FILE *logfile,
		IMessage *message);
	bool registerXML(TiXmlElement *element,
		FILE *logfile,
		IMessage *message);
	bool exportDlls (const ssi_char_t *target_dir);

	ssi_char_t *fullDLLPath(const ssi_char_t *filepath);

	typedef std::map<String, HMODULE> dll_handle_map_t;
	typedef std::pair<String, HMODULE> dll_handle_pair_t;
	dll_handle_map_t _dll_handle_map;

	// create

	IObject *create(const ssi_char_t *name,
		const ssi_char_t *file = 0,
		bool auto_free = true,
		const ssi_char_t *id = 0);
	ssi_char_t *getUniqueObjectId(const ssi_char_t *id);
	IObject *getObjectFromId(const ssi_char_t *id);
	const ssi_char_t *getObjectId(IObject *object);

	void setDownloadDirs(const ssi_char_t *srcdir, const ssi_char_t *dstdir);

	typedef std::map<String, IObject *> object_id_map_t;
	typedef std::pair<String, IObject *> object_id_pair_t;
	object_id_map_t _object_id_map;

	typedef std::map<String, IObject::create_fptr_t> object_create_map_t;
	typedef std::pair<String, IObject::create_fptr_t> object_create_pair_t;
	object_create_map_t _object_create_map;

	// clean up

	void clearObjects ();
	void clear ();

	// singletons

	ITheEventBoard *_board;
	ITheEventBoard *getEventBoard (const ssi_char_t *file = 0);
	ITheFramework *_frame;
	ITheFramework *getFramework (const ssi_char_t *file = 0);
	IWindowManager *_wmanager;
	IWindowManager *getWindowManager();

	// string functions

	ssi_size_t addString (const ssi_char_t *str);
	const ssi_char_t *getString (ssi_size_t id);
	ssi_size_t getStringId (const ssi_char_t *str);
	ssi_size_t getUniqueId ();

	Mutex _mutex;
	ssi_size_t _id_counter;
	ssi_char_t *_strings[SSI_FACTORY_STRINGS_MAX];
	ssi_size_t _strings_counter;

	// download directories
	ssi_char_t *_dstdir;
	ssi_char_t *_srcdir;

	// print function
	void print(FILE *file = ssiout);
};

}

#endif


