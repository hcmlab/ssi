// OptionList.h
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

#pragma once

#ifndef SSI_IOPUT_OPTIONLIST_H
#define SSI_IOPUT_OPTIONLIST_H

#include "SSI_Cons.h"
#include "base/IOptions.h"
#include "ioput/xml/tinyxml.h"
#include "thread/Mutex.h"

namespace ssi {

#define SSI_OPTIONLIST_ADD_ADDRESS(address) { \
	(address)[0] = '\0'; \
	addOption("address", (address), SSI_MAX_CHAR, SSI_CHAR, "event address (event@sender)"); \
}

#define SSI_OPTIONLIST_SET_ADDRESS(string, address, event) { \
	(address).setAddress((string)); \
	(event).sender_id = Factory::AddString((address).getSender(0)); \
	if ((event).sender_id == SSI_FACTORY_STRINGS_INVALID_ID) { \
		ssi_wrn("invalid sender name") \
			return false; \
	} \
	(event).event_id = Factory::AddString((address).getEvent(0)); \
	if ((event).event_id == SSI_FACTORY_STRINGS_INVALID_ID) { \
		ssi_wrn("invalid event name") \
			return false; \
	} \
}

class OptionList : public IOptions {

public:

	static const ssi_char_t SEPARATOR;

	OptionList ();	
	virtual ~OptionList ();

	bool addOption(const ssi_char_t *name,
		void *ptr, 
		ssi_size_t num, 
		ssi_type_t type, 
		const ssi_char_t *help,
		bool lock = true);

	bool getOptionValue(const ssi_char_t *name, void *ptr);
	bool getOptionValueAsString(const ssi_char_t *name,ssi_char_t **ptr, int precision = -1);
	bool setOptionValue(const ssi_char_t *name, void *ptr);	
	bool setOptionValueFromString(const ssi_char_t *name,const ssi_char_t *string);	
	static bool SetOptionValueInPlace (const ssi_char_t *filename, const ssi_char_t *name, const ssi_char_t *string);

	static ssi_char_t *ToString(ssi_option_t &option, int precision = -1);
	static bool FromString(const ssi_char_t *str, ssi_option_t &option);

	ssi_option_t *getOption(const ssi_char_t *name);
	ssi_option_t *getOption (ssi_size_t index);
	ssi_size_t getSize ();

	void lock();
	void unlock();
	bool isReadOnly();
	void setReadOnly(bool toggle);

	void print (FILE *file);

	static bool LoadXML (const ssi_char_t *filename, IOptions *list);		
	static bool SaveXML (const ssi_char_t *filename, IOptions *list);
	static bool LoadXML (FILE *file, IOptions *list);
	static bool LoadBinary (FILE *file, IOptions *list);
	static bool SaveXML (FILE *file, IOptions *list);	
	static bool SaveBinary (FILE *file, IOptions *list);	

protected:

	static ssi_char_t *ssi_log_name;

	std::vector<ssi_option_t *> _list;	
	bool _readOnly;

	static ssi_char_t *Strcpy (const ssi_char_t *string);
	static ssi_size_t GetTypeSize (ssi_type_t);
	static FILE *OpenXML (const ssi_char_t *filename);

	Mutex _mutex;
};

}

#endif
