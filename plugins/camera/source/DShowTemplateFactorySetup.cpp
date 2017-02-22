// DShowTemplateFactorySetup.cpp
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

#include <streams.h>
#include <initguid.h>

#include "iUAProxyForceGrabber.h"
#include "UAProxyForceGrabber.h"
#include "FakeCamPushSource.h"
#include "FakeAudioPushSource.h"


#define TRANSFORM_NAME L"UAProxyForceGrabber Filter"
#define TRANSFORM_NAME2 L"FakeCamPushSource Filter"
#define TRANSFORM_NAME3 L"FakeAudioPushSource Filter"

// setup data - allows the self-registration to work.
const AMOVIESETUP_MEDIATYPE sudPinTypes =
{ &MEDIATYPE_Video        // clsMajorType
, &MEDIASUBTYPE_NULL };  // clsMinorType

const AMOVIESETUP_PIN psudPins[] =
{ { L"Input"            // strName
  , FALSE               // bRendered
  , FALSE               // bOutput
  , FALSE               // bZero
  , FALSE               // bMany
  , &CLSID_NULL         // clsConnectsToFilter
  , L"Output"           // strConnectsToPin
  , 1                   // nTypes
  , &sudPinTypes        // lpTypes
  }
, { L"Output"           // strName
  , FALSE               // bRendered
  , TRUE                // bOutput
  , FALSE               // bZero
  , FALSE               // bMany
  , &CLSID_NULL         // clsConnectsToFilter
  , L"Input"            // strConnectsToPin
  , 1                   // nTypes
  , &sudPinTypes        // lpTypes
  }
};

const AMOVIESETUP_MEDIATYPE sudOpPinTypes =
{
    &MEDIATYPE_Video,       // Major type
    &MEDIASUBTYPE_NULL      // Minor type
};

const AMOVIESETUP_PIN sudOutputPinFakeCamPushSource = 
{
    L"Output",      // Obsolete, not used.
    FALSE,          // Is this pin rendered?
    TRUE,           // Is it an output pin?
    FALSE,          // Can the filter create zero instances?
    FALSE,          // Does the filter create multiple instances?
    &CLSID_NULL,    // Obsolete.
    NULL,           // Obsolete.
    1,              // Number of media types.
    &sudOpPinTypes  // Pointer to media types.
};

const AMOVIESETUP_MEDIATYPE sudOpAuPinTypes =
{
    &MEDIATYPE_Audio,       // Major type
    &MEDIASUBTYPE_PCM      // Minor type
};

const AMOVIESETUP_PIN sudOutputPinFakeAudioPushSource = 
{
    L"Output",      // Obsolete, not used.
    FALSE,          // Is this pin rendered?
    TRUE,           // Is it an output pin?
    FALSE,          // Can the filter create zero instances?
    FALSE,          // Does the filter create multiple instances?
    &CLSID_NULL,    // Obsolete.
    NULL,           // Obsolete.
    1,              // Number of media types.
    &sudOpAuPinTypes  // Pointer to media types.
};

const AMOVIESETUP_FILTER sudUAProxyForceGrabber =
{
	&CLSID_UAProxyForceGrabber,   // clsID,
	TRANSFORM_NAME,					// strName,
	MERIT_DO_NOT_USE,					// dwMerit,
	2,									// nPins,
	psudPins							// lpPin
};						

const AMOVIESETUP_FILTER sudFakeCamPushSource =
{
    &CLSID_FakeCamPushSource,// Filter CLSID
    TRANSFORM_NAME2,       // String name
    MERIT_DO_NOT_USE,       // Filter merit
    1,                      // Number pins
    &sudOutputPinFakeCamPushSource    // Pin details
};

const AMOVIESETUP_FILTER sudFakeAudioPushSource =
{
    &CLSID_FakeAudioPushSource,// Filter CLSID
    TRANSFORM_NAME3,       // String name
    MERIT_DO_NOT_USE,       // Filter merit
    1,                      // Number pins
    &sudOutputPinFakeAudioPushSource    // Pin details
};


// Needed for the CreateInstance mechanism
CFactoryTemplate g_Templates[3]=
{   
	{ 
		TRANSFORM_NAME, 
		&CLSID_UAProxyForceGrabber,
		CUAProxyForceGrabber::CreateInstance,
		NULL,
		&sudUAProxyForceGrabber
	},
//		{ L"UAProxyForceGrabber Filter Properties"
//		, &CLSID_UAProxyForceGrabberPropertyPage
//		, CUAProxyForceGrabberProperties::CreateInstance }
	{ 
		TRANSFORM_NAME2,              // Name
		&CLSID_FakeCamPushSource,       // CLSID
		CFakeCamPushSource::CreateInstance, // Method to create an instance of MyComponent
		NULL,                           // Initialization function
		&sudFakeCamPushSource           // Set-up information (for filters)
	},
	{ 
		TRANSFORM_NAME3,              // Name
		&CLSID_FakeAudioPushSource,       // CLSID
		CFakeAudioPushSource::CreateInstance, // Method to create an instance of MyComponent
		NULL,                           // Initialization function
		&sudFakeAudioPushSource           // Set-up information (for filters)
	},
};
int g_cTemplates = sizeof(g_Templates)/sizeof(g_Templates[0]);
