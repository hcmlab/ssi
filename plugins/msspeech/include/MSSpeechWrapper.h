// MSSpeechWrapper.h
// authors: Johannes Wagner <wagner@hcm-lab.de>, Kathrin Janowski <janowski@hcm-lab.de>
// created: 2012/10/10
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute itand/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

//------------------------------------------------------------------------------
// <copyright file="SpeechBasics.h" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#pragma once

#include "MSSpeechAudioStream.h"
#include "MSSpeechPhraseEvent.h"

// Windows Header Files
#include <windows.h>
#include <Shlobj.h>

// For configuring DMO properties
#include <wmcodecdsp.h>

// For FORMAT_WaveFormatEx and such
#include <uuids.h>

// For assembling the result
#include <sstream>
#include <string>

//--------------------------------------------------------------------------------------------------------------
// For the Speech Platform API
// NOTE: To ensure that the application compiles and links against the Speech Platform SDK instead of SAPI,
//		 VC++ include and library paths must list the appropriate paths in the SDK's installation directory
//		 before listing the default system include and library directories.
#include <sphelper.h>

// Enforce the use of Microsoft Speech Platform by looking for constants
// that only exist in the sapi.h of the Speech Platform SDK,
// but not in the default sapi.h of SAPI alone.
// If these constants are missing, raise a compiler error.
#if (!defined SPCAT_PROMPTVOICES) || (!defined SPCAT_TEXTNORMALIZERS)
	#error Requires Microsoft Speech Platform SDK.
#else
	#pragma message("Found Microsoft Speech Platform SDK.")
#endif

//// NOTE: It might be more reliable to check whether the Registry Key Names include "Speech Server\\v11.0".
//// However, that requires a wchar comparison which doesn't seem to work in the preprocessor directive. 
//#define SAPI_CHECK = wcscmp(SPREG_USER_ROOT,  L"HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Speech Server\\v11.0")
//#if SAPI_CHECK > 0
//	#error Requires Microsoft Speech Platform SDK. 
//#endif
//--------------------------------------------------------------------------------------------------------------


// Safe release for interfaces
template<class Interface>
inline void SafeRelease( Interface *& pInterfaceToRelease )
{
    if ( pInterfaceToRelease != NULL )
    {
        pInterfaceToRelease->Release();
        pInterfaceToRelease = NULL;
    }
}

/// <summary>
/// Main application class for SpeechBasics sample.
/// </summary>
class MSSpeechWrapper
{
public:

	enum OutputFormat {KEYWORD, PLAIN_TEXT, STRUCTURE};

    /// <summary>
    /// Constructor
    /// </summary>
    MSSpeechWrapper (MSSpeechPhraseEvent *callback,
		const char *grammar,
		const char *option,
		MSSpeechAudioStream *stream,
		const char *outputFormat);

    /// <summary>
    /// Destructor
    /// </summary>
    ~MSSpeechWrapper();

    /// <summary>
    /// Handles window messages, passes most to the class instance to handle
    /// </summary>
    /// <param name="hWnd">window message is for</param>
    /// <param name="uMsg">message</param>
    /// <param name="wParam">message data</param>
    /// <param name="lParam">additional message data</param>
    /// <returns>result of message processing</returns>
    static LRESULT CALLBACK MessageRouter(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    /// <summary>
    /// Handle windows messages for a class instance
    /// </summary>
    /// <param name="hWnd">window message is for</param>
    /// <param name="uMsg">message</param>
    /// <param name="wParam">message data</param>
    /// <param name="lParam">additional message data</param>
    /// <returns>result of message processing</returns>
    LRESULT CALLBACK        DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    /// <summary>
    /// Begins processing
    /// </summary>
	bool					Init ();
    bool                    Start();	
    bool					Stop ();

	// Reset Speech Grammar
	HRESULT					ResetSpeechGrammar(const char *grammar);


private:

    wchar_t					*m_grammarFileName;
	wchar_t					*m_optionString;
	MSSpeechPhraseEvent		*m_callback;

    // Audio stream captured from Kinect.
    MSSpeechAudioStream*		m_pAudioStream;

    // Stream given to speech recognition engine
    ISpStream*              m_pSpeechStream;

    // Speech recognizer
    ISpRecognizer*          m_pSpeechRecognizer;

    // Speech recognizer context
    ISpRecoContext*         m_pSpeechContext;

    // Speech grammar
    ISpRecoGrammar*         m_pSpeechGrammar;

    // Output Format
    OutputFormat			m_outputFormat;

	// Event triggered when we detect speech recognition
    HANDLE                  m_hSpeechEvent;

	// Event triggered when we want to stop speech recognition
	HANDLE					m_hStopEvent;

    /// <summary>
    /// Initialize Kinect audio stream object.
    /// </summary>
    /// <returns>S_OK on success, otherwise failure code.</returns>
    HRESULT                 InitializeAudioStream();

    /// <summary>
    /// Create speech recognizer that will read Kinect audio stream data.
    /// </summary>
    /// <returns>
    /// <para>S_OK on success, otherwise failure code.</para>
    /// </returns>
    HRESULT                 CreateSpeechRecognizer();

    /// <summary>
    /// Load speech recognition grammar into recognizer.
    /// </summary>
    /// <returns>
    /// <para>S_OK on success, otherwise failure code.</para>
    /// </returns>
    HRESULT                 LoadSpeechGrammar();

    /// <summary>
    /// Start recognizing speech asynchronously.
    /// </summary>
    /// <returns>
    /// <para>S_OK on success, otherwise failure code.</para>
    /// </returns>
    HRESULT                 StartSpeechRecognition();

    /// <summary>
    /// Pause asynchronous speech recognition.
    /// </summary>
    /// <returns>
    /// <para>S_OK on success, otherwise failure code.</para>
    /// </returns>
    HRESULT                 PauseSpeechRecognition();

    /// <summary>
    /// Resume asynchronous speech recognition.
    /// </summary>
    /// <returns>
    /// <para>S_OK on success, otherwise failure code.</para>
    /// </returns>
    HRESULT                 ResumeSpeechRecognition();

	/// <summary>
    /// Process recently triggered speech recognition events.
    /// </summary>
    void                    ProcessSpeech();

	/// <summary>
	/// Parses the output format code from a string.
	/// </summary>
	/// <returns>
	/// the matching OutputFormat code, or KEYWORD if nothing matches
	/// </returns>
	OutputFormat			ParseOutputFormat (const char* formatStr);

	/// <summary>
	/// Converts a wide string to a std::string.
	/// <returns>
	/// the converted string
	/// </returns>
	std::string				MSSpeechWrapper::convertWideStr(LPCWSTR wideStr);



    /// <summary>
    /// Maps a specified speech semantic tag to the corresponding action to be performed on turtle.
    /// </summary>
    /// <returns>
    /// Action that matches <paramref name="pszSpeechTag"/>, or TurtleActionNone if no matches were found.
    /// </returns>
    void					MapSpeechTagToAction (const SPPHRASEPROPERTY* pSemanticTag, ULONGLONG start_ms, ULONGLONG dur_ms);


    void					AssembleResultProlog (ISpRecoResult* result);
	std::string				PrintSemanticTagProlog (const SPPHRASEPROPERTY* tag, /*const SPPHRASERULE* rule,*/ const SPPHRASEELEMENT* elements);

	void					printSemanticKey(std::string keyStr, std::stringstream &outstream);

	const SPPHRASERULE*		findRule(const SPPHRASEPROPERTY* tag, const SPPHRASERULE* currentRule);
	const SPPHRASERULE*		refineRule(ULONG firstElem, ULONG lastElem, const SPPHRASERULE* currentRule);

	void					Temp_HypothesisResult (ISpRecoResult* result);



};
