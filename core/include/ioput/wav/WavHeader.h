// WavHeader.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/07/23
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute itand/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#ifndef SSI_WAV_WAVHEADER_H
#define SSI_WAV_WAVHEADER_H
#include <stdint.h>

namespace ssi {

struct WavHeader {
   int8_t rID[4];
   int32_t rLen;
   int8_t wID[4];
   int8_t fId[4];
   int32_t pcmHeaderLen;
   int16_t compressionTag;
   int16_t nChannels;
   int32_t nSamplesPerSec;
   int32_t nAvgBytesPerSec;
   int16_t nBlockAlign;
   int16_t nBitsPerSample;
};                                                                  
                                                                          
struct WavChunkHeader {                                                               
   int8_t chunkID[4];
   int32_t chunkLen;
};

}


#endif
