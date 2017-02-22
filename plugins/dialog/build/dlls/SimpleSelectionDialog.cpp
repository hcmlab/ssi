// SimpleSelectionDialog.cpp
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
#include "SimpleSelectionDialog.h"

namespace DialogLib
{

	int SimpleSelectionDialog::AddItem(System::String ^itemID, System::String ^itemValue)
	{
		if(itemID == "Item")
		{
			this->_item->Add(System::String::Copy(itemValue));
			this->comboBoxSelection->DataSource = nullptr;
			this->comboBoxSelection->DataSource = this->_item;
			return this->_item->Count-1;
		}
		return __super::AddItem(itemID, itemValue);
	}
	bool SimpleSelectionDialog::AddItem(System::String ^itemID, int ordinalNumber, System::String ^itemValue)
	{
		if(itemID == "Item")
		{
			if(ordinalNumber < this->_item->Count && ordinalNumber >= 0)
			{
				this->_item[ordinalNumber] = System::String::Copy(itemValue);
				this->comboBoxSelection->DataSource = nullptr;
				this->comboBoxSelection->DataSource = this->_item;
				return true;
			}
			if(ordinalNumber == this->_item->Count)
			{
				this->_item->Add(System::String::Copy(itemValue));
				this->comboBoxSelection->DataSource = nullptr;
				this->comboBoxSelection->DataSource = this->_item;
				return true;
			}
			return false;
		}
		return __super::AddItem(itemID, ordinalNumber, itemValue);;
	}
	bool SimpleSelectionDialog::RemoveItem(System::String ^itemID, int ordinalNumber)
	{
		return __super::RemoveItem(itemID, ordinalNumber);
	}
	bool SimpleSelectionDialog::RemoveItems(System::String ^itemID)
	{
		return __super::RemoveItems(itemID);
	}

}
