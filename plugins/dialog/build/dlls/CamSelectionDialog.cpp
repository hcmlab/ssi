// CamSelectionDialog.cpp
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

#include "DialogLibStdAfx.h"
#include "CamSelectionDialog.h"

namespace DialogLib
{

	int CamSelectionDialog::AddItem(System::String ^itemID, System::String ^itemValue)
	{
		if(itemID == "ToolTipDevicePath")
		{
			this->toolTipDevicePath->SetToolTip(this->textBoxDevicePath, System::String::Copy(itemValue));
			return 0;
		}
		if(itemID == "ToolTipDevicePathTitle")
		{
			this->toolTipDevicePath->ToolTipTitle = System::String::Copy(itemValue);
			return 0;
		}
		if(itemID == "FriendlyName")
		{
			this->_friendlyName->Add(System::String::Copy(itemValue));
			this->_description->Add(System::String::Empty);
			this->_devicePath->Add(System::String::Empty);
			this->comboBoxFriendlyName->DataSource = nullptr;
			this->comboBoxDescription->DataSource = nullptr;
			this->comboBoxFriendlyName->DataSource = this->_friendlyName;
			this->comboBoxDescription->DataSource = this->_description;
			return this->_friendlyName->Count-1;
		}
		if(itemID == "Description")
		{
			this->_friendlyName->Add(System::String::Empty);
			this->_description->Add(System::String::Copy(itemValue));
			this->_devicePath->Add(System::String::Empty);
			this->comboBoxFriendlyName->DataSource = nullptr;
			this->comboBoxDescription->DataSource = nullptr;
			this->comboBoxFriendlyName->DataSource = this->_friendlyName;
			this->comboBoxDescription->DataSource = this->_description;
			return this->_description->Count-1;
		}
		if(itemID == "DevicePath")
		{
			this->_friendlyName->Add(System::String::Empty);
			this->_description->Add(System::String::Empty);
			this->_devicePath->Add(System::String::Copy(itemValue));
			this->comboBoxFriendlyName->DataSource = nullptr;
			this->comboBoxDescription->DataSource = nullptr;
			this->comboBoxFriendlyName->DataSource = this->_friendlyName;
			this->comboBoxDescription->DataSource = this->_description;
			return this->_devicePath->Count-1;
		}
		return __super::AddItem(itemID, itemValue);
	}
	bool CamSelectionDialog::AddItem(System::String ^itemID, int ordinalNumber, System::String ^itemValue)
	{
		if(itemID == "FriendlyName")
		{
			if(ordinalNumber < this->_friendlyName->Count && ordinalNumber >= 0)
			{
				this->_friendlyName[ordinalNumber] = System::String::Copy(itemValue);
				this->comboBoxFriendlyName->DataSource = nullptr;
				this->comboBoxDescription->DataSource = nullptr;
				this->comboBoxFriendlyName->DataSource = this->_friendlyName;
				this->comboBoxDescription->DataSource = this->_description;
				return true;
			}
			if(ordinalNumber == this->_friendlyName->Count)
			{
				this->_friendlyName->Add(System::String::Copy(itemValue));
				this->_description->Add(System::String::Empty);
				this->_devicePath->Add(System::String::Empty);
				this->comboBoxFriendlyName->DataSource = nullptr;
				this->comboBoxDescription->DataSource = nullptr;
				this->comboBoxFriendlyName->DataSource = this->_friendlyName;
				this->comboBoxDescription->DataSource = this->_description;
				return true;
			}
			return false;
		}
		if(itemID == "Description")
		{
			if(ordinalNumber < this->_description->Count && ordinalNumber >= 0)
			{
				this->_description[ordinalNumber] = System::String::Copy(itemValue);
				this->comboBoxFriendlyName->DataSource = nullptr;
				this->comboBoxDescription->DataSource = nullptr;
				this->comboBoxFriendlyName->DataSource = this->_friendlyName;
				this->comboBoxDescription->DataSource = this->_description;
				return true;
			}
			if(ordinalNumber == this->_description->Count)
			{
				this->_friendlyName->Add(System::String::Empty);
				this->_description->Add(System::String::Copy(itemValue));
				this->_devicePath->Add(System::String::Empty);
				this->comboBoxFriendlyName->DataSource = nullptr;
				this->comboBoxDescription->DataSource = nullptr;
				this->comboBoxFriendlyName->DataSource = this->_friendlyName;
				this->comboBoxDescription->DataSource = this->_description;
				return true;
			}
			return false;
		}
		if(itemID == "DevicePath")
		{
			if(ordinalNumber < this->_devicePath->Count && ordinalNumber >= 0)
			{
				this->_devicePath[ordinalNumber] = System::String::Copy(itemValue);
				return true;
			}
			if(ordinalNumber == this->_devicePath->Count)
			{
				this->_friendlyName->Add(System::String::Empty);
				this->_description->Add(System::String::Empty);
				this->_devicePath->Add(System::String::Copy(itemValue));
				this->comboBoxFriendlyName->DataSource = nullptr;
				this->comboBoxDescription->DataSource = nullptr;
				this->comboBoxFriendlyName->DataSource = this->_friendlyName;
				this->comboBoxDescription->DataSource = this->_description;
				return true;
			}
			return false;
		}
		return __super::AddItem(itemID, ordinalNumber, itemValue);;
	}
	bool CamSelectionDialog::RemoveItem(System::String ^itemID, int ordinalNumber)
	{
		return __super::RemoveItem(itemID, ordinalNumber);
	}
	bool CamSelectionDialog::RemoveItems(System::String ^itemID)
	{
		return __super::RemoveItems(itemID);
	}

}
