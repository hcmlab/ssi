// ssiaudio.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2010/04/13
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

#ifndef SSI_AUDIO_H
#define	SSI_AUDIO_H
#if _WIN32|_WIN64
#include "Audio.h"
#include "AudioPlayer.h"

#include "AudioMixer.h"
#elif __ANDROID__
#include "AudioOpenSL.h"
#include "AudioOpenSLPlayer.h"
#else
#include "AudioPortaudio.h"
#include "AudioPortaudioPlayer.h"
#endif
#include "AudioActivity.h"
#include "VoiceActivitySender.h"
#include "VoiceActivityVerifier.h"
#include "AudioIntensity.h"
#include "AudioLpc.h"
#include "AudioConvert.h"
#include "SNRatio.h"
#include "WavProvider.h"
#include "PreEmphasis.h"
#include "WavReader.h"
#include "WavWriter.h"
#include "AudioMono.h"
#include "AudioNoiseGate.h"

#endif

