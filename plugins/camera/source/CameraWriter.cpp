// CameraWriter.cpp
// author: Frank Jung <frank.jung@informatik.uni-augsburg.de>
// created: 2009/06/26
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

#include "CameraWriter.h"

#include <AtlBase.h>
#include <AtlConv.h>
#include <dshow.h>
#include <Ocidl.h>
#include <OAIDL.H>
#include <comutil.h>
#include <streams.h>
#include <initguid.h>

#include "CameraTools.h"
#include "FakeCamPushSource.h"
#include "FakeAudioPushSource.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi
{

int CameraWriter::ssi_log_level_static = SSI_LOG_LEVEL_DEFAULT;
char *CameraWriter::ssi_log_name_static = "camwriter_";

void CameraWriter::setLogLevel (int level)
{
	ssi_log_level = level;
}

void CameraWriter::SetLogLevelStatic (int level)
{
	ssi_log_level_static = level;
}

CameraWriter::CameraWriter(const ssi_char_t *file) :
	_comInitCountConstructor(0),
	_comInitCountEnterConsumeFlush(0),
	_audio_format(0),
	_numberOfAudioSamplesPerVideoFrame(0),
	_pGraph(NULL),
	_pBuild(NULL),
	_pFakeSource(NULL),
	_pFakeAudioSource(NULL),
	_pFakeCamControl(NULL),
	_pFakeAudioControl(NULL),
	_pAviMux(NULL),
	_pEncoder(NULL),
	_pConfigMux(NULL),
	_pInterleave(NULL),
	_pFileSink(NULL),
	_pControl(NULL),
	_file (0),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

	static int device_counter = 1;
	ssi_log_name = new char[SSI_MAX_CHAR];
	sprintf (ssi_log_name, "camwrite%s%d", device_counter > 9 ? "" : "_", device_counter); 
	++device_counter;

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
}

CameraWriter::~CameraWriter()
{
	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
	
	delete[] ssi_log_name;

	SafeReleaseFJ(_pControl);
	SafeReleaseFJ(_pFileSink);
	SafeReleaseFJ(_pAviMux);
	SafeReleaseFJ(_pEncoder);
	SafeReleaseFJ(_pConfigMux);
	SafeReleaseFJ(_pInterleave);
	SafeReleaseFJ(_pFakeCamControl);
	SafeReleaseFJ(_pFakeAudioControl);
	SafeReleaseFJ(_pFakeSource);
	SafeReleaseFJ(_pFakeAudioSource);
	//delete _pFakeSource;
	SafeReleaseFJ(_pBuild);
	SafeReleaseFJ(_pGraph);

	if(_comInitCountEnterConsumeFlush > 0)
	{
		CoUninitialize();
		--_comInitCountEnterConsumeFlush;
	}
}

void CameraWriter::consume_enter(ssi_size_t stream_in_num, ssi_stream_t stream_in[])
{
	SSI_DBG (SSI_LOG_LEVEL_DETAIL, "try to start cam writer...");

	// video

	if (stream_in[0].sr != _video_format.framesPerSecond) {
		ssi_wrn ("sample rate '%lf' of input stream differs from internal sample rate '%lf'", stream_in[0].sr, _video_format.framesPerSecond);
	}
	if (stream_in[0].byte != ssi_video_stride(_video_format) * _video_format.heightInPixels) {
		ssi_wrn ("sample bytes '%u' of input stream differs from internal sample bytes '%u'", stream_in[0].byte, ssi_video_stride(_video_format) * _video_format.heightInPixels);
	}
	if (stream_in[0].dim != 1) {
		ssi_wrn ("sample dimension '%u' of input stream differs from internal sample dimension 1'", stream_in[0].dim);
	}

	if (ssi_exists (_options.path)) {
		remove (_options.path);
	}

	// audio
	if (stream_in_num > 1) {
		_audio_format = new WAVEFORMATEX (ssi_create_wfx (stream_in[1].sr, stream_in[1].dim, stream_in[1].byte));
	}

	// initalize com

	if(ssi_video_stride (_video_format) * _video_format.heightInPixels != stream_in[0].byte)
	{
		ssi_err("stream[0] not compatible to videoParams settings");
	}

	if(_audio_format)
	{
		SSI_ASSERT(stream_in_num == 2);
		if(_audio_format->nChannels != stream_in[1].dim || _audio_format->wBitsPerSample / 8 != stream_in[1].byte)
		{
			ssi_err("stream[1] not compatible to audioParams settings");
		}
	}
	else
	{
		SSI_ASSERT(stream_in_num == 1);
		
	}

	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr))
    {
		if(hr == RPC_E_CHANGED_MODE)
		{
			SSI_DBG (SSI_LOG_LEVEL_DEBUG, "Tried to reinitialize COM with different threading model!");
		}
		else
		{
			ssi_err ("Could not initialize COM library in connect()");
			return;
		}
    }
	else 
	{
		if(hr == S_FALSE)
		{
			SSI_DBG (SSI_LOG_LEVEL_DEBUG, "COM was already initialized for this thread!");
		}
		++_comInitCountEnterConsumeFlush;
	}


	// build graph

	SSI_DBG (SSI_LOG_LEVEL_DEBUG, "Calling InitCaptureGraphBuilder...");
	
	hr = CameraTools::InitCaptureGraphBuilder(&_pGraph, &_pBuild);
	if(SUCCEEDED(hr))
	{
		 SSI_DBG (SSI_LOG_LEVEL_VERBOSE, "...SUCCEEDED!");
	}
	else 
	{
		ssi_err ("InitCapturegraphBuilder failed");
		return;
	}

	SSI_DBG (SSI_LOG_LEVEL_DEBUG, "Calling AddFilterToGraphByCLSID...");
	hr = CameraTools::AddFilterToGraphByCLSID(_pGraph, CLSID_FakeCamPushSource, L"FakeSource", &_pFakeSource);
	if(SUCCEEDED(hr))
	{
		 SSI_DBG (SSI_LOG_LEVEL_VERBOSE, "...SUCCEEDED!");
	}
	else 
	{
		ssi_err ("Adding the FakeCamPushSource failed");
		return;
	}

	SSI_DBG (SSI_LOG_LEVEL_DEBUG, "Calling GetFirstPin on FakeSource...");
	IPin *pPin = CameraTools::GetFirstPin(_pFakeSource, PINDIR_OUTPUT);
	if(!pPin)
	{
		ssi_err ("Could not get output Pin from FakeCamPushSource");
	}

	SSI_DBG (SSI_LOG_LEVEL_DEBUG, "Calling QueryInterface on Pin...");
	hr = pPin->QueryInterface(IID_IFakeCamPushControl, (void**)&_pFakeCamControl);
	if(SUCCEEDED(hr))
	{
		 SSI_DBG (SSI_LOG_LEVEL_VERBOSE, "...SUCCEEDED!");
	}
	else 
	{
		ssi_err ("Getting the FakeCamPushControl Interface failed");
		return;
	}

	SafeReleaseFJ(pPin);

	if(_audio_format)
	{
		SSI_DBG (SSI_LOG_LEVEL_DEBUG, "Calling AddFilterToGraphByCLSID...");
		hr = CameraTools::AddFilterToGraphByCLSID(_pGraph, CLSID_FakeAudioPushSource, L"FakeAudioSource", &_pFakeAudioSource);
		if(SUCCEEDED(hr))
		{
			 SSI_DBG (SSI_LOG_LEVEL_VERBOSE, "...SUCCEEDED!");
		}
		else 
		{
			ssi_err ("Adding the FakeAudioPushSource failed");
			return;
		}

		SSI_DBG (SSI_LOG_LEVEL_DEBUG, "Calling GetFirstPin on FakeAudioSource...");
		IPin *pPin = CameraTools::GetFirstPin(_pFakeAudioSource, PINDIR_OUTPUT);
		if(!pPin)
		{
			ssi_err ("Could not get output Pin from FakeAudioPushSource");
		}

		SSI_DBG (SSI_LOG_LEVEL_DEBUG, "Calling QueryInterface on Pin...");
		hr = pPin->QueryInterface(IID_IFakeAudioPushControl, (void**)&_pFakeAudioControl);
		if(SUCCEEDED(hr))
		{
			 SSI_DBG (SSI_LOG_LEVEL_VERBOSE, "...SUCCEEDED!");
		}
		else 
		{
			ssi_err ("Getting the FakeAudioPushControl Interface failed");
			return;
		}

		SafeReleaseFJ(pPin);

		_numberOfAudioSamplesPerVideoFrame = (int)(((double)(_audio_format->nSamplesPerSec) / _video_format.framesPerSecond) + 1.0);
		_pFakeAudioControl->SetAudioFormatForSource(*_audio_format, _numberOfAudioSamplesPerVideoFrame);
	}
	

	hr = CameraTools::QueryInterfaces(_pGraph, &_pControl);
	if(SUCCEEDED(hr))
	{
		 SSI_DBG (SSI_LOG_LEVEL_VERBOSE, "...SUCCEEDED!");
	}
	else 
	{
		ssi_err ("MediaControl Interface failed");
		return;
	}

	hr = _pFakeCamControl->SetVideoFormatForSource(_video_format.framesPerSecond, _video_format.widthInPixels, _video_format.heightInPixels, _video_format.numOfChannels, !_options.flip, _options.mirror);

	

	wchar_t fileNameW[MAX_PATH*2];
	if(!MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, _options.path, -1, fileNameW, MAX_PATH*2))
	{
		ssi_err("Could not convert filename to wide char");
	}

	SSI_DBG (SSI_LOG_LEVEL_DEBUG, "Calling SetOutputFileName...");
	hr = _pBuild->SetOutputFileName(&MEDIASUBTYPE_Avi, fileNameW, &_pAviMux, &_pFileSink);
	if(SUCCEEDED(hr))
	{
		 SSI_DBG (SSI_LOG_LEVEL_VERBOSE, "...SUCCEEDED!");
	}
	else 
	{
		ssi_err ("SetOutputFileName failed");
		return;
	}

	//delete[] fileNameW;

	//hr = _pAviMux->QueryInterface(IID_IConfigAviMux, (void**)&_pConfigMux);
	//if (SUCCEEDED(hr))
	//{
	//	hr = _pConfigMux->SetMasterStream(1);
	//	SafeReleaseFJ(_pConfigMux);
	//}

	hr = _pAviMux->QueryInterface(IID_IConfigInterleaving, (void**)&_pInterleave);
	if (SUCCEEDED(hr))
	{
		hr = _pInterleave->put_Mode(ssi_cast (InterleavingMode, _options.mode));
		if(FAILED(hr))
		{
			ssi_err("Set interleave mode failed!");
		}
		SafeReleaseFJ(_pInterleave);
	}

	// let user select compression
	if (_options.compression[0] == '\0') {
		StringList list;
		CameraWriter::GetListOfCompressionFilterNames (list);
		int sel = CameraWriter::LetUserSelectDesiredCompression (list, true);
		if (sel >= 0) {
			ssi_strcpy (_options.compression, list.get (sel));
		} else {
			ssi_err ("invalid selection");
		}
	}

	// apply compression
	if (strcmp (_options.compression, SSI_CAMERA_USE_NO_COMPRESSION) != 0)
	{
		ICreateDevEnum *pSysDevEnum = NULL;
		IEnumMoniker *pEnum = NULL;
		IMoniker *pMoniker = NULL;

		hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void**)&pSysDevEnum);
		if (FAILED(hr))
		{
			ssi_err("Could not create Device enumerator");
		}    

		hr = pSysDevEnum->CreateClassEnumerator(CLSID_VideoCompressorCategory, &pEnum, 0);

		if (hr == S_OK)  // S_FALSE means nothing in this category.
		{
			while (S_OK == pEnum->Next(1, &pMoniker, NULL))
			{
				IPropertyBag *pPropBag = NULL;
				pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag);
				VARIANT varName;
				VariantInit(&varName);
				hr = pPropBag->Read(L"FriendlyName", &varName, 0);
				char* lpszText = _com_util::ConvertBSTRToString(varName.bstrVal);
				if (SUCCEEDED(hr))
				{
					if(strcmp(lpszText, _options.compression)==0)
					{
						hr = pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter, (void**)&_pEncoder);
						if (FAILED(hr))
						{
							ssi_err("Could not bind moniker to object!");
						}
					}

				}
				delete[] lpszText;
				VariantClear(&varName); 
				pPropBag->Release();
				pMoniker->Release();
			}
		}
		pSysDevEnum->Release();
		pEnum->Release();
	}

	if(_pEncoder)
	{
		hr = _pGraph->AddFilter(_pEncoder, L"Encoder");
		if(FAILED(hr))
		{
			ssi_err("Adding encoder to Graph failed!");
		}
	}

	hr = _pBuild->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, _pFakeSource, _pEncoder, _pAviMux);
	if(SUCCEEDED(hr))
	{
		 SSI_DBG (SSI_LOG_LEVEL_VERBOSE, "...SUCCEEDED!");
	}
	else 
	{
		ssi_err ("RenderStream failed");
		return;
	}

	if(_audio_format)
	{
		hr = _pAviMux->QueryInterface(IID_IConfigAviMux, (void**)&_pConfigMux );
		if (SUCCEEDED(hr))
		{
			//hr = _pConfigMux->SetMasterStream(1);
			if(FAILED(hr))
			{
				ssi_err("SetMasterStream failed!");
			}
			SafeReleaseFJ(_pConfigMux);
		}

		hr = _pBuild->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Audio, _pFakeAudioSource, NULL, _pAviMux);
		if(SUCCEEDED(hr))
		{
			 SSI_DBG (SSI_LOG_LEVEL_VERBOSE, "...SUCCEEDED!");
		}
		else 
		{
			ssi_err ("RenderStream failed");
			return;
		}

	}

	hr = _pControl->Run();
	if(SUCCEEDED(hr))
	{
		 SSI_DBG (SSI_LOG_LEVEL_VERBOSE, "...SUCCEEDED!");
	}
	else 
	{
		ssi_err ("Running the graph failed");
		return;
	}

}

void CameraWriter::consume(IConsumer::info consume_info, ssi_size_t stream_in_num, ssi_stream_t stream_in[])
{

	HRESULT hr;
	//SSI_ASSERT(stream_in_num == 1);
	for(ssi_size_t i = 0; i < stream_in[0].num; ++i)
	{
		if(_audio_format && i * _numberOfAudioSamplesPerVideoFrame < stream_in[1].num)
		{
			hr = _pFakeAudioControl->PumpSampleIntoFilter((BYTE*)(stream_in[1].ptr + (i * _numberOfAudioSamplesPerVideoFrame * _audio_format->nBlockAlign)),
				min(_numberOfAudioSamplesPerVideoFrame, ssi_cast (int, stream_in[1].num - i * _numberOfAudioSamplesPerVideoFrame)));
		}
		hr = _pFakeCamControl->PumpSampleIntoFilter((BYTE*)(stream_in[0].ptr + (i * ssi_video_stride (_video_format) * _video_format.heightInPixels)), ssi_video_stride (_video_format) * _video_format.heightInPixels);
		
	}
}

void CameraWriter::consume_flush(ssi_size_t stream_in_num, ssi_stream_t stream_in[])
{

	_pFakeCamControl->SignalEndOfStream();
	if(_audio_format)
		_pFakeAudioControl->SignalEndOfStream();
	_pControl->Stop();

	SafeReleaseFJ(_pControl);
	SafeReleaseFJ(_pFileSink);
	SafeReleaseFJ(_pAviMux);
	SafeReleaseFJ(_pEncoder);
	SafeReleaseFJ(_pConfigMux);
	SafeReleaseFJ(_pInterleave);
	SafeReleaseFJ(_pFakeCamControl);
	SafeReleaseFJ(_pFakeAudioControl);
	//SafeReleaseFJ(_pFakeSource);
	//delete _pFakeSource;
	SafeReleaseFJ(_pBuild);
	SafeReleaseFJ(_pGraph);

	delete _audio_format;
	_audio_format = 0;

	if(_comInitCountEnterConsumeFlush > 0)
	{
		CoUninitialize();
		--_comInitCountEnterConsumeFlush;
	}
}

// add empty frames
void CameraWriter::consume_fail (const ssi_time_t fail_time, 
	const ssi_time_t fail_duration,
	ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	// number of missing frames
	int frames_to_add = ssi_cast (int, fail_duration * _video_format.framesPerSecond + 0.5);
	
	_pFakeCamControl->AddSkippedFrames(frames_to_add);

	if(_audio_format)
	{
		int audio_frames_to_add = ssi_cast (int, fail_duration * _audio_format->nSamplesPerSec + 0.5);

		_pFakeAudioControl->AddSkippedFrames(audio_frames_to_add);
		ssi_msg (SSI_LOG_LEVEL_BASIC, "%.2Lf s were missing --> added %u empty video frames and %u empty audio samples", fail_duration, frames_to_add, audio_frames_to_add); 
	}
	else
		ssi_msg (SSI_LOG_LEVEL_BASIC, "%.2Lf s were missing --> added %u empty frames", fail_duration, frames_to_add); 
}


HRESULT CameraWriter::GetListOfCompressionFilterNames (StringList &list) {

	ICreateDevEnum *pSysDevEnum = NULL;
	IEnumMoniker *pEnum = NULL;
	IMoniker *pMoniker = NULL;

	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void**)&pSysDevEnum);

	if (FAILED(hr))
	{
		ssi_err_static ("Could not create device enumerator");
	}    

	hr = pSysDevEnum->CreateClassEnumerator(CLSID_VideoCompressorCategory, &pEnum, 0);

	if (hr == S_OK)  // S_FALSE means nothing in this category.
	{
		SSI_DBG_STATIC (SSI_LOG_LEVEL_VERBOSE, "adding compression filter:\n");

		list.add (SSI_CAMERA_USE_NO_COMPRESSION);
		while (S_OK == pEnum->Next(1, &pMoniker, NULL))
		{
			IPropertyBag *pPropBag = NULL;
			pMoniker->BindToStorage(0, 0, IID_IPropertyBag, 
				(void **)&pPropBag);
			VARIANT varName;
			VariantInit(&varName);
			hr = pPropBag->Read(L"FriendlyName", &varName, 0);
			char* lpszText = _com_util::ConvertBSTRToString(varName.bstrVal);
			if (SUCCEEDED(hr))
			{
				SSI_DBG_STATIC (SSI_LOG_LEVEL_VERBOSE, "%s\n", lpszText);								
				list.add (lpszText);
			}
			delete[] lpszText;
			VariantClear(&varName); 
			pPropBag->Release();
			pMoniker->Release();
		}
	}

	pSysDevEnum->Release();
	pEnum->Release();

	return hr;
}

int CameraWriter::LetUserSelectDesiredCompression(StringList &list, bool fallBackToConsole) {

	if(list.size () == 0)
	{
		ssi_wrn_static ("empty list");
		return -1;
	}

	DialogLibGateway dialogGateway;

	if(!dialogGateway.didInitWork())
	{
		if(fallBackToConsole)
		{
			return LetUserSelectDesiredCompressionOnConsole(list);
		}
		ssi_err_static("DialogLibGateWay initialisation failed and no fallback available");
	}

	if(!dialogGateway.SetNewDialogType("SimpleSelectionDialog"))
	{
		if(fallBackToConsole)
		{
			return LetUserSelectDesiredCompressionOnConsole(list);
		}
		ssi_err_static("Could not set SimpleSelectionDialog and no fallback available");
	}

	int intHandle = dialogGateway.AlterExistingItem("Caption", -1, "Select a compression filter");

	for(ssi_size_t i = 0; i < list.size (); i++)
	{
		intHandle = dialogGateway.AppendItem("Item", list.get (i));
		if(intHandle < 0)
		{
			if(fallBackToConsole)
			{
				return LetUserSelectDesiredCompressionOnConsole(list);
			}

			ssi_err_static("Could not set Item and no fallback available");
		}
	}

	int retVal = dialogGateway.RunDialog();

	return retVal;
}

int CameraWriter::LetUserSelectDesiredCompressionOnConsole(StringList &list) {

	if(list.size () == 0)
	{
		ssi_wrn_static ("empty list");
		return -1;
	}

	ssi_print("Select a compression filter:\n");
	ssi_print("0: None\n");

	for(ssi_size_t i = 0; i < list.size (); i++)
	{
		ssi_print("%d: %s\n", i, list.get (i));
	}

	int selection = -1;
	ssi_print("Your selection: ");
	scanf("%d", &selection);
	if(selection == EOF || selection < 0 || selection > ssi_cast (int, list.size ())) {
		ssi_wrn_static ("invalid selection");
		return -1;
	}
	
	return (selection - 1);
}


}
