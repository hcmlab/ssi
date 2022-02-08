// dllmain.cpp
// author: Fabian Wildgrube
// created: 2021/07/24
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************


/**
* This file defines a so-called "mixed-mode" dll. This is a dll making use of the C++/CLI flavour, which offers managed
* (i.e. garbage-collected) Visual C++. This flavour of C++ is able to load other managed code, like a C# dll, and call into it.
* At the same time it can also provide an unmanaged "pure" C++ surface for other code to interact with.
* [achieved with the compiler flag /clr (common language runtime) enabled. This only works on windows.]
* 
* This way, this dll is a bridge from SSI's Shimmer3 plugin to the ShimmerClosedLibrary, which is a C# dll by ShimmerSensing that contains
* their PPGtoHR conversion algorithm. Since the source for this library is not open and only the dll is distributed with the open source 
* C# ShimmerAPI we needed to build this wrapper to be able to use the algorithms from within SSI's unmanaged C++ context.
* 
* Wihtin this file are a couple components:
*	1. ManagedDLLHelper - a class that takes care of loading the Shimmer C# dll
*	2. ShimmerPPGToHRAlgorithmUnmanagedWrapper - a class that implements the IShimmerPPGToHRAlgorithm interface by calling into the C# API.
*													this class is the literal "bridge" from unmanaged SSI C++ into the managed C++ and in turn C#
*	3. The dll export function "CreatePPGToHRAlgoInstance" - a simple factory function that creates an instance of the ShimmerPPGToHRAlgorithmUnmanagedWrapper
*																and returns a pointer to it.
* 
* IMPORTANT: The C# dll depends on the "MathNet.Numerics" C# package. Therefore this package's dll was added to ssi's bin folder. Make sure this is the case
* if you run into errors!
* 
* This mixed-mode dll approach is adapted from this article: https://support.goldsim.com/hc/en-us/articles/217950017-DLL-using-NET-C-Framework?mobile_site=true
*/


#include "IShimmerPPGToHRAlgorithm.h"
#include <iostream>
#include <msclr/auto_gcroot.h>
#include <msclr/marshal_cppstd.h>

using namespace System;
using namespace System::Reflection;

#using <ShimmerClosedLibraryRev0_5.dll>
#define SHIMMER_CLOSEDLIBRARY_NAME "ShimmerClosedLibraryRev0_5.dll"

#ifndef DLLEXP
#define DLLEXP extern "C" __declspec( dllexport )
#endif

// The following helper class allows the Shimmer Closed .Net assembly to be loaded from the same
// folder in which this (mixed-mode) DLL is located (i.e. the SSI binaries folder)
// taken almost verbatim from https://support.goldsim.com/hc/en-us/articles/217950017-DLL-using-NET-C-Framework?mobile_site=true
public ref class ManagedDLLHelper
{
public:
	static void PrepareAssembly()
	{
		static bool wasAlreadyExecuted = false;

		if (!wasAlreadyExecuted) {
			//only load the Shimmer closed DLL once
			AppDomain^ currentDomain = AppDomain::CurrentDomain;
			currentDomain->AssemblyResolve += gcnew ResolveEventHandler(&ManagedDLLHelper::AssemblyResolveEventHandler);
			wasAlreadyExecuted = true;
		}
	}

private:

	static String^ GetDllPath()
	{
		// Determine the path of the mixed-mode DLL (this DLL).
		String^ myLocation = ManagedDLLHelper::typeid->Assembly->Location;
		return System::IO::Path::GetDirectoryName(myLocation);

	}

	static Assembly^ AssemblyResolveEventHandler(Object^ sender, ResolveEventArgs^ args)
	{
		// Manually load the Shimmer Closed Library dll here by correcting the path to match the location of the mixed-mode DLL because we assume that both are located in ssi bin folder.
		String^ CorrectedPath = GetDllPath() + "\\" + SHIMMER_CLOSEDLIBRARY_NAME;

		try
		{
			Assembly^ a = Assembly::LoadFrom(CorrectedPath);
			std::cout << "Was able to load the" << SHIMMER_CLOSEDLIBRARY_NAME << "\n";
			return a;
		}
		catch (Exception^ ex)
		{
			Console::WriteLine("Exception loading ShimmerClosedLibraryRev0_5 dll");
			Console::WriteLine(ex->Message);
		}

		return Assembly::LoadFrom(CorrectedPath); //just here to satisfy the function return requirement
	}
};

/// <summary>
/// Class that implements the IShimmerPPGToHRAlgorithm by wrapping an instance of the algorithm from the C# dll, thus providing
/// the "translation" between unmanaged and managed contexts
/// </summary>
class ShimmerPPGToHRAlgorithmUnmanagedWrapper : public IShimmerPPGToHRAlgorithm {
public:
	ShimmerPPGToHRAlgorithmUnmanagedWrapper(double samplingRate) : _algorithmInstance(gcnew ShimmerLibrary::PPGToHRAlgorithm(samplingRate, 1, 10)) {};

	double ppgToHrConversion(double data, double timestamp) override {
		try
		{
			return _algorithmInstance->ppgToHrConversion(data, timestamp);
		}
		catch (Exception^ ex)
		{
			Console::WriteLine("Exception during ppgToHrConversion in c# dll");
			Console::WriteLine(ex->Message);
		}
		return -999.0;
	}

private:
	gcroot<ShimmerLibrary::PPGToHRAlgorithm^> _algorithmInstance;
};

/// <summary>
/// Factory function that creates an instance of a class that implements IShimmerPPGToHRAlgorithm
/// </summary>
/// <param name="samplingRate">The sampleRate that the shimmer device is running at</param>
/// <returns></returns>
DLLEXP IShimmerPPGToHRAlgorithm* CreatePPGToHRAlgoInstance(double samplingRate) {
	// Load .Net assembly from same folder as this mixed-mode (native & managed) DLL.
	ManagedDLLHelper::PrepareAssembly();

	return new ShimmerPPGToHRAlgorithmUnmanagedWrapper(samplingRate);
}