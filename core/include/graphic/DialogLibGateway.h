// DialogLibGateway.h
// author: Frank Jung <frank.jung@informatik.uni-augsburg.de>
// created: 2009/04/21
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

#ifndef SSI_GRAPHIC_DIALOGLIBGATEWAY_H
#define	SSI_GRAPHIC_DIALOGLIBGATEWAY_H

#include "SSI_Define.h"

#ifndef SSI_USE_SDL

#include "SSI_Cons.h"
#if __gnu_linux__
	#define HMODULE void*
#endif

namespace ssi
{
typedef int (*GetNewDialogHandle_ptr)(const char*);
typedef int (*RunSpecificDialog_ptr)(int);
typedef bool (*RemoveDialog_ptr)(int);
typedef int (*AddItem_ptr)(int, const char*, int, const char*);
typedef int (*RemoveItem_ptr)(int, const char*, int);
typedef char* (*RetrieveString_ptr)(int, const char*);
typedef double (*RetrieveDouble_ptr)(int, const char*);
typedef float (*RetrieveFloat_ptr)(int, const char*);
typedef int (*RetrieveInt_ptr)(int, const char*);

	class DialogLibGateway
	{
	public:

		DialogLibGateway();
		~DialogLibGateway();

		//Calling this function will make all prior changes undone on success and a new Dialog is generated and wrapped.
		//It has to be successfully called before any other operation is done.
		bool SetNewDialogType(const char *dialogType);

		int RunDialog();

		int AppendItem(const char *itemCategory, const char *itemValue);
		int AlterExistingItem(const char *itemCategory, int ordinalValueOfItem, const char *itemValue);

		bool RemoveItem(const char *itemCategory, int ordinalValueOfItem);
		bool RemoveItems(const char *itemCategory);

		bool RetrieveString(const char *itemID, char **retrievedString);
		bool RetrieveDouble(const char *itemID, double *retrievedDouble);
		bool RetrieveFloat(const char *itemID, float *retrievedFloat);
		bool RetrieveInt(const char *itemID, int *retrievedInt);

		void setLogLevel(int level);
		static void SetLogLevelStatic(int level);

		bool didInitWork();
		
	protected:
		char					*ssi_log_name;
		static char 			ssi_log_name_static[];
		int						ssi_log_level;
		static int				ssi_log_level_static;

		int						_dialogHandle;

		bool					_initValid;

		HMODULE					hDLL;
		GetNewDialogHandle_ptr	getNewHandle;
		RunSpecificDialog_ptr	runSpecificDialog;
		RemoveDialog_ptr		removeDialog;
		AddItem_ptr				addItem;
		RemoveItem_ptr			removeItem;
		RetrieveString_ptr		retrieveString;
		RetrieveDouble_ptr		retrieveDouble;
		RetrieveFloat_ptr		retrieveFloat;
		RetrieveInt_ptr			retrieveInt;
	};
}

#endif

#endif