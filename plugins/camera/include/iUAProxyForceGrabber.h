// iUAProxyForceGrabber.h
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

#ifndef SSI_SENSOR_IUAPROXYFORCEGRABBER_H
#define SSI_SENSOR_IUAPROXYFORCEGRABBER_H

// DE0EEB1F-59BC-4511-BFAD-BC404355BDDC
DEFINE_GUID(CLSID_UAProxyForceGrabber, 
0xde0eeb1f, 0x59bc, 0x4511, 0xbf, 0xad, 0xbc, 0x40, 0x43, 0x55, 0xbd, 0xdc);

// 2E7A007E-0247-4AED-BB7B-E697DC0C00FD
//DEFINE_GUID(CLSID_UAProxyForceGrabberPropertyPage, 
//0x2e7a007e, 0x247, 0x4aed, 0xbb, 0x7b, 0xe6, 0x97, 0xdc, 0xc, 0, 0xfd);

#ifdef __cplusplus
extern "C" {
#endif

// 5DD9839F-33A1-477F-9F47-C1937AE3B46A
DEFINE_GUID(IID_IUAProxyForceGrabber, 
0x5dd9839f, 0x33a1, 0x477f, 0x9f, 0x47, 0xc1, 0x93, 0x7a, 0xe3, 0xb4, 0x6a);

DECLARE_INTERFACE_(IUAProxyForceGrabber, IUnknown)
{
	STDMETHOD(SetCallback) (THIS_
             void (*transform)(void*), bool activateCallback) PURE;

	STDMETHOD(SetCallback) (THIS_
             void (*transform)(void*)) PURE;

	STDMETHOD(GrabFrame) (THIS_
			BYTE *pBufferForSample, int *pBytesInBuffer, void *pBitMapInfoHeaderStruc) PURE;

	STDMETHOD(ToggleCallBackActivation) (THIS_
			bool activateCallback) PURE;

	STDMETHOD(ToggleCallFrameBufferingForGrabbing) (THIS_
			bool activateFrameBufferForGrabbing) PURE;

	STDMETHOD(GetConnectedMediaType) (THIS_ 
		AM_MEDIA_TYPE OUT **ppMediaType) PURE;
};

#ifdef __cplusplus
}
#endif

#endif
