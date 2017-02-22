// PinAndMediaSelectionDialog.cpp
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
#include "PinAndMediaSelectionDialog.h"

namespace DialogLib
{

	int PinAndMediaSelectionDialog::AddItem(System::String ^itemID, System::String ^itemValue)
	{

		if (itemID == "WidthInPixels")
		{
			/*int value = 0;
			if (System::Int32::TryParse(itemValue, value)) {
				this->numericUpDownWidthSelection->Value = value;
			}*/
			this->numericUpDownWidthSelection->Value = 5;
			return 0;
		}
		
		return __super::AddItem(itemID, itemValue);
	}
	bool PinAndMediaSelectionDialog::AddItem(System::String ^itemID, int ordinalNumber, System::String ^itemValue)
	{
		System::Reflection::PropertyInfo^ propertyInfo = nullptr;
		try
		{
			System::Type^ type = System::Type::GetType("DialogLib.MediaTypeInfo");
			
			propertyInfo = type->GetProperty(itemID, System::Reflection::BindingFlags::Public |
												System::Reflection::BindingFlags::Instance |
												System::Reflection::BindingFlags::DeclaredOnly);
		}
		catch (System::Exception^)
		{
			propertyInfo = nullptr;
		}

		if(propertyInfo != nullptr)
		{
			if(ordinalNumber < 0)
			{
				return false;
			}
			int typeIndexNumber = (ordinalNumber % 100);
			int pinNumber = ((ordinalNumber - typeIndexNumber) % 10000) / 100;
			int leadingNumber = (ordinalNumber - typeIndexNumber - pinNumber);

			System::Collections::Generic::List<MediaTypeInfo^>^ mediaTypeInfoListOfPin = nullptr;
			if(_mediaTypeInfo->Count <= pinNumber)
			{
				for(int i = _mediaTypeInfo->Count; i <= pinNumber; ++i)
				{
					System::Collections::Generic::List<MediaTypeInfo^>^ tmp = gcnew System::Collections::Generic::List<MediaTypeInfo^>();
					_mediaTypeInfo->Add(tmp);
					comboBoxPinIndex->Items->Add(i.ToString());
				}
			}
			try
			{
				mediaTypeInfoListOfPin = _mediaTypeInfo[pinNumber];
			}
			catch (System::Exception^)
			{
				return false;
			}

			MediaTypeInfo^ currentMediaType = nullptr;
			if(mediaTypeInfoListOfPin->Count <= typeIndexNumber)
			{
				for(int i = mediaTypeInfoListOfPin->Count; i <= typeIndexNumber; ++i)
				{
					mediaTypeInfoListOfPin->Add(gcnew MediaTypeInfo());
				}
			}
			try
			{
				currentMediaType = mediaTypeInfoListOfPin[typeIndexNumber];
			}
			catch (System::Exception^)
			{
				return false;
			}

			System::Drawing::Size tmpSize;
			System::Drawing::Rectangle tmpRectangle;
			Object^ itemVal = nullptr;
			if(propertyInfo->PropertyType->Equals(System::Type::GetType("System.String")))
			{
				if(!GUIDStringToName::_isInit)
				{
					GUIDStringToName::InitGUIDMap();
				}
				if(GUIDStringToName::GUIDMap->ContainsKey(itemValue))
				{
					itemVal = gcnew System::String(GUIDStringToName::GUIDMap[itemValue]);
				}
				else
				{
					itemVal = gcnew System::String(itemValue);
				}
			}
			else if(propertyInfo->PropertyType->Equals(System::Type::GetType("System.UInt16")))
			{
				//itemVal = gcnew System::UInt16();
				System::UInt16 intermed = 0;
				if(!System::UInt16::TryParse(itemValue, intermed))
					return false;
				itemVal = intermed;
			}
			else if(propertyInfo->PropertyType->Equals(System::Type::GetType("System.UInt32")))
			{
				System::UInt32 intermed = 0;
				if(!System::UInt32::TryParse(itemValue, intermed))
					return false;
				itemVal = intermed;
			}
			else if(propertyInfo->PropertyType->Equals(System::Type::GetType("System.UInt64")))
			{
				System::UInt64 intermed = 0;
				if(!System::UInt64::TryParse(itemValue, intermed))
					return false;
				itemVal = intermed;
			}
			else if(propertyInfo->PropertyType->Equals(System::Type::GetType("System.Int16")))
			{
				System::Int16 intermed = 0;
				if(!System::Int16::TryParse(itemValue, intermed))
					return false;
				itemVal = intermed;
			}
			else if(propertyInfo->PropertyType->Equals(System::Type::GetType("System.Int32")))
			{
				System::Int32 intermed = 0;
				if(!System::Int32::TryParse(itemValue, intermed))
					return false;
				itemVal = intermed;
			}
			else if(propertyInfo->PropertyType->Equals(System::Type::GetType("System.Int64")))
			{
				System::Int64 intermed = 0;
				if(!System::Int64::TryParse(itemValue, intermed))
					return false;
				itemVal = intermed;
			}
			else if(propertyInfo->PropertyType->Equals(System::Type::GetType("System.Boolean")))
			{
				System::Boolean intermed = 0;
				if(!System::Boolean::TryParse(itemValue, intermed))
					return false;
				itemVal = intermed;
			}
			else if(propertyInfo->PropertyType->Equals(tmpSize.GetType()))
			{
				System::Drawing::Size intermed = (System::Drawing::Size)propertyInfo->GetValue(currentMediaType, nullptr);
				array<System::Char>^ delims = {'_'};
				array<System::String^>^ stringArray = itemValue->Split(delims);
				if(stringArray->Length != 2)
					return false;
				int intermedWidth = intermed.Width;
				int intermedHeight = intermed.Height;
				if(System::Int32::TryParse(stringArray[0], intermedWidth))
				{
					intermed.Width = intermedWidth;
				}
				if(System::Int32::TryParse(stringArray[1], intermedHeight))
				{
					intermed.Height = intermedHeight;
				}
				itemVal = intermed;
			}
			else if(propertyInfo->PropertyType->Equals(tmpRectangle.GetType()))
			{
				System::Drawing::Rectangle intermed = (System::Drawing::Rectangle)propertyInfo->GetValue(currentMediaType, nullptr);
				array<System::Char>^ delims = {'_'};
				array<System::String^>^ stringArray = itemValue->Split(delims);
				if(stringArray->Length != 4)
					return false;
				int intermedX = intermed.X;
				int intermedY = intermed.Y;
				int intermedWidth = intermed.Width;
				int intermedHeight = intermed.Height;
				if(System::Int32::TryParse(stringArray[0], intermedX))
				{
					intermed.X = intermedX;
				}
				if(System::Int32::TryParse(stringArray[1], intermedY))
				{
					intermed.Y = intermedY;
				}
				if(System::Int32::TryParse(stringArray[2], intermedWidth))
				{
					intermed.Width = intermedWidth - intermed.X;
				}
				if(System::Int32::TryParse(stringArray[3], intermedHeight))
				{
					intermed.Height = intermedHeight - intermed.Y;
				}
				itemVal = intermed;
			}

			propertyInfo->SetValue(currentMediaType, itemVal, nullptr);
			return true;
		}

		return __super::AddItem(itemID, ordinalNumber, itemValue);;
	}
	bool PinAndMediaSelectionDialog::RemoveItem(System::String ^itemID, int ordinalNumber)
	{
		return __super::RemoveItem(itemID, ordinalNumber);
	}
	bool PinAndMediaSelectionDialog::RemoveItems(System::String ^itemID)
	{
		return __super::RemoveItems(itemID);
	}

}

