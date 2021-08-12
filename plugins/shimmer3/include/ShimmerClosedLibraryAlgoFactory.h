// ShimmerClosedLibraryAlgoFactory.h
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

#pragma once

#ifndef ShimmerClosedLibraryAlgoFactory_H
#define ShimmerClosedLibraryAlgoFactory_H

#include <vector>
#include <iostream>
#include <Windows.h>

#include "IShimmerPPGToHRAlgorithm.h"

//the name of the mixed mode dll that wraps Shimmer's closed source C# dll
//only change this if the name of the project that builds the wrapper dll is changed
#define MIXED_MODE_WRAPPER_DLL_NAME "ShimmerPPGtoHRMixedModeWrapper"

//function type definition for the function exported by the mixed mode dll
typedef IShimmerPPGToHRAlgorithm* (*createPPGToHRAlgorithm_fptr_t)(double);

/// <summary>
/// Factory class (implemented as a singleton) that provides access to ShimmerSensing's closed source algorithm classes (e.g. PPGtoHR conversion), which are only accessible
/// in the form of a C# dll binary.
/// The factory internally loads the mixed mode dll written in C++/CLI which is able to forward calls from unmanaged C++ into the C# dll.
/// See the project "ShimmerPPGtoHRMixedModeWrapper" for details.
/// 
/// Usage:
/// 
/// Simply call ShimmerClosedLibraryAlgoFactory::load() once during program startup.
/// Then you can use the factory function ShimmerClosedLibraryAlgoFactory::createPPGToHRAlgoInstance(..) to get an instance of the conversion algorithm.
/// This instance then behaves like any normal object.
/// </summary>
class ShimmerClosedLibraryAlgoFactory {
public:
	/// <summary>
	/// Load the mixed-mode wrapper dll that provides acces to Shimmer's Closed library (C# dll) which contains their Heart Rate algorithms.
	/// Must be called before any other function of this Factory is used.
	/// Multiple calls of this function are garantued to be idempotent, i.e. the Shimmer DLL is only loaded once.
	/// </summary>
	/// <returns>true if mixed-mode library could be loaded</returns>
	static bool load();

	/// <summary>
	/// Factory function that creates a new instance of the PPGtoHRAlgorithm and gives ownership to the caller
	/// </summary>
	/// <param name="samplingRate">The sampling rate that is used</param>
	/// <returns>std::unique_ptr to an IShimmerPPGToHRAlgorithm</returns>
	static PPGToHRAlgorithmUniquePtr createPPGToHRAlgoInstance(double samplingRate);

private:
	ShimmerClosedLibraryAlgoFactory() : _loadedAlgorithmDLL(false), _hDLL(0), _createAlgorithmFuncPtr(0) {};
	~ShimmerClosedLibraryAlgoFactory();

	static ShimmerClosedLibraryAlgoFactory& getInstance() {
		if (_instance == 0) {
			_instance = new ShimmerClosedLibraryAlgoFactory();
		}

		return *_instance;
	}

	bool loadAlgorithmDLL();

	bool _loadedAlgorithmDLL;
	HMODULE _hDLL;
	createPPGToHRAlgorithm_fptr_t _createAlgorithmFuncPtr;

	static ShimmerClosedLibraryAlgoFactory* _instance;
};

#endif // !ShimmerClosedLibraryAlgoFactory_H