#include "ShimmerClosedLibraryAlgoFactory.h"
#include <iostream>

bool ShimmerClosedLibraryAlgoFactory::load()
{
	auto& instance = getInstance();
	if (!instance._loadedAlgorithmDLL) {
		instance._loadedAlgorithmDLL = instance.loadAlgorithmDLL();
	}

	return instance._loadedAlgorithmDLL;
}

PPGToHRAlgorithmUniquePtr ShimmerClosedLibraryAlgoFactory::createPPGToHRAlgoInstance(double samplingRate)
{
	auto& instance = getInstance();
	if (instance._loadedAlgorithmDLL) {
		try
		{
			return PPGToHRAlgorithmUniquePtr(instance._createAlgorithmFuncPtr(samplingRate));
		}
		catch (const std::exception& e)
		{
			std::cout << "Error: " << e.what() << "\n";
		}		
	}

	return 0;
}

ShimmerClosedLibraryAlgoFactory::~ShimmerClosedLibraryAlgoFactory()
{
	delete _instance;
	_instance = nullptr;
	_loadedAlgorithmDLL = false;
	if (!FreeLibrary(_hDLL))
	{
		DWORD freeLibraryError = GetLastError();
		std::cout << "Could not unload CreatePPGToHRAlgoInstance.dll!\n";
	}
	_hDLL = 0;
}

ShimmerClosedLibraryAlgoFactory* ShimmerClosedLibraryAlgoFactory::_instance = 0;

bool ShimmerClosedLibraryAlgoFactory::loadAlgorithmDLL()
{
	std::string libraryName = MIXED_MODE_WRAPPER_DLL_NAME;
	#ifdef _DEBUG
		libraryName += "d";
	#endif // _DEBUG
	libraryName += ".dll";

	_hDLL = ::LoadLibrary(libraryName.c_str());

	if (_hDLL) {

		createPPGToHRAlgorithm_fptr_t ppToHrAlgorithmCreation_fptr = (createPPGToHRAlgorithm_fptr_t) ::GetProcAddress(_hDLL, "CreatePPGToHRAlgoInstance");

		if (ppToHrAlgorithmCreation_fptr)
		{
			_createAlgorithmFuncPtr = ppToHrAlgorithmCreation_fptr;
		}
		else
		{
			std::cout << "Could not load PPGToHR constructor!\n";
			DWORD loadLibraryError = GetLastError();
			if (!FreeLibrary(_hDLL))
			{
				DWORD freeLibraryError = GetLastError();
				std::cout << "Could not unload CreatePPGToHRAlgoInstance.dll!\n";
			}
			_hDLL = 0;
			return false;
		}
	}
	else
	{
		std::cout << "Couldn't find ShimmerPPGtoHRMixedModeWrapper.dll";
		return false;
	}

	return true;
}
