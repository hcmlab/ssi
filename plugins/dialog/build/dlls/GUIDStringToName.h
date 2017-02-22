// GUIDStringToName.h
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

namespace DialogLib
{
public ref class GUIDStringToName
{
public:
	GUIDStringToName(void);

	static void InitGUIDMap();
	

	static bool _isInit = false;

public: static System::Collections::Generic::Dictionary<System::String^, System::String^>^ StringMap = 
		gcnew System::Collections::Generic::Dictionary<System::String^, System::String^>();

public: static System::Collections::Generic::Dictionary<System::String^, System::String^>^ GUIDMap = 
		gcnew System::Collections::Generic::Dictionary<System::String^, System::String^>();
};

}
