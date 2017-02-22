#pragma once

// iFakeAudioPushControl.h
// author: Frank Jung <frank.jung@informatik.uni-augsburg.de>
// created: 2009/07/23
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


#ifdef __cplusplus
extern "C" {
#endif


// {9E0E1BE7-465F-40a6-A4DB-4D094A6E82CC}
DEFINE_GUID(IID_IFakeAudioPushControl, 
0x9e0e1be7, 0x465f, 0x40a6, 0xa4, 0xdb, 0x4d, 0x9, 0x4a, 0x6e, 0x82, 0xcc);



DECLARE_INTERFACE_(IFakeAudioPushControl, IUnknown)
{
	STDMETHOD(PumpSampleIntoFilter) (THIS_
        BYTE *pSampleBuffer, LONG nSamples) PURE;

	STDMETHOD(SetAudioFormatForSource) (THIS_
        WAVEFORMATEX &theWaveFormatExToSetAsOutput, int maxSampleSize) PURE;

	/*STDMETHOD(GetConnectedMediaType) (THIS_ 
		AM_MEDIA_TYPE OUT **ppMediaType) PURE;*/

	STDMETHOD(SignalEndOfStream) (THIS_) PURE;

	STDMETHOD(AddSkippedFrames) (THIS_ int numberOfSkippedSamplesToAdd) PURE;
};

#ifdef __cplusplus
}
#endif
