// OSFunctionals.cpp
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

#include "OSFunctionals.h"


#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

OSFunctionals::OSFunctionals (const ssi_char_t *file)
	: _file (0) {
	
		_options.intializeOptions();
		for(int i=0 ; i<FUNCT_ENAB_N ; i++) functN[i] = 0;
		NFeatures = 0;
		reset = 0;
		counter = 0;

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
}

OSFunctionals::~OSFunctionals () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

void OSFunctionals::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

		T = ssi_cast (ssi_real_t, 1.0/stream_in.sr);
				
}

void OSFunctionals::addPeakDist(int idx, long dist)
{
  if (_options.peakdists == NULL) {
	_options.peakdists = (long*)calloc(1,sizeof(long)*(idx+PEAKDIST_BLOCKALLOC));
    _options.nPeakdists = idx+PEAKDIST_BLOCKALLOC;
	
  } else if (idx >= _options.nPeakdists) {
    _options.peakdists = (long*)crealloc(_options.peakdists,sizeof(long)*(idx+PEAKDIST_BLOCKALLOC),sizeof(long)*_options.nPeakdists);
    _options.nPeakdists = idx+PEAKDIST_BLOCKALLOC;
  }
  _options.peakdists[idx] = dist;
}

void * OSFunctionals::crealloc(void *a, size_t size, size_t old_size)
{
  a = realloc(a,size);
  if ((old_size < size)&&(a!=NULL)) {
    char *b = (char *)a + old_size;
    //fill with zeros:
    memset((void*)b, 0, size-old_size);
  }
  return a;
}

// convert percentile to absolute index
long OSFunctionals::getPctlIdx(double p, long N)
{
  long ret = (long)floor((p*(double)(N-1)) + 0.5);
  if (ret<0) return 0;
  if (ret>=N) return N-1;
  return ret;
}

// get linearly interpolated percentile
FLOAT_DMEM OSFunctionals::getInterpPctl(double p, FLOAT_DMEM *sorted, long N)
{
  double idx = p*(double)(N-1);
  long i1,i2;
  i1=(long)floor(idx);
  i2=(long)ceil(idx);
  if (i1<0) i1=0;
  if (i2<0) i2=0;
  if (i1>=N) i1=N-1;
  if (i2>=N) i2=N-1;
  if (i1!=i2) {
    double w1,w2;
    w1 = idx-(double)i1;
    w2 = (double)i2 - idx;
    return sorted[i1]*(FLOAT_DMEM)w2 + sorted[i2]*(FLOAT_DMEM)w1;
  } else {
    return sorted[i1];
  }
}

void OSFunctionals::addMinMax(int type, FLOAT_DMEM y, long x)
{
  struct peakMinMaxListEl * listEl = (struct peakMinMaxListEl*)malloc(sizeof(struct peakMinMaxListEl));
  listEl->type = type;
  listEl->x = x;
  listEl->y = y;
  listEl->next = NULL;
  listEl->prev = NULL;

  if (_options.mmlistFirst == NULL) {
    _options.mmlistFirst = listEl;
    _options.mmlistLast = listEl;
  } else {
    _options.mmlistLast->next = listEl;
    listEl->prev = _options.mmlistLast;
    _options.mmlistLast = listEl;
  }
}

void OSFunctionals::removeFromMinMaxList( struct peakMinMaxListEl * listEl )
{
  if (listEl->prev != NULL) {
    listEl->prev->next = listEl->next;
    if (listEl-> next != NULL) {
      listEl->next->prev = listEl->prev;
    } else {
      _options.mmlistLast = listEl->prev;
    }
  } else {
    _options.mmlistFirst = listEl->next;
    if (listEl-> next != NULL) {
      listEl->next->prev = NULL;
    } else {
      _options.mmlistLast = NULL;
    }
  }
  //  NOTE: the caller is responsible for freeing the listEl pointer after calling this function
}

int OSFunctionals::isBelowThresh(FLOAT_DMEM diff, FLOAT_DMEM base)
{
  if (_options.dynRelThresh) {

    if (base == 0.0) {
      if (diff != 0.0) return 1;
      else return 0;
    }
    if (fabs(diff/base) < _options.relThresh) {
      return 1;
    }
    return 0;

  } else {

    if (diff < _options.absThresh) {
      return 1;
    }
    return 0;

  }
}

void OSFunctionals::updateNFeatures() {

	//if(reset) return;

	NFeatures = 0;
	
	//gather number of outputs required for each enabled functional

		for(int i=0 ; i<FUNCT_ENAB_N ; i++) {

			functN[i] = 0;

			if(!_options.enab_funct[i]) {
				
				continue;
			}

			switch(i){
					
					case FUNCTIONAL_DCT: functN[i] = _options.enab_output[i][FUNCT_LC]-_options.enab_output[i][FUNCT_FC]+1;break;
					case FUNCTIONAL_SAMPLES: functN[i] = countColons(_options.samplepos);break;
					case FUNCTIONAL_TIMES: _options.enab_output[FUNCTIONAL_TIMES][FUNCT_UPLEVELTIME] = countColons(_options.upleveltime);
						_options.enab_output[FUNCTIONAL_TIMES][FUNCT_DOWNLEVELTIME] = countColons(_options.downleveltime);
						break;
					case FUNCTIONAL_PERCENTILES: _options.enab_output[FUNCTIONAL_PERCENTILES][FUNCT_PERCENTILE] = countColons(_options.percentile);
						_options.enab_output[FUNCTIONAL_PERCENTILES][FUNCT_PCTLRANGE] = countColons(_options.pctlrange)/2;
						if(!_options.enabQuart) _options.enab_output[FUNCTIONAL_PERCENTILES][FUNCT_QUART1] 
						= _options.enab_output[FUNCTIONAL_PERCENTILES][FUNCT_QUART2]
						= _options.enab_output[FUNCTIONAL_PERCENTILES][FUNCT_QUART3] = 0;
						if(!_options.enabIrq) _options.enab_output[FUNCTIONAL_PERCENTILES][FUNCT_IQR12] 
						= _options.enab_output[FUNCTIONAL_PERCENTILES][FUNCT_IQR23]
						= _options.enab_output[FUNCTIONAL_PERCENTILES][FUNCT_IQR13] = 0;
						break;


			}
				
			for(int k=0; k<MAX_FEATURES_N; k++)
					if(!(i == FUNCTIONAL_DCT || i== FUNCTIONAL_SAMPLES)) functN[i] += _options.enab_output[i][k];
								
				NFeatures += functN[i];

				reset = 1;
			
		}
}

int OSFunctionals::Crossings(Variables vs) {
	
	FLOAT_DMEM *in = vs.in; 
	FLOAT_DMEM *inSorted = vs.inSorted; 
	FLOAT_DMEM *out = vs.out; 
	long Nin = vs.Nin; 
	
	int i;
	if ((Nin>0)&&(out!=NULL)) {
    double amean;
    long zcr=0, mcr=0;

    if (_options.enab_output[FUNCTIONAL_CROSSINGS][FUNCT_MCR]||_options.enab_output[FUNCTIONAL_CROSSINGS][FUNCT_AMEAN]) {
      amean = (double)*in;
      for (i=1; i<Nin; i++) {
        amean += in[i];
      }
      amean /= (double)Nin;
    }
    
    for (i=1; i<Nin-1; i++) {
      in++;
      if (  ( (*(in-1) * *(in+1) <= 0.0) && (*(in)==0.0) ) || (*(in-1) * *(in) < 0.0)  ) zcr++;
	  if (_options.enab_output[FUNCTIONAL_CROSSINGS][FUNCT_MCR])
        if (  ( ((*(in-1)-amean) * (*(in+1)-amean) <= 0.0) && ((*(in)-amean)==0.0) ) || ((*(in-1)-amean) * (*(in)-amean) < 0.0)  ) mcr++;
    }
    
    int n=0;
    if (_options.enab_output[FUNCTIONAL_CROSSINGS][FUNCT_ZCR]) out[n++]=(FLOAT_DMEM) ( (double)zcr / (double)Nin );
    if (_options.enab_output[FUNCTIONAL_CROSSINGS][FUNCT_MCR]) out[n++]=(FLOAT_DMEM) ( (double)mcr / (double)Nin );
    if (_options.enab_output[FUNCTIONAL_CROSSINGS][FUNCT_AMEAN]) out[n++]=(FLOAT_DMEM)amean;
    return n;
  }
  
	return 0;

}
int OSFunctionals::Dct(Variables vs) {
	
	int i,m,N;
	FLOAT_DMEM * costable;
	FLOAT_DMEM factor;
	int nCo = functN[FUNCTIONAL_DCT];
	int firstCoeff = _options.enab_output[FUNCTIONAL_DCT][FUNCT_FC];
	int lastCoeff = _options.enab_output[FUNCTIONAL_DCT][FUNCT_LC];

	FLOAT_DMEM *in = vs.in; 
	FLOAT_DMEM *out = vs.out; 
	long Nin = vs.Nin; 
	long Nout = nCo;

	  
	if ((Nin>0)&&(out!=NULL)) {
		
		if ((Nin>0)&&(Nout>0)) {
		
		costable = (FLOAT_DMEM *) malloc(sizeof(FLOAT_DMEM)*Nin*Nout);
		
		if (costable==NULL) ssi_err("Error initializing costable, probably Nin or Nout == 0 in OSFunctionals::Dct or OUT of Memory");
		
		N=Nin;
		
		for (i=firstCoeff; i<=lastCoeff; i++) {
		  for (m=0; m<Nin; m++) {
			costable[m + (i-firstCoeff)*Nin] = (FLOAT_DMEM)cos(M_PI*(double)i/(double)(N) * ((FLOAT_DMEM)(m) + 0.5) );
		  }
		}
		
		factor = (FLOAT_DMEM)sqrt((double)2.0/(double)(N));
	 	  
		}
		
		for (i=0; i < nCo; i++) {
		  out[i] = 0.0;
		  for (m=0; m<Nin; m++) {
			out[i] +=  in[m] * costable[m+i*N];
		  }
		  out[i] *= factor; 
		}

		if(costable != NULL)
			
			free(costable);

		return nCo;
	}

	

	return 0;

}
int OSFunctionals::Samples(Variables vs) {

	int nSamples = functN[FUNCTIONAL_SAMPLES];
	FLOAT_DMEM* samplepos = ParseFloatSamples(_options.samplepos);

	long Nin = vs.Nin;
	FLOAT_DMEM* in = vs.in;
	FLOAT_DMEM* out = vs.out;
		
	if ((Nin>0)&&(out!=NULL)) {
		FLOAT_DMEM Nind = (FLOAT_DMEM)Nin;
		for (int spi = 0; spi < nSamples; ++spi) {
		  
		if(samplepos[spi] < 0.0) samplepos[spi] = 0.0;
		  else if(samplepos[spi] > 1.0) samplepos[spi] = 1.0;
		  
		  int si = (int)((Nind - 1.0) * samplepos[spi]);
		  out[spi] = in[si];
		}
		return nSamples;
	  }
	  
		return 0;
	  

}
int OSFunctionals::Segments(Variables vs) {
	
	long Nin = vs.Nin;
	FLOAT_DMEM* out = vs.out;
	FLOAT_DMEM* in = vs.in;

	int i;
	if ((Nin>0)&&(out!=NULL)) {
    int nSegments = 0;

	FLOAT_DMEM max = vs.max;
	FLOAT_DMEM min = vs.min;
    FLOAT_DMEM range = max-min;
	long maxNumSeg = _options.maxSegments;

	long Nin = vs.Nin;
	FLOAT_DMEM* in = vs.in;
	FLOAT_DMEM* out = vs.out;

	FLOAT_DMEM segThresh = range * _options.rangeRelThreshold;

    long segMinLng = Nin/maxNumSeg-1;
    if (segMinLng < 2) segMinLng = 2;
    long ravgLng = Nin/(maxNumSeg/2);
    long lastSeg = -segMinLng/2;

    long meanSegLen = 0;
    long maxSegLen = 0;
    long minSegLen = 0;

    FLOAT_DMEM ravg = 0.0;
    for (i=0; i<Nin; i++) {
      ravg += in[i];
      if (i>=ravgLng) ravg -= in[i-ravgLng];

      FLOAT_DMEM ravgLngCur = (FLOAT_DMEM)( MIN(i,ravgLng) );
      FLOAT_DMEM ra = ravg / ravgLngCur;

      if ((in[i]-ra > segThresh)&&(i - lastSeg > segMinLng) )
      { // found new segment begin
        nSegments++;
        long segLen = i-lastSeg;
        meanSegLen += segLen;
        if (segLen > maxSegLen) maxSegLen = segLen;
        if ((minSegLen==0)||(segLen<minSegLen)) minSegLen = segLen;
        lastSeg = i;
      }

    }

	int n=0;
	
	if (_options.enab_output[FUNCTIONAL_SEGMENTS][FUNCT_NUMSEGMENTS]) out[n++]=(FLOAT_DMEM)nSegments/(FLOAT_DMEM)(maxNumSeg);
    
	if (_options.masterTimeNorm==TIMENORM_SEGMENT) {
      if (_options.enab_output[FUNCTIONAL_SEGMENTS][FUNCT_SEGMEANLEN]) {
        if (nSegments > 1) 
          out[n++]=(FLOAT_DMEM)meanSegLen/((FLOAT_DMEM)nSegments*(FLOAT_DMEM)(Nin));
        else 
          out[n++]=(FLOAT_DMEM)meanSegLen/((FLOAT_DMEM)(Nin));
      }
      if (_options.enab_output[FUNCTIONAL_SEGMENTS][FUNCT_SEGMAXLEN]) out[n++]=(FLOAT_DMEM)maxSegLen/(FLOAT_DMEM)(Nin);
      if (_options.enab_output[FUNCTIONAL_SEGMENTS][FUNCT_SEGMINLEN]) out[n++]=(FLOAT_DMEM)minSegLen/(FLOAT_DMEM)(Nin);
	} else if (_options.masterTimeNorm == TIMENORM_FRAMES) {
      if (_options.enab_output[FUNCTIONAL_SEGMENTS][FUNCT_SEGMEANLEN]) {
        if (nSegments > 1) 
          out[n++]=(FLOAT_DMEM)meanSegLen/((FLOAT_DMEM)nSegments);
        else 
          out[n++]=(FLOAT_DMEM)meanSegLen;
      }
      if (_options.enab_output[FUNCTIONAL_SEGMENTS][FUNCT_SEGMAXLEN]) out[n++]=(FLOAT_DMEM)maxSegLen;
      if (_options.enab_output[FUNCTIONAL_SEGMENTS][FUNCT_SEGMINLEN]) out[n++]=(FLOAT_DMEM)minSegLen;
	} else if (_options.masterTimeNorm == TIMENORM_SECONDS) {
      FLOAT_DMEM _T = (FLOAT_DMEM)getInputPeriod();
                
      FLOAT_DMEM Norm = 1.0;
      if (_T != 0.0) { Norm = _T; }

      if (_options.enab_output[FUNCTIONAL_SEGMENTS][FUNCT_SEGMEANLEN]) {
        if (nSegments > 1) 
          out[n++]=(FLOAT_DMEM)meanSegLen*Norm/((FLOAT_DMEM)nSegments);
        else 
          out[n++]=(FLOAT_DMEM)meanSegLen*Norm;
      }
      if (_options.enab_output[FUNCTIONAL_SEGMENTS][FUNCT_SEGMAXLEN]) out[n++]=(FLOAT_DMEM)maxSegLen*Norm;
      if (_options.enab_output[FUNCTIONAL_SEGMENTS][FUNCT_SEGMINLEN]) out[n++]=(FLOAT_DMEM)minSegLen*Norm;
    }

    return n;
  }
  return 0;

}
int OSFunctionals::Times(Variables vs) {

	long Nin = vs.Nin;
	FLOAT_DMEM* out = vs.out;
	FLOAT_DMEM* in = vs.in;
	FLOAT_DMEM max = vs.max;
	FLOAT_DMEM min = vs.min;

	if ((Nin>0)&&(out!=NULL)) {
    int n=0;

    FLOAT_DMEM *i0 = in;
    FLOAT_DMEM *iE = in+Nin;
    FLOAT_DMEM Nind = (FLOAT_DMEM)Nin;
	int buggySecNorm = 0;

    FLOAT_DMEM Norm, Norm1, Norm2;
    Norm=Nind; Norm1=Nind-(FLOAT_DMEM)1.0; Norm2=Nind-(FLOAT_DMEM)2.0;

    FLOAT_DMEM _T = 1.0; 
	if (_options.masterTimeNorm==TIMENORM_SECOND) {
      _T = (FLOAT_DMEM)getInputPeriod();
                           
      if (_T != 0.0) {
        if (buggySecNorm) {//OLD mode!!: 
          Norm /= _T; Norm1 /= _T; Norm2 /= _T;
        } else {
          Norm = (FLOAT_DMEM)(1.0)/_T;
          Norm1 /= Nind*_T;
          Norm2 /= Nind*_T;
        }
      }
    }
	if (_options.masterTimeNorm==TIMENORM_FRAME) {
      Norm = 1.0; Norm1 /= Nind; Norm2 /= Nind;
    }

    
    FLOAT_DMEM range = max-min;
    FLOAT_DMEM l25, l50, l75, l90;
    l25 = (FLOAT_DMEM)0.25*range+min;
    l50 = (FLOAT_DMEM)0.50*range+min;
    l75 = (FLOAT_DMEM)0.75*range+min;
    l90 = (FLOAT_DMEM)0.90*range+min;
    long n25=0, n50=0, n75=0, n90=0;
    long nR=0, nF=0, nLC=0, nRC=0;
    
    // first pass: predefined ul/dl times AND rise/fall, etc.
	if ((_options.enab_output[FUNCTIONAL_TIMES][FUNCT_UPLEVELTIME25])||(_options.enab_output[FUNCTIONAL_TIMES][FUNCT_DOWNLEVELTIME25]) ||
        (_options.enab_output[FUNCTIONAL_TIMES][FUNCT_UPLEVELTIME50])||(_options.enab_output[FUNCTIONAL_TIMES][FUNCT_DOWNLEVELTIME50]) ||
        (_options.enab_output[FUNCTIONAL_TIMES][FUNCT_UPLEVELTIME75])||(_options.enab_output[FUNCTIONAL_TIMES][FUNCT_DOWNLEVELTIME75]) ||
        (_options.enab_output[FUNCTIONAL_TIMES][FUNCT_UPLEVELTIME90])||(_options.enab_output[FUNCTIONAL_TIMES][FUNCT_DOWNLEVELTIME90])) {
      while (in<iE) {
        if (*in <= l25) n25++;
        if (*in <= l50) n50++;
        if (*in <= l75) n75++;
        if (*(in++) <= l90) n90++;
      }
      in = i0;
    }
    if ((_options.enab_output[FUNCTIONAL_TIMES][FUNCT_RISETIME])||(_options.enab_output[FUNCTIONAL_TIMES][FUNCT_FALLTIME])) {
      while (++in<iE) {
        if (*(in-1) < *in) nR++;      // rise
        else if (*(in-1) > *in) nF++; // fall
      }
      in = i0;
    }
    if ((_options.enab_output[FUNCTIONAL_TIMES][FUNCT_LEFTCTIME])||(_options.enab_output[FUNCTIONAL_TIMES][FUNCT_RIGHTCTIME])) {
      FLOAT_DMEM a1,a2;
      while (++in<iE-1) {
        a1 = *(in)-*(in-1);
        a2 = *(in+1)-*(in);
        if ( a2 < a1 ) nRC++;      // right curve
        else if ( a1 < a2) nLC++;  // left curve
      }
      in = i0;
    }

    if (_options.enab_output[FUNCTIONAL_TIMES][FUNCT_UPLEVELTIME25]) out[n++]=((FLOAT_DMEM)(Nin-n25))/Norm;
    if (_options.enab_output[FUNCTIONAL_TIMES][FUNCT_DOWNLEVELTIME25]) out[n++]=((FLOAT_DMEM)(n25))/Norm;
    if (_options.enab_output[FUNCTIONAL_TIMES][FUNCT_UPLEVELTIME50]) out[n++]=((FLOAT_DMEM)(Nin-n50))/Norm;
    if (_options.enab_output[FUNCTIONAL_TIMES][FUNCT_DOWNLEVELTIME50]) out[n++]=((FLOAT_DMEM)(n50))/Norm;
    if (_options.enab_output[FUNCTIONAL_TIMES][FUNCT_UPLEVELTIME75]) out[n++]=((FLOAT_DMEM)(Nin-n75))/Norm;
    if (_options.enab_output[FUNCTIONAL_TIMES][FUNCT_DOWNLEVELTIME75]) out[n++]=((FLOAT_DMEM)(n75))/Norm;
    if (_options.enab_output[FUNCTIONAL_TIMES][FUNCT_UPLEVELTIME90]) out[n++]=((FLOAT_DMEM)(Nin-n90))/Norm;
    if (_options.enab_output[FUNCTIONAL_TIMES][FUNCT_DOWNLEVELTIME90]) out[n++]=((FLOAT_DMEM)(n90))/Norm;

    if (Norm1 != 0.0) {
      if (_options.enab_output[FUNCTIONAL_TIMES][FUNCT_RISETIME]) out[n++]=((FLOAT_DMEM)nR)/Norm1;
      if (_options.enab_output[FUNCTIONAL_TIMES][FUNCT_FALLTIME]) out[n++]=((FLOAT_DMEM)nF)/Norm1;
    } else {
      if (_options.enab_output[FUNCTIONAL_TIMES][FUNCT_RISETIME]) out[n++]=0.0;
      if (_options.enab_output[FUNCTIONAL_TIMES][FUNCT_FALLTIME]) out[n++]=0.0;
    }
    if (Norm2 != 0.0) {
      if (_options.enab_output[FUNCTIONAL_TIMES][FUNCT_LEFTCTIME]) out[n++]=((FLOAT_DMEM)nLC)/Norm2;
      if (_options.enab_output[FUNCTIONAL_TIMES][FUNCT_RIGHTCTIME]) out[n++]=((FLOAT_DMEM)nRC)/Norm2;
    } else {
      if (_options.enab_output[FUNCTIONAL_TIMES][FUNCT_LEFTCTIME]) out[n++]=0.0;
      if (_options.enab_output[FUNCTIONAL_TIMES][FUNCT_RIGHTCTIME]) out[n++]=0.0;
    }

    if (_options.enab_output[FUNCTIONAL_TIMES][FUNCT_DURATION]) {
		if (_options.masterTimeNorm==TIMENORM_SECONDS) {
        out[n++]=((FLOAT_DMEM)(Nin)*_T);
      } else {
        out[n++]=((FLOAT_DMEM)(Nin));
      }
    }

    // second pass, user defined times
	FLOAT_DMEM* ultime = ParseFloatSamples(_options.upleveltime);
	FLOAT_DMEM* dltime = ParseFloatSamples(_options.downleveltime);

	int nUltime = _options.enab_output[FUNCTIONAL_TIMES][FUNCT_UPLEVELTIME];
	int nDltime = _options.enab_output[FUNCTIONAL_TIMES][FUNCT_DOWNLEVELTIME];


    int j;
    if (_options.enab_output[FUNCTIONAL_TIMES][FUNCT_UPLEVELTIME]) {
      for (j=0; j<nUltime; j++) {
        FLOAT_DMEM lX = (FLOAT_DMEM)(ultime[j]*range+min);
        long nX=0;
        while (in<iE) if (*(in++) > lX) nX++;
        in = i0;
        out[n++] = ((FLOAT_DMEM)(nX))/Norm;
      }
    }
    if (_options.enab_output[FUNCTIONAL_TIMES][FUNCT_DOWNLEVELTIME]) {
      for (j=0; j<nDltime; j++) {
        FLOAT_DMEM lX = (FLOAT_DMEM)(dltime[j]*range+min);
        long nX=0;
        while (in<iE) if (*(in++) <= lX) nX++;
        in = i0;
        out[n++] = ((FLOAT_DMEM)(nX))/Norm;
      }
    }

	free(dltime);
	free(ultime);
    
    return n;
  }
  return 0;

}
int OSFunctionals::Extremes(Variables vs) {

	FLOAT_DMEM* in = vs.in;
	FLOAT_DMEM* out = vs.out;
	FLOAT_DMEM max = vs.max;
	FLOAT_DMEM min = vs.min;
	FLOAT_DMEM mean = vs.mean;
	long Nin = vs.Nin;


	int i;
  if ((Nin>0)&&(out!=NULL)) {
    long minpos=-1, maxpos=-1;
    
    for (i=0; i<Nin; i++) {
      if ((*in == max)&&(maxpos==-1)) { maxpos=i; }
      if ((*in == min)&&(minpos==-1)) { minpos=i; }
      in++;
    }

    FLOAT_DMEM maxposD = (FLOAT_DMEM)maxpos;
    FLOAT_DMEM minposD = (FLOAT_DMEM)minpos;

    // normalise max/min pos ...
	if (_options.masterTimeNorm==TIMENORM_SEGMENT) {
        maxposD /= (FLOAT_DMEM)(Nin);
        minposD /= (FLOAT_DMEM)(Nin);
	} else if (_options.masterTimeNorm==TIMENORM_SECOND) {
      FLOAT_DMEM _T = (FLOAT_DMEM)getInputPeriod();
                           
      if (_T != 0.0) {
        maxposD *= _T;
        minposD *= _T;
      }
    } // default is TIMENORM_FRAME...

    int n=0;
    if (_options.enab_output[FUNCTIONAL_EXTREMES][FUNCT_MAX]) out[n++]=max;
    if (_options.enab_output[FUNCTIONAL_EXTREMES][FUNCT_MIN]) out[n++]=min;
    if (_options.enab_output[FUNCTIONAL_EXTREMES][FUNCT_RANGE]) out[n++]=max-min;
    if (_options.enab_output[FUNCTIONAL_EXTREMES][FUNCT_MAXPOS]) out[n++]=maxposD;
    if (_options.enab_output[FUNCTIONAL_EXTREMES][FUNCT_MINPOS]) out[n++]=minposD;
    if (_options.enab_output[FUNCTIONAL_EXTREMES][FUNCT_MAXAMEANDIST]) out[n++]=max-mean;
    if (_options.enab_output[FUNCTIONAL_EXTREMES][FUNCT_MINAMEANDIST]) out[n++]=mean-min;
    return n;
  }
  return 0;

}

int OSFunctionals::Means(Variables vs) {

	FLOAT_DMEM* in = vs.in;
	FLOAT_DMEM* out = vs.out;
	FLOAT_DMEM max = vs.max;
	FLOAT_DMEM min = vs.min;
	FLOAT_DMEM mean = vs.mean;
	long Nin = vs.Nin;

	int i;
  if ((Nin>0)&&(out!=NULL)) {

    double tmp=(double)*in;
    double fa = fabs(tmp);

    double absmean = fa;
    double qmean = tmp*tmp;
    long nnz;

    double nzamean;
    double nzabsmean;
    double nzqmean;
    double nzgmean;
    double posamean=0.0, negamean=0.0;
    double posqmean=0.0, negqmean=0.0;
    long nPos=0,nNeg=0;

    if (tmp!=0.0) {
      nzamean = tmp;
      nzabsmean = fa;
      nzqmean = tmp*tmp;
      nzgmean = log(fa);
      nnz=1;
      if (tmp > 0) {
        posamean += tmp;
        posqmean += tmp*tmp;
        nPos++;
      } else {
        negamean += tmp;
        negqmean += tmp*tmp;
        nNeg++;
      }
    } else {
      nzamean = 0.0;
      nzabsmean = 0.0;
      nzqmean = 0.0;
      nzgmean = 0.0;
      nnz=0;
    }
    for (i=1; i<Nin; i++) {
      in++;
      tmp=(double)*in;
      fa = fabs(tmp);
      //      amean += tmp;
      absmean += fa;
      if (tmp > 0) {
        posamean += tmp;
        nPos++;
      }
      if (tmp < 0) {
        negamean += tmp;
        nNeg++;
      }
      double _tmp = tmp;
      if (tmp!=0.0) {
        nzamean += tmp;
        nzabsmean += fa;
        nzgmean += log(fa);
        tmp *= tmp;
        nzqmean += tmp;
        nnz++;
        if (_tmp > 0) posqmean += tmp;
        if (_tmp < 0) negqmean += tmp;
        qmean += tmp;
      }
    }
    tmp = (double)Nin;
    //    amean = amean / tmp;
    absmean = absmean / tmp;
    qmean = qmean / tmp;

    if (nnz>0) {
      tmp = (double)nnz;
      nzamean = nzamean / tmp;
      nzabsmean = nzabsmean / tmp;
      nzqmean = nzqmean / tmp;
      nzgmean /= tmp; //pow( 1.0/nzgmean, 1.0/tmp );
      nzgmean = exp(nzgmean);
    }
    if (nPos > 0) {
      posamean /= (double)nPos;
      posqmean /= (double)nPos;
    }
    if (nNeg > 0) {
      negamean /= (double)nNeg;
      negqmean /= (double)nNeg;
    }

    int n=0;
	if (_options.enab_output[FUNCTIONAL_MEANS][FUNCT_AMEAN]) out[n++]=(FLOAT_DMEM)mean;
    if (_options.enab_output[FUNCTIONAL_MEANS][FUNCT_ABSMEAN]) out[n++]=(FLOAT_DMEM)absmean;
    if (_options.enab_output[FUNCTIONAL_MEANS][FUNCT_QMEAN]) out[n++]=(FLOAT_DMEM)qmean;
    if (_options.enab_output[FUNCTIONAL_MEANS][FUNCT_NZAMEAN]) out[n++]=(FLOAT_DMEM)nzamean;
    if (_options.enab_output[FUNCTIONAL_MEANS][FUNCT_NZABSMEAN]) out[n++]=(FLOAT_DMEM)nzabsmean;
    if (_options.enab_output[FUNCTIONAL_MEANS][FUNCT_NZQMEAN]) out[n++]=(FLOAT_DMEM)nzqmean;
    if (_options.enab_output[FUNCTIONAL_MEANS][FUNCT_NZGMEAN]) out[n++]=(FLOAT_DMEM)nzgmean;
	if (_options.masterTimeNorm==TIMENORM_FRAMES) {
      if (_options.enab_output[FUNCTIONAL_MEANS][FUNCT_NNZ]) out[n++]=(FLOAT_DMEM)nnz;
    } else if (_options.masterTimeNorm==TIMENORM_SEGMENT) {
      if (_options.enab_output[FUNCTIONAL_MEANS][FUNCT_NNZ]) out[n++]=(FLOAT_DMEM)nnz/(FLOAT_DMEM)Nin;
    } else if (_options.masterTimeNorm==TIMENORM_SECONDS) {
      if (_options.enab_output[FUNCTIONAL_MEANS][FUNCT_NNZ]) out[n++]=(FLOAT_DMEM)nnz/(FLOAT_DMEM)getInputPeriod();
    }
    if (_options.enab_output[FUNCTIONAL_MEANS][FUNCT_FLATNESS]) {
      if (absmean != 0.0)
        out[n++] = (FLOAT_DMEM)(nzgmean/absmean);
      else out[n++] = 1.0;
    }
    
    if (_options.enab_output[FUNCTIONAL_MEANS][FUNCT_POSAMEAN]) {
      out[n++] = (FLOAT_DMEM)posamean;
    }
    if (_options.enab_output[FUNCTIONAL_MEANS][FUNCT_NEGAMEAN]) {
      out[n++] = (FLOAT_DMEM)negamean;
    }
    if (_options.enab_output[FUNCTIONAL_MEANS][FUNCT_POSQMEAN]) {
      out[n++] = (FLOAT_DMEM)posqmean;
    }
    if (_options.enab_output[FUNCTIONAL_MEANS][FUNCT_POSRQMEAN]) {
      out[n++] = (FLOAT_DMEM)sqrt(posqmean);
    }
    if (_options.enab_output[FUNCTIONAL_MEANS][FUNCT_NEGQMEAN]) {
      out[n++] = (FLOAT_DMEM)negqmean;
    }
    if (_options.enab_output[FUNCTIONAL_MEANS][FUNCT_NEGRQMEAN]) {
      out[n++] = (FLOAT_DMEM)sqrt(negqmean);
    }

    if (_options.enab_output[FUNCTIONAL_MEANS][FUNCT_RQMEAN]) {
      out[n++] = (FLOAT_DMEM)sqrt(qmean);
    }
    if (_options.enab_output[FUNCTIONAL_MEANS][FUNCT_NZRQMEAN]) {
      out[n++] = (FLOAT_DMEM)sqrt(nzqmean);
    }


    return n;
  }
  return 0;

}
int OSFunctionals::Onset(Variables vs) {

	long i;
	FLOAT_DMEM* in = vs.in;
	FLOAT_DMEM* out = vs.out;
	long Nin = vs.Nin;

	FLOAT_DMEM thresholdOnset,thresholdOffset;

	if (_options.setThresholdOnset) 
		thresholdOnset = _options.thresholdOnset;
	else thresholdOnset = _options.threshold;

	if (_options.setThresholdOffset)
		thresholdOffset = _options.thresholdOffset;
    else thresholdOffset = _options.threshold;
  
	if ((Nin>0)&&(out!=NULL)) {
    long onsetPos = -1;
    long offsetPos = -1;
    long nOnsets = 0;
    long nOffsets = 0;
    int oo = 0; // sttus memory.. last value above threshold = 1
    if (in[0] > thresholdOnset) oo = 1;
    for (i=1; i<Nin; i++) {
      FLOAT_DMEM cur;
	  if (_options.useAbsVal) cur = fabs(in[i]);
      else cur = in[i];
      if (cur > thresholdOnset) {
        if (oo == 0) {
          nOnsets++;
          if (onsetPos == -1) onsetPos = i;
          oo = 1;
        }
      }
      if (cur <= thresholdOffset) {
        if (oo == 1) {
          nOffsets++;
          offsetPos = i;
          oo = 0;
        }
      }
    }
    if (offsetPos == -1) offsetPos = Nin-1;
    if (onsetPos == -1) onsetPos = 0;

    int n=0;

	if (_options.masterTimeNorm == TIMENORM_SEGMENT) {
		if (_options.enab_output[FUNCTIONAL_ONSET][FUNCT_ONSETPOS]) out[n++]=(FLOAT_DMEM)onsetPos/(FLOAT_DMEM)(Nin);
      if (_options.enab_output[FUNCTIONAL_ONSET][FUNCT_OFFSETPOS]) out[n++]=(FLOAT_DMEM)offsetPos/(FLOAT_DMEM)(Nin);
    } else if (_options.masterTimeNorm == TIMENORM_SECONDS) {
      FLOAT_DMEM _T = (FLOAT_DMEM)getInputPeriod();
      if (_options.enab_output[FUNCTIONAL_ONSET][FUNCT_ONSETPOS]) out[n++]=(FLOAT_DMEM)onsetPos*_T;
      if (_options.enab_output[FUNCTIONAL_ONSET][FUNCT_OFFSETPOS]) out[n++]=(FLOAT_DMEM)offsetPos*_T;
    } else if (_options.masterTimeNorm == TIMENORM_FRAMES) {
      if (_options.enab_output[FUNCTIONAL_ONSET][FUNCT_ONSETPOS]) out[n++]=(FLOAT_DMEM)onsetPos;
      if (_options.enab_output[FUNCTIONAL_ONSET][FUNCT_OFFSETPOS]) out[n++]=(FLOAT_DMEM)offsetPos;
    }

    if (_options.enab_output[FUNCTIONAL_ONSET][FUNCT_NUMONSETS]) 
      out[n++]=(FLOAT_DMEM)nOnsets;

    if (_options.enab_output[FUNCTIONAL_ONSET][FUNCT_NUMOFFSETS]) 
      out[n++]=(FLOAT_DMEM)nOffsets;


    return n;
  }
  return 0;

}
int OSFunctionals::Peaks(Variables vs) {

	FLOAT_DMEM* in = vs.in;
	FLOAT_DMEM* out = vs.out;
	FLOAT_DMEM max = vs.max;
	FLOAT_DMEM min = vs.min;
	FLOAT_DMEM mean = vs.mean;
	long Nin = vs.Nin;

	int i;
  if ((Nin>0)&&(out!=NULL)) {
    FLOAT_DMEM max = *in;
    FLOAT_DMEM min = *in;
    FLOAT_DMEM mean = *in;

    FLOAT_DMEM peakDist = (FLOAT_DMEM)0.0;
    long nPeakDist = 0;
    FLOAT_DMEM peakMean = (FLOAT_DMEM)0.0;
    long nPeaks = 0;

    FLOAT_DMEM lastMin=(FLOAT_DMEM)0.0; // in[0];
    FLOAT_DMEM lastMax=(FLOAT_DMEM)0.0; //in[0];
    long curmaxPos=0, lastmaxPos=-1;
	FLOAT_DMEM lastlastVal,lastVal;
	
	lastlastVal = lastVal = 0.0;

    int peakflag = 0;  

    // range, min, max, mean
    for (i=1; i<Nin; i++) {
      if (in[i]<min) min=in[i];
      if (in[i]>max) max=in[i];
      mean += in[i];
    }
    mean /= (FLOAT_DMEM)Nin;
    FLOAT_DMEM range = max-min;

	if (_options.overlapFlag) i=2;
    else i=0;

    if (_options.overlapFlag) {
      lastlastVal=in[0];
      lastVal=in[1];
    }

    // advanced peak detection
    for (; i<Nin; i++) {
      // first, find ALL peaks:
      if ((lastlastVal < lastVal)&&(lastVal > in[i])) { // max
        if (!peakflag) lastMax = in[i];
        else { if (in[i] > lastMax) { lastMax = in[i]; curmaxPos = i; } }

        if (lastMax - lastMin > 0.11*range) { 
          peakflag = 1; curmaxPos = i;
        }

      } else {
        if ((lastlastVal > lastVal)&&(lastVal < in[i])) { // min
          lastMin = in[i];
        } 
      }

      // detect peak only, if x[i] falls below lastmax - 0.09*range
      if ((peakflag)&&( (in[i] < lastMax-0.09*range) || (i==Nin-1)) ) {
        nPeaks++;
        peakMean += lastMax;
        if (lastmaxPos >= 0) {
          FLOAT_DMEM dist = (FLOAT_DMEM)(curmaxPos-lastmaxPos);
          peakDist += dist;
          // add dist to list for variance computation
          addPeakDist(nPeakDist,ssi_cast (long, dist));
          nPeakDist++;
        }
        lastmaxPos = curmaxPos;
        peakflag = 0;
      }

      lastlastVal = lastVal;
      lastVal = in[i];
    }


    FLOAT_DMEM stddev = 0.0;

    if (nPeakDist > 0.0) {
      peakDist /= (FLOAT_DMEM)nPeakDist;
      // compute peak distance variance / standard deviation
      for (i=0; i<nPeakDist; i++) {
        stddev += (_options.peakdists[i]-peakDist)*(_options.peakdists[i]-peakDist);
      }
      stddev /= (FLOAT_DMEM)nPeakDist;
      stddev = sqrt(stddev);
    } else {
      peakDist = (FLOAT_DMEM)(Nin+1);
      stddev = 0.0;
    }


    int n=0;
	if (_options.enab_output[FUNCTIONAL_PEAKS][FUNCT_NUMPEAKS]) out[n++]=(FLOAT_DMEM)nPeaks;

	if (_options.masterTimeNorm==TIMENORM_SECONDS) {
      peakDist *= (FLOAT_DMEM)getInputPeriod();
      stddev *= (FLOAT_DMEM)getInputPeriod();
    } else if (_options.masterTimeNorm==TIMENORM_SEGMENT) {
      peakDist /= (FLOAT_DMEM)Nin;
      stddev /= (FLOAT_DMEM)Nin;
    }
    if (_options.enab_output[FUNCTIONAL_PEAKS][FUNCT_MEANPEAKDIST]) out[n++]=peakDist;

    if (nPeaks > 0.0) {
      peakMean /= (FLOAT_DMEM)nPeaks;
    } else {
      peakMean = (FLOAT_DMEM)0.0;
    }
    if (_options.enab_output[FUNCTIONAL_PEAKS][FUNCT_PEAKMEAN]) out[n++] = peakMean;

    if (_options.enab_output[FUNCTIONAL_PEAKS][FUNCT_PEAKMEANMEANDIST]) out[n++]=peakMean-mean;

    if (_options.enab_output[FUNCTIONAL_PEAKS][FUNCT_PEAKDISTSTDDEV]) out[n++]=stddev;

    return n;
  }
  return 0;

}
int OSFunctionals::Percentiles(Variables vs) {

	FLOAT_DMEM* in = vs.in;
	FLOAT_DMEM* inSorted = vs.inSorted;
	FLOAT_DMEM* out = vs.out;
	FLOAT_DMEM max = vs.max;
	FLOAT_DMEM min = vs.min;
	FLOAT_DMEM mean = vs.mean;
	long Nin = vs.Nin;

	long i;
  if ((Nin>0)&&(out!=NULL)) {
    int n=0;
    FLOAT_DMEM q1, q2, q3;
    
    long minpos=0, maxpos=0;
      if (inSorted == NULL) {
		  ssi_err("Expected sorted input, however got NULL!");
      }
      // quartiles:
	  if (_options.interp) {
        q1 = getInterpPctl(0.25,inSorted,Nin);
        q2 = getInterpPctl(0.50,inSorted,Nin);
        q3 = getInterpPctl(0.75,inSorted,Nin);
      } else {
        q1 = inSorted[getPctlIdx(0.25,Nin)];
        q2 = inSorted[getPctlIdx(0.50,Nin)];
        q3 = inSorted[getPctlIdx(0.75,Nin)];
      }
	  if (_options.enab_output[FUNCTIONAL_PERCENTILES][FUNCT_QUART1]) out[n++]=q1;
      if (_options.enab_output[FUNCTIONAL_PERCENTILES][FUNCT_QUART2]) out[n++]=q2;
      if (_options.enab_output[FUNCTIONAL_PERCENTILES][FUNCT_QUART3]) out[n++]=q3;
      if (_options.enab_output[FUNCTIONAL_PERCENTILES][FUNCT_IQR12]) out[n++]=q2-q1;
      if (_options.enab_output[FUNCTIONAL_PERCENTILES][FUNCT_IQR23]) out[n++]=q3-q2;
      if (_options.enab_output[FUNCTIONAL_PERCENTILES][FUNCT_IQR13]) out[n++]=q3-q1;

      // percentiles
	  int nPctlRange = _options.enab_output[FUNCTIONAL_PERCENTILES][FUNCT_PCTLRANGE];
	  int nPctl = _options.enab_output[FUNCTIONAL_PERCENTILES][FUNCT_PERCENTILE];
	  FLOAT_DMEM* pctl = ParseFloatSamples(_options.percentile);
	  int* pctlr1 = ParseIntSamples(_options.pctlrange,0);
	  int* pctlr2 = ParseIntSamples(_options.pctlrange,1);

      if ((_options.enab_output[FUNCTIONAL_PERCENTILES][FUNCT_PERCENTILE])||(_options.enab_output[FUNCTIONAL_PERCENTILES][FUNCT_PCTLRANGE])) {
        int n0 = n; // start of percentiles array (used later for computation of pctlranges)
        if (_options.interp) {
          for (i=0; i<nPctl; i++) {
            out[n++] = getInterpPctl(pctl[i],inSorted,Nin);
          }
        } else {
          for (i=0; i<nPctl; i++) {
            out[n++] = inSorted[getPctlIdx(pctl[i],Nin)];
          }
        }
        if (_options.enab_output[FUNCTIONAL_PERCENTILES][FUNCT_PCTLRANGE]) {
          for (i=0; i<nPctlRange; i++) {
            if ((pctlr1[i]>=0)&&(pctlr2[i]>=0)) {
              out[n++] = fabs(out[n0+pctlr2[i]] - out[n0+pctlr1[i]]);
            } else { out[n++] = 0.0; }
          }
        }
	  }
    return n;
  }
  return 0;

}
int OSFunctionals::Regression(Variables vs) {

	FLOAT_DMEM *in = vs.in;
	FLOAT_DMEM *inSorted = vs.inSorted;
	FLOAT_DMEM max = vs.max;
	FLOAT_DMEM min = vs.min;
	FLOAT_DMEM mean = vs.mean;
	FLOAT_DMEM *out = vs.out; 
	long Nin = vs.Nin;

	int enQreg = 0;
	const int oldBuggyQerr = 0;
	if(_options.enab_output[FUNCTIONAL_REGRESSION][FUNCT_QREGC1] ||
	_options.enab_output[FUNCTIONAL_REGRESSION][FUNCT_QREGC2] ||
	_options.enab_output[FUNCTIONAL_REGRESSION][FUNCT_QREGC3] ||
	_options.enab_output[FUNCTIONAL_REGRESSION][FUNCT_QREGERRA] ||
	_options.enab_output[FUNCTIONAL_REGRESSION][FUNCT_QREGERRQ] ||
	_options.enab_output[FUNCTIONAL_REGRESSION][FUNCT_CENTROID]) enQreg = 1;
	

	if ((Nin>0)&&(out!=NULL)) {
    //compute centroid
    FLOAT_DMEM *iE=in+Nin, *i0=in;
    double Nind = (double)Nin;
    double range = max-min;
    if (range <= 0.0) range = 1.0;
    
    double num=0.0, num2=0.0;
    double tmp,asum=mean*Nind;
    double ii=0.0;
    while (in<iE) {
//      asum += *(in);
      tmp = (double)(*(in++)) * ii;
      num += tmp;
      tmp *= ii;
      ii += 1.0;
      num2 += tmp;
    }

    double centroid;
    if (asum != 0.0)
      centroid = num / ( asum * Nind);
    else
      centroid = 0.0;

    in=i0;

    
    double m=0.0,t=0.0,leq=0.0,lea=0.0;
    double a=0.0,b=0.0,c=0.0,qeq=0.0,qea=0.0;
    double S1,S2,S3,S4;
    if (Nin > 1) {
      double NNm1 = (Nind)*(Nind-(double)1.0);

      S1 = NNm1/(double)2.0;  // sum of all i=0..N-1
      S2 = NNm1*((double)2.0*Nind-(double)1.0)/(double)6.0; // sum of all i^2 for i=0..N-1

      double S1dS2 = S1/S2;
      double tmp = ( Nind - S1*S1dS2 );
      if (tmp == 0.0) t = 0.0;
      else t = ( asum - num*S1dS2) / tmp;
      m = ( num - t * S1 ) / S2;

      S3 = S1*S1;
      double Nind1 = Nind-(double)1.0;
      S4 = S2 * ((double)3.0*(Nind1*Nind1 + Nind1)-(double)1.0) / (double)5.0;

      // QUADRATIC REGRESSION:
      if (enQreg) {

        double det;
        double S3S3 = S3*S3;
        double S2S2 = S2*S2;
        double S1S2 = S1*S2;
        double S1S1 = S3;
        det = S4*S2*Nind + (double)2.0*S3*S1S2 - S2S2*S2 - S3S3*Nind - S1S1*S4;

        if (det != 0.0) {
          a = ( (S2*Nind - S1S1)*num2 + (S1S2 - S3*Nind)*num + (S3*S1 - S2S2)*asum ) / det;
          b = ( (S1S2 - S3*Nind)*num2 + (S4*Nind - S2S2)*num + (S3*S2 - S4*S1)*asum ) / det;
          c = ( (S3*S1 - S2S2)*num2 + (S3*S2-S4*S1)*num + (S4*S2 - S3S3)*asum ) / det;
        } else {
          a=0.0; b=0.0; c=0.0;
        }

      }
//    printf("nind:%f  S1=%f,  S2=%f  S3=%f  S4=%f  num2=%f  num=%f  asum=%f t=%f\n",Nind,S1,S2,S3,S4,num2,num,asum,t);
    } else {
      m = 0; t=c=*in;
      a = 0.0; b=0.0;
    }
    
    // linear regression error:
    ii=0.0; double e;
    while (in<iE) {
      e = (double)(*(in++)) - (m*ii + t);
      if (_options.normInputs) e /= range;
      lea += fabs(e);
      ii += 1.0;
      leq += e*e;
    }
    in=i0;

    double rs=0.0, ls=0.0;
    double x0=0.0, y0=0.0;
    double yr=0.0;
    double yrnn=0.0, c3nn=0.0;
    double x0nn=0.0, y0nn=0.0;

    // quadratic regresssion error:
    if (enQreg) {
      ii=0.0; double e;
      while (in<iE) {
        e = (double)(*(in++)) - (a*ii*ii + b*ii + c);
        if (_options.normInputs) e /= range;
        qea += fabs(e);
        ii += 1.0;
        qeq += e*e;
      }
      in=i0;

      // parabola vertex (x coordinate clipped to range -Nind + Nind!)
      x0 = b/(-2.0*a);
      if (x0 < -1.0*Nind) x0 = -Nind;
      if (x0 > Nind) x0 = Nind;
      if (!finite(x0)) x0 = Nind;
      x0nn = x0;
      y0 = c - b*b/(4.0*a);
      if (!finite(y0)) y0 = 0.0;
      y0nn = y0;

      // parabola left / right points
      yrnn = yr = a * (Nind-1.0)*(Nind-1.0) + b*(Nind-1.0) + c;
      if (!finite(yr)) { yr = 0.0; yrnn = 0.0; }
      c3nn = c;
    }

    int n=0;
    


    if (_options.normRegCoeff) {
      m *= Nind-1.0;
      a *= (Nind-1.0)*(Nind-1.0);
      b *= Nind-1.0;
      if (Nind != 1.0)
        x0 /= Nind-1.0;
    }
    if (_options.normInputs) {
      m /= range;
      t = (t-min)/range;
      a /= range;
      b /= range;
      c = (c-min)/range;
      y0 = (y0 - min)/range;
      yr = (yr - min)/range;
    }

    if (enQreg) {
      // parabola partial slopes
      if (x0 > 0) ls = (y0-c)/x0;
      if (_options.normRegCoeff) {
        if (x0 < Nind-1.0) rs = (yr-y0)/(1.0-x0);
      } else {
        if (x0 < Nind-1.0) rs = (yr-y0)/(Nind-1.0-x0);
      }
    }

    // security checks:
    if (!finite(m)) m = 0.0;
    if (!finite(t)) t = 0.0;
    if (!finite(lea/Nind)) lea = 0.0;
    if (!finite(leq/Nind)) leq = 0.0;
    if (!finite(a)) a = 0.0;
    if (!finite(b)) b = 0.0;
    if (!finite(c)) { c = 0.0; c3nn = 0.0; }
    if (!finite(ls)) ls = 0.0;
    if (!finite(rs)) rs = 0.0;
    if (!finite(qea/Nind)) qea = 0.0;
    if (!finite(qeq/Nind)) qeq = 0.0;
    if (!finite(centroid)) centroid = 0.0;

    // save values:
	if (_options.enab_output[FUNCTIONAL_REGRESSION][FUNCT_LINREGC1]) out[n++]=(FLOAT_DMEM)m;
    if (_options.enab_output[FUNCTIONAL_REGRESSION][FUNCT_LINREGC2]) out[n++]=(FLOAT_DMEM)t;
    if (_options.enab_output[FUNCTIONAL_REGRESSION][FUNCT_LINREGERRA]) out[n++]=(FLOAT_DMEM)(lea/Nind);
    if (_options.enab_output[FUNCTIONAL_REGRESSION][FUNCT_LINREGERRQ]) out[n++]=(FLOAT_DMEM)(leq/Nind);

    if (_options.enab_output[FUNCTIONAL_REGRESSION][FUNCT_QREGC1]) out[n++]=(FLOAT_DMEM)a;
    if (_options.enab_output[FUNCTIONAL_REGRESSION][FUNCT_QREGC2]) out[n++]=(FLOAT_DMEM)b;
    if (_options.enab_output[FUNCTIONAL_REGRESSION][FUNCT_QREGC3]) out[n++]=(FLOAT_DMEM)c;
    if (!oldBuggyQerr) {
      if (_options.enab_output[FUNCTIONAL_REGRESSION][FUNCT_QREGERRA]) out[n++]=(FLOAT_DMEM)(qea/Nind);
      if (_options.enab_output[FUNCTIONAL_REGRESSION][FUNCT_QREGERRQ]) out[n++]=(FLOAT_DMEM)(qeq/Nind);
    } else {
      if (_options.enab_output[FUNCTIONAL_REGRESSION][FUNCT_QREGERRA]) out[n++]=(FLOAT_DMEM)(qea);
      if (_options.enab_output[FUNCTIONAL_REGRESSION][FUNCT_QREGERRQ]) out[n++]=(FLOAT_DMEM)(qeq);
    }

    if (_options.enab_output[FUNCT_CENTROID]) out[n++]=(FLOAT_DMEM)centroid;

    if (_options.enab_output[FUNCT_QREGLS]) out[n++]=(FLOAT_DMEM)ls;
    if (_options.enab_output[FUNCT_QREGRS]) out[n++]=(FLOAT_DMEM)rs;
    if (_options.enab_output[FUNCT_QREGX0]) out[n++]=(FLOAT_DMEM)x0;
    if (_options.enab_output[FUNCT_QREGY0]) out[n++]=(FLOAT_DMEM)y0;
    if (_options.enab_output[FUNCT_QREGYR]) out[n++]=(FLOAT_DMEM)yr;
    if (_options.enab_output[FUNCT_QREGY0NN]) out[n++]=(FLOAT_DMEM)y0nn;
    if (_options.enab_output[FUNCT_QREGC3NN]) out[n++]=(FLOAT_DMEM)c3nn;
    if (_options.enab_output[FUNCT_QREGYRNN]) out[n++]=(FLOAT_DMEM)yrnn;

	//delete[] iE;
	//delete[] i0;

    return n;
  }
  return 0;

}

int OSFunctionals::Moments(Variables vs) {

	FLOAT_DMEM *out = vs.out; 
	FLOAT_DMEM *in = vs.in;
	FLOAT_DMEM mean = vs.mean;
	long Nin = vs.Nin;


	int i;
  if ((Nin>0)&&(out!=NULL)) {
    double m2=0.0, m3=0.0, m4=0.0;

    double Nind = (double)Nin;
    double tmp, tmp2;
    FLOAT_DMEM *in0=in;
    
    for (i=0; i<Nin; i++) {
      tmp = ((double)*(in++) - mean); // ?? * p(x) ??
      tmp2 = tmp*tmp;
      m2 += tmp2;
      tmp2 *= tmp;
      m3 += tmp2;
      m4 += tmp2*tmp;
    }
    m2 /= Nind;  // variance

    int n=0;
	if (_options.enab_output[FUNCTIONAL_MOMENTS][FUNCT_VAR]) out[n++]=(FLOAT_DMEM)m2;
    double sqm2=sqrt(m2);
    if (_options.enab_output[FUNCTIONAL_MOMENTS][FUNCT_STDDEV]) {
      if (m2 > 0.0) out[n++]=(FLOAT_DMEM)sqm2;
      else out[n++] = 0.0;
    }
    if (_options.enab_output[FUNCTIONAL_MOMENTS][FUNCT_SKEWNESS]) {
      if (m2 > 0.0) out[n++]=(FLOAT_DMEM)( m3/(Nind*m2*sqm2) );
      else out[n++] = 0.0;
    }
    if (_options.enab_output[FUNCTIONAL_MOMENTS][FUNCT_KURTOSIS]) {
      if (m2 > 0.0) out[n++]=(FLOAT_DMEM)( m4/(Nind*m2*m2) );
      else out[n++] = 0.0;
    }
	if (_options.enab_output[FUNCTIONAL_MOMENTS][FUNCT_AMEAN_MOM]) out[n++]=(FLOAT_DMEM)mean;
    return n;
  }
  return 0;

}

int OSFunctionals::Peaks2(Variables vs) {

	ssi_wrn("Functional Peaks 2 takes too long for a big stream!");

	long i;
	int enabSlope = 0;
	FLOAT_DMEM* in = vs.in;
	FLOAT_DMEM* out = vs.out;
	FLOAT_DMEM max = vs.max;
	FLOAT_DMEM min = vs.min;
	FLOAT_DMEM mean = vs.mean;
	long Nin = vs.Nin;
	int n=0;

	if (_options.relThresh < 0) {
    _options.relThresh = 0.0;
  } else if (_options.relThresh > 1.0 && !_options.dynRelThresh) {
	_options.relThresh = 1.0;
  }
	if(_options.useAbsThresh) _options.dynRelThresh = 0;

    if (_options.enab_output[FUNCTIONAL_PEAKS2][FUNCT_MEANRISINGSLOPE] || _options.enab_output[FUNCTIONAL_PEAKS2][FUNCT_MAXRISINGSLOPE] || _options.enab_output[FUNCTIONAL_PEAKS2][FUNCT_MINRISINGSLOPE] ||
	  _options.enab_output[FUNCTIONAL_PEAKS2][FUNCT_STDDEVRISINGSLOPE] || _options.enab_output[FUNCTIONAL_PEAKS2][FUNCT_MEANFALLINGSLOPE] || 
	  _options.enab_output[FUNCTIONAL_PEAKS2][FUNCT_MAXFALLINGSLOPE] || _options.enab_output[FUNCTIONAL_PEAKS2][FUNCT_MINFALLINGSLOPE] || 
	  _options.enab_output[FUNCTIONAL_PEAKS2][FUNCT_STDDEVFALLINGSLOPE]) enabSlope = 1;

  if ((Nin>0)&&(out!=NULL)) {
    
    /* algorithm:
     *   1st: detect all local min/max
         2nd: enforce constraint of minimum rise/fall and discard min/max from list
         3rd: enforce constraint of alternating min/max (if double candidates, choose the highest/lowest max/min)
     *
     */

    FLOAT_DMEM range = max-min;
    if (!_options.useAbsThresh) _options.absThresh = _options.relThresh*range;

    // step 1: detect ALL local min/max
    for (i=2; i<Nin-2; i++) {
      if (in[i] > in[i-1] && in[i] > in[i+1]) { // max
        addMinMax(1, in[i], i);
      } else if (in[i] < in[i-1] && in[i] < in[i+1]) { // min
        addMinMax(0, in[i], i);
      }
    }


    // step 2a: enforce constraint of mutual minimum rise/fall and discard small peaks from list
    struct peakMinMaxListEl * listEl = _options.mmlistFirst;
    FLOAT_DMEM lastVal = in[0];
    FLOAT_DMEM lastMin = in[0];
    FLOAT_DMEM lastMax = in[0];
    int maxFlag=0, minFlag=0;
    struct peakMinMaxListEl * lastMaxPtr=NULL;
    struct peakMinMaxListEl * lastMinPtr=NULL;
    while (listEl != NULL) { // iterate through list elements
      int doFree = 0;
      if (listEl->type == 1) { //max
        if (isBelowThresh( fabs(listEl->y-lastVal), MIN(listEl->y,lastVal))) {
          if (isBelowThresh( listEl->y - lastMin, lastMin)) {
            // discard this item from the list...
            removeFromMinMaxList(listEl);
            doFree=1;
          } else {
            // next local-global candidate
            if (listEl->y > lastMax*1.05) {
              // discard previous max
              if (lastMaxPtr != NULL) {
                removeFromMinMaxList(lastMaxPtr);
                if (lastMaxPtr != listEl) free(lastMaxPtr);
                else doFree = 1;
              }
              lastMax = listEl->y;
              lastMaxPtr = listEl;
            } else {
              if (minFlag) {
                lastMax = listEl->y;
                lastMaxPtr = listEl;
              } else {
                // discard this, keep last
                removeFromMinMaxList(listEl);
                doFree=1;
              }
            }
            maxFlag = 1;
            minFlag = 0;
          }
        } else {
          // manage the flags and pointers...
          maxFlag = 1; minFlag = 0;
          lastMax = listEl->y;
          lastMaxPtr = listEl;
        }

      } else if (listEl->type == 0) { //min
        if (!isBelowThresh( fabs(listEl->y-lastVal), MIN(listEl->y,lastVal))) {
          // manage the flags and pointers...
          minFlag = 1; maxFlag = 0;
          lastMin = listEl->y;
          lastMinPtr = listEl;
        }

      }
      lastVal = listEl->y;
      struct peakMinMaxListEl * nextPtr =listEl->next;
      if (doFree) free(listEl);
      listEl = nextPtr;
    }

    // step 2b: eliminate small minima
    listEl = _options.mmlistFirst;
    lastMax = in[0];
    while (listEl != NULL) { // iterate through list elements
      int doFree = 0;
      if (listEl->type == 0) { //min
        if (isBelowThresh(lastMax - listEl->y, listEl->y)) {
          // discard this item from the list...
          removeFromMinMaxList(listEl);
          doFree=1;
        }
      } else if (listEl->type == 1) { //max
        lastMax = listEl->y;
      }

      struct peakMinMaxListEl * nextPtr =listEl->next;
      if (doFree) free(listEl);
      listEl = nextPtr;
    }

    // step 3: eliminate duplicate maxima / minima (i.e. with no min/max in between)
    listEl = _options.mmlistFirst;
    lastMax = in[0]; lastMin = in[0];
    minFlag = 0; maxFlag = 0;
    int init = 1;
    while (listEl != NULL) { // iterate through list elements
      int doFree = 0;
      if (listEl->type == 0) { //min
        if (!minFlag || init) { // change of type..
          lastMin = listEl->y;
          lastMinPtr = listEl;
          minFlag = 1;
          init = 0;
        } else {
          if (listEl->y >= lastMin) {
            // eliminate this one
            removeFromMinMaxList(listEl);
            doFree=1;
          } else {
           // eliminate the last one , if not the current one
           if (lastMinPtr != listEl) {
             removeFromMinMaxList(lastMinPtr);
             free(lastMinPtr);
             lastMinPtr = listEl;
             lastMin = listEl->y;
           }
          }
        }
      } else if (listEl->type == 1) { //max
        if (minFlag||init) { // change of type...
          lastMax = listEl->y;
          lastMaxPtr = listEl;
          minFlag = 0;
          init = 0;
        } else {
          if (listEl->y <= lastMax) {
            // eliminate this one
            removeFromMinMaxList(listEl);
            doFree=1;
          } else {
           // eliminate the last one , if not the current one
           if (lastMaxPtr != listEl) {
             removeFromMinMaxList(lastMaxPtr);
             free(lastMaxPtr);
             lastMaxPtr = listEl;
             lastMax = listEl->y;
           }
          }
        }
      }

      struct peakMinMaxListEl * nextPtr =listEl->next;
      if (doFree) free(listEl);
      listEl = nextPtr;
    }

    ////////////////////////////////// now collect statistics
    FLOAT_DMEM peakMax = (FLOAT_DMEM)0.0, peakMin = (FLOAT_DMEM)0.0;
    FLOAT_DMEM peakDist = (FLOAT_DMEM)0.0;
    FLOAT_DMEM peakDiff = (FLOAT_DMEM)0.0;
    FLOAT_DMEM peakStddevDist = (FLOAT_DMEM)0.0;
    FLOAT_DMEM peakStddevDiff = (FLOAT_DMEM)0.0;
    long nPeakDist = 0;
    FLOAT_DMEM peakMean = (FLOAT_DMEM)0.0;
    long nPeaks = 0;

    FLOAT_DMEM minMax = (FLOAT_DMEM)0.0, minMin = (FLOAT_DMEM)0.0;
    FLOAT_DMEM minDist = (FLOAT_DMEM)0.0;
    FLOAT_DMEM minDiff = (FLOAT_DMEM)0.0;
    FLOAT_DMEM minStddevDist = (FLOAT_DMEM)0.0;
    FLOAT_DMEM minStddevDiff = (FLOAT_DMEM)0.0;
    long nMinDist = 0;
    FLOAT_DMEM minMean = (FLOAT_DMEM)0.0;
    long nMins = 0;

    // iterate through list elements, 1st pass computations
    listEl = _options.mmlistFirst;
    lastMaxPtr=NULL;
    lastMinPtr=NULL;
    while (listEl != NULL) {
      if (listEl->type == 0) { //min
        // distances and amp. diff., min/max for range
        if (lastMinPtr == NULL) {
          lastMinPtr = listEl;
          minMin = listEl->y;
          minMax = listEl->y;
        } else  {
          nMinDist++;
          minDist += (FLOAT_DMEM)(listEl->x - lastMinPtr->x);
          minDiff += (FLOAT_DMEM)fabs(listEl->y - lastMinPtr->y);
          if (minMin > listEl->y) minMin = listEl->y;
          if (minMax < listEl->y) minMax = listEl->y;
          lastMinPtr = listEl;
        }
        // mean
        minMean += listEl->y;
        nMins++;
      } else { //max
        // distances and amp. diff., min/max for range
        if (lastMaxPtr == NULL) {
          lastMaxPtr = listEl;
          peakMin = listEl->y;
          peakMax = listEl->y;
        } else  {
          nPeakDist++;
          peakDist += (FLOAT_DMEM)(listEl->x - lastMaxPtr->x);
          peakDiff += (FLOAT_DMEM)fabs(listEl->y - lastMaxPtr->y);
          if (peakMin > listEl->y) peakMin = listEl->y;
          if (peakMax < listEl->y) peakMax = listEl->y;
          lastMaxPtr = listEl;
        }
        // mean
        peakMean += listEl->y;
        nPeaks++;
      }

      struct peakMinMaxListEl * nextPtr =listEl->next;
      listEl = nextPtr;
    }

    // mean computation:
    if (nPeaks > 1) {
      peakMean /= (FLOAT_DMEM)nPeaks;
      if (nPeakDist > 1) {
        peakDist /= (FLOAT_DMEM)nPeakDist;
        peakDiff /= (FLOAT_DMEM)nPeakDist;
      }
    }
    if (nMins > 0) {
      minMean /= (FLOAT_DMEM)nMins;
      if (nMinDist > 1) {
        minDist /= (FLOAT_DMEM)nMinDist;
        minDiff /= (FLOAT_DMEM)nMinDist;
      }
    }

    // iterate through list elements, 2nd pass computations (stddev)
    listEl = _options.mmlistFirst;
    lastMaxPtr=NULL;
    lastMinPtr=NULL;
    while (listEl != NULL) {
      if (listEl->type == 0) { //min
        // stddev of distances and amp. diffs.
        if (lastMinPtr == NULL) {
          lastMinPtr = listEl;
        } else  {
          minStddevDist += ((FLOAT_DMEM)(listEl->x - lastMinPtr->x) - minDist) * ((FLOAT_DMEM)(listEl->x - lastMinPtr->x) - minDist);
          minStddevDiff += ((FLOAT_DMEM)fabs(listEl->y - lastMinPtr->y) - minDiff) * ((FLOAT_DMEM)fabs(listEl->y - lastMinPtr->y) - minDiff);
          lastMinPtr = listEl;
        }
      } else { //max
        // stddev of distances and amp. diffs.
        if (lastMaxPtr == NULL) {
          lastMaxPtr = listEl;
        } else  {
          peakStddevDist += ((FLOAT_DMEM)(listEl->x - lastMinPtr->x) - peakDist) * ((FLOAT_DMEM)(listEl->x - lastMinPtr->x) - peakDist);
          peakStddevDiff += ((FLOAT_DMEM)fabs(listEl->y - lastMinPtr->y) - peakDiff) * ((FLOAT_DMEM)fabs(listEl->y - lastMinPtr->y) - peakDiff);
          lastMaxPtr = listEl;
        }
      }

      struct peakMinMaxListEl * nextPtr =listEl->next;
      listEl = nextPtr;
    }

    // normalise stddev:
    if (nPeakDist > 1) {
      peakStddevDist /= (FLOAT_DMEM)nPeakDist;
      peakStddevDiff /= (FLOAT_DMEM)nPeakDist;
    }
    if (peakStddevDist > 0.0) peakStddevDist = sqrt(peakStddevDist);
    else peakStddevDist = 0.0;
    if (peakStddevDiff > 0.0) peakStddevDiff = sqrt(peakStddevDiff);
    else peakStddevDiff = 0.0;

    if (nMinDist > 1) {
      minStddevDist /= (FLOAT_DMEM)nMinDist;
      minStddevDiff /= (FLOAT_DMEM)nMinDist;
    }
    if (minStddevDist > 0.0) minStddevDist = sqrt(minStddevDist);
    else minStddevDist = 0.0;
    if (minStddevDiff > 0.0) minStddevDiff = sqrt(minStddevDiff);
    else minStddevDiff = 0.0;


    /// slopes....

    FLOAT_DMEM meanRisingSlope = (FLOAT_DMEM)0.0; int nRising=0;
    FLOAT_DMEM meanFallingSlope = (FLOAT_DMEM)0.0; int nFalling=0;
    FLOAT_DMEM minRisingSlope = (FLOAT_DMEM)0.0;
    FLOAT_DMEM maxRisingSlope = (FLOAT_DMEM)0.0;
    FLOAT_DMEM minFallingSlope = (FLOAT_DMEM)0.0;
    FLOAT_DMEM maxFallingSlope = (FLOAT_DMEM)0.0;
    FLOAT_DMEM stddevRisingSlope = (FLOAT_DMEM)0.0;
    FLOAT_DMEM stddevFallingSlope = (FLOAT_DMEM)0.0;

    int lastIsMax=-1;

    if (enabSlope) {
      FLOAT_DMEM T = (FLOAT_DMEM)getInputPeriod();
      // iterate through list elements, slope statistics computation
      listEl = _options.mmlistFirst;
      lastMax = in[0]; long lastMaxPos = 0;
      lastMin = in[0]; long lastMinPos = 0;
      while (listEl != NULL) {
        if (listEl->type == 0) { //min
          lastMin = listEl->y;
          lastMinPos = listEl->x;
          if (lastMinPos - lastMaxPos > 0) {
            FLOAT_DMEM slope = (lastMax - lastMin)/((FLOAT_DMEM)(lastMinPos - lastMaxPos)*T);
            meanFallingSlope += slope;
            if (nFalling == 0) {
              minFallingSlope = slope;
              maxFallingSlope = slope;
            } else {
              if (slope < minFallingSlope) minFallingSlope = slope;
              if (slope > maxFallingSlope) maxFallingSlope = slope;
            }
            nFalling++; lastIsMax = 0;
          }
        } else { //max
          lastMax = listEl->y;
          lastMaxPos = listEl->x;
          if (lastMaxPos - lastMinPos > 0) {
            FLOAT_DMEM slope = (lastMax - lastMin)/((FLOAT_DMEM)(lastMaxPos - lastMinPos)*T);
            meanRisingSlope += slope;
            if (nRising == 0) {
              minRisingSlope = slope;
              maxRisingSlope = slope;
            } else {
              if (slope < minRisingSlope) minRisingSlope = slope;
              if (slope > maxRisingSlope) maxRisingSlope = slope;
            }
            nRising++; lastIsMax = 1;
          }
        }

        struct peakMinMaxListEl * nextPtr =listEl->next;
        listEl = nextPtr;
      }

      // compute slope at the end of the input
      if (lastIsMax==1) {
        if (Nin - 1 - lastMaxPos > 0) {
          FLOAT_DMEM slope = (in[Nin-1] - lastMax)/((FLOAT_DMEM)(Nin - 1 - lastMaxPos)*T);
          meanFallingSlope += slope;
          if (nFalling == 0) {
            minFallingSlope = slope;
            maxFallingSlope = slope;
          } else {
            if (slope < minFallingSlope) minFallingSlope = slope;
            if (slope > maxFallingSlope) maxFallingSlope = slope;
          }
          nFalling++;
        }
      } else if (lastIsMax==0) {
        if (Nin - 1 - lastMinPos > 0) {
          FLOAT_DMEM slope = (in[Nin-1] - lastMin)/((FLOAT_DMEM)(Nin - 1 - lastMinPos)*T);
          meanRisingSlope += slope;
          if (nRising == 0) {
            minRisingSlope = slope;
            maxRisingSlope = slope;
          } else {
            if (slope < minRisingSlope) minRisingSlope = slope;
            if (slope > maxRisingSlope) maxRisingSlope = slope;
          }
          nRising++;
        }
      } else if (lastIsMax==-1) {  // no max/min at all...
        FLOAT_DMEM slope = (in[Nin-1]-in[0])/(FLOAT_DMEM)Nin;
        if (slope > 0) { meanRisingSlope = maxRisingSlope = minRisingSlope = slope; nRising = 1; }
        else if (slope < 0) { meanFallingSlope = maxFallingSlope = minFallingSlope = slope; nFalling = 1; }
      }

      if (nRising > 1) meanRisingSlope /= (FLOAT_DMEM)nRising;
      if (nFalling > 1) meanFallingSlope /= (FLOAT_DMEM)nFalling;

      // iterate through list elements, slope statistics computation, 2nd pass for stddev
      listEl = _options.mmlistFirst;
      lastMax = in[0]; lastMaxPos = 0;
      lastMin = in[0]; lastMinPos = 0;
      while (listEl != NULL) {
        if (listEl->type == 0) { //min
          lastMin = listEl->y;
          lastMinPos = listEl->x;
          if (lastMinPos - lastMaxPos > 0) {
            FLOAT_DMEM slope = (lastMax - lastMin)/((FLOAT_DMEM)(lastMinPos - lastMaxPos)*T);
            stddevFallingSlope += (slope-meanFallingSlope)*(slope-meanFallingSlope);
          }
        } else { //max
          lastMax = listEl->y;
          lastMaxPos = listEl->x;
          if (lastMaxPos - lastMinPos) {
            FLOAT_DMEM slope = (lastMax - lastMin)/((FLOAT_DMEM)(lastMaxPos - lastMinPos)*T);
            stddevRisingSlope += (slope-meanRisingSlope)*(slope-meanRisingSlope);
          }
        }

        struct peakMinMaxListEl * nextPtr =listEl->next;
        listEl = nextPtr;
      }

      if (nRising > 1) stddevRisingSlope /= (FLOAT_DMEM)nRising;
      if (nFalling > 1) stddevFallingSlope /= (FLOAT_DMEM)nFalling;

      if (stddevRisingSlope > 0.0) stddevRisingSlope = sqrt(stddevRisingSlope);
      else stddevRisingSlope = 0.0;
      if (stddevFallingSlope > 0.0) stddevFallingSlope = sqrt(stddevFallingSlope);
      else stddevFallingSlope = 0.0;
    }

    //// normalisation
	if (_options.masterTimeNorm==TIMENORM_SECONDS) {
      peakDist *= (FLOAT_DMEM)getInputPeriod();
      peakStddevDist *= (FLOAT_DMEM)getInputPeriod();
      //minDist *= (FLOAT_DMEM)getInputPeriod();
      //minStddev *= (FLOAT_DMEM)getInputPeriod();
	} else if (_options.masterTimeNorm==TIMENORM_SEGMENT) {
      peakDist /= (FLOAT_DMEM)Nin;
      peakStddevDist /= (FLOAT_DMEM)Nin;
      //minDist /= (FLOAT_DMEM)Nin;
      //minStddev /= (FLOAT_DMEM)Nin;
    }


    //////////// value output


	if (_options.enab_output[FUNCTIONAL_PEAKS2][FUNCT_NUMPEAKS]) out[n++]=(FLOAT_DMEM)nPeaks;
    if (_options.enab_output[FUNCTIONAL_PEAKS2][FUNCT_MEANPEAKDIST]) out[n++]=peakDist;
    if (_options.enab_output[FUNCTIONAL_PEAKS2][FUNCT_MEANPEAKDISTDELTA]) out[n++]=0.0; // TODO!
    if (_options.enab_output[FUNCTIONAL_PEAKS2][FUNCT_PEAKDISTSTDDEV2]) out[n++]=peakStddevDist;
    if (_options.enab_output[FUNCTIONAL_PEAKS2][FUNCT_PEAKRANGEABS]) out[n++]=peakMax-peakMin;
    if (_options.enab_output[FUNCTIONAL_PEAKS2][FUNCT_PEAKRANGEREL]) {
      if (range != 0.0) {
        out[n++]=(FLOAT_DMEM)fabs( (peakMax-peakMin)/range );
      } else {
        out[n++]=peakMax-peakMin;
      }
    }
    if (_options.enab_output[FUNCTIONAL_PEAKS2][FUNCT_PEAKMEAN_ABS]) out[n++]=peakMean;
    if (_options.enab_output[FUNCTIONAL_PEAKS2][FUNCT_PEAKMEANMEANDIST2]) {
      out[n++]=peakMean - mean;
    }
    if (_options.enab_output[FUNCTIONAL_PEAKS2][FUNCT_PEAKMEANMEANRATIO]) {
      if (mean != 0.0) {
        out[n++]=peakMean/mean;
      } else {
        out[n++]=peakMean;
      }
    }
    if (_options.enab_output[FUNCTIONAL_PEAKS2][FUNCT_PTPAMPMEANABS]) out[n++] = peakDiff;
    if (_options.enab_output[FUNCTIONAL_PEAKS2][FUNCT_PTPAMPMEANREL]) {
      if (range != 0.0) {
        out[n++]=peakDiff/range;
      } else {
        out[n++]=peakDiff;
      }
    }
    if (_options.enab_output[FUNCTIONAL_PEAKS2][FUNCT_PTPAMPSTDDEVABS]) out[n++] = peakStddevDiff;
    if (_options.enab_output[FUNCTIONAL_PEAKS2][FUNCT_PTPAMPSTDDEVREL]) {
      if (range != 0.0) {
        out[n++]=peakStddevDiff/range;
      } else {
        out[n++]=peakStddevDiff;
      }
    }

    if (_options.enab_output[FUNCTIONAL_PEAKS2][FUNCT_MINRANGEABS]) out[n++]=minMax-minMin;
    if (_options.enab_output[FUNCTIONAL_PEAKS2][FUNCT_MINRANGEREL]) {
      if (range != 0.0) {
        out[n++]=(FLOAT_DMEM)fabs( (minMax-minMin)/range );
      } else {
        out[n++]=minMax-minMin;
      }
    }
    if (_options.enab_output[FUNCTIONAL_PEAKS2][FUNCT_MINMEAN]) out[n++]=minMean;
    if (_options.enab_output[FUNCTIONAL_PEAKS2][FUNCT_MINMEANMEANDIST]) {
      out[n++]= mean - minMean;
    }
    if (_options.enab_output[FUNCTIONAL_PEAKS2][FUNCT_MINMEANMEANRATIO]) {
      if (mean != 0.0) {
        out[n++]=minMean/mean;
      } else {
        out[n++]=minMean;
      }
    }
    if (_options.enab_output[FUNCTIONAL_PEAKS2][FUNCT_MTMAMPMEANABS]) out[n++] = minDiff;
    if (_options.enab_output[FUNCTIONAL_PEAKS2][FUNCT_MTMAMPMEANREL]) {
      if (range != 0.0) {
        out[n++]=minDiff/range;
      } else {
        out[n++]=minDiff;
      }
    }
    if (_options.enab_output[FUNCTIONAL_PEAKS2][FUNCT_MTMAMPSTDDEVABS]) out[n++] = minStddevDiff;
    if (_options.enab_output[FUNCTIONAL_PEAKS2][FUNCT_MTMAMPSTDDEVREL]) {
      if (range != 0.0) {
        out[n++]=minStddevDiff/range;
      } else {
        out[n++]=minStddevDiff;
      }
    }


    if (_options.enab_output[FUNCTIONAL_PEAKS2][FUNCT_MEANRISINGSLOPE]) out[n++] = meanRisingSlope;
    if (_options.enab_output[FUNCTIONAL_PEAKS2][FUNCT_MAXRISINGSLOPE]) out[n++] = maxRisingSlope;
    if (_options.enab_output[FUNCTIONAL_PEAKS2][FUNCT_MINRISINGSLOPE]) out[n++] = minRisingSlope;
    if (_options.enab_output[FUNCTIONAL_PEAKS2][FUNCT_STDDEVRISINGSLOPE]) out[n++] = stddevRisingSlope;

    if (_options.enab_output[FUNCTIONAL_PEAKS2][FUNCT_MEANFALLINGSLOPE]) out[n++] = meanFallingSlope;
    if (_options.enab_output[FUNCTIONAL_PEAKS2][FUNCT_MAXFALLINGSLOPE]) out[n++] = maxFallingSlope;
    if (_options.enab_output[FUNCTIONAL_PEAKS2][FUNCT_MINFALLINGSLOPE]) out[n++] = minFallingSlope;
    if (_options.enab_output[FUNCTIONAL_PEAKS2][FUNCT_STDDEVFALLINGSLOPE]) out[n++] = stddevFallingSlope;

	if(_options.mmlistFirst != NULL)
		free(_options.mmlistFirst);
	
	if(_options.mmlistLast != NULL)
		free(_options.mmlistLast);


    return n;


  }
  return 0;

}

void OSFunctionals::processOneDimension(ssi_size_t NSamples, ssi_real_t* data_in, ssi_real_t* data_out) {
	
	// following code taken from openSMILE 1.0.1, energy.cpp
	// http://opensmile.sourceforge.net/
	
	long i; int ok=0; long NN = NSamples;
	FLOAT_DMEM * unsorted = data_in;
	FLOAT_DMEM * sorted = NULL;
  
	if (_options.nonZeroFunct) {
    NN = 0;
	unsorted = (FLOAT_DMEM*)malloc(sizeof(FLOAT_DMEM)*NSamples);
	if (_options.nonZeroFunct == 2) {
		for (i=0; i<(int) NSamples; i++) {
        if (data_in[i] > 0.0) unsorted[NN++] = data_in[i];
      }
    } else {
      for (i=0; i<(int) NSamples; i++) {
        if (data_in[i] != 0.0) unsorted[NN++] = data_in[i];
      }
    }
  }

  // requireSorted is assumed to be true

    sorted = (FLOAT_DMEM*)malloc( sizeof(FLOAT_DMEM)*NN );
    // quicksort:
    memcpy( sorted, unsorted, sizeof(FLOAT_DMEM) * NN );
    // TODO: check for float_dmem::: with #if ...
    #if FLOAT_DMEM_NUM == FLOAT_DMEM_FLOAT
    smileUtil_quickSort_float( sorted, NN );
    #else
    smileUtil_quickSort_double( sorted, NN );
    #endif
  

  // find max and min value, also compute arithmetic mean
  
  FLOAT_DMEM *x=unsorted;
  FLOAT_DMEM min=*x,max=*x;
  double mean=*x;
  FLOAT_DMEM *xE = unsorted+NN;
  while (++x<xE) {
    if (*x<min) min=*x;
    if (*x>max) max=*x;
    mean += (double)*x;
  } mean /= (double)NN;
  
  
  Variables vs;
  vs.in = unsorted;
  vs.inSorted = sorted;
  vs.min = min;
  vs.max = max;
  vs.mean = (FLOAT_DMEM)mean;
  vs.out = data_out;
  vs.Nin = NN;

  for(int i=0; i<FUNCT_ENAB_N ; i++){
	
	int ret;
	int choice;
	
	if(!_options.enab_funct[i]) continue;
	
	choice = i;

	switch(i) {
    
	case FUNCTIONAL_CROSSINGS	: ret = Crossings(vs);break;
	case FUNCTIONAL_DCT			: ret = Dct(vs);break;
	case FUNCTIONAL_SAMPLES		: ret = Samples(vs);break;
	case FUNCTIONAL_SEGMENTS	: ret = Segments(vs);break;
	case FUNCTIONAL_TIMES		: ret = Times(vs);break;
	case FUNCTIONAL_EXTREMES	: ret = Extremes(vs);break;
	case FUNCTIONAL_MEANS		: ret = Means(vs);break;
	case FUNCTIONAL_ONSET		: ret = Onset(vs);break;
	case FUNCTIONAL_PEAKS		: ret = Peaks(vs);break;
	case FUNCTIONAL_PERCENTILES : ret = Percentiles(vs);break;
	case FUNCTIONAL_REGRESSION	: ret = Regression(vs);break;
	case FUNCTIONAL_MOMENTS		: ret = Moments(vs);break;
	case FUNCTIONAL_PEAKS2      : ret = Peaks2(vs);break;
	
	}

	if (ret < functN[i]) {
        int j;
		for (j=ret; j<functN[i]; j++) vs.out[j] = 0.0;
      }
    
	if (ret>0) ok++;
  	  vs.out += functN[i];
    
	}


  // free memory
  	free(sorted);
  
	if (_options.nonZeroFunct) {
		free(unsorted);
  }
  
}

void OSFunctionals::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	if (stream_in.num <= 0) {
	ssi_wrn("Wrong Stream size !!");
    return;

	}

#ifdef PRINT_SAMPLE_NUMBER_DIMENSION

	ssi_print("Sample No : %i  Dim = %i \n",++counter,stream_out.dim);

#endif
	
	int dim = stream_in.dim;
	int n = stream_in.num;
	ssi_real_t *src = ssi_pcast (ssi_real_t, stream_in.ptr);
	ssi_real_t *dst = ssi_pcast (ssi_real_t, stream_out.ptr);
	ssi_real_t *data_in;
	int nn;
	
	for(int i=0; i<dim ; i++) {

		if(!dim_in[i])
			continue;
		
		data_in = (ssi_real_t*) malloc(sizeof(ssi_real_t)*n);
		nn = 0;

		//ssi_print("Sample No : %i \n",++counter);
		
		for(int j=0 ; j < n; j++) {
			data_in[nn++] = src[(j*dim)+i];			
		}
		//process data_in
		processOneDimension(n,data_in,dst);
		dst += NFeatures;
		
		free (data_in);
				
	}
		// transpose matrix
		//transpose(dst,getOptions()->NFeatures,dim);

}

int* OSFunctionals::ParseIntSamples (const ssi_char_t *indices, int choice) {
	
	if (!indices || indices[0] == '\0') {
		return NULL;
	}

	ssi_char_t string[SSI_MAX_CHAR];
	
	char *pch;
	strcpy (string, indices);
	pch = strtok (string, ", ");
	int index;
	int k = 0;

	std::vector<int> items;
	
	while (pch != NULL) {
		index = (int) atoi(pch);
		if((k%2) == choice || choice == -1) items.push_back(index);
		pch = strtok (NULL, ", ");
		k++;
	}
	
	int* values = (int*)malloc(items.size()*sizeof(int));
	
	for(size_t i = 0; i < items.size(); i++)
		values[i] = items[i];		

	return values;
}

FLOAT_DMEM* OSFunctionals::ParseFloatSamples(const ssi_char_t *indices) {
	
	if (!indices || indices[0] == '\0') {
		return NULL;
	}

	ssi_char_t string[SSI_MAX_CHAR];
	
	char *pch;
	strcpy (string, indices);
	pch = strtok (string, ", ");
	FLOAT_DMEM index;

	std::vector<FLOAT_DMEM> items;
	
	while (pch != NULL) {
		index = (FLOAT_DMEM) atof(pch);
		items.push_back(index);
		pch = strtok (NULL, ", ");
	}
	
	FLOAT_DMEM* values = (FLOAT_DMEM*)malloc(items.size()*sizeof(FLOAT_DMEM));
	
	for(size_t i = 0; i < items.size(); i++)
		values[i] = items[i];		

	return values;
}

int OSFunctionals::countColons(const ssi_char_t *indices) {
	
	int iLength = strlen(indices);
	if(iLength == 0) return 0;
	int count = 1;

	for(int i = 0; i < iLength; i++)
	{
		if(indices[i] == ',')
				count ++;
	}

	return count;

}


void OSFunctionals::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {



	
}

}

