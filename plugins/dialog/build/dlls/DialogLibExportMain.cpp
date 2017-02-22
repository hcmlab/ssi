// DialogLibExportMain.cpp
// author: Frank Jung <frank.jung@informatik.uni-augsburg.de>
// created: 2009/03/01
// Copyright (C) University of Augsburg

#include "DialogLib.h"

#ifndef DLLEXP
#define DLLEXP extern "C" __declspec( dllexport )
#endif


#pragma unmanaged
DLLEXP int GetNewDialogHandle(const char *dialogType)
{
	DialogLib::DialogManager *theInstance = DialogLib::DialogManager::Instance();
	return theInstance->GetNewDialog(dialogType);
}

#pragma unmanaged
DLLEXP int RunSpecificDialog(int dialogHandle)
{
	DialogLib::DialogManager *theInstance = DialogLib::DialogManager::Instance();
	return theInstance->RunDialog(dialogHandle);
}

#pragma unmanaged
DLLEXP bool RemoveDialog(int dialogHandle)
{
	DialogLib::DialogManager *theInstance = DialogLib::DialogManager::Instance();
	return theInstance->RemoveDialog(dialogHandle);
}

#pragma unmanaged
DLLEXP int AddItem(int dialogHandle, const char *itemID, int ordinalNumber, const char *itemValue)
{
	DialogLib::DialogManager *theInstance = DialogLib::DialogManager::Instance();
	return theInstance->AddItem(dialogHandle, itemID, ordinalNumber, itemValue);
}

#pragma unmanaged
DLLEXP int RemoveItem(int dialogHandle, const char *itemID, int ordinalNumber)
{
	DialogLib::DialogManager *theInstance = DialogLib::DialogManager::Instance();
	return theInstance->RemoveItem(dialogHandle, itemID, ordinalNumber);
}

#pragma unmanaged
DLLEXP char* RetrieveString(int dialogHandle, const char *itemID)
{
	DialogLib::DialogManager *theInstance = DialogLib::DialogManager::Instance();
	return theInstance->RetrieveString(dialogHandle, itemID);
}

#pragma unmanaged
DLLEXP double RetrieveDouble(int dialogHandle, const char *itemID)
{
	DialogLib::DialogManager *theInstance = DialogLib::DialogManager::Instance();
	return theInstance->RetrieveDouble(dialogHandle, itemID);
}

#pragma unmanaged
DLLEXP float RetrieveFloat(int dialogHandle, const char *itemID)
{
	DialogLib::DialogManager *theInstance = DialogLib::DialogManager::Instance();
	return theInstance->RetrieveFloat(dialogHandle, itemID);
}

#pragma unmanaged
DLLEXP int RetrieveInt(int dialogHandle, const char *itemID)
{
	DialogLib::DialogManager *theInstance = DialogLib::DialogManager::Instance();
	return theInstance->RetrieveInt(dialogHandle, itemID);
}