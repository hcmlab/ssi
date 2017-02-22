// DialogTemplate.cpp
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
#include "DialogTemplate.h"

namespace DialogLib
{
	int DialogTemplate::AddItem(System::String ^itemID, System::String ^itemValue)
	{
		if(itemID == "Caption")
		{
			this->Text = System::String::Copy(itemValue);
			return 0;
		}
		if(itemID == "Help")
		{
			this->_helpText = System::String::Copy(itemValue);
			return 0;
		}
		return -1;
	}
	bool DialogTemplate::AddItem(System::String ^itemID, int ordinalNumber, System::String ^itemValue)
	{
		if(itemID == "Caption")
		{
			this->Text = System::String::Copy(itemValue);
			return true;
		}
		if(itemID == "Help")
		{
			_helpText = System::String::Copy(itemValue);
			return true;
		}
		return false;
	}
	bool DialogTemplate::RemoveItem(System::String ^itemID, int ordinalNumber)
	{
		if(itemID == "Help")
		{
			_helpText = gcnew System::String("No additional Help available");
			return true;
		}
		return false;
	}
	bool DialogTemplate::RemoveItems(System::String ^itemID)
	{
		if(itemID == "Help")
		{
			_helpText = gcnew System::String("No additional Help available");
			return true;
		}
		return false;
	}
}
