// AudioIntensity.h
// author: Johannes Wager <wagner@hcm-lab.de>
// created: 2014/03/18
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

// based on code of openSMILE 1.0.1
// http://opensmile.sourceforge.net/

/*F******************************************************************************
 *
 * openSMILE - open Speech and Music Interpretation by Large-space Extraction
 *       the open-source Munich Audio Feature Extraction Toolkit
 * Copyright (C) 2008-2009  Florian Eyben, Martin Woellmer, Bjoern Schuller
 *
 *
 * Institute for Human-Machine Communication
 * Technische Universitaet Muenchen (TUM)
 * D-80333 Munich, Germany
 *
 *
 * If you use openSMILE or any code from openSMILE in your research work,
 * you are kindly asked to acknowledge the use of openSMILE in your publications.
 * See the file CITING.txt for details.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 ******************************************************************************E*/

#include "AudioIntensity.h"
#include "base/Factory.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

AudioIntensity::AudioIntensity (const ssi_char_t *file)
	: _file (0) {

	if (file) {

		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	
	}
	
	hamWin = NULL,
	I0 = 1.0;
	nWin = 0; 
	winSum = 0.0;
}

AudioIntensity::~AudioIntensity () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

SSI_INLINE double *AudioIntensity::smileDsp_winHam(long num)
{
	double i;
    double * ret = (double *)malloc(sizeof(double)*num);
	double * x = ret;
    double NN = (double)num;
	for (i=0.0; i<NN; i += 1.0) {
		/*    *x = 0.53836 - 0.46164 * cos( (2.0*M_PI*i)/(NN-1.0) ); */
		*x = 0.54 - 0.46 * cos( (2.0*3.14159265358979323846264338327950288*i)/(NN-1.0) );
		x++;
	}
	return ret;
}

void AudioIntensity::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {
}

void AudioIntensity::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	ssi_real_t *src = ssi_pcast (ssi_real_t, stream_in.ptr);
	ssi_real_t *dst = ssi_pcast (ssi_real_t, stream_out.ptr);
	ssi_size_t N = stream_in.num;

	if (N != nWin) {
		free (hamWin);
		nWin = N;
		hamWin = smileDsp_winHam(nWin);
		winSum = 0.0;
		for (int j=0; j<nWin; j++) {			
			winSum += hamWin[j];		
		}		
		if (winSum <= 0.0) winSum = 1.0;
	}

	double Im = 0.0;	
	double val = 0.0;			
	for (ssi_size_t j=0; j<N; j++) {
		val = *src++;
		Im += hamWin[j]*val*val;				
	}
			
	Im /= winSum;						

	if (_options.intensity) { 
		*dst++ = (ssi_real_t)Im; 
	}
	if (_options.loudness) {
		*dst++ = (ssi_real_t)pow( Im/I0 , 0.3 ); 
	}
					
}

void AudioIntensity::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	free (hamWin);
	hamWin = NULL;
	nWin = 0;
}


}

