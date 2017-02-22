// DialogLib.cpp
// author: Frank Jung <frank.jung@informatik.uni-augsburg.de>
// created: 2009/03/01
// Copyright (C) University of Augsburg, Johannes Wagner
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Multimedia Concepts and Applications of the University of Augsburg
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


#include "DialogLibStdafx.h"

#include "DialogLib.h"

namespace DialogLib
{

#pragma unmanaged
	CriticalSection::CriticalSection()
	{
		::InitializeCriticalSection(&thisCritSection);
	}

#pragma unmanaged
	CriticalSection::~CriticalSection()
	{
		::DeleteCriticalSection(&thisCritSection);
	}

#pragma unmanaged
	void CriticalSection::Enter()
	{
		::EnterCriticalSection(&thisCritSection);
	}

#pragma unmanaged
	void CriticalSection::Leave()
	{
		::LeaveCriticalSection(&thisCritSection);
	}

#pragma managed
	DialogManager* DialogManager::_dialogManagerHandle = nullptr;

	CriticalSection DialogManager::critSection;

	DialogManager::DialogManager()
	{
		this->_listOfDialogs = gcnew System::Collections::Generic::List<DialogTemplate^>;
		DialogManager::_dialogManagerHandle = this;
	}

	DialogManager::~DialogManager()
	{
		DialogManager::_dialogManagerHandle = nullptr;
	}

	DialogManager* DialogManager::Instance()
	{
		DialogManager::critSection.Enter();
		if(DialogManager::_dialogManagerHandle == nullptr)
		{
			DialogManager::_dialogManagerHandle = new DialogManager();
		}
		DialogManager::critSection.Leave();
		return DialogManager::_dialogManagerHandle;
	}

	int DialogManager::GetNewDialog(const char *dialogType)
	{
		if(dialogType == nullptr)
		{
			return -1;
		}

		if(strcmp(dialogType, "SimpleSelectionDialog") == 0)
		{
			DialogManager::critSection.Enter();
			this->_listOfDialogs->Add(gcnew SimpleSelectionDialog());
			int retVal = this->_listOfDialogs->Count - 1;
			DialogManager::critSection.Leave();
			return retVal;
		}
		if(strcmp(dialogType, "CamSelectionDialog") == 0)
		{
			DialogManager::critSection.Enter();
			this->_listOfDialogs->Add(gcnew CamSelectionDialog());
			int retVal = this->_listOfDialogs->Count - 1;
			DialogManager::critSection.Leave();
			return retVal;
		}
		if(strcmp(dialogType, "PinAndMediaSelectionDialog") == 0)
		{
			DialogManager::critSection.Enter();
			this->_listOfDialogs->Add(gcnew PinAndMediaSelectionDialog());
			int retVal = this->_listOfDialogs->Count - 1;
			DialogManager::critSection.Leave();
			return retVal;
		}
		return -1;
	}

	bool DialogManager::RemoveDialog(int dialogHandle)
	{
		DialogManager::critSection.Enter();
		if(dialogHandle < 0 || dialogHandle >= this->_listOfDialogs->Count)
		{
			DialogManager::critSection.Leave();
			return false;
		}

		System::Collections::Generic::List<DialogTemplate^>^ listHandle = this->_listOfDialogs;

		DialogTemplate^ itemHandle = listHandle[dialogHandle];
		if(itemHandle == nullptr)
		{
			DialogManager::critSection.Leave();
			return true;
		}

		delete itemHandle;
		itemHandle = nullptr;

		listHandle[dialogHandle] = itemHandle;

		DialogManager::critSection.Leave();
		return true;

	}

	void DialogManager::DialogShowThreadMethod(System::Object^ dialogTemplate)
	{
		System::Windows::Forms::Application::EnableVisualStyles();
		System::Windows::Forms::Application::SetCompatibleTextRenderingDefault(false);
		DialogTemplate^ dialogHandle = dynamic_cast<DialogTemplate^>(dialogTemplate);
		System::Windows::Forms::Application::Run(dialogHandle);
		return;
	}

	int DialogManager::AddItem(int dialogHandle, const char *itemID, int ordinalNumber, const char *itemValue)
	{
		DialogManager::critSection.Enter();
		if(dialogHandle < 0 || dialogHandle >= this->_listOfDialogs->Count)
		{
			DialogManager::critSection.Leave();
			return -2;
		}
		System::Collections::Generic::List<DialogTemplate^>^ listHandle = this->_listOfDialogs;
		DialogTemplate^ tmpDialog = listHandle[dialogHandle];
		if(tmpDialog == nullptr)
		{
			DialogManager::critSection.Leave();
			return -2;
		}

		gcroot<System::String^> strItemID;
		if(itemID != nullptr)
			strItemID = gcnew System::String(itemID);
		else
			strItemID = System::String::Empty;

		gcroot<System::String^> strItemValue;
		if(itemValue != nullptr)
			strItemValue = gcnew System::String(itemValue);
		else
			strItemValue = System::String::Empty;

		if(ordinalNumber == -1)
		{
			int retVal = tmpDialog->AddItem(strItemID, strItemValue);
			DialogManager::critSection.Leave();
			return retVal;
		}
		
		bool retBool = tmpDialog->AddItem(strItemID, ordinalNumber, strItemValue);
		DialogManager::critSection.Leave();
		return (retBool) ? ordinalNumber : -1;
	}

	int DialogManager::RemoveItem(int dialogHandle, const char *itemID, int ordinalNumber)
	{
		DialogManager::critSection.Enter();
		if(dialogHandle < 0 || dialogHandle >= this->_listOfDialogs->Count)
		{
			DialogManager::critSection.Leave();
			return -2;
		}
		System::Collections::Generic::List<DialogTemplate^>^ listHandle = this->_listOfDialogs;
		DialogTemplate^ tmpDialog = listHandle[dialogHandle];
		if(tmpDialog == nullptr)
		{
			DialogManager::critSection.Leave();
			return -2;
		}

		gcroot<System::String^> strItemID;
		if(itemID != nullptr)
			strItemID = gcnew System::String(itemID);
		else
			strItemID = System::String::Empty;

		if(ordinalNumber == -1)
		{
			int retVal = tmpDialog->RemoveItems(strItemID);
			DialogManager::critSection.Leave();
			return (retVal) ? 0 : -1;
		}
		
		int retVal = tmpDialog->RemoveItem(strItemID, ordinalNumber);
		DialogManager::critSection.Leave();
		return (retVal) ? ordinalNumber : -1;
	}

	int DialogManager::RunDialog(int dialogHandle)
	{
		DialogManager::critSection.Enter();
		if(dialogHandle < 0 || dialogHandle >= this->_listOfDialogs->Count)
		{
			DialogManager::critSection.Leave();
			return -2;
		}
		System::Collections::Generic::List<DialogTemplate^>^ listHandle = this->_listOfDialogs;
		DialogTemplate^ wantedDialog = listHandle[dialogHandle];
		if(wantedDialog == nullptr)
		{
			DialogManager::critSection.Leave();
			return -2;
		}

		DialogManager::critSection.Leave();

		gcroot<System::Threading::ParameterizedThreadStart^> threadDelegate = gcnew System::Threading::ParameterizedThreadStart(&DialogManager::DialogShowThreadMethod);
		gcroot<System::Threading::Thread^> newWindowThread = gcnew System::Threading::Thread(threadDelegate);
		newWindowThread->SetApartmentState(System::Threading::ApartmentState::STA);
		newWindowThread->Start(wantedDialog);
		newWindowThread->Join();

		return wantedDialog->_selectedOption;
	}

	char* DialogManager::RetrieveString(int dialogHandle, const char *itemID)
	{
		DialogManager::critSection.Enter();
		if(dialogHandle < 0 || dialogHandle >= this->_listOfDialogs->Count)
		{
			DialogManager::critSection.Leave();
			return NULL;
		}

		System::Collections::Generic::List<DialogTemplate^>^ listHandle = this->_listOfDialogs;
		DialogTemplate^ wantedDialog = listHandle[dialogHandle];
		if(wantedDialog == nullptr)
		{
			DialogManager::critSection.Leave();
			return NULL;
		}

		gcroot<System::String^> strItemID;
		if(itemID != nullptr)
			strItemID = gcnew System::String(itemID);
		else
			strItemID = System::String::Empty;

		if(!wantedDialog->_returnString->ContainsKey(strItemID))
		{
			DialogManager::critSection.Leave();
			return NULL;
		}

		System::String^ resString = wantedDialog->_returnString[strItemID];
		
		char *retCharString = new char[resString->Length + 1];

		char *tmpCharString = (char*)(void*) System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(resString);

		memcpy_s((void*)retCharString, resString->Length + 1, (void*)tmpCharString, resString->Length);
		retCharString[resString->Length] = '\0';
		
		System::Runtime::InteropServices::Marshal::FreeHGlobal((System::IntPtr)(void*)tmpCharString);

		DialogManager::critSection.Leave();

		return retCharString;
	}

	double DialogManager::RetrieveDouble(int dialogHandle, const char *itemID)
	{
		DialogManager::critSection.Enter();
		if(dialogHandle < 0 || dialogHandle >= this->_listOfDialogs->Count)
		{
			DialogManager::critSection.Leave();
			return System::Double::NaN;
		}

		System::Collections::Generic::List<DialogTemplate^>^ listHandle = this->_listOfDialogs;
		DialogTemplate^ wantedDialog = listHandle[dialogHandle];
		if(wantedDialog == nullptr)
		{
			DialogManager::critSection.Leave();
			return System::Double::NaN;
		}

		gcroot<System::String^> strItemID;
		if(itemID != nullptr)
			strItemID = gcnew System::String(itemID);
		else
			strItemID = System::String::Empty;

		if(!wantedDialog->_returnDouble->ContainsKey(strItemID))
		{
			DialogManager::critSection.Leave();
			return System::Double::NaN;
		}

		double retDouble = wantedDialog->_returnDouble[strItemID];

		DialogManager::critSection.Leave();

		return retDouble;

	}

	float DialogManager::RetrieveFloat(int dialogHandle, const char *itemID)
	{
		DialogManager::critSection.Enter();
		if(dialogHandle < 0 || dialogHandle >= this->_listOfDialogs->Count)
		{
			DialogManager::critSection.Leave();
			return System::Single::NaN;
		}

		System::Collections::Generic::List<DialogTemplate^>^ listHandle = this->_listOfDialogs;
		DialogTemplate^ wantedDialog = listHandle[dialogHandle];
		if(wantedDialog == nullptr)
		{
			DialogManager::critSection.Leave();
			return System::Single::NaN;
		}

		gcroot<System::String^> strItemID;
		if(itemID != nullptr)
			strItemID = gcnew System::String(itemID);
		else
			strItemID = System::String::Empty;

		if(!wantedDialog->_returnFloat->ContainsKey(strItemID))
		{
			DialogManager::critSection.Leave();
			return System::Single::NaN;
		}

		float retFloat = wantedDialog->_returnFloat[strItemID];

		DialogManager::critSection.Leave();

		return retFloat;

	}

	int DialogManager::RetrieveInt(int dialogHandle, const char *itemID)
	{
		DialogManager::critSection.Enter();
		if(dialogHandle < 0 || dialogHandle >= this->_listOfDialogs->Count)
		{
			DialogManager::critSection.Leave();
			return System::Int32::MinValue;
		}

		System::Collections::Generic::List<DialogTemplate^>^ listHandle = this->_listOfDialogs;
		DialogTemplate^ wantedDialog = listHandle[dialogHandle];
		if(wantedDialog == nullptr)
		{
			DialogManager::critSection.Leave();
			return System::Int32::MinValue;
		}

		gcroot<System::String^> strItemID;
		if(itemID != nullptr)
			strItemID = gcnew System::String(itemID);
		else
			strItemID = System::String::Empty;

		if(!wantedDialog->_returnInt->ContainsKey(strItemID))
		{
			DialogManager::critSection.Leave();
			return System::Int32::MinValue;
		}

		int retInt = wantedDialog->_returnInt[strItemID];

		DialogManager::critSection.Leave();

		return retInt;

	}
}
