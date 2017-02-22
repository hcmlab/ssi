// OSFunctionals.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/09/21 
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

/**

Provides Functionals analysis.

*/

#pragma once

#ifndef SSI_OPENSMILE_FUNCTIONALS_H
#define SSI_OPENSMILE_FUNCTIONALS_H

#include "base/IFeature.h"
#include "OSTools.h"
#include "ioput/option/OptionList.h"

//#define PRINT_SAMPLE_NUMBER_DIMENSION

#define FUNCTIONAL_CROSSINGS 0
#define FUNCTIONAL_DCT 1
#define FUNCTIONAL_SAMPLES 2
#define FUNCTIONAL_SEGMENTS 3
#define FUNCTIONAL_TIMES 4
#define FUNCTIONAL_EXTREMES 5
#define FUNCTIONAL_MEANS 6
#define FUNCTIONAL_ONSET 7
#define FUNCTIONAL_PEAKS 8
#define FUNCTIONAL_PERCENTILES 9
#define FUNCTIONAL_REGRESSION 10
#define FUNCTIONAL_MOMENTS 11
#define FUNCTIONAL_PEAKS2 12

#define functionals_names "Crossings","DCT","Samples","Segments","Times","Extremes","Means","Onset","Peaks","Percentiles","Regression","Moments"

//Time Norm
#define TIMENORM_SEGMENT   0
#define TIMENORM_SEGMENTS   0
#define TIMENORM_SECOND    1
#define TIMENORM_SECONDS    1
#define TIMENORM_FRAME     2
#define TIMENORM_FRAMES     2
#if _WIN32||_WIN64
#define finite _finite
#else
#define finite std::isfinite
#endif

//Functional Crossings
#define FUNCT_ZCR     0 //zero crossing
#define FUNCT_MCR     1
#define FUNCT_AMEAN   2

//Functional DCT
#define FUNCT_FC     0 //first coefficient
#define FUNCT_LC     1 //last coefficient

//Functional Segments
#define FUNCT_NUMSEGMENTS      0  // number of segments (relative to maxNumSeg)
#define FUNCT_SEGMEANLEN       1  // mean length of segment (relative to input length)
#define FUNCT_SEGMAXLEN        2  // max length of segment (relative to input length)
#define FUNCT_SEGMINLEN        3  // min length of segment (relative to input length)

//Functional Times
#define FUNCT_UPLEVELTIME25     0
#define FUNCT_DOWNLEVELTIME25   1
#define FUNCT_UPLEVELTIME50     2
#define FUNCT_DOWNLEVELTIME50   3
#define FUNCT_UPLEVELTIME75     4
#define FUNCT_DOWNLEVELTIME75   5
#define FUNCT_UPLEVELTIME90     6
#define FUNCT_DOWNLEVELTIME90   7
#define FUNCT_RISETIME      8
#define FUNCT_FALLTIME      9
#define FUNCT_LEFTCTIME     10
#define FUNCT_RIGHTCTIME    11
#define FUNCT_DURATION      12
#define FUNCT_UPLEVELTIME        13
#define FUNCT_DOWNLEVELTIME      14

//Functional Extremes
#define FUNCT_MAX     0
#define FUNCT_MIN     1
#define FUNCT_RANGE   2
#define FUNCT_MAXPOS   3
#define FUNCT_MINPOS   4
#define FUNCT_MAXAMEANDIST   5
#define FUNCT_MINAMEANDIST   6

//Functional Means
#define FUNCT_AMEAN_ME  0
#define FUNCT_ABSMEAN   1
#define FUNCT_QMEAN     2
#define FUNCT_NZAMEAN   3
#define FUNCT_NZABSMEAN 4
#define FUNCT_NZQMEAN   5
#define FUNCT_NZGMEAN   6
#define FUNCT_NNZ       7
#define FUNCT_FLATNESS  8  // flatness : geometric mean / arithmetic mean
#define FUNCT_POSAMEAN  9  // arithmetic mean from positive values only
#define FUNCT_NEGAMEAN  10 // arithmetic mean from negative values only
#define FUNCT_POSQMEAN  11
#define FUNCT_POSRQMEAN 12
#define FUNCT_NEGQMEAN  13
#define FUNCT_NEGRQMEAN 14
#define FUNCT_RQMEAN    15  // *r*oot *q*uadratic mean
#define FUNCT_NZRQMEAN  16

//Functional OnSet
#define FUNCT_ONSETPOS       0  // number of segments (relative to maxNumSeg)
#define FUNCT_OFFSETPOS      1  // mean length of segment (relative to input length)
#define FUNCT_NUMONSETS      2  // number of segments (relative to maxNumSeg)
#define FUNCT_NUMOFFSETS     3  // mean length of segment (relative to input length)

//Functional Peaks
#define FUNCT_NUMPEAKS         0  // number of peaks
#define FUNCT_MEANPEAKDIST     1  // mean distance between peaks
#define FUNCT_PEAKMEAN         2  // arithmetic mean of peaks
#define FUNCT_PEAKMEANMEANDIST 3  // aritmetic mean of peaks - aritmetic mean
#define FUNCT_PEAKDISTSTDDEV   4  // standard deviation of inter peak distances
#define PEAKDIST_BLOCKALLOC 50

//Functional Pecentiles
#define FUNCT_QUART1        0
#define FUNCT_QUART2        1
#define FUNCT_QUART3        2
#define FUNCT_IQR12         3
#define FUNCT_IQR23         4
#define FUNCT_IQR13         5
#define FUNCT_PERCENTILE    6
#define FUNCT_PCTLRANGE     7

//Functional Regression
#define FUNCT_LINREGC1     0
#define FUNCT_LINREGC2     1
#define FUNCT_LINREGERRA   2
#define FUNCT_LINREGERRQ   3
#define FUNCT_QREGC1       4
#define FUNCT_QREGC2       5
#define FUNCT_QREGC3       6
#define FUNCT_QREGERRA     7
#define FUNCT_QREGERRQ     8
#define FUNCT_CENTROID     9
#define FUNCT_QREGLS       10  // left slope of parabola
#define FUNCT_QREGRS       11  // right slope of parabola
#define FUNCT_QREGX0       12  // vertex coordinates
#define FUNCT_QREGY0       13  // vertex coordinates
#define FUNCT_QREGYR       14  // right value (t=Nin) of parabola
#define FUNCT_QREGY0NN    15  // vertex y coordinate, not normalised
#define FUNCT_QREGC3NN    16  // qregc3, not normalised
#define FUNCT_QREGYRNN    17  // not normalised

//Functional Moments
#define FUNCT_VAR        0
#define FUNCT_STDDEV     1
#define FUNCT_SKEWNESS   2
#define FUNCT_KURTOSIS   3
#define FUNCT_AMEAN_MOM      4


//Functional Peaks 2
#define FUNCT_NUMPEAKS          0  // number of peaks
#define FUNCT_MEANPEAKDIST      1  // mean distance between peaks
#define FUNCT_MEANPEAKDISTDELTA 2  // mean of difference of consecutive peak distances
#define FUNCT_PEAKDISTSTDDEV2    3  // standard deviation of inter peak distances
#define FUNCT_PEAKRANGEABS      4  // absolute peak amplitude range (max peak - min peak)
#define FUNCT_PEAKRANGEREL      5  // peak amplitude range normalised to the input contour's arithmetic mean
#define FUNCT_PEAKMEAN_ABS          6  // arithmetic mean of peak amplitudes
#define FUNCT_PEAKMEANMEANDIST2  7  // arithmetic mean of peak amplitudes - arithmetic mean
#define FUNCT_PEAKMEANMEANRATIO 8  // arithmetic mean of peak amplitudes / arithmetic mean
#define FUNCT_PTPAMPMEANABS     9  // mean of peak to peak amplitude differences
#define FUNCT_PTPAMPMEANREL     10  // mean of peak to peak amplitude differences / arithmetic mean
#define FUNCT_PTPAMPSTDDEVABS   11  // standard deviation of peak to peak amplitude differences
#define FUNCT_PTPAMPSTDDEVREL   12  // standard deviation of peak to peak amplitude differences / arithmetic mean

#define FUNCT_MINRANGEABS       13  // absolute minima amplitude range (max peak - min peak)
#define FUNCT_MINRANGEREL       14  // minima amplitude range normalised to the input contour's arithmetic mean
#define FUNCT_MINMEAN           15  // arithmetic mean of minima amplitudes
#define FUNCT_MINMEANMEANDIST   16  // arithmetic mean of minima amplitudes - arithmetic mean
#define FUNCT_MINMEANMEANRATIO  17  // arithmetic mean of minima amplitudes / arithmetic mean
#define FUNCT_MTMAMPMEANABS     18  // mean of minima to minima amplitude differences
#define FUNCT_MTMAMPMEANREL     19  // mean of minima to minima amplitude differences / arithmetic mean
#define FUNCT_MTMAMPSTDDEVABS   20  // standard deviation of min to min amplitude differences
#define FUNCT_MTMAMPSTDDEVREL   21  // standard deviation of min to min amplitude differences / arithmetic mean

#define FUNCT_MEANRISINGSLOPE     22
#define FUNCT_MAXRISINGSLOPE      23
#define FUNCT_MINRISINGSLOPE      24
#define FUNCT_STDDEVRISINGSLOPE   25

#define FUNCT_MEANFALLINGSLOPE    26
#define FUNCT_MAXFALLINGSLOPE     27
#define FUNCT_MINFALLINGSLOPE     28
#define FUNCT_STDDEVFALLINGSLOPE  29


#define FUNCT_ENAB_N  13
#define MAX_FEATURES_N 30

//#define PRINT_SAMPLE_NUMBER_DIMENSION

namespace ssi {

	

class OSFunctionals : public IFeature {
	
	struct Variables {
		FLOAT_DMEM *in;
		FLOAT_DMEM *inSorted; 
		FLOAT_DMEM min; 
		FLOAT_DMEM max; 
		FLOAT_DMEM mean; 
		FLOAT_DMEM *out; 
		long Nin;
		
	};
	struct peakMinMaxListEl {
      int type;
      FLOAT_DMEM y;
      long x;
      struct peakMinMaxListEl * next, *prev;
    };
	
protected: int functN[FUNCT_ENAB_N]; // number of features required per each functional

public: ssi_size_t reset;
			
public:

	class Options : public OptionList {

	public:	ssi_size_t enab_output[FUNCT_ENAB_N][MAX_FEATURES_N]; // 1/0 = enable/disable for an output or an integer (i.e. coeff,no of segments,....)
			ssi_size_t enab_funct[FUNCT_ENAB_N]; // 1/0 = enable/disable for each functional
			ssi_size_t nonZeroFunct;
			ssi_char_t enabled_dimensions[SSI_MAX_CHAR];
			bool enabled_deltas;
			const static ssi_size_t masterTimeNorm = TIMENORM_FRAMES;

			ssi_char_t samplepos[SSI_MAX_CHAR]; // for Functional Samples
			
			ssi_size_t maxSegments; // for Functional Segments
			ssi_real_t rangeRelThreshold; // for Functional Segments
			
			ssi_char_t upleveltime[SSI_MAX_CHAR]; // for Functional Times
			ssi_char_t downleveltime[SSI_MAX_CHAR]; // for Functional Times
			
			ssi_real_t threshold,thresholdOnset,thresholdOffset; // for Functional Onsets
			ssi_size_t setThresholdOnset,setThresholdOffset; // for Functional OnSets
			ssi_size_t useAbsVal; // for Functional OnSets
			
			ssi_size_t overlapFlag; //for Functional Peaks
			long* peakdists; //for Functional Peaks
			int nPeakdists; //for Functional Peaks

			ssi_size_t enabQuart,enabIrq,interp; // for Functional Percentiles
			ssi_char_t percentile[SSI_MAX_CHAR]; // for Functional Percentiles
			ssi_char_t pctlrange[SSI_MAX_CHAR]; // for Functional Percentiles

			ssi_size_t normRegCoeff,normInputs; //for Functional Regression

			ssi_size_t useAbsThresh,dynRelThresh; //for Functional Peaks2
			ssi_real_t absThresh,relThresh;
			struct peakMinMaxListEl * mmlistFirst;
			struct peakMinMaxListEl * mmlistLast;
			
	void intializeOptions() {

		ssi_size_t last = 0;

			for(int i=0 ; i<FUNCT_ENAB_N ; i++) {

				enab_funct[i] = 0;

				switch(i) {
						case FUNCTIONAL_CROSSINGS	: last = FUNCT_AMEAN;break;
						case FUNCTIONAL_DCT			: last = FUNCT_LC;break;
						case FUNCTIONAL_SAMPLES		: last = -1;break;
						case FUNCTIONAL_SEGMENTS	: last = FUNCT_SEGMINLEN;break;
						case FUNCTIONAL_TIMES		: last = FUNCT_DURATION;break;
						case FUNCTIONAL_EXTREMES	: last = FUNCT_MINAMEANDIST;break;
						case FUNCTIONAL_MEANS		: last = FUNCT_NZRQMEAN;break;
						case FUNCTIONAL_ONSET		: last = FUNCT_NUMOFFSETS;break;
						case FUNCTIONAL_PEAKS		: last = FUNCT_PEAKDISTSTDDEV;break;
						case FUNCTIONAL_PERCENTILES : last = FUNCT_IQR13;break;
						case FUNCTIONAL_REGRESSION	: last = FUNCT_QREGYRNN;break;
						case FUNCTIONAL_MOMENTS		: last = FUNCT_AMEAN_MOM;break;
						case FUNCTIONAL_PEAKS2      : last = FUNCT_STDDEVFALLINGSLOPE;break;					
					}
				
				for(ssi_size_t k=0; k<MAX_FEATURES_N; k++) {
					
					if(k > last)
						enab_output[i][k] = 0;
					else
						enab_output[i][k] = 1;
														
				}

				

			}

			nonZeroFunct = 0;
			enabled_deltas = true;
			enabled_dimensions[0] = '\0';
			samplepos[0] = '\0';
			maxSegments = 100;
			rangeRelThreshold = 0.25f;
			upleveltime[0] = '\0';
			downleveltime[0] = '\0';
			threshold = thresholdOffset = thresholdOnset = 0.0;
			setThresholdOnset = setThresholdOffset = 1;
			useAbsVal = enabIrq = enabQuart = interp = 1;
			overlapFlag = 1;
			percentile[0] = '\0';
			pctlrange[0] = '\0';
			normRegCoeff = normInputs = 1;
			peakdists = NULL;
			nPeakdists = 0;
			mmlistFirst = mmlistLast = NULL;
			useAbsThresh = 0;
			dynRelThresh = 1;
			absThresh = relThresh = 0.0;
			
			return;
		};
			
	public:

		Options () {

			intializeOptions();

			// General Options for Functionals

			addOption ("nonZeroFuncts", &nonZeroFunct, 1, SSI_INT, "If this is set to 1, functionals are only applied to input values unequal 0. If this is set to 2, functionals are only applied to input values greater than 0.");
			addOption ("enabledDimensions", enabled_dimensions, SSI_MAX_CHAR, SSI_CHAR, "array of enabled dimensions on which functionals will apply, if left empty all of them will be enabled");
			

			// Options for functional Crossings

			addOption("Crossings",&enab_funct[FUNCTIONAL_CROSSINGS],1,SSI_INT,"1/0=enable/disable outputs of functional Crossings");
			
			addOption ("zcr", &enab_output[FUNCTIONAL_CROSSINGS][FUNCT_ZCR], 1, SSI_INT, "1/0=enable/disable output of zero crossing rate");		
			addOption ("mcr", &enab_output[FUNCTIONAL_CROSSINGS][FUNCT_MCR], 1, SSI_INT, "1/0=enable/disable output of mean crossing rate (the rate at which the signal crosses its arithmetic mean value (same as zcr for mean normalised signals)");		
			addOption ("amean", &enab_output[FUNCTIONAL_CROSSINGS][FUNCT_AMEAN], 1, SSI_INT, "1/0=enable/disable output of arithmetic mean");
			
			// Options for functional DCT
			addOption("DCT",&enab_funct[FUNCTIONAL_DCT],1,SSI_INT,"1/0=enable/disable outputs of functional dct");
			
			addOption("firstCoeff",&enab_output[FUNCTIONAL_DCT][FUNCT_FC],1,SSI_INT,"The first DCT coefficient to compute (coefficient 0 corresponds to the DC component)");
			addOption("lastCoeff",&enab_output[FUNCTIONAL_DCT][FUNCT_LC],1,SSI_INT,"The last DCT coefficient to compute");
			
			// Options for functional Samples
			addOption("Samples",&enab_funct[FUNCTIONAL_SAMPLES],1,SSI_INT,"1/0=enable/disable outputs of functional samples");
			
			addOption ("samplepos", samplepos, SSI_MAX_CHAR, SSI_CHAR, "Array of positions of samples to copy to the output (i.e. 0.1,0.5,..). The size of this array determines the number of sample frames that will be passed to the output. The given positions must be in the range from 0 to 1, indicating the relative position whithin the input segment, where 0 is the beginning and 1 the end of the segment.");		
			
			//Options for functional Segments
			addOption("Segments",&enab_funct[FUNCTIONAL_SEGMENTS],1,SSI_INT,"1/0=enable/disable outputs of functional segments");
			
			addOption("maxNumSeg",&maxSegments,1,SSI_INT,"Maximum number of segments to detect");
			addOption("rangeRelThreshold",&rangeRelThreshold,1,SSI_FLOAT,"The segment threshold relative to the signal's range (max-min)");
			addOption("numSegments",&enab_output[FUNCTIONAL_SEGMENTS][FUNCT_NUMSEGMENTS],1,SSI_INT,"1/0=enable/disable output of the number of segments (output is relative to maxNumSeg)");
			addOption("meanSegLen",&enab_output[FUNCTIONAL_SEGMENTS][FUNCT_SEGMEANLEN],1,SSI_INT,"1/0=enable/disable output of the mean segment length");
			addOption("maxSegLen",&enab_output[FUNCTIONAL_SEGMENTS][FUNCT_SEGMAXLEN],1,SSI_INT,"1/0=enable/disable output of the maximum segment length");
			addOption("minSegLen",&enab_output[FUNCTIONAL_SEGMENTS][FUNCT_SEGMINLEN],1,SSI_INT,"1/0=enable/disable output of the minimum segment length");
			
			//Options for Functional Times
			addOption("Times",&enab_funct[FUNCTIONAL_TIMES],1,SSI_INT,"1/0=enable/disable outputs of functional times");
			
			addOption("upleveltime25",&enab_output[FUNCTIONAL_TIMES][FUNCT_UPLEVELTIME25],1,SSI_INT,"(1/0=yes/no) compute time where signal is above 0.25*range");
			addOption("downleveltime25",&enab_output[FUNCTIONAL_TIMES][FUNCT_DOWNLEVELTIME25],1,SSI_INT,"(1/0=yes/no) compute time where signal is below 0.25*range");
			addOption("upleveltime50",&enab_output[FUNCTIONAL_TIMES][FUNCT_UPLEVELTIME50],1,SSI_INT,"(1/0=yes/no) compute time where signal is above 0.50*range");
			addOption("downleveltime50",&enab_output[FUNCTIONAL_TIMES][FUNCT_DOWNLEVELTIME50],1,SSI_INT,"(1/0=yes/no) compute time where signal is below 0.50*range");
			addOption("upleveltime75",&enab_output[FUNCTIONAL_TIMES][FUNCT_UPLEVELTIME75],1,SSI_INT,"(1/0=yes/no) compute time where signal is above 0.75*range");
			addOption("downleveltime75",&enab_output[FUNCTIONAL_TIMES][FUNCT_DOWNLEVELTIME75],1,SSI_INT,"(1/0=yes/no) compute time where signal is below 0.75*range");
			addOption("upleveltime90",&enab_output[FUNCTIONAL_TIMES][FUNCT_UPLEVELTIME90],1,SSI_INT,"(1/0=yes/no) compute time where signal is above 0.90*range");
			addOption("downleveltime90",&enab_output[FUNCTIONAL_TIMES][FUNCT_DOWNLEVELTIME90],1,SSI_INT,"(1/0=yes/no) compute time where signal is below 0.90*range");
			addOption("risetime",&enab_output[FUNCTIONAL_TIMES][FUNCT_RISETIME],1,SSI_INT,"(1/0=yes/no) compute time where signal is rising");
			addOption("falltime",&enab_output[FUNCTIONAL_TIMES][FUNCT_FALLTIME],1,SSI_INT,"(1/0=yes/no) compute time where signal is falling");
			addOption("leftctime",&enab_output[FUNCTIONAL_TIMES][FUNCT_LEFTCTIME],1,SSI_INT,"(1/0=yes/no) compute time where signal has left curvature");
			addOption("rightctime",&enab_output[FUNCTIONAL_TIMES][FUNCT_RIGHTCTIME],1,SSI_INT,"(1/0=yes/no) compute time where signal has right curvature");
			addOption("duration",&enab_output[FUNCTIONAL_TIMES][FUNCT_DURATION],1,SSI_INT,"(1/0=yes/no) compute duration time, in frames (or seconds, if (time)norm==seconds)");
			addOption("upleveltime",upleveltime,SSI_MAX_CHAR, SSI_CHAR,"compute time where signal is above X*range : upleveltime[n]=X i.e. (i.e. 0.1,7.5,..) if empty thus disabled ");
			addOption("downleveltime",downleveltime,SSI_MAX_CHAR, SSI_CHAR,"compute time where signal is below X*range : downleveltime[n]=X i.e. (i.e. 1.1,0.5,..) if empty thus disabled");
			
			//Options for Functional Extremes
			addOption("Extremes",&enab_funct[FUNCTIONAL_EXTREMES],1,SSI_INT,"1/0=enable/disable outputs of functional extremes");
			
			addOption("max",&enab_output[FUNCTIONAL_EXTREMES][FUNCT_MAX],1,SSI_INT,"1/0=enable/disable output of maximum value");
			addOption("min",&enab_output[FUNCTIONAL_EXTREMES][FUNCT_MIN],1,SSI_INT,"1/0=enable/disable output of minimum value");
			addOption("range",&enab_output[FUNCTIONAL_EXTREMES][FUNCT_RANGE],1,SSI_INT,"1/0=enable/disable output of range (max-min)");
			addOption("maxpos",&enab_output[FUNCTIONAL_EXTREMES][FUNCT_MAXPOS],1,SSI_INT,"1/0=enable/disable output of position of maximum value");
			addOption("minpos",&enab_output[FUNCTIONAL_EXTREMES][FUNCT_MINPOS],1,SSI_INT,"1/0=enable/disable output of position of minimum value");
			addOption("maxameandist",&enab_output[FUNCTIONAL_EXTREMES][FUNCT_MAXAMEANDIST],1,SSI_INT,"1/0=enable/disable output of (max-arithmetic_mean)");
			addOption("minameandist",&enab_output[FUNCTIONAL_EXTREMES][FUNCT_MINAMEANDIST],1,SSI_INT,"1/0=enable/disable output of (arithmetic_mean-min)");

			//Options for Functional Means
			addOption("Means",&enab_funct[FUNCTIONAL_MEANS],1,SSI_INT,"1/0=enable/disable outputs of functional means");

			addOption("amean_means",&enab_output[FUNCTIONAL_MEANS][FUNCT_AMEAN_ME],1,SSI_INT,"1/0=enable/disable output of arithmetic mean");
			addOption("absmean",&enab_output[FUNCTIONAL_MEANS][FUNCT_ABSMEAN],1,SSI_INT,"1/0=enable/disable output of arithmetic mean of absolute values");
			addOption("qmean",&enab_output[FUNCTIONAL_MEANS][FUNCT_QMEAN],1,SSI_INT,"1/0=enable/disable output of quadratic mean");
			addOption("nzamean",&enab_output[FUNCTIONAL_MEANS][FUNCT_NZAMEAN],1,SSI_INT,"1/0=enable/disable output of arithmetic mean (of non-zero values only)");
			addOption("nzabsmean",&enab_output[FUNCTIONAL_MEANS][FUNCT_NZABSMEAN],1,SSI_INT,"1/0=enable/disable output of arithmetic mean of absolute values (of non-zero values only)");
			addOption("nzqmean",&enab_output[FUNCTIONAL_MEANS][FUNCT_NZQMEAN],1,SSI_INT,"1/0=enable/disable output of quadratic mean (of non-zero values only)");
			addOption("nzgmean",&enab_output[FUNCTIONAL_MEANS][FUNCT_NZGMEAN],1,SSI_INT,"1/0=enable/disable output of geometric mean (of absolute values of non-zero values only)");
			addOption("nnz",&enab_output[FUNCTIONAL_MEANS][FUNCT_NNZ],1,SSI_INT,"1/0=enable/disable output of number of non-zero values");
		    addOption("flatness",&enab_output[FUNCTIONAL_MEANS][FUNCT_FLATNESS],1,SSI_INT,"1/0=enable/disable output of contour flatness (ratio of geometric mean and absolute value arithmetic mean(absmean)))");
			addOption("posamean",&enab_output[FUNCTIONAL_MEANS][FUNCT_POSAMEAN],1,SSI_INT,"1/0=enable/disable output of arithmetic mean of positive values only (usually you would apply this to a differential signal to measure how much the original signal is rising)");
			addOption("negamean",&enab_output[FUNCTIONAL_MEANS][FUNCT_NEGAMEAN],1,SSI_INT,"1/0=enable/disable output of arithmetic mean of negative values only");
			addOption("posqmean",&enab_output[FUNCTIONAL_MEANS][FUNCT_POSQMEAN],1,SSI_INT,"1/0=enable/disable output of quadratic mean of positive values only");
			addOption("posrqmean",&enab_output[FUNCTIONAL_MEANS][FUNCT_POSRQMEAN],1,SSI_INT,"1/0=enable/disable output of root of quadratic mean of positive values only");
			addOption("negqmean",&enab_output[FUNCTIONAL_MEANS][FUNCT_NEGQMEAN],1,SSI_INT,"1/0=enable/disable output of quadratic mean of negative values only");
			addOption("negrqmean",&enab_output[FUNCTIONAL_MEANS][FUNCT_NEGRQMEAN],1,SSI_INT,"1/0=enable/disable output of root of quadratic mean of negative values only");
			addOption("rqmean",&enab_output[FUNCTIONAL_MEANS][FUNCT_RQMEAN],1,SSI_INT,"1/0=enable/disable output of square root of quadratic mean");
			addOption("nzrqmean",&enab_output[FUNCTIONAL_MEANS][FUNCT_NZRQMEAN],1,SSI_INT,"1/0=enable/disable output of square root of quadratic mean of non zero values");
		    

			//Options for Functional OnSet
			addOption("OnSet",&enab_funct[FUNCTIONAL_ONSET],1,SSI_INT,"1/0=enable/disable outputs of functional Onsets");

			addOption("threshold",&threshold,1,SSI_FLOAT,"The absolute threshold used for onset/offset detection (i.e. the first onset will be where the input value is above the threshold for the first time)");
			addOption("thresholdOnset",&thresholdOnset,1,SSI_FLOAT,"A separate threshold only for onset detection. This will override the 'threshold' option, if set");
			addOption("thresholdOffset",&thresholdOffset,1,SSI_FLOAT,"A separate threshold only for offset detection. This will override the 'threshold' option, if set");
			addOption("setThresholdOnset",&setThresholdOnset,1,SSI_INT,"1/0=set/unset thresholdOnset value");
			addOption("setThresholdOffset",&setThresholdOffset,1,SSI_INT,"1/0=set/unset thresholdOnset value");
			addOption("useAbsVal",&useAbsVal,1,SSI_INT,"1/0=yes/no : apply thresholds to absolute input value instead of original input value");
			addOption("onsetPos",&enab_output[FUNCTIONAL_ONSET][FUNCT_ONSETPOS],1,SSI_INT,"1/0=enable/disable output of relative position of first onset found [output name: onsetPos]");
			addOption("offsetPos",&enab_output[FUNCTIONAL_ONSET][FUNCT_OFFSETPOS],1,SSI_INT,"1/0=enable/disable output of position of last offset found [output name: offsetPos]");
			addOption("numOnsets",&enab_output[FUNCTIONAL_ONSET][FUNCT_NUMONSETS],1,SSI_INT,"1/0=enable/disable output of the number of onsets found [output name: numOnsets]");
			addOption("numOffsets",&enab_output[FUNCTIONAL_ONSET][FUNCT_NUMOFFSETS],1,SSI_INT,"1/0=enable/disable output of the number of offsets found (this is usually redundant and the same as numOnsets, use this only for special applications where it may make sense to use it) [output name: numOffsets]");
			
			//Options for Functional Peaks
			addOption("Peaks",&enab_funct[FUNCTIONAL_PEAKS],1,SSI_INT,"1/0=enable/disable outputs of functional Peaks");

			addOption("numPeaks",&enab_output[FUNCTIONAL_PEAKS][FUNCT_NUMPEAKS],1,SSI_INT,"1/0=enable/disable output of number of peaks [output name: numPeaks]");
			addOption("meanPeakDist",&enab_output[FUNCTIONAL_PEAKS][FUNCT_MEANPEAKDIST],1,SSI_INT,"1/0=enable/disable output of mean distance between peaks [output name: meanPeakDist]");
			addOption("peakMean",&enab_output[FUNCTIONAL_PEAKS][FUNCT_PEAKMEAN],1,SSI_INT,"1/0=enable/disable output of arithmetic mean of peaks [output name: peakMean]");
			addOption("peakMeanMeanDist",&enab_output[FUNCTIONAL_PEAKS][FUNCT_PEAKMEANMEANDIST],1,SSI_INT,"1/0=enable/disable output of aritmetic mean of peaks - aritmetic mean of all values [output name: peakMeanMeanDist]");
			addOption("peakDistStddev",&enab_output[FUNCTIONAL_PEAKS][FUNCT_PEAKDISTSTDDEV],1,SSI_INT,"1/0=enable/disable output of standard deviation of inter peak distances [output name: peakDistStddev]");
    		addOption("overlapFlag",&overlapFlag,1,SSI_INT,"1/0=yes/no frames overlap (i.e. compute peaks locally only)");

			//Options for Functional Percentiles
			addOption("Percentiles",&enab_funct[FUNCTIONAL_PERCENTILES],1,SSI_INT,"1/0=enable/disable outputs of functional Percentiles");
			
			addOption("quartiles",&enabQuart,1,SSI_INT,"1/0=enable/disable output of all quartiles (overrides individual settings quartile1, quartile2, and quartile3 if disabled)");
			addOption("quartile1",&enab_output[FUNCTIONAL_PERCENTILES][FUNCT_QUART1],1,SSI_INT,"1/0=enable/disable output of quartile1 (0.25)");
			addOption("quartile2",&enab_output[FUNCTIONAL_PERCENTILES][FUNCT_QUART2],1,SSI_INT,"1/0=enable/disable output of quartile2 (0.50)");
			addOption("quartile3",&enab_output[FUNCTIONAL_PERCENTILES][FUNCT_QUART3],1,SSI_INT,"1/0=enable/disable output of quartile3 (0.75)");
			addOption("iqr",&enabIrq,1,SSI_INT,"1/0=enable/disable output of all inter-quartile ranges (overrides individual settings iqr12, iqr23, and iqr13 if disabled) ");
			addOption("iqr12",&enab_output[FUNCTIONAL_PERCENTILES][FUNCT_IQR12],1,SSI_INT,"1/0=enable/disable output of inter-quartile range 1-2 (quartile2-quartile1)");
			addOption("iqr23",&enab_output[FUNCTIONAL_PERCENTILES][FUNCT_IQR23],1,SSI_INT,"1/0=enable/disable output of inter-quartile range 2-3 (quartile3-quartile2)");
			addOption("iqr13",&enab_output[FUNCTIONAL_PERCENTILES][FUNCT_IQR13],1,SSI_INT,"1/0=enable/disable output of inter-quartile range 1-3 (quartile3-quartile1)");
			addOption("percentile",&percentile,SSI_MAX_CHAR, SSI_CHAR,"Array of p*100 percent percentiles to compute. p = 0..1. (i.e. 0.2,0.8,0.5,....) Array size indicates the number of total percentiles to compute (excluding quartiles), duplicate entries are not checked for and not removed  : percentile[n] = p (p=0..1) if empty then disabled");
			addOption("pctlrange",&pctlrange,SSI_MAX_CHAR, SSI_CHAR,"Array that specifies which inter percentile ranges to compute. A range is specified as (n11,n12,n21,n22,n31,n32,....) (where ni1 and ni2 are the indicies of the percentiles as they appear in the percentile[] array, starting at 0 with the index of the first percentile) if empty then disabled");
			addOption("interp",&interp,1,SSI_INT,"If set to 1, percentile values will be linearly interpolated, instead of being rounded to the nearest index in the sorted array");
			
			//Options for Funcional Regression
			addOption("Regression",&enab_funct[FUNCTIONAL_REGRESSION],1,SSI_INT,"1/0=enable/disable outputs of functional Regression");

			addOption("linregc1",&enab_output[FUNCTIONAL_REGRESSION][FUNCT_LINREGC1],1,SSI_INT,"1/0=enable/disable output of slope m (linear regression line)");
			addOption("linregc2",&enab_output[FUNCTIONAL_REGRESSION][FUNCT_LINREGC2],1,SSI_INT,"1/0=enable/disable output of offset t (linear regression line)");
			addOption("linregerrA",&enab_output[FUNCTIONAL_REGRESSION][FUNCT_LINREGERRA],1,SSI_INT,"1/0=enable/disable output of linear error between contour and linear regression line");
			addOption("linregerrQ",&enab_output[FUNCTIONAL_REGRESSION][FUNCT_LINREGERRQ],1,SSI_INT,"1/0=enable/disable output of quadratic error between contour and linear regression line");
			addOption("qregc1",&enab_output[FUNCTIONAL_REGRESSION][FUNCT_QREGC1],1,SSI_INT,"1/0=enable/disable output of quadratic regression coefficient 1 (a)");
			addOption("qregc2",&enab_output[FUNCTIONAL_REGRESSION][FUNCT_QREGC2],1,SSI_INT,"1/0=enable/disable output of quadratic regression coefficient 2 (b)");
			addOption("qregc3",&enab_output[FUNCTIONAL_REGRESSION][FUNCT_QREGC3],1,SSI_INT,"1/0=enable/disable output of quadratic regression coefficient 3 (c = offset)");
			addOption("qregerrA",&enab_output[FUNCTIONAL_REGRESSION][FUNCT_QREGERRA],1,SSI_INT,"1/0=enable/disable output of linear error between contour and quadratic regression line (parabola)");
			addOption("qregerrQ",&enab_output[FUNCTIONAL_REGRESSION][FUNCT_QREGERRQ],1,SSI_INT,"1/0=enable/disable output of quadratic error between contour and quadratic regression line (parabola)");
			addOption("centroid",&enab_output[FUNCTIONAL_REGRESSION][FUNCT_CENTROID],1,SSI_INT,"1/0=enable/disable output of centroid of contour (this is computed as a by-product of the regression coefficients).");
			addOption("qregls",&enab_output[FUNCTIONAL_REGRESSION][FUNCT_QREGLS],1,SSI_INT,"1/0=enable/disable output of left slope of parabola (slope of the line from first point on the parabola at t=0 to the vertex).");
			addOption("qregrs",&enab_output[FUNCTIONAL_REGRESSION][FUNCT_QREGRS],1,SSI_INT,"1/0=enable/disable output of right slope of parabola (slope of the line from the vertex to the last point on the parabola at t=N).");
			addOption("qregx0",&enab_output[FUNCTIONAL_REGRESSION][FUNCT_QREGX0],1,SSI_INT,"1/0=enable/disable output of x coordinate of the parabola vertex (since for very flat parabolas this can be very large/small, it is clipped to range -Nin - +Nin ).");
			addOption("qregy0",&enab_output[FUNCTIONAL_REGRESSION][FUNCT_QREGY0],1,SSI_INT,"1/0=enable/disable output of y coordinate of the parabola vertex.");
			addOption("qregyr",&enab_output[FUNCTIONAL_REGRESSION][FUNCT_QREGYR],1,SSI_INT,"1/0=enable/disable output of y coordinate of the last point on the parabola (t=N).");
			addOption("qregy0nn",&enab_output[FUNCTIONAL_REGRESSION][FUNCT_QREGY0NN],1,SSI_INT,"1/0=enable/disable output of y coordinate of the parabola vertex. This value is unnormalised, regardless of value of normInput.");
			addOption("qregc3nn",&enab_output[FUNCTIONAL_REGRESSION][FUNCT_QREGC3NN],1,SSI_INT,"1/0=enable/disable output of y coordinate of the first point on the parabola (t=0). This value is unnormalised, regardless of value of normInput.");
			addOption("qregyrnn",&enab_output[FUNCTIONAL_REGRESSION][FUNCT_QREGYRNN],1,SSI_INT,"1/0=enable/disable output of y coordinate of the last point on the parabola (t=N). This value is unnormalised, regardless of value of normInput.");
			addOption("normInputs",&normInputs,1,SSI_INT,"1/0=enable/disable normalisation of regression coefficients, coordinates, and regression errors on the value scale. If enabled all input values will be normalised to the range 0..1. Use this in conjunction with normRegCoeff.");
   			addOption("normRegCoeff",&normRegCoeff,1,SSI_INT,"1/0=enable/disable normalisation of regression coefficients. If enabled, the coefficients are scaled (multiplied by the contour length) so that a regression line or parabola approximating the contour can be plotted over an x-axis range from 0 to 1, i.e. this makes the coefficients independent of the contour length (a longer contour with a lower slope will then have the same 'm' (slope) linear regression coefficient as a shorter but steeper slope).");
			
			//Options for Functional Moments
			addOption("Moments",&enab_funct[FUNCTIONAL_MOMENTS],1,SSI_INT,"1/0=enable/disable outputs of functional Moments");

			addOption("variance",&enab_output[FUNCTIONAL_MOMENTS][FUNCT_VAR],1,SSI_INT,"1/0=enable/disable output of variance");
			addOption("stddev",&enab_output[FUNCTIONAL_MOMENTS][FUNCT_STDDEV],1,SSI_INT,"1/0=enable/disable output of standard deviation");
			addOption("skewness",&enab_output[FUNCTIONAL_MOMENTS][FUNCT_SKEWNESS],1,SSI_INT,"1/0=enable/disable output of skewness");
			addOption("kurtosis",&enab_output[FUNCTIONAL_MOMENTS][FUNCT_KURTOSIS],1,SSI_INT,"1/0=enable/disable output of kurtosis");
			addOption("amean_moments",&enab_output[FUNCTIONAL_MOMENTS][FUNCT_AMEAN_MOM],1,SSI_INT,"1/0=enable/disable output of arithmetic mean");

			//Options for Functional Peaks2
			addOption("Peaks2",&enab_funct[FUNCTIONAL_PEAKS2],1,SSI_INT,"1/0=enable/disable outputs of functional Peaks2");

			addOption("numPeaks2",&enab_output[FUNCTIONAL_PEAKS2][FUNCT_NUMPEAKS],1,SSI_INT,"1/0=enable/disable output of number of peaks");
			addOption("meanPeakDist2",&enab_output[FUNCTIONAL_PEAKS2][FUNCT_MEANPEAKDIST],1,SSI_INT,"1/0=enable/disable output of mean distance between peaks (relative to the input segment length, in seconds, or in frames, see the 'norm' option or the 'masterTimeNorm' option of the cFunctionals parent component)");
			addOption("meanPeakDistDelta",&enab_output[FUNCTIONAL_PEAKS2][FUNCT_MEANPEAKDISTDELTA],1,SSI_INT,"1/0=enable/disable output of mean difference between consecutive inter peak distances (relative to the input segment length, in seconds, or in frames, see the 'norm' option or the 'masterTimeNorm' option of the cFunctionals parent component) [NOT YET IMPLEMENTED!]");
			addOption("peakDistStddev2",&enab_output[FUNCTIONAL_PEAKS2][FUNCT_PEAKDISTSTDDEV2],1,SSI_INT,"1/0=enable/disable output of standard deviation of inter peak distances");
			addOption("peakRangeAbs",&enab_output[FUNCTIONAL_PEAKS2][FUNCT_PEAKRANGEABS],1,SSI_INT,"1/0=enable/disable output of peak range (max peak value - min peak value)");
			addOption("peakRangeRel",&enab_output[FUNCTIONAL_PEAKS2][FUNCT_PEAKRANGEREL],1,SSI_INT,"1/0=enable/disable output of peak range (max peak value - min peak value) / arithmetic mean");
			addOption("peakMeanAbs",&enab_output[FUNCTIONAL_PEAKS2][FUNCT_PEAKMEAN_ABS],1,SSI_INT,"1/0=enable/disable output of arithmetic mean of peaks (local maxima)");
			addOption("peakMeanMeanDist2",&enab_output[FUNCTIONAL_PEAKS2][FUNCT_PEAKMEANMEANDIST2],1,SSI_INT,"1/0=enable/disable output of arithmetic mean of peaks - arithmetic mean of all values");
			addOption("peakMeanRel",&enab_output[FUNCTIONAL_PEAKS2][FUNCT_PEAKMEANMEANRATIO],1,SSI_INT,"1/0=enable/disable output of arithmetic mean of peaks (local maxima) / arithmetic mean of all values");
			addOption("ptpAmpMeanAbs",&enab_output[FUNCTIONAL_PEAKS2][FUNCT_PTPAMPMEANABS],1,SSI_INT,"1/0=enable/disable output of mean peak to peak (amplitude) difference");
			addOption("ptpAmpMeanRel",&enab_output[FUNCTIONAL_PEAKS2][FUNCT_PTPAMPMEANREL],1,SSI_INT,"1/0=enable/disable output of mean peak to peak (amplitude) difference / arithmetic mean");
			addOption("ptpAmpStddevAbs",&enab_output[FUNCTIONAL_PEAKS2][FUNCT_PTPAMPSTDDEVABS],1,SSI_INT,"1/0=enable/disable output of mean peak to peak (amplitude) standard deviation");
			addOption("ptpAmpStddevRel",&enab_output[FUNCTIONAL_PEAKS2][FUNCT_PTPAMPSTDDEVREL],1,SSI_INT,"1/0=enable/disable output of mean peak to peak (amplitude) standard deviation / arithmetic mean");
			addOption("minRangeAbs",&enab_output[FUNCTIONAL_PEAKS2][FUNCT_MINRANGEABS],1,SSI_INT,"1/0=enable/disable output of local minima range (max minmum value - min minimum value)");
			addOption("minRangeRel",&enab_output[FUNCTIONAL_PEAKS2][FUNCT_MINRANGEREL],1,SSI_INT,"1/0=enable/disable output of local minima range (max minmum value - min minimum value) / arithmetic mean");
			addOption("minMeanAbs",&enab_output[FUNCTIONAL_PEAKS2][FUNCT_MINMEAN],1,SSI_INT,"1/0=enable/disable output of arithmetic mean of local minima");
			addOption("minMeanMeanDist",&enab_output[FUNCTIONAL_PEAKS2][FUNCT_MINMEANMEANDIST],1,SSI_INT,"1/0=enable/disable output of arithmetic mean of local minima - arithmetic mean of all values");
			addOption("minMeanRel",&enab_output[FUNCTIONAL_PEAKS2][FUNCT_MINMEANMEANRATIO],1,SSI_INT,"1/0=enable/disable output of arithmetic mean of local minima / arithmetic mean");
			addOption("mtmAmpMeanAbs",&enab_output[FUNCTIONAL_PEAKS2][FUNCT_MTMAMPMEANABS],1,SSI_INT,"1/0=enable/disable output of mean minimum to minimum (amplitude) difference");
			addOption("mtmAmpMeanRel",&enab_output[FUNCTIONAL_PEAKS2][FUNCT_MTMAMPMEANREL],1,SSI_INT,"1/0=enable/disable output of mean minimum to minimum (amplitude) difference / arithmetic mean");
			addOption("mtmAmpStddevAbs",&enab_output[FUNCTIONAL_PEAKS2][FUNCT_MTMAMPSTDDEVABS],1,SSI_INT,"1/0=enable/disable output of mean minimum to minimum (amplitude) standard deviation");
			addOption("mtmAmpStddevRel",&enab_output[FUNCTIONAL_PEAKS2][FUNCT_MTMAMPSTDDEVREL],1,SSI_INT,"1/0=enable/disable output of mean minimum to minimum (amplitude) standard deviation");
			addOption("meanRisingSlope",&enab_output[FUNCTIONAL_PEAKS2][FUNCT_MEANRISINGSLOPE],1,SSI_INT,"1/0=enable/disable output of the mean of the rising slopes (rising slope is the slope of the line connecting a local minimum (or the beginning of input sample) with the following local maximum/peak or the end of input sample)");
			addOption("maxRisingSlope",&enab_output[FUNCTIONAL_PEAKS2][FUNCT_MAXRISINGSLOPE],1,SSI_INT,"1/0=enable/disable output of maximum rising slope");
			addOption("minRisingSlope",&enab_output[FUNCTIONAL_PEAKS2][FUNCT_MINRISINGSLOPE],1,SSI_INT,"1/0=enable/disable output of minimum rising slope");
			addOption("stddevRisingSlope",&enab_output[FUNCTIONAL_PEAKS2][FUNCT_STDDEVRISINGSLOPE],1,SSI_INT,"1/0=enable/disable output of the standard deviation of the rising slopes");
			addOption("meanFallingSlope",&enab_output[FUNCTIONAL_PEAKS2][FUNCT_MEANFALLINGSLOPE],1,SSI_INT,"1/0=enable/disable output of the mean of the falling slopes (falling slope is the slope of the line connecting a local maximum/peak (or the beginning of input sample) with the following local minimum (or the end of input sample))");
			addOption("maxFallingSlope",&enab_output[FUNCTIONAL_PEAKS2][FUNCT_MAXFALLINGSLOPE],1,SSI_INT,"1/0=enable/disable output of maximum falling slope.");
			addOption("minFallingSlope",&enab_output[FUNCTIONAL_PEAKS2][FUNCT_MINFALLINGSLOPE],1,SSI_INT,"1/0=enable/disable output of minimum falling slope");
			addOption("stddevFallingSlope",&enab_output[FUNCTIONAL_PEAKS2][FUNCT_STDDEVFALLINGSLOPE],1,SSI_INT,"1/0=enable/disable output of the standard deviation of the falling slopes");
			
			addOption("useAbsThresh",&useAbsThresh,1,SSI_INT,"1/0 = enable/disable absolute threshold");
			addOption("absThresh",&absThresh,1,SSI_FLOAT,"Gives an absolute threshold for the minimum peak height. Use with caution, use only if you know what you are doing. If this option is not set, relThresh will be used.");
			addOption("relThresh",&relThresh,1,SSI_FLOAT,"Gives the threshold relative to the input contour range, which is used to remove peaks and minimima below this threshold. Valid values: 0..1, a higher value will remove more peaks, while a lower value will keep more and less salient peaks. If not using dynRelThresh=1 you should use a default value of ~0.10 otherwise a default of ~0.35");
			addOption("dynRelThresh",&dynRelThresh,1,SSI_INT,"1/0 = enable disable dynamic relative threshold. Instead of converting the relThresh to an absolute threshold relThresh*range, the threshold is applied as: abs(a/b-1.0) < relThresh , where a is always larger than b.");


			 
		};
			
	};

public:

	static const ssi_char_t *GetCreateName () { return "OSFunctionals"; };
	static IObject *Create (const ssi_char_t *file) { return new OSFunctionals (file); };
	~OSFunctionals ();

	OSFunctionals::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "This component computes functionals."; };

	void transform_enter (ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);
	void transform (ITransformer::info info,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);
	void transform_flush (ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);
	
protected:

	void processOneDimension(ssi_size_t NSamples, ssi_real_t* data_in, ssi_real_t* data_out);
	void transpose(ssi_real_t* data, ssi_size_t rn, ssi_size_t cn);
	int Crossings(Variables vs);
	int Dct(Variables vs);
	int Samples(Variables vs);
	int Segments(Variables vs);
	int Times(Variables vs);
	int Extremes(Variables vs);
	int Means(Variables vs);
	int Onset(Variables vs);
	int Peaks(Variables vs);
	int Percentiles(Variables vs);
	int Regression(Variables vs);
	int Moments(Variables vs);
	int Peaks2(Variables vs);
	void updateNFeatures();
	float getInputPeriod() { return T;}
	long getPctlIdx(double p, long N);
	FLOAT_DMEM getInterpPctl(double p, FLOAT_DMEM *sorted, long N);
	void addPeakDist(int idx, long dist);
	void * crealloc(void *a, size_t size, size_t old_size);
	void addMinMax(int type, FLOAT_DMEM y, long x);
	void removeFromMinMaxList( struct peakMinMaxListEl * listEl );
	int isBelowThresh(FLOAT_DMEM diff, FLOAT_DMEM base);
	 
	 
public:

	ssi_size_t updateNdim(ssi_size_t sample_dimension_in) {

		dim_in = new bool [sample_dimension_in];
		
		ssi_size_t size_dim = countColons(_options.enabled_dimensions);
		int* dim_enab = ParseIntSamples(_options.enabled_dimensions,-1);
				 

		for(ssi_size_t i=0; i<sample_dimension_in; i++) {
			
			if(dim_enab)
				dim_in[i] = false;
			else
				dim_in[i] = true;
					
		}

		for(ssi_size_t i=0; i<size_dim; i++) {
			
			dim_in[dim_enab[i]] = true;
			
			if(_options.enabled_deltas)
				dim_in[dim_enab[i] + (sample_dimension_in/2)] = true;
			

		}

		if(_options.enabled_deltas)
			size_dim *= 2;

		if(!size_dim)
			return sample_dimension_in;
		
		else
			return size_dim;


	}

	ssi_size_t getSampleDimensionOut (ssi_size_t sample_dimension_in) {

		sample_dimension_in = updateNdim(sample_dimension_in);
		updateNFeatures();

		return sample_dimension_in * NFeatures;

	}

	ssi_size_t getSampleBytesOut (ssi_size_t sample_bytes_in) {
		return sample_bytes_in;
		
	}

	ssi_type_t getSampleTypeOut (ssi_type_t sample_type_in) {
		if (sample_type_in != SSI_REAL) {
			ssi_err ("type %s not supported", SSI_TYPE_NAMES[sample_type_in]);
		}
		return SSI_REAL;
	}

protected:

	OSFunctionals (const ssi_char_t *file = 0);
	OSFunctionals::Options _options;
	ssi_char_t *_file;
	ssi_size_t NFeatures;
	ssi_real_t T;
        bool Parse (const ssi_char_t *indices, ssi_size_t* values);
        FLOAT_DMEM* ParseFloatSamples(const ssi_char_t *indices);
        int* ParseIntSamples(const ssi_char_t *indices, int choice);
        int countColons(const ssi_char_t *indices);
	int counter;
	bool* dim_in;

};

}

#endif
