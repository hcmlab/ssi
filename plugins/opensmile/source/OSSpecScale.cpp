// OSSpecScale.cpp
// author: Johannes Wagner <johannes.wagner@informatik.uni-augsburg.de>
// created: 2011/09/22
// Copyright (C) 2007-11 University of Augsburg, Johannes Wagner
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

#include "OSSpecScale.h"

#ifdef USE_SSI_LEAK_DETECTOR
    #include "SSI_LeakWatcher.h"
    #ifdef _DEBUG
        #define new DEBUG_NEW
        #undef THIS_FILE
        static char THIS_FILE[] = __FILE__;
    #endif
#endif

        namespace ssi {

OSSpecScale::OSSpecScale (const ssi_char_t *file)
    : _file (0),
    f_t (0),
    audw (0),
    spline_work (0),
    y (0),
    y2 (0) {

    pmdata = &mdata[0];

    if (file) {
        if (!OptionList::LoadXML (file, _options)) {
            OptionList::SaveXML (file, _options);
        }
        _file = ssi_strcpy (file);
    }
}

OSSpecScale::~OSSpecScale () {

    release ();

    if (_file) {
        OptionList::SaveXML (_file, _options);
        delete[] _file;
    }
}

void OSSpecScale::release () {

    delete[] audw; audw = 0;
    delete[] f_t; f_t = 0;
    delete[] spline_work; spline_work = 0;
    delete[] y; y = 0;
    delete[] y2; y2 = 0;
}

double OSSpecScale::specScaleTransfFwd(double x, OSSpecScale::SPECTSCALE scale, double param)
{
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

void OSSpecScale::transform_enter (ssi_stream_t &stream_in,
    ssi_stream_t &stream_out,
    ssi_size_t xtra_stream_in_num,
    ssi_stream_t xtra_stream_in[]) {

    release ();

    // following code taken from openSMILE 1.0.1, specScale.cpp
    // http://opensmile.sourceforge.net/

    nMag = stream_in.dim;
    fsSec = _options.fsSec;
    if (fsSec == -1.0) {
        fsSec = (float) (1.0/stream_in.sr);
    }
    magStart = 0;
    deltaF = 1.0/fsSec;
    nPointsTarget = _options.nPoints;
    if (nPointsTarget <= 0) {
        nPointsTarget = nMag;
    }
    int i;

    scale = _options.dstScale;
    logScaleBase = _options.dstLogScaleBase;
    firstNote = _options.firstNote;

    if (scale == LOG) {
        if ((logScaleBase <= 0.0)||(logScaleBase==1.0)) {
            ssi_wrn ("logScaleBase '%f' must be > 0.0 and != 1.0. Setting it to 2.0 now.", logScaleBase);
            logScaleBase = 2.0;
        }
    }

    sourceScale = _options.srcScale;
    logSourceScaleBase = _options.srcLogScaleBase;
    if (sourceScale == LOG) {
        if ((logSourceScaleBase <= 0.0)||(logSourceScaleBase==1.0)) {
            ssi_wrn ("logScaleBase '%f' must be > 0.0 and != 1.0. Setting it to 2.0 now.", logScaleBase);
            logSourceScaleBase = 2.0;
        }
    }

    specEnhance = _options.enhance ? 1 : 0;
    specSmooth = _options.smooth ? 1 : 0;
    auditoryWeighting = _options.weight ? 1 : 0;
    if (auditoryWeighting) {
        if (!(scale == LOG && (logScaleBase == 2.0))) {
            auditoryWeighting = 0;
            ssi_wrn ("auditory weighting is currently only supported for octave target scales (log 2)! Disabling auditory weighting.");
        }
    }

    minF = _options.minF;
    if (minF < 1.0) {
        minF = 1.0;
        ssi_wrn ("minF (%f) must be >= 1.0! Setting minF to 1.0", minF);
    }
    maxF = _options.maxF;

    if (scale == LOG)
        param = logScaleBase;
    else if (scale == SEMITONE)
        param = firstNote;
    else
        param = 0.0;

    // check maxF < sampleFreq.
    double samplF = deltaF * (double)nMag; // sampling frequency
    if ((maxF <= minF)||(maxF > stream_in.sr)) {
        maxF = samplF;
    }

    fmin_t = specScaleTransfFwd (minF,scale,param);
    fmax_t = specScaleTransfFwd (maxF,scale,param);

    // target delta f
    deltaF_t = (fmax_t - fmin_t) / (nPointsTarget - 1);

    // calculate the target frequencies of the linear scale fft input points
    f_t = new double[nMag];

    if (scale == LOG) {
        for (i=1; i < nMag; i++) {
            f_t[i] = specScaleTransfFwd( (double)i * (double)deltaF , scale, param );
        }
        f_t[0] = 2.0 * f_t[1] - f_t[2]; // heuristic for the 0th frequency (only valid for log2 ??)
    } else { // generic transform:
        for (i=0; i < nMag; i++) {
            f_t[i] = specScaleTransfFwd( (double)i * (double)deltaF , scale, param );
        }
    }

    double nOctaves = log(maxF / minF)/log(2.0);
    double nPointsPerOctave = nPointsTarget / nOctaves; // this is valid for log(2.0) scale only...
    if (auditoryWeighting) {
        /* auditory weighting function (octave scale only...)*/
        double atan_s = nPointsPerOctave * smileMath_log2(65.0 / 50.0) - 1.0;
        audw = new double[nPointsTarget];
        for (i=0; i < nPointsTarget; i++) {
            audw[i] = 0.5 + atan (3.0 * (i + 1 - atan_s) / nPointsPerOctave) / M_PI;
        }
    }

    // allocate workspace arrays
    y = new double[nMag];
    y2 = new double[nMag];

    mdata[0] = (FLOAT_DMEM)minF; /* min frequency (source) */
    mdata[1] = (FLOAT_DMEM)maxF; /* max frequency (source) */
    mdata[2] = (FLOAT_DMEM)nOctaves; /* number of octaves (log/octave scales only) */
    mdata[3] = (FLOAT_DMEM)nPointsPerOctave; /* points per octave, valid only for log(2) scale! */
    mdata[4] = (FLOAT_DMEM)fmin_t; /* min frequency (target) */
    mdata[5] = (FLOAT_DMEM)fmax_t; /* max frequency (target) */
    mdata[6] = (FLOAT_DMEM)scale; /* constant indicating the type of target scale */
    mdata[7] = (FLOAT_DMEM)param; /* target scale param (if applicable) */
}

void OSSpecScale::transform (ITransformer::info info,
    ssi_stream_t &stream_in,
    ssi_stream_t &stream_out,
    ssi_size_t xtra_stream_in_num,
    ssi_stream_t xtra_stream_in[]) {

    ssi_size_t num = stream_in.num;
    long Nsrc = stream_in.dim;
    long Ndst = stream_out.dim;
    ssi_real_t *src = ssi_pcast (ssi_real_t, stream_in.ptr);
    ssi_real_t *dst = ssi_pcast (ssi_real_t, stream_out.ptr);

    static ssi_size_t count = 0;
    for (ssi_size_t j = 0; j < num; j++) {

        count++;

        ssi_real_t *src = ssi_pcast (ssi_real_t, stream_in.ptr) + j * Nsrc;
        ssi_real_t *dst = ssi_pcast (ssi_real_t, stream_out.ptr) + j * Ndst;

        // following code taken from openSMILE 1.0.1, specScale.cpp
        // http://opensmile.sourceforge.net/

        // we assume we have fft magnitude as input...
        double _N = (double)(Nsrc);
        int i;

        // copy and (possibly) convert input data
        for (i=magStart; i<magStart+nMag; i++) {
            y[i-magStart] = (double)src[i];
        }

        // do spectral peak enhancement, if enabled
        if (specEnhance) smileDsp_specEnhanceSHS(y,nMag);

        // do spectral smoothing, if enabled
        if (specSmooth) smileDsp_specSmoothSHS(y,nMag);

        // scale to the target scale and interpolate missing values via spline interpolation
        if ( smileMath_spline( f_t, y, nMag, 1e30, 1e30, y2, &spline_work ) ) {
            // after successful spline computation, do the actual interpolation point by point
            for (i=0; i < nPointsTarget; i++) {
                double f = fmin_t + (double)i * deltaF_t;
                double out;
                smileMath_splint( f_t, y, y2, nMag, f, &out);
                dst[i] = (FLOAT_DMEM)out; // save in output vector
            }
        } else {
            ssi_wrn ("spline computation failed on current frame, zeroing the output (?!)");
            // zero output
            for (i=0; i < nPointsTarget; i++) {
                dst[i] = 0.0;
            }
        }


        // multiply by frequency selectivity of the auditory system (octave-scale only)
        if (auditoryWeighting) {
            for (i=0; i < nPointsTarget; i++) {
                if (dst[i] > 0.0) {
                    dst[i] = (FLOAT_DMEM)( (double)dst[i] * audw[i] );
                } else {
                    dst[i] = 0.0;
                }
            }
        }

    }
}


}
