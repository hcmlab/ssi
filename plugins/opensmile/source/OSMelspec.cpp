// OSMelspec.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/09/22
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

#include "OSMelspec.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

OSMelspec::OSMelspec (const ssi_char_t *file)
	: _file (0),
	_filterCoeffs (0),
	_filterCfs (0),
	_chanMap (0) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
}

OSMelspec::~OSMelspec () {

	releaseFilters ();

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}

void OSMelspec::releaseFilters () {
	
	delete[] _filterCoeffs; _filterCoeffs = 0;
	delete[] _chanMap; _chanMap = 0;
	delete[] _filterCfs; _filterCfs = 0;
}

void OSMelspec::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	nBands = _options.nBands;
	lofreq = _options.loFreq;
	hifreq = _options.hiFreq;
	usePower = _options.usePower ? 1 : 0;
	htkcompatible = _options.htk ? 1 : 0;
	specScale = _options.scale;
	logScaleBase = _options.logScaleBase;
	if (_options.scale == LOG) {
		if ((logScaleBase <= 0.0)||(logScaleBase==1.0)) {
			ssi_wrn ("logScaleBase '%f' must be > 0.0 and != 1.0. Setting it to 2.0 now!)", logScaleBase);
			logScaleBase = 2.0;
		}	
	}
	firstNote = _options.firstNote;
	if (_options.scale == LOG) {
		param = logScaleBase;
	} else if (_options.scale == SEMITONE) {
		param = firstNote;
	} else {
		param = 0.0;
	}
	hfcc = 0;
	if (_options.bwMethod == ERB) {
		hfcc = 1;
	}
	showFbank = _options.showBank ? 1 : 0;

	computeFilters (stream_in.dim, 1.0/stream_in.sr);
}

double OSMelspec::specScaleTransfFwd (double x, OSMelspec::SPECTSCALE scale, double param)
{

	// following code taken from openSMILE 1.0.1, melspec.cpp
	// http://opensmile.sourceforge.net/

	switch(scale) {
	case LOG: 
		return log(x)/log(param); // param = logScaleBase
	case SEMITONE:
		if (x/param>1.0) // param = firstNote
			return 12.0 * smileMath_log2(x / param); // param = firstNote
		else return 0.0;
	case BARK: // Bark scale according to : H. Traunmüller (1990) "Analytical expressions for the tonotopic sensory scale" J. Acoust. Soc. Am. 88: 97-100.   
		if (x>0) {
			return (26.81 / (1.0 + 1960.0/x)) - 0.53;
		} else return 0.0;
	case BARK_SCHROED:
		if (x>0) {
			double f6 = x/600.0;
			return (6.0 * log(f6 + sqrt(f6*f6 + 1.0) ) );
		} else return 0.0;
	case BARK_SPEEX:
		return (13.1*atan(.00074*x)+2.24*atan(x*x*1.85e-8)+1e-4*x);
	case MEL: // Mel scale according to: L.L. Beranek (1949) Acoustic Measurements, New York: Wiley. 
		if (x>0.0) {
			return 1127.0 * log(1.0 + x/700.0);
		} else return 0.0;
	case LINEAR: 
	default:
		return x;
	}
	return x;
}

double OSMelspec::specScaleTransfInv(double x, OSMelspec::SPECTSCALE scale, double param)
{

	// following code taken from openSMILE 1.0.1, melspec.cpp
	// http://opensmile.sourceforge.net/

	switch(scale) {
	case LOG: 
		return exp(x * log(param)); // param = logScaleBase
	case SEMITONE:
		return param * pow(2.0, x/12.0); // param = firstNote
	case BARK: { // Bark scale according to : H. Traunmüller (1990) "Analytical expressions for the tonotopic sensory scale" J. Acoust. Soc. Am. 88: 97-100.   
		double z0 = (x+0.53)/26.81;
		if (z0 != 1.0) return (1960.0 * z0)/(1.0-z0);
		else return 0.0;
						  }
	case BARK_SCHROED:
		return 600.0 * sinh(x/6.0);
		//return 0.0;
	case BARK_SPEEX:
		ssi_err ("SPECTSCALE_BARK_SPEEX: inversion not yet implemented");
	case MEL :  // Mel scale according to: L.L. Beranek (1949) Acoustic Measurements, New York: Wiley. 
		return 700.0*(exp(x/1127.0)-1.0);        
	case LINEAR:
	default:
		return x;
	}
	return x;
}


// blocksize is size of fft block, _T is period of fft frames
// sampling period is reconstructed by: _T/((blocksize-1)*2)
int OSMelspec::computeFilters (long blocksize, double frameSizeSec)
{

	// following code taken from openSMILE 1.0.1, melspec.cpp
	// http://opensmile.sourceforge.net/


	long _nBands = nBands;
	bs = blocksize;

	if (hfcc) {  // || custom filter
		// independent coefficients for every mel band
		_filterCoeffs = new FLOAT_DMEM[blocksize * _nBands];
		// channel map is different here: start and end fft bins of each band's filter
		_chanMap = new long[_nBands * 2];
	} else {
		_filterCoeffs = new FLOAT_DMEM[blocksize];
		_chanMap = new long[blocksize];
	}
	_filterCfs = new FLOAT_DMEM[_nBands+2];

	FLOAT_DMEM _N = (FLOAT_DMEM) ((blocksize-1)*2);
	FLOAT_DMEM F0 = (FLOAT_DMEM)(1.0/frameSizeSec);
	FLOAT_DMEM Fs = (FLOAT_DMEM)(_N/frameSizeSec);
	FLOAT_DMEM M = (FLOAT_DMEM)_nBands;

	if ((lofreq < 0.0)||(lofreq>Fs/2.0)||(lofreq>hifreq)) lofreq = 0.0;
	if ((hifreq<lofreq)||(hifreq>Fs/2.0)||(hifreq<=0.0)) hifreq = Fs/(FLOAT_DMEM)2.0; // Hertz(NtoFmel(blocksize+1,F0));
	FLOAT_DMEM LoF = (FLOAT_DMEM)specScaleTransfFwd(lofreq, specScale, param);  // Lo Cutoff Freq (mel)
	FLOAT_DMEM HiF = (FLOAT_DMEM)specScaleTransfFwd(hifreq, specScale, param);  // Hi Cutoff Freq (mel)
	nLoF = FtoN(lofreq,F0);  // Lo Cutoff Freq (fft bin)
	nHiF = FtoN(hifreq,F0);  // Hi Cutoff Freq (fft bin)

	if (nLoF > blocksize) nLoF = blocksize;
	if (nHiF > blocksize) nHiF = blocksize;
	if (nLoF < 0) nLoF = 0; //  exclude DC component??
	if (nHiF < 0) nHiF = 0;

	// TODO: option for fully flexible filter bank: n (cf, bw) pairs


	if (hfcc) {
		// custom bandwidth hfcc bank:

		// centre frequencies (similar to standard mfcc, but still a bit different for first and last filter)
		double B,C,b,c,a, bh, ch, ah;
		int m,n;

		// Moore and Glasberg's ERB coefficients for Hz scaling
		a = 0.00000623;
		b = 0.09339;
		c = 28.52;

		// first:
		double fl1 = lofreq;
		double fc1, fh1;

		ah = 0.5/(700.0 + fl1);
		bh = 700.0/(700.0 + fl1);
		ch = -fl1/2.0 * (1.0 + 700.0/(700.0+fl1));

		B = (b-bh)/(a-ah);
		C = (c-ch)/(a-ah);
		fc1 = 0.5*(-B+sqrt(B*B - 4*C));
		fh1 = 2.0 * (a*fc1*fc1 + b*fc1 + c) + fl1; //?

		// last:
		double fhN = hifreq;
		double fcN, flN;

		ah = -0.5/(700.0 + fhN);
		bh = -700.0/(700.0 + fhN);
		ch = fhN/2.0 * (1.0 + 700.0/(700.0+fhN));

		B = (b-bh)/(a-ah);
		C = (c-ch)/(a-ah);
		fcN = 0.5*(-B+sqrt(B*B - 4*C));
		flN = -2.0 * (a*fcN*fcN + b*fcN + c) + fhN; //?

		// equidistant spacing on mel-scale from fc1 to fcN of nBands-2 filters
		double fcNmel = specScaleTransfFwd(fcN, specScale, param);
		double fc1mel = specScaleTransfFwd(fc1, specScale, param);

		FLOAT_DMEM mBandw = (FLOAT_DMEM) ((fcNmel-fc1mel)/(M-1.0f));
		for (m=0; m<_nBands-1; m++) {
			_filterCfs[m] = (FLOAT_DMEM) (fc1mel + m*mBandw);
		}
		_filterCfs[m] = (FLOAT_DMEM) fcNmel;

		for (m=0; m<_nBands; m++) {
			double fc = specScaleTransfInv(_filterCfs[m], specScale, param);
			double ERB7 = a*fc*fc + b* fc + c + 700.0;
			double fl,fh;

			fl = -(ERB7) + sqrt((ERB7*ERB7) + fc*(fc+1400.0));
			fh = fl + 2*(ERB7-700.0);

			if (showFbank) {
				ssi_fprint (ssiout, "Band %i : center = %f Hz (fl: %f , fh: %f Hz ; ERB: %f Hz)",m,fc,fl,fh,(fh-fl)/2.0);
			}

			// start and end fft bins in chanMap
			long flI = _chanMap[m*2] =   FtoN((FLOAT_DMEM) fl,F0); // start
			long fcI = FtoN((FLOAT_DMEM) fc,F0);
			long fhI = _chanMap[m*2+1] = FtoN((FLOAT_DMEM) fh,F0); // end

			//// triangular filters
			// rising slope
			for (n=MAX(flI,0); (n<=fcI)&&(n<blocksize); n++) {
				double f = NtoF(n,F0);
				_filterCoeffs[m*blocksize + n] = (FLOAT_DMEM) ((f-fl)/(fc-fl));
			}
			// falling slope
			for (n=MAX(fcI,0)+1; (n<=fhI)&&(n<blocksize); n++) {
				double f = NtoF(n,F0);
				_filterCoeffs[m*blocksize + n] = (FLOAT_DMEM) ((fh-f)/(fh-fc));
			}
		}
	} else {
		// standard mel filter bank:

		int m,n;
		// compute mel center frequencies
		FLOAT_DMEM mBandw = (HiF-LoF)/(M+(FLOAT_DMEM)1.0); // half bandwidth of mel bands
		for (m=0; m<=_nBands+1; m++) {
			_filterCfs[m] = LoF+(FLOAT_DMEM)m*mBandw;
		}

		if (showFbank) {
			for (m=0; m<=_nBands+1; m++) {
				ssi_fprint (ssiout, "Band %i : center = %f Hz",m-1,specScaleTransfInv(_filterCfs[m], specScale, param));
			}
		}

		// compute channel mapping table:
		m = 0;
		for (n=0; n<blocksize; n++) {
			if ( (n<=nLoF)||(n>=nHiF) ) _chanMap[n] = -3;
			else {
				//printf("II: Cfs[%i]=%f n=%i F0=%f NtoFmel(n,F0)=%f\n",m,_filterCfs[m],n,F0,NtoFmel(n,F0));
				while (_filterCfs[m] < NtoFmel(n,F0)) {
					if (m>_nBands) break;
					m++;
					//printf("Cfs[%i]=%f n=%i F0=%f\n",m,_filterCfs[m],n,F0);
				}
				_chanMap[n] = m-2;
			}
		}

		// compute filter weights (falling slope only):
		m = 0;
		FLOAT_DMEM nM;
		for (n=nLoF;n<nHiF;n++) {
			nM = NtoFmel(n,F0);
			while ((nM > _filterCfs[m+1]) && (m<=_nBands)) m++;
			_filterCoeffs[n] = ( _filterCfs[m+1] - nM )/(_filterCfs[m+1] - _filterCfs[m]);
		}

	}
	return 0;
}


void OSMelspec::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	ssi_size_t num = stream_in.num;
	long Nsrc = stream_in.dim;
	long Ndst = stream_out.dim;
	ssi_real_t *src = ssi_pcast (ssi_real_t, stream_in.ptr);
	ssi_real_t *dst = ssi_pcast (ssi_real_t, stream_out.ptr);

	for (ssi_size_t i = 0; i < num; i++) { 

		ssi_real_t *src = ssi_pcast (ssi_real_t, stream_in.ptr) + i * Nsrc;
		ssi_real_t *dst = ssi_pcast (ssi_real_t, stream_out.ptr) + i * Ndst;

		// following code taken from openSMILE 1.0.1, melspec.cpp
		// http://opensmile.sourceforge.net/

		int m,n;
		FLOAT_DMEM *_src;
		
		// copy & square the fft magnitude
		if (usePower) {
			_src = new FLOAT_DMEM[Nsrc];
			for (n=0; n<Nsrc; n++) {
				_src[n] = src[n]*src[n];
			}
			src = _src;
		}

		// do the mel filtering by multiplying with the filters and summing up
		memset(dst, 0, Ndst*sizeof(FLOAT_DMEM));

		if (hfcc) {
			// TODO: perform alternate (less optimised) multiplication for custom bandwidth methods.
			//for (n=nLoF[idxi]; n<nHiF[idxi]; n++) {
			for (m=0; m<nBands; m++) {
				long n1, n2;
				n1 = _chanMap[m*2];
				n2 = _chanMap[m*2+1];
				//        for (n=nLoF[idxi]; n<nHiF[idxi]; n++) {
				for (n=MAX(n1,nLoF); (n<=n2)&&(n<nHiF); n++) {
					dst[m] += (FLOAT_DMEM) ((double)src[n] * (double)_filterCoeffs[m*Nsrc+n]);
				}
			}


		} else {

			double a;
			for (n=nLoF; n<nHiF; n++) {
				m = _chanMap[n];
				a = (double)src[n] * (double)_filterCoeffs[n];
				if (m>-2) {
					if (m>-1) dst[m] += (FLOAT_DMEM)a;
					if (m < nBands-1)
						dst[m+1] += src[n] - (FLOAT_DMEM)a;
				}
			}

		}

		if ((usePower)&&(_src!=NULL)) free((void *)_src);

		if (htkcompatible) {
			for (m=0; m<nBands; m++) {
				if (usePower) {
					dst[m] *= (FLOAT_DMEM)(32767.0*32767.0);
				} else {
					dst[m] *= (FLOAT_DMEM)32767.0;
				}
				// the above is for HTK compatibility....
				// HTK does not scale the input sample values to -1 / +1
				// thus, we must multiply by the max 16bit sample value again.
			}
		}
	}
}

}
