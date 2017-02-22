// CUAProxyForceGrabberProp.h
// author: Frank Jung <frank.jung@informatik.uni-augsburg.de>
// created: 2008/04/01
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

#pragma once

#ifndef SSI_SENSOR_UAPROXYFORCEGRABBERPROP_H
#define SSI_SENSOR_UAPROXYFORCEGRABBERPROP_H

class CUAProxyForceGrabberProperties : public CBasePropertyPage
{

public:

    static CUnknown * WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);

private:

    INT_PTR OnReceiveMessage(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
    HRESULT OnConnect(IUnknown *pUnknown);
    HRESULT OnDisconnect();
    HRESULT OnActivate();
    HRESULT OnDeactivate();
    HRESULT OnApplyChanges();

    void    GetControlValues();

    CUAProxyForceGrabberProperties(LPUNKNOWN lpunk, HRESULT *phr);

    BOOL m_bIsInitialized;													// Used to ignore startup messages
    IUAProxyForceGrabber *m_pIUAProxyForceGrabber;				// The custom interface on the filter
};

#endif
