// Factory.cpp
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

#include "base/Factory.h"
#include "event/include/TheEventBoard.h"
#include "frame/include/TheFramework.h"
#include "thread/Lock.h"
#include "ioput/file/FilePath.h"
#include "ioput/web/WebTools.h"
#include "graphic/Console.h"
#ifdef SSI_USE_SDL
#include "graphic/SDL_WindowManager.h"
#endif

#if __gnu_linux__

    #include <stdlib.h>
    #include <stdio.h>
    #include <dlfcn.h>
    #include <iostream>
	#include <fstream>

#endif

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {
	
ssi_char_t *Factory::ssi_log_name = "factory___";
int Factory::ssi_log_level = SSI_LOG_LEVEL_DEFAULT;
Factory *Factory::_factory = 0;

Factory::Factory()
	: _id_counter(1),
	_strings_counter(0),
	_board(0),
	_frame(0),
	_srcdir(0),
	_dstdir(0),
	_wmanager(0) {
		
	#if __ANDROID__
		android=new ssiAndroidApp();
	#endif

	for (ssi_size_t i = 0; i < SSI_FACTORY_STRINGS_MAX; i++) {
		_strings[i] = 0;
	}
}

Factory::~Factory () {
	delete[] _srcdir;
	delete[] _dstdir;
}

Factory *Factory::GetFactory() {
	if (_factory == 0) {
		static Factory factory;
		_factory = &factory;
		_factory->register_h(Console::GetCreateName(), Console::Create);
		_factory->create(Console::GetCreateName(), 0, true, SSI_FACTORY_CONSOLE_ID);
	}
	return _factory;
}

bool Factory::register_h (const ssi_char_t *name,
	IObject::create_fptr_t create_fptr) {
	
	ssi_msg (SSI_LOG_LEVEL_BASIC, "found '%s'", name);

	object_create_map_t::iterator it;
	std::pair<object_create_map_t::iterator,bool> ret;

	String string (name);
	ret = _object_create_map.insert (object_create_pair_t (string,create_fptr));

	if (!ret.second) {
		ssi_wrn ("already loaded '%s'", name);
		return false;
	}

	return true;
}

ssi_char_t *Factory::fullDLLPath(const ssi_char_t *filepath)
{
	ssi_char_t *filepath_x = 0;
	FilePath fp(filepath);

	const ssi_char_t *name = fp.getName();
	ssi_size_t n_name = ssi_strlen(name);

	#if _WIN32|_WIN64
	#ifdef DEBUG				
		if (ssi_strcmp(fp.getName(), "ssi", true, 3))
		{
			filepath_x = ssi_strcat(fp.getPath(), name[n_name-1] == 'd' ? ".dll" : "d.dll");
		}
		else
		{
			filepath_x = ssi_strcat(fp.getDir(), "ssi", fp.getName(), name[n_name - 1] == 'd' ? ".dll" : "d.dll");
		}
	#else
		if (ssi_strcmp(fp.getName(), "ssi", true, 3))
		{
			filepath_x = ssi_strcat(fp.getPath(), ".dll");
		}
		else
		{
			filepath_x = ssi_strcat(fp.getDir(), "ssi", fp.getName(), ".dll");
		}
	#endif
	#else //unix
		//add shared libryry postfix
	#ifdef DEBUG
		if (ssi_strcmp(fp.getName(), "ssi", true, 3))
		{
			filepath_x = ssi_strcat(fp.getPath(), name[n_name - 1] == 'd' ? ".so" : "d.so");
		}
		else
		{
			filepath_x = ssi_strcat(fp.getDir(), "ssi", fp.getName(), name[n_name - 1] == 'd' ? ".so" : "d.so");
		}
	#else 
		if (ssi_strcmp(fp.getName(), "ssi", true, 3))
		{
			filepath_x = ssi_strcat(fp.getPath(), ".so");
		}
		else
		{
			filepath_x = ssi_strcat(fp.getDir(), "ssi", fp.getName(), ".so");
		}
	#if __ANDROID__
		//add lib prefix
		char* tmp = ssi_strcat("lib", filepath_x);

		//add apps lib path
		filepath_x = ssi_strcat(android->getLibDir(), tmp);

	#else

		char* tmp = ssi_strcat("", filepath_x);
		filepath_x = tmp;

	#endif
	#endif
	#endif

	return filepath_x;
}

bool Factory::registerDLL (const ssi_char_t *name,
	FILE *logfile, 
	IMessage *message) {

	ssi_char_t *fullname = fullDLLPath(name);
	FilePath fullnameFp(fullname);
	ssi_char_t *fullpath = 0;
	if (ssi_strcmp(fullnameFp.getDir(), ""))
	{
		fullpath = _dstdir ? ssi_strcat(_dstdir, fullname) : ssi_strcpy(fullname);
	}
	else
	{
		fullpath = ssi_strcpy(fullname);
	}

	if (_srcdir && !ssi_exists(fullpath))
	{
		ssi_char_t *url = ssi_strcat(_srcdir, "/", fullname);
		ssi_msg(SSI_LOG_LEVEL_BASIC, "download '%s'", url);
#if _WIN32||_WIN64
		WebTools::DownloadFile(url, fullpath);
#endif
		delete[] url;
	}

	dll_handle_map_t::iterator it = _dll_handle_map.find (String (fullname));
	bool succeeded = true;

	#if _WIN32|_WIN64

	if (it == _dll_handle_map.end ()) 
	{

		HMODULE hDLL = ::LoadLibrary (fullname);

		succeeded = false;
		if(hDLL) {

			register_fptr_from_dll_t register_fptr = (register_fptr_from_dll_t) ::GetProcAddress (hDLL, SSI_FACTORY_REGISTER_FUNCNAME);
			
			if (register_fptr) 
			{				
				_dll_handle_map.insert (dll_handle_pair_t (String (fullname), hDLL));
				ssi_msg (SSI_LOG_LEVEL_DEFAULT, "load '%s'", fullname);
				succeeded = register_fptr (GetFactory (), logfile, message);
			}
			else 
			{
				ssi_wrn ("Register() function not found '%s'", fullname);
				DWORD loadLibraryError = GetLastError();				
				if(!FreeLibrary(hDLL))
				{
					DWORD freeLibraryError = GetLastError();
					ssi_wrn ("FreeLibrary() failed (error: %u)", freeLibraryError);
				}
				hDLL = 0;
			}
		} 
		else 
		{
			ssi_wrn ("not found '%s'", fullname);
		}
	} 
	else 
	{
		ssi_msg (SSI_LOG_LEVEL_DEFAULT, "already loaded '%s'", fullname);
	}

	#else

	if (it == _dll_handle_map.end ()) {

		void* hDLL=0;
        hDLL=dlopen(fullname, RTLD_LAZY);
		dlerror();
		
		const char* error;
		succeeded = false;
		if(hDLL) {

			register_fptr_from_dll_t register_fptr = (register_fptr_from_dll_t) dlsym(hDLL, SSI_FACTORY_REGISTER_FUNCNAME);
			
			if (!((error = dlerror()) != NULL)) {
                _dll_handle_map.insert (dll_handle_pair_t (String (fullname), hDLL));
                ssi_msg (SSI_LOG_LEVEL_DEFAULT, "register '%s'", fullname);
				succeeded = register_fptr (GetFactory (), logfile, message);
			} else {
                ssi_wrn ("Register() function not found '%s' '%s'", fullname, error);
				//char* loadLibraryError = dlerror();
				if(!dlclose(hDLL))
				{
					const char* freeLibraryError = dlerror();
					ssi_wrn ("FreeLibrary() failed (error: %s)", error);
				}
				hDLL = 0;
			}
		} else {
            ssi_wrn ("not found '%s'", fullname);
		}
	} else {
        ssi_msg (SSI_LOG_LEVEL_DEFAULT, "already loaded '%s'", fullname);
	}

	#endif

	delete[] fullname;
	delete[] fullpath;
	
	return succeeded;
}

bool Factory::registerXML(TiXmlElement *element,
	FILE *logfile,
	IMessage *message)
{
	ssi_char_t *tags[2] = { "load", "item" };
	for (ssi_size_t i = 0; i < 2; i++)
	{
		TiXmlElement *load = element->FirstChildElement(tags[i]);
		const ssi_char_t *name = 0;
		const ssi_char_t *depend = 0;
		if (load)
		{
			do
			{
				if (_srcdir && _srcdir[0] != '\0')
				{
					depend = load->Attribute("depend");
					if (depend)
					{
						ssi_size_t n_tokens = ssi_split_string_count(depend, ';');
						ssi_char_t **tokens = new ssi_char_t*[n_tokens];
						ssi_split_string(n_tokens, tokens, depend, ';');

						for (ssi_size_t i = 0; i < n_tokens; i++)
						{
							ssi_char_t *fullname = tokens[i];
							ssi_char_t *fullpath = _dstdir ? ssi_strcat(_dstdir, fullname) : ssi_strcpy(fullname);						
							if (!ssi_exists(fullpath))
							{
								ssi_char_t *url = ssi_strcat(_srcdir, "/", fullname);
								ssi_msg(SSI_LOG_LEVEL_BASIC, "download dependency '%s'", url);
#if _WIN32||_WIN64
								WebTools::DownloadFile(url, fullpath);
#endif
								delete[] url;
							}
							delete[] fullpath;
							delete[] tokens[i];
						}
						delete[] tokens;
					}
				}

				name = load->Attribute("name");
				if (!name) 
				{
					ssi_wrn("register->%s: attribute 'name' is missing", tags[i]);
					return false;
				}

				SSI_DBG(SSI_LOG_LEVEL_DEBUG, "register->%s: <name='%s'>", tags[i], name);

				registerDLL(name, logfile, message);

			} while (load = load->NextSiblingElement(tags[i]));
		}
	}

	return true;
}

ssi_size_t Factory::GetDllNames (ssi_char_t ***names) {

	*names = 0;

	ssi_size_t n = ssi_cast (ssi_size_t, GetFactory ()->_dll_handle_map.size ());

	if (n > 0) {
		(*names) = new ssi_char_t *[n];
		dll_handle_map_t::iterator it;
		ssi_size_t i = 0;
		for (it = GetFactory ()->_dll_handle_map.begin (); it != GetFactory ()->_dll_handle_map.end (); it++) {
			(*names)[i++] = ssi_strcpy (it->first.str ());
		}
	}

	return n;
}

ssi_size_t Factory::GetObjectNames (ssi_char_t ***names) {

	*names = 0;
		
	ssi_size_t n = ssi_cast (ssi_size_t, GetFactory ()->_object_create_map.size ());
	
	if (n > 0) {
		(*names) = new ssi_char_t *[n];
		object_create_map_t::iterator it;
		ssi_size_t i = 0;
		for (it = GetFactory ()->_object_create_map.begin (); it != GetFactory ()->_object_create_map.end (); it++) {
			(*names)[i++] = ssi_strcpy (it->first.str ());

		}
	}

	return n;
}

ssi_size_t Factory::GetObjectIds(ssi_char_t ***names, const ssi_char_t *filter) {

	*names = 0;
	ssi_size_t n_names = 0;

	if (!filter || filter[0] == '\0' || filter[0] == '*' ) {

		n_names = ssi_cast(ssi_size_t, GetFactory()->_object_id_map.size());

		if (n_names > 0) {
			(*names) = new ssi_char_t *[n_names];
			object_id_map_t::iterator it;
			ssi_size_t i = 0;
			for (it = GetFactory()->_object_id_map.begin(); it != GetFactory()->_object_id_map.end(); it++) {
				(*names)[i++] = ssi_strcpy(it->first.str());				
			}
		}		

	}
	else {

		ssi_size_t n = ssi_cast (ssi_size_t, GetFactory()->_object_id_map.size());

		if (n > 0) {

			ssi_char_t **select = new ssi_char_t *[n];
			for (ssi_size_t i = 0; i < n; i++) {
				select[i] = 0;
			}			

			ssi_size_t n_tokens = ssi_split_string_count(filter, ',');
			if (n_tokens > 0) {

				ssi_char_t **tokens = new ssi_char_t *[n_tokens];
				ssi_split_string(n_tokens, tokens, filter, ',');

				for (ssi_size_t i = 0; i < n_tokens; i++) {

					ssi_char_t *compare = tokens[i];
					ssi_size_t n_compare = 0;
					if (compare[0] != '\0' && compare[ssi_strlen(compare) - 1] == '*') {
						n_compare = ssi_strlen(compare) - 1;
					}
					
					object_id_map_t::iterator it;
					ssi_size_t j = 0;
					for (it = GetFactory()->_object_id_map.begin(); it != GetFactory()->_object_id_map.end(); it++) {
						if (n_compare == 0) {
							if (ssi_strcmp(it->first.str(), compare)) {
								select[j] = it->first.str();
							}
						} else {
							if (ssi_strcmp(it->first.str(), compare, true, n_compare)) {
								select[j] = it->first.str();
							}
						}
						j++;
					}
				}

				for (ssi_size_t i = 0; i < n_tokens; i++) {
					delete[] tokens[i];
				}
				delete[] tokens;
			}

			for (ssi_size_t i = 0; i < n; i++) {
				if (select[i]) {
					n_names++;
				}
			}

			*names = new ssi_char_t *[n_names];
			ssi_size_t j = 0;
			for (ssi_size_t i = 0; i < n; i++) {	
				if (select[i]) {
					(*names)[j++] = ssi_strcpy(select[i]);
				}
			}

			delete[] select;
		}
	}

	return n_names;
}

IObject *Factory::CreateXML(TiXmlElement *element,
	bool auto_free)
{
	const ssi_char_t *create = element->Attribute("create");
	if (!create) {
		ssi_wrn("%s: attribute 'create' is missing", element->Value());
		return 0;
	}
	const ssi_char_t *option = element->Attribute("option");
	if (!option || option[0] == '\0') {
		option = 0;
	}

	SSI_DBG(SSI_LOG_LEVEL_DEBUG, "%s: <create='%s' option='%s'>", element->Value(), create, option);

	IObject *object = Create(create, option, auto_free);
	if (!object) {

		// check for old format
		ssi_size_t n_tokens = ssi_split_string_count(create, '_');
		if (n_tokens == 3) {

			ssi_char_t **tokens = new ssi_char_t *[n_tokens];
			ssi_split_string(n_tokens, tokens, create, '_');

			ssi_wrn("deprecated: use '%s' instead of '%s'", tokens[2], create);

			object = Create(tokens[2], option, auto_free);

			for (ssi_size_t i = 0; i < n_tokens; i++) {
				delete[] tokens[i];
			}
			delete[] tokens;
		}

		if (!object) {
			ssi_wrn("%s: could not create object '%s'", element->Value(), create);
			return 0;
		}
	}

	TiXmlAttribute *attribute = element->FirstAttribute();
	while (attribute)
	{
		if (strcmp(attribute->Name(), "create") != 0 && strcmp(attribute->Name(), "option") != 0) {
			if (object->getOptions()->setOptionValueFromString(attribute->Name(), attribute->Value())) {
				SSI_DBG(SSI_LOG_LEVEL_DEBUG, "%s: set option <'%s'='%s'>", element->Value(), attribute->Name(), attribute->Value());
			}
		}
		attribute = attribute->Next();
	}

	return object;
}

IObject *Factory::Create(const ssi_char_t *name,
	const ssi_char_t *file,
	bool auto_free,
	const ssi_char_t *id) {

	if (id == 0) {
		ssi_size_t n = ssi_split_string_count(name, ':');
		if (n == 1) {
			return GetFactory()->create(name, file, auto_free);
		}
		if (n == 2) {
			ssi_char_t **split = new ssi_char_t *[2];
			ssi_split_string(2, split, name, ':');
			return GetFactory()->create(split[0], file, auto_free, split[1]);
			delete[] split[0];
			delete[] split[1];
			delete[] split;
		} else {
			ssi_wrn("invalid object name '%s'", name);
			return 0;
		}

	} else {
		return GetFactory()->create(name, file, auto_free, id);
	}
}

ssi_char_t *Factory::getUniqueObjectId(const ssi_char_t *id) {

	ssi_char_t *uid;

	if (id == 0) {
		uid = new ssi_char_t[10];
		ssi_sprint (uid, "%s%03u", SSI_FACTORY_DEFAULT_ID, (ssi_size_t) _object_id_map.size() + 1);
	} else {
		uid = ssi_strcpy(id);
	}
	
	if (_object_id_map.find(String(uid)) != _object_id_map.end()) {		
		ssi_char_t *uid_new = new ssi_char_t[ssi_strlen(uid) + 4];
		ssi_size_t tries = 0;
		do {
			ssi_sprint(uid_new, "%s%u", uid, ++tries);
		} while (_object_id_map.find(String(uid_new)) != _object_id_map.end());
		
		ssi_msg(SSI_LOG_LEVEL_DETAIL, "an object with id '%s' already exists, new id is '%s'", uid, uid_new);

		delete[] uid;
		uid = uid_new;		
	}

	return uid;
}

IObject *Factory::create(const ssi_char_t *name, const ssi_char_t *file, bool auto_free, const ssi_char_t *id) {

	if (strcmp (name, TheEventBoard::GetCreateName ()) == 0 && _board) {
		return _board;
	} else if (strcmp (name, TheFramework::GetCreateName ()) == 0 && _frame) {
		return _frame;
	}

	ssi_msg(SSI_LOG_LEVEL_BASIC, "create instance of '%s'", name);

	String string (name);
	object_create_map_t::iterator it;

	it = _object_create_map.find (name);

	if (it == _object_create_map.end ()) {
		ssi_wrn ("not found '%s'", name);
		return 0;
	}

	IObject *object = (*it).second (file);
	if (auto_free) {
		ssi_char_t *uid = getUniqueObjectId(id);
		ssi_msg(SSI_LOG_LEVEL_BASIC, "store instance of '%s' as '%s'", name, uid);
		_object_id_map.insert(object_id_pair_t(String(uid), object));
		delete[] uid;
	} else {
		if (id) {
			ssi_wrn("id '%s' is ignored since 'auto free' is turned off", id);
		}
	}

	if (strcmp (name, TheEventBoard::GetCreateName ()) == 0) {
		_board = ssi_pcast (TheEventBoard, object);
	} else if (strcmp (name, TheFramework::GetCreateName ()) == 0) {
		_frame = ssi_pcast (TheFramework, object);
	}

	return object;
}

void Factory::setDownloadDirs(const ssi_char_t *srcdir, const ssi_char_t *dstdir)
{
	delete[] _srcdir; _srcdir = 0;
	if (srcdir && srcdir[0] != '\0')
	{
		_srcdir = ssi_strcpy(srcdir);
	}
	delete[] _dstdir; _dstdir = 0;
	if (dstdir && dstdir[0] != '\0')
	{
		_dstdir = ssi_strcpy(dstdir);
	}

}

IObject *Factory::GetObjectFromId(const ssi_char_t *id) {
	return GetFactory()->getObjectFromId(id);
}
const ssi_char_t *Factory::GetObjectId(IObject *object) {
	return GetFactory()->getObjectId(object);
}

IObject *Factory::getObjectFromId(const ssi_char_t *id) {

	object_id_map_t::iterator it = _object_id_map.find(String(id));

	if (it != _object_id_map.end()) {
		return it->second;
	}

	ssi_wrn("an object with id '%s' does not exist", id);

	return 0;
}

const ssi_char_t *Factory::getObjectId(IObject *object) {

	if (!object) {
		return 0;
	}

	object_id_map_t::iterator it;
	for (it = _object_id_map.begin(); it != _object_id_map.end(); it++) {
		if (it->second == object) {
			return it->first.str();
		}
	}

	ssi_wrn("the requested object of type '%s' was not found", object->getName());

	return 0;
}

void Factory::clearObjects () {

	ssi_msg (SSI_LOG_LEVEL_DEFAULT, "clear objects");

	delete _board; _board = 0;
	delete _frame; _frame = 0;

	{
		object_id_map_t::iterator it;		
		for (it = _object_id_map.begin(); it != _object_id_map.end(); it++) {
			delete it->second;
		}
		_object_id_map.clear ();
	}
}

void Factory::clear () {

	ssi_msg (SSI_LOG_LEVEL_DEFAULT, "clear factory");

	ClearObjects ();

	{
		_object_create_map.clear ();
	}

	{       
		dll_handle_map_t::iterator it;
		for (it = _dll_handle_map.begin() ; it != _dll_handle_map.end(); it++) 
		{

			#if _WIN32|_WIN64
			unregister_fptr_from_dll_t unregister_fptr = (unregister_fptr_from_dll_t) ::GetProcAddress(it->second, SSI_FACTORY_UNREGISTER_FUNCNAME);
			if (unregister_fptr)
			{
				ssi_msg(SSI_LOG_LEVEL_DEFAULT, "unregister '%s'", it->first.str());
				unregister_fptr();
			}
			#else
			const char* error;
			unregister_fptr_from_dll_t unregister_fptr = (unregister_fptr_from_dll_t)dlsym(it->second, SSI_FACTORY_UNREGISTER_FUNCNAME);
			if (!((error = dlerror()) != NULL)) 
			{				
				ssi_msg(SSI_LOG_LEVEL_DEFAULT, "unregister '%s'", it->first.str());
				unregister_fptr();
			}
			#endif

			if(strcmp((it->first.str()), "ssixsensd.dll") != 0 && strcmp((it->first.str()), "ssixsens.dll") != 0) //bug in xsens api 4.2.1
			{
				if (it->second) {
					#if __gnu_linux__
					if(!dlclose(it->second))
					{
						const char* error;
						const char* freeLibraryError = dlerror();
						ssi_wrn("FreeLibrary() failed (error: %s)", freeLibraryError);
					}
					#else
					if(!FreeLibrary(it->second))
					{
						DWORD freeLibraryError = GetLastError();
						ssi_wrn("FreeLibrary() failed (error: %u)", freeLibraryError);
					}
					#endif
					it->second = 0;
				}
			}
		}
		_dll_handle_map.clear ();
	}
	#if __ANDROID__
	delete android;
	android=0;
	#endif
	{
		Lock lock (_mutex);

		for (ssi_size_t i = 0; i < _strings_counter; i++) {
			delete[] _strings[i];
			_strings[i] = 0;
		}
		_strings_counter = 0;
	}
}

ITheEventBoard *Factory::getEventBoard (const ssi_char_t *file) {

	if (!_board) {
		_board = ssi_create (TheEventBoard, file, false);
	}

	return _board;
}

ITheFramework *Factory::getFramework (const ssi_char_t *file) {

	if (!_frame) {
		_frame = ssi_create (TheFramework, file, false);
	}

	return _frame;
}

IWindowManager *Factory::getWindowManager()
{

#ifdef SSI_USE_SDL
	if (!_wmanager) {
		_wmanager = new SDL_WindowManager();
	}	
#endif

	return _wmanager;
}

#if __gnu_linux__
char *GetModuleFileName(void* addr, char *pDest, int nDestLen )
{
        Dl_info rInfo;
        char    *pPos;

        *pDest = 0;

        memset( &rInfo, 0, sizeof(rInfo) );
        if ( !dladdr(addr, &rInfo)  ||
             !rInfo.dli_fname ) {

                // Return empty string if failed
                return( pDest );
        }

        strncpy( pDest,  rInfo.dli_fname, nDestLen);
        return( pDest );

}

int CopyFile(char* from, char* to, bool b)
{
    std::ifstream source(from, std::ios::binary);
    std::ofstream dest(to, std::ios::binary);

    dest << source.rdbuf();

    source.close();
    dest.close();
    return 1;
}
#endif

bool Factory::exportDlls (const ssi_char_t *target_dir) {

	dll_handle_map_t::iterator it;
	ssi_char_t dll_path[SSI_MAX_CHAR];
	ssi_char_t target_path[SSI_MAX_CHAR];
	for (it = _dll_handle_map.begin() ; it != _dll_handle_map.end(); it++) {
		if (GetModuleFileName (it->second, dll_path, SSI_MAX_CHAR) == 0) {
			ssi_wrn ("could not get module name for '%s'", (char*)it->first.str());
		} else {
			FilePath fp (dll_path);
			ssi_sprint (target_path, "%s\\%s", target_dir, fp.getNameFull ());
			if (CopyFile (dll_path, target_path, false) == 0) {
				ssi_wrn ("could not copy '%s' to '%s'", dll_path, target_path);
			}
		}
		ssi_msg (SSI_LOG_LEVEL_DETAIL, "copied '%s' to '%s'", (char*)it->first.str(), target_dir);
	}

	return true;
}

ssi_size_t Factory::addString (const ssi_char_t *name) {

	Lock lock (_mutex);

	ssi_msg (SSI_LOG_LEVEL_DEBUG, "add string '%s'", name);

	if (name == 0) {
		return SSI_FACTORY_STRINGS_INVALID_ID;
	}

	// check if string is in list
	const ssi_char_t *str = 0;
	for (ssi_size_t i = 0; i < _strings_counter; i++) {
		str = _strings[i];
		if (strcmp (name, str) == 0) {
			return i;
		}
	}

	// otherwise add string
	ssi_size_t index = 0;
	{
		index = _strings_counter;
		if (_strings_counter == SSI_FACTORY_STRINGS_MAX) {
			ssi_wrn ("#string exceeds available space '%u'", SSI_FACTORY_STRINGS_MAX);
			return SSI_FACTORY_STRINGS_INVALID_ID;
		} else {
			_strings[_strings_counter++] = ssi_strcpy (name);
		}
	}

	return index;
}

ssi_size_t Factory::getStringId (const ssi_char_t *name) {

	Lock lock (_mutex);

	for (ssi_size_t i = 0; i < _strings_counter; i++) {
		if (strcmp (name, _strings[i]) == 0) {
			return i;
		}
	}

	ssi_wrn ("string '%s' does not exist", name);
	return SSI_FACTORY_STRINGS_INVALID_ID;
}

const ssi_char_t *Factory::getString (ssi_size_t id) {

	Lock lock (_mutex);

	if (id == SSI_FACTORY_STRINGS_INVALID_ID) {
		return "undef";
	}

	if (id >= _strings_counter) {
		ssi_wrn ("id '%u' exceeds available #strings '%u'", id, _strings_counter);
		return 0;
	}

	return _strings[id];
}

ssi_size_t Factory::getUniqueId () {

	{
		Lock lock (_mutex);
		return _id_counter++;
	}
}

void Factory::Print(FILE *file) {

	GetFactory()->print(file);
}

void Factory::print(FILE *file) {

	{
		ssi_fprint(file, "DLLs:\n");
		dll_handle_map_t::iterator it;
		for (it = _dll_handle_map.begin(); it != _dll_handle_map.end(); it++) {
			ssi_fprint(file, " > %s\n", it->first.str());
		}
	}

	{
		ssi_fprint(file, "Objects:\n");
		object_id_map_t::iterator it;
		for (it = _object_id_map.begin(); it != _object_id_map.end(); it++) {
			ssi_fprint(file, " > %s [ %s ]\n", it->first.str(), it->second->getName());
		}
	}

	{
		ssi_fprint(file, "Strings:\n");
		object_id_map_t::iterator it;
		for (ssi_size_t i = 0; i < _strings_counter; i++) {
			ssi_fprint(file, " > %s [ %u ]\n", _strings[i], i);
		}
	}

}

}
