// IKeywordEvent.h
// authors: Johannes Wagner <wagner@hcm-lab.de>, Kathrin Janowski <janowski@hcm-lab.de>, Tobias Baur <baur@hcm-lab.de>, Arthur Baude
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
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

//------------------------------------------------------------------------------
// <copyright file="SpeechBasics.cpp" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#include "MSSpeechWrapper.h"

#define INITGUID
#include <guiddef.h>

// This is the class ID we expect for the Microsoft Speech recognizer.
// Other values indicate that we're using a version of sapi.h that is
// incompatible with this sample.
DEFINE_GUID(CLSID_ExpectedRecognizer, 0x495648e7, 0xf7ab, 0x4267, 0x8e, 0x0f, 0xca, 0xfb, 0x7a, 0x33, 0xc1, 0x60);

/// <summary>
/// Constructor
/// </summary>
MSSpeechWrapper::MSSpeechWrapper(MSSpeechPhraseEvent *callback,
	const char *grammar,
	const char *option,
	MSSpeechAudioStream *stream,
	const char *outputFormat) :
	m_callback (callback),
	m_grammarFileName (0),
    m_pAudioStream(stream),
    m_pSpeechStream(NULL),
    m_pSpeechRecognizer(NULL),
    m_pSpeechContext(NULL),
    m_pSpeechGrammar(NULL),
	m_outputFormat(KEYWORD),
    m_hSpeechEvent(INVALID_HANDLE_VALUE),
	m_hStopEvent (INVALID_HANDLE_VALUE)
{

	int len = (int)strlen(grammar)+1;
	m_grammarFileName = new wchar_t[len];
	memset (m_grammarFileName,0,len);
	::MultiByteToWideChar (CP_ACP, NULL, grammar, -1, m_grammarFileName, len);

	len = (int)strlen(option)+1;
	m_optionString = new wchar_t[len];
	memset (m_optionString, 0, len);
	::MultiByteToWideChar (CP_ACP, NULL, option, -1, m_optionString, len);

	m_outputFormat = ParseOutputFormat(outputFormat);

	m_hStopEvent = ::CreateEvent (NULL, TRUE, FALSE, NULL);	
}

/// <summary>
/// Destructor
/// </summary>
MSSpeechWrapper::~MSSpeechWrapper()
{
	if (m_hStopEvent != NULL) {
		CloseHandle (m_hStopEvent);
	}
	
	if (m_pSpeechRecognizer)
    {
        m_pSpeechRecognizer->SetRecoState(SPRST_INACTIVE);
    }

	//TODO:
	//Why does the plugin crash if the pipeline is stopped
	//before any speech input was detected?
	// Why Comment This?
    //SafeRelease(m_pSpeechStream);
    //SafeRelease(m_pSpeechRecognizer);
    SafeRelease(m_pSpeechContext);
    SafeRelease(m_pSpeechGrammar);
}
 
bool MSSpeechWrapper::Init () {

    HRESULT hr;

    hr = InitializeAudioStream();
    if (FAILED(hr))
    {
		printf("[speech] Could not initialize audio stream.\n");
        return false;
    }

    hr = CreateSpeechRecognizer();
    if (FAILED(hr))
    {
        printf("[speech] Could not create speech recognizer. Please ensure that Microsoft Speech Platform SDK and other sample requirements are installed.\n");
        return false;
    }

    hr = LoadSpeechGrammar();
    if (FAILED(hr))
    {
        printf("[speech] Could not load speech grammar. Please ensure that grammar configuration file was properly deployed.\n");
        return false;
    }

    return true;
}

/// <summary>
/// Begins processing
/// </summary>
bool MSSpeechWrapper::Start()
{
	HRESULT hr = StartSpeechRecognition();
    if (FAILED(hr))
    {
        printf("[speech] Could not start recognizing speech.\n");
        return false;
    }

    const int eventCount = 2;
    HANDLE hEvents[eventCount];

    hEvents[0] = m_hSpeechEvent;
	hEvents[1] = m_hStopEvent;

	while (true) {

		// Check to see if we have either a message (by passing in QS_ALLINPUT)
		// Or a speech event (hEvents)
		DWORD dwEvent = MsgWaitForMultipleObjectsEx(eventCount, hEvents, INFINITE, NULL, 0);

		// Check if this is an event we're waiting on and not a timeout or message
		if (WAIT_OBJECT_0 == dwEvent)
		{
			ProcessSpeech();
		} else {
			break;
		}
	}

	return true;
}


bool MSSpeechWrapper::Stop () {

	if (::SetEvent (m_hStopEvent) == FALSE) {
		return false;
	}
	return true;
}


/// <summary>
/// Initialize audio stream object.
/// </summary>
/// <returns>
/// <para>S_OK on success, otherwise failure code.</para>
/// </returns>
HRESULT MSSpeechWrapper::InitializeAudioStream()
{
    IStream* pStream = NULL;

    // Set DMO output format
    WAVEFORMATEX wfxOut = {AudioFormat, AudioChannels, AudioSamplesPerSecond, AudioAverageBytesPerSecond, AudioBlockAlign, AudioBitsPerSample, 0};
    DMO_MEDIA_TYPE mt = {0};
    MoInitMediaType(&mt, sizeof(WAVEFORMATEX));
    
    mt.majortype = MEDIATYPE_Audio;
    mt.subtype = MEDIASUBTYPE_PCM;
    mt.lSampleSize = 0;
    mt.bFixedSizeSamples = TRUE;
    mt.bTemporalCompression = FALSE;
    mt.formattype = FORMAT_WaveFormatEx;	
    memcpy(mt.pbFormat, &wfxOut, sizeof(WAVEFORMATEX));
    
    HRESULT hr = m_pAudioStream->QueryInterface(IID_IStream, (void**)&pStream);

    if (SUCCEEDED(hr))
    {
        hr = CoCreateInstance(CLSID_SpStream, NULL, CLSCTX_INPROC_SERVER, __uuidof(ISpStream), (void**)&m_pSpeechStream);

        if (SUCCEEDED(hr))
        {
            hr = m_pSpeechStream->SetBaseStream(pStream, SPDFID_WaveFormatEx, &wfxOut);
        }
    }


    MoFreeMediaType(&mt);
    SafeRelease(pStream);

    return hr;
}

/// <summary>
/// Create speech recognizer that will read audio stream data.
/// </summary>
/// <returns>
/// <para>S_OK on success, otherwise failure code.</para>
/// </returns>
HRESULT MSSpeechWrapper::CreateSpeechRecognizer()
{
    ISpObjectToken *pEngineToken = NULL; 
	HRESULT hr = NULL;

	hr = SpFindBestToken(SPCAT_RECOGNIZERS,m_optionString,NULL,&pEngineToken);
	if (!SUCCEEDED(hr))
    {
		printf("[speech] Error: specified recognizier not available.");
		return hr;
	}

    hr = CoCreateInstance(CLSID_SpInprocRecognizer, NULL, CLSCTX_INPROC_SERVER, __uuidof(ISpRecognizer), (void**)&m_pSpeechRecognizer);
    if (SUCCEEDED(hr))
    {		
        hr = m_pSpeechRecognizer->SetInput(m_pSpeechStream, FALSE);
        if (SUCCEEDED(hr))
        {
			m_pSpeechRecognizer->SetRecognizer(pEngineToken);
            hr = m_pSpeechRecognizer->CreateRecoContext(&m_pSpeechContext);
        }
    }

	//DEBUG: get the recognizer ID string
	WCHAR *recognizerID;
	pEngineToken->GetId(&recognizerID);
	std::string recognizerIDStr = convertWideStr(recognizerID);

	
    SafeRelease(pEngineToken);

	if (!SUCCEEDED (hr)) {
		printf("[speech] Error: could not create speech recognizer.");
	}


	//#ifdef DEBUG
	printf("[speech] -------------------------------------------------------\nSpeech plugin parameters:\n");
	
	printf("[speech] \t speech recognizer: %s\n", recognizerIDStr.c_str());

	switch(m_outputFormat)
	{
		case KEYWORD: printf("[speech] \toutput mode:\tkeyword\n"); break;
		case PLAIN_TEXT: printf("[speech] \toutput mode:\tplain text\n"); break;
		case STRUCTURE: printf("[speech] \toutput mode:\tPROLOG feature structure\n"); break;
		default: printf("[speech] \toutput mode:\tunknown\n"); break;
	}


	////display recognizer parameters ------------------------------------
	////TODO: the values do not make sense (yet?) -> when exactly are they set?

	//printf("[speech] -------------------------------------------------------\nSpeech recognizer parameters:\n");
	//

	//LONG normalRespSpeed, complexRespSpeed, threshHigh, threshNorm, threshLow, threshRej;
	//
	//m_pSpeechRecognizer->GetPropertyNum(L"SPPROP_RESPONSE_SPEED", &normalRespSpeed); 
	//m_pSpeechRecognizer->GetPropertyNum(L"SPPROP_COMPLEX_RESPONSE_SPEED", &normalRespSpeed); 

	//m_pSpeechRecognizer->GetPropertyNum(L"SPPROP_HIGH_CONFIDENCE_THRESHOLD", &threshHigh); 
	//m_pSpeechRecognizer->GetPropertyNum(L"SPPROP_NORMAL_CONFIDENCE_THRESHOLD", &threshNorm); 
	//m_pSpeechRecognizer->GetPropertyNum(L"SPPROP_LOW_CONFIDENCE_THRESHOLD", &threshLow); 
	//m_pSpeechRecognizer->GetPropertyNum(L"SPPROP_REJECTION_CONFIDENCE_THRESHOLD", &threshRej); 
	//
	//printf("[speech] \ttime out after unambiguous speech:\t%li\n", normalRespSpeed);
	//printf("[speech] \ttime out after ambiguous speech:\t%li\n", complexRespSpeed);
	//printf("[speech] \n");

	//printf("[speech] \thigh confidence threshold:\t%li\n", threshHigh);
	//printf("[speech] \tnormal confidence threshold:\t%li\n", threshNorm);
	//printf("[speech] \tlow confidence threshold:\t%li\n", threshLow);
	//printf("[speech] \trejection confidence threshold:\t%li\n", threshRej);

	printf("[speech] -------------------------------------------------------\n");
//
//#endif


    return hr;
}

/// <summary>
/// Load speech recognition grammar into recognizer.
/// </summary>
/// <returns>
/// <para>S_OK on success, otherwise failure code.</para>
/// </returns>
HRESULT MSSpeechWrapper::LoadSpeechGrammar()
{
	printf("[speech] Trying to load grammar...\n");
	HRESULT hr = m_pSpeechContext->CreateGrammar(1, &m_pSpeechGrammar);

    if (SUCCEEDED(hr))
    {
        // Populate recognition grammar from file
		hr = m_pSpeechGrammar->LoadCmdFromFile(m_grammarFileName, SPLO_STATIC);
		//
		if(SUCCEEDED(hr)) 
		{
			// Debug some information
			wprintf(L"[speech] Loaded grammar '%s'\n", m_grammarFileName);
		} else {
			// Debug some information
			wprintf(L"[speech] Could not load grammar '%s'\n", m_grammarFileName);
		}

    } else {
		// Debug some information
		wprintf(L"[speech] Could not create new grammar '%s'\n", m_grammarFileName);
	}

    return hr;
}

// Reset Speech Grammar
HRESULT MSSpeechWrapper::ResetSpeechGrammar(const char *grammar)
{
	////DEBUG
	//printf("[speech] received command to load grammar \"%s\"\n", grammar);

	// Convert grammar file name
	int len = (int)strlen(grammar)+1;
	m_grammarFileName = new wchar_t[len];
	memset (m_grammarFileName,0,len);
	::MultiByteToWideChar (CP_ACP, NULL, grammar, -1, m_grammarFileName, len);

	//try to pause the recognition
	HRESULT hr = PauseSpeechRecognition();

	if(SUCCEEDED(hr))
	{
		//load the grammar
		hr = LoadSpeechGrammar();
		if(SUCCEEDED(hr))
		{
			//try to resume the recognition
			hr = ResumeSpeechRecognition();
			printf("[speech] grammar successfully replaced\n");
		}
		else
		{
			printf("[speech] grammar invalid, can't resume recognition\n");
		}
	}
	else
	{
		printf("[speech] Can't load grammar while recognition is active.\n");
	}
	

	delete[] m_grammarFileName;
    return hr;
}

/// <summary>
/// Start recognizing speech asynchronously.
/// </summary>
/// <returns>
/// <param>S_OK on success, otherwise failure code.</param>
/// </returns>
HRESULT MSSpeechWrapper::StartSpeechRecognition()
{
    // Specify that all top level rules in grammar are now active
    m_pSpeechGrammar->SetRuleState(NULL, NULL, SPRS_ACTIVE);

    // Specify that engine should always be reading audio
    m_pSpeechRecognizer->SetRecoState(SPRST_ACTIVE_ALWAYS);

    // Specify that we're only interested in receiving recognition events
    m_pSpeechContext->SetInterest(SPFEI(SPEI_RECOGNITION), SPFEI(SPEI_RECOGNITION));

    // Ensure that engine is recognizing speech and not in paused state
    HRESULT hr = m_pSpeechContext->Resume(0);
    if (SUCCEEDED(hr))
    {
        m_hSpeechEvent = m_pSpeechContext->GetNotifyEventHandle();
    }

        
    return hr;
}


/// <summary>
/// Pause asynchronous speech recognition.
/// </summary>
/// <returns>
/// <param>S_OK on success, otherwise failure code.</param>
/// </returns>
HRESULT MSSpeechWrapper::PauseSpeechRecognition()
{
	HRESULT hr = S_OK;
	
	m_pSpeechGrammar->SetRuleState(NULL, NULL, SPRS_INACTIVE);
	printf("[speech] rules deactivated\n");

    // Pause the speech context
	// (Don't deactivate the recognition engine - that would reset the audio stream and time.)
	hr = m_pSpeechContext->Pause(0);

    if (SUCCEEDED(hr))
    {
        printf("[speech] recognition context paused\n");

		// printf("[speech] recognizer paused\n");
		// disable old grammar and rules
		hr = m_pSpeechGrammar->SetGrammarState(SPGS_DISABLED);
		if (SUCCEEDED(hr)){
			printf("[speech] grammar deactivated\n");
		}
		else{				
			printf("[speech] failed to deactivate grammar\n");
		}
	}
	else 
    {
        printf("[speech] failed to pause recognition context\n");
    }


	return hr;
}




/// <summary>
/// Resume asynchronous speech recognition.
/// </summary>
/// <returns>
/// <param>S_OK on success, otherwise failure code.</param>
/// </returns>
HRESULT MSSpeechWrapper::ResumeSpeechRecognition()
{
	HRESULT hr = S_OK;
	
	// activate all top level rules in the grammar
    m_pSpeechGrammar->SetRuleState(NULL, NULL, SPRS_ACTIVE);

	hr = m_pSpeechGrammar->SetGrammarState(SPGS_ENABLED);
	if(SUCCEEDED(hr))
	{
		printf("[speech] grammar activated\n");

		// Specify that we're interested in receiving recognition events
		m_pSpeechContext->SetInterest(SPFEI(SPEI_RECOGNITION), SPFEI(SPEI_RECOGNITION));

		// resume the speech context
		hr = m_pSpeechContext->Resume(0);
		if (SUCCEEDED(hr))
		{
			printf("[speech] recognition context resumed\n");

			//get the event handle
		    m_hSpeechEvent = m_pSpeechContext->GetNotifyEventHandle();
		}
		else 
		{
			printf("[speech] failed to resume recognition context\n");
		}		
	}
	else
	{
		printf("[speech] failed to activate grammar: %d\n", hr);
	}

	return hr;
}

/// <summary>
/// Process recently triggered speech recognition events.
/// </summary>
void MSSpeechWrapper::ProcessSpeech()
{
    SPEVENT curEvent;
    ULONG fetched = 0;
    HRESULT hr = S_OK;

    m_pSpeechContext->GetEvents(1, &curEvent, &fetched);

    while (fetched > 0)
    {
        switch (curEvent.eEventId)
        {
            case SPEI_RECOGNITION:
                if (SPET_LPARAM_IS_OBJECT == curEvent.elParamType)
                {
                    //printf("[speech] MSSpeechWrapper | recognized speech\n");

					// this is an ISpRecoResult
                    ISpRecoResult* result = reinterpret_cast<ISpRecoResult*>(curEvent.lParam);
                    
						//TODO K make an enum out of that/pass it down as an option
					switch(m_outputFormat)
					{
						case KEYWORD:
							{
								SPPHRASE* pPhrase = NULL;

								SPRECORESULTTIMES recTimes;
								result->GetResultTimes (&recTimes);	
	
								ULONGLONG start_ms = recTimes.ullStart / 10000;
								ULONGLONG dur_ms = recTimes.ullLength / 10000;
                    
			                    hr = result->GetPhrase(&pPhrase);
						        if (SUCCEEDED(hr))
								{
									//original implementation: first keyword
									if ((pPhrase->pProperties != NULL) && (pPhrase->pProperties->pFirstChild != NULL))
									{
										const SPPHRASEPROPERTY* pSemanticTag = pPhrase->pProperties->pFirstChild;
										MapSpeechTagToAction(pSemanticTag, start_ms, dur_ms);

										/*
										while ((pSemanticTag = pSemanticTag->pNextSibling) != NULL) {
												MapSpeechTagToAction(pSemanticTag, start_ms, dur_ms);
											}*/
									}
									::CoTaskMemFree(pPhrase);
								}
								break;
							}
						case PLAIN_TEXT:
							{
								SPPHRASE* pPhrase = NULL;

								SPRECORESULTTIMES recTimes;
								result->GetResultTimes (&recTimes);	
	
								ULONGLONG start_ms = recTimes.ullStart / 10000;
								ULONGLONG dur_ms = recTimes.ullLength / 10000;

								hr = result->GetPhrase(&pPhrase);
						        if (SUCCEEDED(hr))
								{
									float confidence = 0.0f;
									if (pPhrase->pProperties != NULL)
										confidence = pPhrase->pProperties->Confidence;
									
									WCHAR *utterance;
									hr = result->GetText(SP_GETWHOLEPHRASE, SP_GETWHOLEPHRASE, TRUE, &utterance, NULL);
									if (SUCCEEDED(hr))
									{
										int iSize = WideCharToMultiByte (CP_ACP,0,utterance, -1, NULL, 0, NULL, NULL );
										char* lpNarrow = new char[iSize];
										lpNarrow[0] = 0;
										iSize= WideCharToMultiByte(CP_ACP, 0, utterance, -1, lpNarrow, iSize, NULL, NULL );	

										if (m_callback) {
											m_callback->update (lpNarrow, start_ms, dur_ms, confidence);
										} else {
											printf("[speech] %s[%.2f]\n",lpNarrow,confidence);
										}
									}
									
									::CoTaskMemFree(pPhrase);
								}
								break;
							}
						case STRUCTURE:
							{
								AssembleResultProlog(result);
								break;
							}
						default: break;
					}
				}
			case SPEI_HYPOTHESIS:
				//if (SPET_LPARAM_IS_OBJECT == curEvent.elParamType)
    //            {
				//	// printf("[speech] MSSpeechWrapper | hypothesis\n");
				//	// this is an ISpRecoResult
    //                ISpRecoResult* result = reinterpret_cast<ISpRecoResult*>(curEvent.lParam);
				//	Temp_HypothesisResult(result);
				//}
				break;
			case SPEI_FALSE_RECOGNITION: //printf("[speech] MSSpeechWrapper | false recognition\n");
				break;
			default: //printf("[speech] MSSpeechWrapper | unknown event\n");
				break;
        }

        m_pSpeechContext->GetEvents(1, &curEvent, &fetched);
    }

    return;
}

/// <summary>
/// Maps a specified speech semantic tag to the corresponding action to be performed on turtle.
/// </summary>
/// <returns>
/// Action that matches <paramref name="pszSpeechTag"/>, or TurtleActionNone if no matches were found.
/// </returns>
void MSSpeechWrapper::MapSpeechTagToAction (const SPPHRASEPROPERTY* pSemanticTag, ULONGLONG start_ms, ULONGLONG dur_ms)
{    
	LPCWSTR pszSpeechTag = pSemanticTag->pszValue;
	float confidence = pSemanticTag->SREngineConfidence;

	// convert wide char
	int iSize = WideCharToMultiByte (CP_ACP,0,pszSpeechTag, -1, NULL, 0, NULL, NULL );
	char* lpNarrow = new char[iSize];
	lpNarrow[0] = 0;
	iSize= WideCharToMultiByte(CP_ACP, 0, pszSpeechTag, -1, lpNarrow, iSize, NULL, NULL );	

	if (m_callback) {
		m_callback->update (lpNarrow, start_ms, dur_ms, confidence);
	} else {
		printf("[speech] %s[%.2f]\n",lpNarrow,confidence);
	}


	delete [] lpNarrow;
}




/// <summary>
/// Assembles a string containing the semantic content of the recognized phrase.
/// </summary>
void MSSpeechWrapper::AssembleResultProlog (ISpRecoResult* result)
{    
    SPPHRASE* pPhrase = NULL;

	HRESULT hr = S_OK;

	hr = result->GetPhrase(&pPhrase);
    if (SUCCEEDED(hr))
    {
		const SPPHRASEELEMENT *pElements = pPhrase->pElements;
		ULONG elemCount = pPhrase->Rule.ulCountOfElements;
	
		//------------------------------------------------------------------------------------
        // prepare data string  
        //------------------------------------------------------------------------------------
        std::stringstream dataWriter;

        // speech header ---------------------------------------------------------------------

		dataWriter	<< "[";
		
		//// get the raw text that was spoken
		//WCHAR *utterance;
		//result->GetText(SP_GETWHOLEPHRASE, SP_GETWHOLEPHRASE, FALSE, &utterance, NULL);
		//std::string utteranceStr = convertWideStr(utterance);
		//
		////workaround for GARBAGE result  
		//if(utteranceStr.compare("...") != 0)
		//	dataWriter		<< "utterance:\'" << utteranceStr << "\',";
		
		//------------------------------------------------------------------------------------
		// confidence value
		//------------------------------------------------------------------------------------

		// get the rule confidence -----------------------------------------------------------
		
		// from the Speech Platform documentation:
		//		"Confidence for this rule computed by the SR engine.
		//		 The value is engine dependent
		//		 and not standardized across multiple SR engines."

		float ruleConfidence = pPhrase->Rule.SREngineConfidence;
		//printf("[speech] rule confidence: %f\n", ruleConfidence);


		//------------------------------------------------------------------------------------
		// (TODO) get alternative interpretations
		//------------------------------------------------------------------------------------

		//ISpPhraseAlt* alternates;
		//ULONG altCount;

		//result->GetAlternates(0, 10, 10, &alternates, &altCount);
		//for(int i=0; i<altCount; i++)
		//{
		//	SPPHRASE* altPhrase = NULL;
		//	alternates[i].GetPhrase(&altPhrase); 
		//	printf("[speech] \talternate %d: rule confidence = %f, SR confidence= %f", i, altPhrase->Rule.SREngineConfidence, altPhrase->pProperties->SREngineConfidence); 

		//	::CoTaskMemFree(altPhrase);
		//}


		//------------------------------------------------------------------------------------
		// semantic data
		//------------------------------------------------------------------------------------

		if(pPhrase->pProperties != NULL)
		{
			//printf("[speech] assembling semantic information...\n");

			// get the confidence of the semantic (root) element ---------------------------------
		
			// from the Speech Platform documentation:
			//		"Confidence value for this semantic property
			//		 computed by the speech recognition (SR) engine.
			//		 The value range is specific to each SR engine."
		
			//float propConfidence = pPhrase->pProperties->SREngineConfidence;
			//printf("[speech] properties confidence: %f\n", propConfidence);

			// start with the first child tag
			if (pPhrase->pProperties->pFirstChild != NULL)
			{
				//printf("[speech] speech has semantics...\n");

				const SPPHRASEPROPERTY* pSemanticTag = pPhrase->pProperties->pFirstChild;
				const SPPHRASERULE* pRule = &(pPhrase->Rule);	//TODO: required for manually reconstructing semantics during incremental processing

				
				//// get the confidence of the current semantic element ---------------------------
		
				//// from the Speech Platform documentation:
				////		"Confidence value for this semantic property
				////		 computed by the speech recognition (SR) engine.
				////		 The value range is specific to each SR engine."
				//float tagConfidence = pSemanticTag->SREngineConfidence;
				//dataWriter << ",conf:" << tagConfidence;				


				// print all the semantics attached to it -----------------------------------------
				
				bool isArray = false;
				//TODO: how much of this can be moved to the recursive part?
				//TODO: use pSemanticTag->usArrayIndex for this

				// name of the first child 
				LPCWSTR tagName = pSemanticTag->pszName;
				if(tagName != NULL)
				{
					// convert child tag name to wide char
					std::string tagNameStr = convertWideStr(tagName);
					
					// If we're dealing with an array on this level, the first child is called "item"
					// and its siblings are called "item"+number.
					isArray = (tagNameStr.compare("item") == 0);
					
					// It it's an individual element, it is added as "<key>:<value>".
					// If it's part of an array, the name is ignored and only the value is added. 
					if(!isArray)
					{
						printSemanticKey(tagNameStr, dataWriter);
						dataWriter <<":";
					}
				}

				// print the value of the first child
				std::string tagStr = PrintSemanticTagProlog(pSemanticTag, /*pRule,*/ pElements);
				dataWriter << tagStr;

				
				// print the sibling tags ----------------------------------------------------------
				pSemanticTag = pSemanticTag->pNextSibling;
				pRule = pRule->pNextSibling;

				while (pSemanticTag != NULL)
				{
					//separate tags with a comma
					dataWriter <<",";

					// It it's an individual element, it is added as "<key>:<value>".
					// If it's part of an array, the name is ignored and only the value is added.
					if(!isArray)
					{
						//get the name of the current sibling
						LPCWSTR tagName = pSemanticTag->pszName;
						if(tagName != NULL)
						{
							std::string tagNameStr = convertWideStr(tagName);
							
							printSemanticKey(tagNameStr, dataWriter);
							dataWriter << ":";
						}
					}

					// print the value of the current sibling
					std::string tagStr = PrintSemanticTagProlog(pSemanticTag, /*pRule,*/ pElements);
					dataWriter << tagStr;

					pSemanticTag = pSemanticTag->pNextSibling;
				}

			}

			//--------------------------------------------------------------------------------
			// TODO word details?
			//--------------------------------------------------------------------------------


			//--------------------------------------------------------------------------------
			// finish the feature structure
			//--------------------------------------------------------------------------------

			dataWriter << "]";
		}

		std::string dataString = dataWriter.str();
#ifdef DEBUG
		//printf("[speech] data: %s\n", dataString.c_str());
#endif

		//------------------------------------------------------------------------------------
        // time information
		// (calculated as late as possible)
        //------------------------------------------------------------------------------------

		
		SPRECORESULTTIMES recTimes;
		result->GetResultTimes (&recTimes);
	
		// start time of the phrase, x*100 nanoseconds relative to the start of the stream
		ULONGLONG phraseStart = recTimes.ullStart;
		
		// start time of the first word, x*100 nanoseconds relative to the phrase start time 
		ULONG wordStart = pElements[0].ulAudioTimeOffset;

		// end time of the last word, x*100 nanoseconds relative to the phrase start time 
		ULONG wordEnd = (pElements[elemCount-1].ulAudioTimeOffset
			+ pElements[elemCount-1].ulAudioSizeTime);

		////DEBUG
		////(for some reason, printf can't print two ULONG values in the same command...) 
		//printf("[speech] phrase time: start = %u\n", phraseStart);
		////printf("[speech] , duration = %u", phraseDur);
		//printf("[speech] word time: start = %u\n", wordStart);
		//printf("[speech] , end = %u\n", wordEnd);
			

		// start time of the first word, milliseconds relative to the start of the stream 
		ULONG startTime = (ULONG)((phraseStart + wordStart) * 0.0001);

		// duration of the speech input in milliseconds  
		ULONG duration  = (ULONG)((wordEnd - wordStart) * 0.0001);
		//printf("[speech] -> start time = %u", startTime);
		//printf("[speech] , duration = %u", duration);

		//--------------------------------------------------------------------------------
		// final output
		//--------------------------------------------------------------------------------
		
		if (m_callback) {
			//create speech event from
			//- the recognition data given as a Prolog typed feature structure
			//- the start time in milliseconds relative to the start of the stream
			//- the speech duration in milliseconds
			//- the rule confidence
			m_callback->update (dataString.c_str(), startTime, duration, ruleConfidence);
		} else {
			printf("[speech] %s[%.2f]\n",dataWriter.str().c_str(),ruleConfidence);
		}

		//clean up
		::CoTaskMemFree(pPhrase);
	}

}


/// <summary>
/// Recursively prints the semantic tag and its children.
/// </summary>
/// <returns>
/// a Prolog-compatible representation of the semantic tag
/// </returns>
std::string MSSpeechWrapper::PrintSemanticTagProlog (const SPPHRASEPROPERTY* tag, /*const SPPHRASERULE* rule, */const SPPHRASEELEMENT* elements)
{
	std::stringstream tagWriter;
	
	//--------------------------------------------------------------------------------------
	// print the current tag's value
	//-------------------------------------------------------------------------------------- 
	LPCWSTR tagValue = tag->pszValue;
	
	if(tagValue != NULL)
	{
		
		////DEBUG array
		//unsigned short index = tag->usArrayIndex;
		//unsigned short length = tag->usArrayLength;

		//printf("[speech] current node's array info: index: %u, length: %u\n", index, length);

		// convert the value to wide char
		std::string tagValueStr = convertWideStr(tagValue); 

		////DEBUG
		//LPCWSTR ruleName = rule->pszName;
		//std::string ruleNameStr = convertWideStr(ruleName); 
		//printf("[speech] tag value = %s, rule name = %s\n", tagValueStr.c_str(), ruleNameStr.c_str());

		//Is the value string a placeholder for recognition information?
		bool isRecoInfo = (tagValueStr.at(0) == '$');
		
		if(isRecoInfo)
		{
			//replace the tag value with the requested information
			printf("[speech] request for recognition information (%s)\n", tagValueStr.c_str());

			//get the start time (ms relative to stream start) ---------------------------------------
			if(tagValueStr.compare("$startTime") == 0)
			{
				ULONG firstElemIdx = tag->ulFirstElement;
				
				const SPPHRASEELEMENT firstElem = elements[firstElemIdx];
				ULONG startTime = (ULONG)(firstElem.ulAudioTimeOffset * 0.0001);
				
				////DEBUG: get the text as well
				//std::string name = convertWideStr(firstElem.pszDisplayText);
				//printf("[speech] firstElem: index %u, word %s\n", firstElemIdx, name.c_str());				

				//append this time stamp to the result string
				tagWriter << startTime;
			}
			//get the end time (ms relative to stream start) ---------------------------------------
			else if (tagValueStr.compare("$endTime") == 0)
			{
				ULONG firstElemIdx = tag->ulFirstElement;
				ULONG lastElemIdx = firstElemIdx + tag->ulCountOfElements -1;
				
				const SPPHRASEELEMENT lastElem  = elements[lastElemIdx];
				ULONG endTime = (ULONG)((lastElem.ulAudioTimeOffset+lastElem.ulAudioSizeTime) * 0.0001);

				////DEBUG: get the text as well
				//std::string name1 = convertWideStr(elements[firstElemIdx].pszDisplayText);
				//std::string name2 = convertWideStr(lastElem.pszDisplayText);
				//printf("[speech] firstElem: %u, word: %s, startTime = %u\n", firstElemIdx, name1.c_str(), startTime);
				//printf("[speech] lastElem: %u, word: %s, endTime = %u\n", lastElemIdx, name2.c_str(), endTime);
			
				//append this time stamp to the result string
				tagWriter << endTime;
			}
		}
		else
		{
			//print the normal value
			tagWriter << tagValueStr;

			////DEBUG
			//printf("[speech] regular tag, value: %s\n", lpValue);
		}
	}

	//-----------------------------------------------------------------------------------
	// print the tag's children
	//-----------------------------------------------------------------------------------
	
	// Note:
	// has EITHER a value (= leaf in the semantic tree) OR an arbitrary number of children

	const SPPHRASEPROPERTY *childTag = tag->pFirstChild;
	if(childTag != NULL)
	{
		////DEBUG
		//printf("[speech] tag has children...\n");

		//open a square bracket and print the child elements
		tagWriter <<"[";

		////TODO check rule: is it covered by the same one or a child rule?
		//const SPPHRASERULE *childRule = findRule(childTag, rule);
		
		// init variables -------------------------------------------------------
		//check once: if parent is an array, children are named [item, item_0, ... item_n]
		bool isArray = false;

		//check for every child: is it a placeholder for recognition information?
		bool isRecoInfo = false;


		//-----------------------------------------------------------------------
	    // first child tag
		//-----------------------------------------------------------------------
		
		LPCWSTR childName = childTag->pszName;
		if(childName != NULL)
		{
			// convert child tag name to wide char
			std::string childNameStr = convertWideStr(childName);

			//check: is this an array?
			isArray = (childNameStr.compare("item") == 0);
			////DEBUG array
			//unsigned short index = childTag->usArrayIndex;
			//unsigned short length = childTag->usArrayLength;
			//printf("[speech] child is part of array: %d, index: %u, length: %u\n", isArray, index, length);

			// It it's an individual element, it is added as "<key>:<value>".
			// If it's part of an array, the name is ignored and only the value is added. 
			if (!isArray)
			{
				printSemanticKey(childNameStr, tagWriter);
				tagWriter << ":";
			}
			/*else
			{
				printf("[speech] array detected: first child name = %s\n", lpChildName);
			}
			*/
    
			//print the value of the first child
			std::string childValueStr = PrintSemanticTagProlog(childTag, /*childRule,*/ elements);
			tagWriter << childValueStr;
		}


		//-----------------------------------------------------------------------
		// sibling tags
		//-----------------------------------------------------------------------
		childTag = childTag->pNextSibling;		
		while(childTag != NULL)
		{
			//separate tags with a comma
			tagWriter << ",";

			////DEBUG array
			//unsigned short index = childTag->usArrayIndex;
			//unsigned short length = childTag->usArrayLength;
			//printf("[speech] child is part of array: %d, index: %u, length: %u\n", isArray, index, length);

			//childRule = findRule(childTag, rule);
			
			// It it's an individual element, it is added as "<key>:<value>".
			// If it's part of an array, the name is ignored and only the value is added.
			
			if (!isArray)
			{
				//get the name of the current sibling
				childName = childTag->pszName;
				if(childName != NULL)
				{
					// convert sibling tag name to wide char
					std::string childNameStr = convertWideStr(childName);	
	
					printSemanticKey(childNameStr, tagWriter);
					tagWriter << ":";
				}
			}
			
			//print the value of the current sibling
			std::string childValueStr = PrintSemanticTagProlog(childTag, /*childRule,*/ elements);
			tagWriter << childValueStr;

			childTag = childTag->pNextSibling;
		}
			
		//all children printed
		tagWriter << "]";
	}

	return tagWriter.str();
}


/**
 * Prints a semantic key in the proper format.
 *
 * Removes "TAG" prefix from key names which would be reserved words
 * in the script language that is used for defining the semantic tags.
 */
//TODO: enforce lowercase?
void MSSpeechWrapper::printSemanticKey(std::string keyStr, std::stringstream &outstream)
{
	////DEBUG
	//printf("[speech] checking name: prefix = %s, suffix = %s\n",
	//keyStr.substr(0,3).c_str(), keyStr.substr(3).c_str());

	if (keyStr.substr(0,3).compare("TAG") == 0)
			outstream << keyStr.substr(3); //copy the legal part of the name
	else outstream << keyStr; //copy the full name
}


//==================================================================================================
// rule identification
//
// useful for debugging, and required for manually attaching the semantics to incremental results
//================================================================================================== 


/**
 * Checks which of the rules produced the given semantic tag.
 */
const SPPHRASERULE* MSSpeechWrapper::findRule(const SPPHRASEPROPERTY* tag, const SPPHRASERULE* currentRule)
{
	//which words does this tag belong to?
	ULONG firstElem = tag->ulFirstElement;
	ULONG elemCount = tag->ulCountOfElements;
	ULONG lastElem  = firstElem + elemCount;

	//printf("[speech] checking rule responsibility: tag covers [%d, %d]\n",
	//		firstElem, lastElem);
	
	const SPPHRASERULE *result = refineRule(firstElem, lastElem, currentRule);

	//if(result != currentRule) printf("[speech] rule was refined\n");
	//	else printf("[speech] rule was not refined further\n");

	return result;
}


/**
 * Checks the rules recursively to find the last child rule which produced the given words.
 */
const SPPHRASERULE* MSSpeechWrapper::refineRule(ULONG firstElem, ULONG lastElem, const SPPHRASERULE* currentRule)
{
	//start checking the first child rule
	bool searching=true;
	const SPPHRASERULE *searchRule = currentRule;

	//check the child nodes
	//start at the first one
	const SPPHRASERULE *childRule = currentRule->pFirstChild;
	ULONG childRuleFirstElem, childRuleElemCount, childRuleLastElem;

	while(childRule != NULL)
	{	
		//does this child cover the same words?
		childRuleFirstElem = childRule->ulFirstElement;
		childRuleElemCount = childRule->ulCountOfElements;
		childRuleLastElem  = childRuleFirstElem + childRuleElemCount;

		//printf("[speech] \trefining rule: child rule covers [%d, %d]\n",
		//	childRuleFirstElem, childRuleLastElem);


		if((firstElem >= childRuleFirstElem) && (lastElem <= childRuleLastElem))
			//suitable candidate -> try to refine it further
			return refineRule(firstElem, lastElem, childRule);
		else childRule = childRule->pNextSibling;
	}
		
	//no child rule matches -> stick to this one
	//printf("[speech] \tno refinement possible\n");
	return currentRule;
}


//======================================================================================
// other helper functions
//======================================================================================

/*
 * Note: This conversion to std::string makes the code easier to read and maintain
 * (avoiding mistakes from copy&paste or memory management),
 * but also adds some computational overhead.
 * 
 * This might become noticeable later when the grammars get more complex
 * and the semantic feature structures become too large.
 * If that actually happens, we might need to go back to char*
 * and make sure to manually delete the array afterwards.
 */
std::string MSSpeechWrapper::convertWideStr(LPCWSTR wideStr)
{
	int iSize = WideCharToMultiByte (CP_ACP,0,wideStr, -1, NULL, 0, NULL, NULL );
	char* lpNarrow = new char[iSize];
	lpNarrow[0] = 0;
	iSize= WideCharToMultiByte(CP_ACP, 0, wideStr, -1, lpNarrow, iSize, NULL, NULL );

	std::string result (lpNarrow);
	// ="";
	//result.append(lpNarrow);

	delete[] lpNarrow;

	return result;
}

/// <summary>
/// Parses the output format code from a string.
/// </summary>
/// <returns>
/// the matching OutputFormat code, or KEYWORD if nothing matches
/// </returns>
MSSpeechWrapper::OutputFormat MSSpeechWrapper::ParseOutputFormat(const char* formatStr)
{
	if (formatStr)
	{
		if(strcmp(formatStr, "plain_text") == 0)
		{
			return PLAIN_TEXT;
		}
		else if(strcmp(formatStr, "structure") == 0)
		{
			return STRUCTURE;
		}
		else if(strcmp(formatStr, "keyword") == 0)
		{
			return KEYWORD;
		}
		else return KEYWORD;
	}
	else return KEYWORD;
}





/// <summary>
/// Experimental: returns the latest hypothesized word
/// </summary>
void MSSpeechWrapper::Temp_HypothesisResult (ISpRecoResult* result)
{    
    SPPHRASE* pPhrase = NULL;

	HRESULT hr = S_OK;

	hr = result->GetPhrase(&pPhrase);
    if (SUCCEEDED(hr))
    {
		const SPPHRASEELEMENT *pElements = pPhrase->pElements;
		ULONG elemCount = pPhrase->Rule.ulCountOfElements;
	
		//------------------------------------------------------------------------------------
        // prepare data string  
        //------------------------------------------------------------------------------------
        std::stringstream dataWriter;

        // speech header ---------------------------------------------------------------------

		WCHAR *utterance;
		result->GetText(SP_GETWHOLEPHRASE, SP_GETWHOLEPHRASE, FALSE, &utterance, NULL);

		std::string utteranceStr = convertWideStr(utterance);
		dataWriter << "[type:word," 
		           << "uttr:\'" << utteranceStr << "\'";
		

		// interpretation data ---------------------------------------------------------------
        
		float ruleConfidence = pPhrase->Rule.SREngineConfidence; //overall interpretation confidence
		dataWriter << ",conf:" << ruleConfidence;

		const SPPHRASEELEMENT lastElem = pPhrase->pElements[elemCount-1];
		std::string word = convertWideStr(lastElem.pszDisplayText);

		dataWriter << ",data:[" << word	<< "]]";
		std::string dataString = dataWriter.str();

#ifdef DEBUG
		//printf("[speech] data: %s\n", dataString.c_str());
#endif

		//------------------------------------------------------------------------------------
        // time information
        //------------------------------------------------------------------------------------

		
		SPRECORESULTTIMES recTimes;
		result->GetResultTimes (&recTimes);
	
		ULONGLONG phraseStart = recTimes.ullStart / 10000;
		//ULONGLONG phraseDur = recTimes.ullLength / 10000;

		ULONG wordStart = pElements[0].ulAudioTimeOffset / 10000;
		ULONG wordEnd = (pElements[elemCount-1].ulAudioTimeOffset
			+ pElements[elemCount-1].ulAudioSizeTime) / 10000;
					
		//proper event times, going by the actual words
		ULONG startTime = (ULONG)(phraseStart + wordStart);
		ULONG duration  = wordEnd - wordStart;
		/*printf("[speech] -> start time = %u", startTime);
		printf("[speech] , duration = %u", duration);*/

		// -------------------------------------------------------------------------------
		// output
		// -------------------------------------------------------------------------------
		
		if (m_callback) {
			m_callback->update (dataWriter.str().c_str(), startTime, duration, ruleConfidence);
		} else {
			printf("[speech] %s[%.2f]\n",dataWriter.str().c_str(),ruleConfidence);
		}

		//clean up
		::CoTaskMemFree(pPhrase);
	}

}
