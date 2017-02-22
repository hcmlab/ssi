// DialogLib.h
// author: Frank Jung <frank.jung@informatik.uni-augsburg.de>
// created: 2009/03/01
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

//using namespace System;
//using namespace System::Collections::Generic;

#include <gcroot.h>
#include "DialogTemplate.h"
#include "SimpleSelectionDialog.h"
#include "CamSelectionDialog.h"
#include "PinAndMediaSelectionDialog.h"
#include <string.h>

#pragma managed(push, off)
#include <Windows.h>
#pragma managed(pop)


namespace DialogLib {

#pragma unmanaged
	class CriticalSection
	{
	public:
		CriticalSection();
		~CriticalSection();
		void Enter();
		void Leave();
	private:
		CRITICAL_SECTION thisCritSection;
	};

#pragma managed
	public class DialogManager
	{
	public:
		static DialogManager* Instance();
		int RunDialog(int dialogHandle);
		int GetNewDialog(const char *dialogType);
		bool RemoveDialog(int dialogHandle);
		int AddItem(int dialogHandle, const char *itemID, int ordinalNumber, const char *itemValue);
		int RemoveItem(int dialogHandle, const char *itemID, int ordinalNumber);

		char* RetrieveString(int dialogHandle, const char *itemID);
		double RetrieveDouble(int dialogHandle, const char *itemID);
		float RetrieveFloat(int dialogHandle, const char *itemID);
		int RetrieveInt(int dialogHandle, const char *itemID);

	private:
		DialogManager();
		~DialogManager();

		static void DialogShowThreadMethod(System::Object^ dialogTemplate);

		gcroot<System::Collections::Generic::List<DialogTemplate^>^> _listOfDialogs;

		static DialogManager *_dialogManagerHandle;

	private:
		static CriticalSection critSection;


	};
}
