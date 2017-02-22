// UAProxyGrabberProp.cpp
// author: Frank Jung <frank.jung@informatik.uni-augsburg.de>
// created: 2009/03/27
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

#include <windows.h>
#include <windowsx.h>
#include <streams.h>
#include <commctrl.h>
#include <olectl.h>
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>
#include "UAProxyResource.h"
#include "iUAProxyForceGrabber.h"
#include "UAProxyForceGrabber.h"
#include "UAProxyForceGrabberProp.h"

#ifdef _DEBUG
	#include "SSI_LeakWatcher.h"
	#define new DEBUG_NEW
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif
/*
//
// CreateInstance
//
// Used by the DirectShow base classes to create instances
//
CUnknown *CUAProxyForceGrabberProperties::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{
    CUnknown *punk = new CUAProxyForceGrabberProperties(lpunk, phr);
    if (punk == NULL) {
	*phr = E_OUTOFMEMORY;
    }
    return punk;

}
//*/
//
// Constructor
//
CUAProxyForceGrabberProperties::CUAProxyForceGrabberProperties(LPUNKNOWN pUnk, HRESULT *phr) :
    CBasePropertyPage(NAME("UAProxyForceGrabber Property Page"),
                      pUnk,IDD_PROPERTIES,IDS_TITLE),
    m_pIUAProxyForceGrabber(NULL),
    m_bIsInitialized(FALSE)
{
    ASSERT(phr);
}

//
// OnReceiveMessage
//
// Handles the messages for our property window
//
INT_PTR CUAProxyForceGrabberProperties::OnReceiveMessage(HWND hwnd,
                                          UINT uMsg,
                                          WPARAM wParam,
                                          LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_COMMAND:
        {
            if (m_bIsInitialized)
            {
                m_bDirty = TRUE;
                if (m_pPageSite)
                {
                    m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
                }
            }
            return (LRESULT) 1;
        }

    }
    return CBasePropertyPage::OnReceiveMessage(hwnd,uMsg,wParam,lParam);

}

//
// OnConnect
//
// Called when we connect to a transform filter
//
HRESULT CUAProxyForceGrabberProperties::OnConnect(IUnknown *pUnknown)
{
    ASSERT(m_pIUAProxyForceGrabber == NULL);

    HRESULT hr = pUnknown->QueryInterface(IID_IUAProxyForceGrabber, (void **) &m_pIUAProxyForceGrabber);
    if (FAILED(hr)) {
        return E_NOINTERFACE;
    }

    ASSERT(m_pIUAProxyForceGrabber);

    
    m_bIsInitialized = FALSE ;
    return NOERROR;
}

//
// OnDisconnect
//
// Likewise called when we disconnect from a filter
//
HRESULT CUAProxyForceGrabberProperties::OnDisconnect()
{
    if (m_pIUAProxyForceGrabber == NULL) {
        return E_UNEXPECTED;
    }

    m_pIUAProxyForceGrabber->Release();
    m_pIUAProxyForceGrabber = NULL;
    return NOERROR;
}

//
// OnActivate
//
// We are being activated
//
HRESULT CUAProxyForceGrabberProperties::OnActivate()
{

	m_bIsInitialized = TRUE;

	return NOERROR;
}

//
// OnDeactivate
//
// We are being deactivated
//
HRESULT CUAProxyForceGrabberProperties::OnDeactivate(void)
{
    ASSERT(m_pIUAProxyForceGrabber);
    m_bIsInitialized = FALSE;
    return NOERROR;
}

//
// OnApplyChanges
//
// Apply any changes so far made
//
HRESULT CUAProxyForceGrabberProperties::OnApplyChanges()
{
    m_bDirty = FALSE;

    return NOERROR;
}


