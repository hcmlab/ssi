// iFakeCamPushControl.h
// author: Frank Jung <frank.jung@informatik.uni-augsburg.de>
// created: 2009/03/31
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

#ifndef SSI_SENSOR_FAKECAMPUSHCONTROL_H
#define SSI_SENSOR_FAKECAMPUSHCONTROL_H

#ifdef __cplusplus
extern "C" {
#endif

	// {01C341C5-AFD4-4d42-9727-7D196F86B493}
DEFINE_GUID(IID_IFakeCamPushControl, 
0x1c341c5, 0xafd4, 0x4d42, 0x97, 0x27, 0x7d, 0x19, 0x6f, 0x86, 0xb4, 0x93);


DECLARE_INTERFACE_(IFakeCamPushControl, IUnknown)
{
	STDMETHOD(PumpSampleIntoFilter) (THIS_
        BYTE *pSampleBuffer, LONG nBufferSize) PURE;

	STDMETHOD(SetVideoFormatForSource) (THIS_
        double fps, int width, int height, int channels, BOOL bFlipSample, BOOL bMirrorSample) PURE;

	STDMETHOD(GetConnectedMediaType) (THIS_ 
		AM_MEDIA_TYPE OUT **ppMediaType) PURE;

	STDMETHOD(SignalEndOfStream) (THIS_) PURE;

	STDMETHOD(AddSkippedFrames) (THIS_ int numberOfSkippedFramesToAdd) PURE;
};

#ifdef __cplusplus
}
#endif

#endif
