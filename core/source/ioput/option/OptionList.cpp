// OptionList.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/03/17
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

#include "ioput/option/OptionList.h"
#include "ioput/file/FilePath.h"

namespace ssi {

ssi_char_t *OptionList::ssi_log_name = "options___";
const ssi_char_t OptionList::SEPARATOR = ',';

OptionList::OptionList () 
: _readOnly(false) {
}

OptionList::~OptionList () {

	std::vector<ssi_option_t *>::iterator iter;
	for (iter = _list.begin (); iter != _list.end (); iter++) {
		delete[] (*iter)->help;
		delete[] (*iter)->name;
		delete *iter;
	}
};

ssi_option_t *OptionList::getOption (const ssi_char_t *name) {

	ssi_option_t *option = 0;

	bool flag = false;
	std::vector<ssi_option_t *>::iterator iter;
	for (iter = _list.begin (); iter != _list.end (); iter++) {
		if (strcmp ((*iter)->name, name) == 0) {
			option = *iter;
			break;
		}
	}

	return option;
}

bool OptionList::addOption (const ssi_char_t *name, 
	void *ptr, 
	ssi_size_t num, 
	ssi_type_t type, 
	const ssi_char_t *help,
	bool look) {

	if (ssi_strcmp(name, "option")) {
		ssi_err("sorry, you are not allowed to have an option with name 'option'");
		return false;
	}
	
	if (OptionList::getOption (name)) {
		ssi_wrn ("an option with name '%s' already exists", name);
		return false;
	}

	ssi_option_t *option = new ssi_option_t;
	
	option->name = Strcpy (name);
	option->help = Strcpy (help);
	option->ptr = ptr;
	option->num = num;
	option->type = type;
	option->lock = look;

	_list.push_back (option);

	return true;
}

bool OptionList::setOptionValue (const ssi_char_t *name, 
	void *ptr) {

	ssi_option_t *option = getOption (name);
	if (!option) {
		return false;
	}

	memcpy (option->ptr, ptr, option->num * GetTypeSize (option->type));

	return true;
}

bool OptionList::getOptionValue (const ssi_char_t *name, 
	void *ptr) {

	ssi_option_t *option = getOption (name);
	if (!option) {
		return false;
	}

	memcpy (ptr, option->ptr, option->num * GetTypeSize (option->type));

	return true;
}

bool OptionList::getOptionValueAsString(const ssi_char_t *name,
	ssi_char_t **ptr,
	int precision) {

	*ptr = 0;

	ssi_option_t *option = getOption(name);
	if (!option) {		
		return false;
	}

	*ptr = ToString(*option, precision);

	return true;
}

ssi_option_t *OptionList::getOption (ssi_size_t index) {

	if (index >= getSize ()) {
		return 0;
	}

	return _list.at (index);
}

inline unsigned int OptionList::getSize () {

	return ssi_cast (ssi_size_t, _list.size ());
}

inline ssi_char_t *OptionList::Strcpy (const ssi_char_t *string) {
	ssi_char_t *result = new ssi_char_t[strlen (string) + 1];
	ssi_strcpy (result, string);
	return result;
}

void OptionList::print (FILE *file) {
	
	std::vector<ssi_option_t *>::iterator iter;
	for (iter = _list.begin (); iter != _list.end (); iter++) {
		ssi_char_t *value = OptionList::ToString (*(*iter));
		fprintf (file, "%s:%s -> %s %s [%s]\n", (*iter)->name, SSI_TYPE_NAMES[(*iter)->type], value, (*iter)->lock ? "LOCK" : "", (*iter)->help);						
		delete[] value;
	}
}

inline ssi_size_t OptionList::GetTypeSize (ssi_type_t type) {

	switch (type) {
		case SSI_BOOL:
			return sizeof (bool);
		case SSI_CHAR:
			return sizeof (ssi_char_t);
		case SSI_INT:
			return sizeof (int);
		case SSI_UINT:
			return sizeof (unsigned int);
		case SSI_FLOAT:
			return sizeof (float);
		case SSI_DOUBLE:
			return sizeof (double);
	}

	return 0;
}

FILE *OptionList::OpenXML (const ssi_char_t *filepath) {

	FilePath fp (filepath);
	ssi_char_t *filepath_with_ext = 0;
	if (strcmp (fp.getExtension (), SSI_FILE_TYPE_OPTION) != 0) {
		filepath_with_ext = ssi_strcat (filepath, SSI_FILE_TYPE_OPTION);
	} else {
		filepath_with_ext = ssi_strcpy (filepath);
	}

	ssi_msg (SSI_LOG_LEVEL_DETAIL, "load options from '%s'", filepath_with_ext);
		
	FILE *file = ssi_fopen (filepath_with_ext, "r");
	if (!file) {
		ssi_wrn ("could not open file '%s'", filepath_with_ext);
	}
	
	delete[] filepath_with_ext;
	return file;
}

bool OptionList::LoadXML (const ssi_char_t *filepath, IOptions *list) {

	FILE *file = OpenXML (filepath);
	bool success = false;
	if (file) {
		success = OptionList::LoadXML (file, list);			
		fclose (file);	
	}
	return success;
}

bool OptionList::SaveXML (const ssi_char_t *filepath, IOptions *list) {

	if (list->isReadOnly())
	{
		ssi_msg(SSI_LOG_LEVEL_WARNING, "skip save, list is readonly '%s'", filepath);
		return false;
	}

	FilePath fp (filepath);
	ssi_char_t *filepath_with_ext = 0;
	if (strcmp (fp.getExtension (), SSI_FILE_TYPE_OPTION) != 0) {
		filepath_with_ext = ssi_strcat (filepath, SSI_FILE_TYPE_OPTION);
	} else {
		filepath_with_ext = ssi_strcpy (filepath);
	}

	ssi_msg (SSI_LOG_LEVEL_DETAIL, "save options to '%s'", filepath_with_ext);

	FILE *file = ssi_fopen (filepath_with_ext, "w");
	if (!file) {
		ssi_wrn ("could not open file '%s'", filepath_with_ext);
		delete[] filepath_with_ext;
		return false;
	}
	if (!OptionList::SaveXML (file, list)) {	
		delete[] filepath_with_ext;
		return false;
	}
	fclose (file);	

	delete[] filepath_with_ext;

	return true;
}


bool OptionList::SetOptionValueInPlace (const ssi_char_t *filepath, 
	const ssi_char_t *name, 
	const ssi_char_t *string) {

	FilePath fp (filepath);
	ssi_char_t *filepath_with_ext = 0;
	if (strcmp (fp.getExtension (), SSI_FILE_TYPE_OPTION) != 0) {
		filepath_with_ext = ssi_strcat (filepath, SSI_FILE_TYPE_OPTION);
	} else {
		filepath_with_ext = ssi_strcpy (filepath);
	}

	FILE *file = OpenXML (filepath_with_ext);
	bool success = false;
	if (file) {
		
		TiXmlDocument doc;
		doc.LoadFile (file, false);

		TiXmlElement *body = doc.FirstChildElement();	
		if (body && strcmp (body->Value (), "options") == 0) {
			TiXmlElement *item = body->FirstChildElement ();
			for ( item; item; item = item->NextSiblingElement()) {
				if (strcmp (item->Value (), "item") == 0 && strcmp (name, item->Attribute ("name")) == 0) {
					item->SetAttribute ("value", string);
					success = true;
					break;
				}
			}
		}

		fclose (file);	

		if (success) {
			doc.SaveFile (filepath_with_ext);
		} else {
			ssi_wrn ("setting option '%s' in file '%s' failed", name, filepath_with_ext);
		}
	}

	delete[] filepath_with_ext;
	return success;
}

bool OptionList::LoadXML (FILE *file, IOptions *list) {

	TiXmlDocument doc;
	doc.LoadFile (file, false);

	TiXmlElement *body = doc.FirstChildElement();	
	if (!body || strcmp (body->Value (), "options") != 0) {
		ssi_wrn ("tag <options> missing");
		return false;	
	}

	bool readOnly = false;
	const ssi_char_t *readOnlyStr = 0;
	if (readOnlyStr = body->Attribute("readOnly"))
	{
		if (ssi_strcmp(readOnlyStr, "true", false))
		{
			readOnly = true;
		}
	}
	list->setReadOnly(readOnly);

	TiXmlElement *item = body->FirstChildElement ();
	for ( item; item; item = item->NextSiblingElement()) {

		if (strcmp (item->Value (), "item") != 0) {
			ssi_wrn ("tag <%s> ignored", item->Value ());
			continue;
		}

		const ssi_char_t *name = item->Attribute ("name");
		ssi_option_t *o = list->getOption (name);
		if (o) {						
			ssi_type_t type = SSI_UNDEF;
			ssi_name2type (item->Attribute ("type"), type);
			if (o->type != ssi_cast (ssi_type_t, type)) {
				ssi_err ("incompatible option type ('%s')", name);
				return false;
			}
			int num = 0;
			item->QueryIntAttribute ("num", &num);
			if (o->num != ssi_cast (ssi_size_t, num)) {
				ssi_err ("incompatible option size ('%s')", name);
				return false;
			}			
			OptionList::FromString (item->Attribute ("value"), *o);	
			bool lock = false;
			const ssi_char_t *lock_s = item->Attribute("lock");
			if (lock_s) {
				if (ssi_strcmp(lock_s, "true", false)) {
					lock = true;
				}
			}
			o->lock = lock;
		} else {
			ssi_wrn ("option '%s' ignored", name);
			continue;
		}
	}


	return true;
}

bool OptionList::LoadBinary (FILE *file, IOptions *list) {

	std::vector<ssi_option_t *>::iterator iter;
	for (ssi_size_t i = 0; i < list->getSize (); i++) {

		ssi_option_t &option = *list->getOption (i);
		ssi_size_t res = ssi_cast (ssi_size_t, fread (option.ptr, ssi_type2bytes(option.type), option.num, file));
		if(res != option.num)
			ssi_wrn ("fread() failed");
	}
	return true;
}

bool OptionList::SaveXML (FILE *file, IOptions *list) {

	TiXmlDocument doc;

	TiXmlDeclaration head ("1.0", "", "");
	doc.InsertEndChild (head);
	TiXmlElement body ("options");	
	std::vector<ssi_option_t *>::iterator iter;
	for (ssi_size_t i = 0; i < list->getSize (); i++) {
		TiXmlElement item ("item" );
		ssi_option_t &option = *list->getOption (i);
		item.SetAttribute ("name", option.name);								
		item.SetAttribute ("type", SSI_TYPE_NAMES[option.type]);		
		item.SetAttribute ("num", option.num);
		ssi_char_t *value = OptionList::ToString (option);
		item.SetAttribute ("value", value);	
		delete[] value;
		item.SetAttribute("lock", option.lock ? "true" : "false");
		item.SetAttribute ("help", option.help);
		body.InsertEndChild (item);
	}
	doc.InsertEndChild (body);
	doc.SaveFile (file);

	return true;
}

bool OptionList::SaveBinary (FILE *file, IOptions *list) {

	std::vector<ssi_option_t *>::iterator iter;
	for (ssi_size_t i = 0; i < list->getSize (); i++) {

		ssi_option_t &option = *list->getOption (i);
		ssi_size_t res = ssi_cast (ssi_size_t, fwrite (option.ptr, ssi_type2bytes(option.type), option.num, file));
		if(res != option.num)
			ssi_wrn ("fwrite() failed");
	}
	return true;
}

bool OptionList::setOptionValueFromString (const ssi_char_t *name, 
	const ssi_char_t *string) {

	ssi_option_t *option = getOption (name);
	if (!option) {
		ssi_wrn ("could not find an option with name '%s'", name);
		return false;
	}

	return FromString (string, *option);
}

bool OptionList::FromString (const ssi_char_t *str, ssi_option_t &option) {

	ssi_char_t *token_begin = 0;
	ssi_char_t *token_end = ssi_ccast (ssi_char_t *, str);
	ssi_size_t token_pos = 0;	

	if (option.type == SSI_CHAR && option.num > 1) {
		size_t len = strlen (str) + 1;
		if (option.num < len) {
			ssi_wrn ("could not parse token '%s' of option '%s'", str, option.name);
			return false;
		}
		memcpy (option.ptr, str, len);
		return true;
	}

	for (ssi_size_t i = 0; i < option.num; i++) {

		token_begin = token_end;
		while (*token_end != SEPARATOR && *token_end != '\0')
			++token_end;

		bool is_end = *token_end == '\0';
		*token_end = '\0';
		switch (option.type) {
			case SSI_BOOL: {							
				if (strcmp (token_begin, "true") == 0 || strcmp (token_begin, "True") == 0) {
					*(ssi_pcast (bool, option.ptr) + i) = true;
				} else if (strcmp (token_begin, "false") == 0 || strcmp (token_begin, "False") == 0) {					
					*(ssi_pcast (bool, option.ptr) + i) = false;
				} else {
					ssi_wrn ("could not parse token '%s' of option '%s'", str, option.name);
					return false;
				}				
				break;
			}
			case SSI_CHAR : {		
				/*int tmp;
				if (EOF == sscanf (token_begin, "%d", &tmp)) {
					ssi_err ("could not parse token");
				}
				*(ssi_pcast (ssi_char_t, option.ptr) + i) = tmp;*/
				*(ssi_pcast (char, option.ptr) + i) = *token_begin;
				break;
			}
			case SSI_UCHAR : {		
				unsigned int tmp;
				if (1 != sscanf(token_begin, "%u", &tmp)) {
					ssi_wrn ("could not parse token '%s' of option '%s'", str, option.name);
					return false;
				}
				*(ssi_pcast (unsigned char, option.ptr) + i) = tmp;
				break;
			}
			case SSI_SHORT: {
				if (1 != sscanf(token_begin, "%hd", ssi_pcast(short, option.ptr) + i)) {
					ssi_wrn("could not parse token '%s' of option '%s'", str, option.name);
					return false;
				}
				break;
			}
			case SSI_USHORT: {
				if (1 != sscanf(token_begin, "%hu", ssi_pcast(unsigned short, option.ptr) + i)) {
					ssi_wrn("could not parse token '%s' of option '%s'", str, option.name);
					return false;
				}
				break;
			}
		   case SSI_INT : {		
				if (1 != sscanf (token_begin, "%d", ssi_pcast (int, option.ptr) + i)) {
					ssi_wrn ("could not parse token '%s' of option '%s'", str, option.name);
					return false;
				}
				break;
			}
			case SSI_UINT : {		
				if (1 != sscanf(token_begin, "%u", ssi_pcast(unsigned int, option.ptr) + i)) {
					ssi_wrn ("could not parse token '%s' of option '%s'", str, option.name);
					return false;
				}
				break;
			}
			case SSI_FLOAT : {		
				if (1 != sscanf (token_begin, "%e", ssi_pcast (float, option.ptr) + i)) {
					ssi_wrn ("could not parse token '%s' of option '%s'", str, option.name);
					return false;
				}
				break;
			}
			case SSI_DOUBLE : {		
				if (1 != sscanf(token_begin, "%le", ssi_pcast(double, option.ptr) + i)) {
					ssi_wrn ("could not parse token '%s' of option '%s'", str, option.name);
					return false;
				}
				break;
			}	
			default:
				ssi_wrn ("unkown option type '%d'", ssi_cast (int, option.type));
				return false;
		}
		
		if ((is_end && i != option.num-1) || (i == option.num-1 && !is_end))  {
			ssi_wrn ("number of tokens does not match '%s'", option.name);
			return false;
		}

		++token_end;
	}

	return true;
}

ssi_char_t *OptionList::ToString(ssi_option_t &option,
	int precision) {

	ssi_char_t *string_h = new ssi_char_t[1];
	string_h[0] = '\0';
	ssi_char_t *string = 0;
	ssi_char_t tmp[SSI_MAX_CHAR];	
	
	switch (option.type) {
		case SSI_BOOL: {
			bool *ptr = ssi_pcast (bool, option.ptr);
			sprintf (tmp, "%s", *ptr++ ? "true" : "false");
			string = ssi_strcat (string_h, tmp);
			delete[] string_h;
			for (ssi_size_t i = 1; i < option.num; i++) {								
				string_h = ssi_strcpy (string);				
				delete[] string;
				sprintf (tmp, "%c%s", SEPARATOR, *ptr++ ? "true" : "false");
				string = ssi_strcat (string_h, tmp);
				delete[] string_h;
			}
			break;
		}		
		case SSI_CHAR: {
			char *ptr = ssi_pcast (ssi_char_t, option.ptr);
			string = new ssi_char_t[option.num + 1];
			memcpy (string, option.ptr, option.num);
			string[option.num] = '\0';
			delete[] string_h;
			break;
		}	
		case SSI_UCHAR: {
			unsigned char *ptr = ssi_pcast (unsigned char, option.ptr);
			sprintf (tmp, "%u", ssi_cast (int, *ptr++));
			string = ssi_strcat (string_h, tmp);
			delete[] string_h;
			for (ssi_size_t i = 1; i < option.num; i++) {								
				string_h = ssi_strcpy (string);				
				delete[] string;
				sprintf (tmp, "%c%d", SEPARATOR, ssi_cast (int, *ptr++));
				string = ssi_strcat (string_h, tmp);
				delete[] string_h;
			}
			break;
		}
		case SSI_SHORT: {
			short *ptr = ssi_pcast(short, option.ptr);
			sprintf(tmp, "%hd", *ptr++);
			string = ssi_strcat(string_h, tmp);
			delete[] string_h;
			for (ssi_size_t i = 1; i < option.num; i++) {
				string_h = ssi_strcpy(string);
				delete[] string;
				sprintf(tmp, "%c%hd", SEPARATOR, *ptr++);
				string = ssi_strcat(string_h, tmp);
				delete[] string_h;
			}
			break;
		}
		case SSI_USHORT: {
			unsigned short *ptr = ssi_pcast(unsigned short, option.ptr);
			sprintf(tmp, "%hu", *ptr++);
			string = ssi_strcat(string_h, tmp);
			delete[] string_h;
			for (ssi_size_t i = 1; i < option.num; i++) {
				string_h = ssi_strcpy(string);
				delete[] string;
				sprintf(tmp, "%c%hu", SEPARATOR, *ptr++);
				string = ssi_strcat(string_h, tmp);
				delete[] string_h;
			}
			break;
		}
		case SSI_INT: {
			int *ptr = ssi_pcast (int, option.ptr);
			sprintf (tmp, "%d", *ptr++);
			string = ssi_strcat (string_h, tmp);
			delete[] string_h;
			for (ssi_size_t i = 1; i < option.num; i++) {								
				string_h = ssi_strcpy (string);				
				delete[] string;
				sprintf (tmp, "%c%d", SEPARATOR, *ptr++);
				string = ssi_strcat (string_h, tmp);
				delete[] string_h;
			}
			break;
		}			
		case SSI_UINT: {
			unsigned int *ptr = ssi_pcast (unsigned int, option.ptr);
			sprintf (tmp, "%u", *ptr++);
			string = ssi_strcat (string_h, tmp);
			delete[] string_h;
			for (ssi_size_t i = 1; i < option.num; i++) {								
				string_h = ssi_strcpy (string);				
				delete[] string;
				sprintf (tmp, "%c%u", SEPARATOR, *ptr++);
				string = ssi_strcat (string_h, tmp);
				delete[] string_h;
			}
			break;
		}
		case SSI_FLOAT: {
			float *ptr = ssi_pcast (float, option.ptr);
			if (precision == -1) {
				sprintf(tmp, "%e", *ptr++);				
			} else {
				sprintf(tmp, "%.*f", precision, *ptr++);
			}			
			string = ssi_strcat (string_h, tmp);
			delete[] string_h;
			for (ssi_size_t i = 1; i < option.num; i++) {								
				string_h = ssi_strcpy (string);				
				delete[] string;
				if (precision == -1) {
					sprintf(tmp, "%c%e", SEPARATOR, *ptr++);
				} else {
					sprintf(tmp, "%c%.*f", SEPARATOR, precision, *ptr++);
				}
				string = ssi_strcat (string_h, tmp);
				delete[] string_h;
			}
			break;
		}
		case SSI_DOUBLE: {
			double *ptr = ssi_pcast (double, option.ptr);
			if (precision == -1) {
				sprintf(tmp, "%le", *ptr++);				
			} else {
				sprintf(tmp, "%.*lf", precision, *ptr++);
			}
			string = ssi_strcat (string_h, tmp);
			delete[] string_h;
			for (unsigned int i = 1; i < option.num; i++) {								
				string_h = ssi_strcpy (string);				
				delete[] string;
				if (precision == -1) {
					sprintf(tmp, "%c%le", SEPARATOR, *ptr++);
				} else {
					sprintf(tmp, "%c%.*lf", SEPARATOR, precision, *ptr++);
				}
				string = ssi_strcat (string_h, tmp);
				delete[] string_h;
			}
			break;
		}
		default:
			ssi_err ("type not supported");
			return 0;
	}

	return string;

}

void OptionList::lock(){
	_mutex.acquire();
}

void OptionList::unlock(){
	_mutex.release();
}

bool OptionList::isReadOnly()
{
	return _readOnly;
}

void OptionList::setReadOnly(bool toggle)
{
	_readOnly = toggle;
}

}

