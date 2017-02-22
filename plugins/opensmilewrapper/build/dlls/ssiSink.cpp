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


/*  openSMILE component:

example dataSink:
reads data from data memory and outputs it to console/logfile (via smileLogger)
this component is also useful for debugging

*/


#include "ssiSink.hpp"

#define MODULE "ssiSink"


SMILECOMPONENT_STATICS(ssiSink)

SMILECOMPONENT_REGCOMP(ssiSink)
{
  SMILECOMPONENT_REGCOMP_INIT

  scname = COMPONENT_NAME_SSISINK;
  sdescription = COMPONENT_DESCRIPTION_SSISINK;

  // we inherit cDataSink configType and extend it:
  SMILECOMPONENT_INHERIT_CONFIGTYPE("cDataSink")
  
  SMILECOMPONENT_IFNOTREGAGAIN(
    ct->setField("filename","The name of a text file to dump values to (this file will be overwritten, if it exists)",(const char *)NULL);
    ct->setField("lag","Output data <lag> frames behind",0,0,0);
  )

  SMILECOMPONENT_MAKEINFO(ssiSink);
}

SMILECOMPONENT_CREATE(ssiSink)

//-----

ssiSink::ssiSink(const char *_name) :
  cDataSink(_name),
  _dataPointer(0),
  _sampleNum(0),
  _sampleNumMax(0)
{
}

void ssiSink::fetchConfig()
{
  cDataSink::fetchConfig();
}

int ssiSink::myFinaliseInstance()
{
  int ret = cDataSink::myFinaliseInstance();
  return ret;
}

int ssiSink::myTick(long long t)
{
  SMILE_DBG(4,"ssiSink: tick # %i, reading value vector:",t);
  cVector *vec= reader_->getFrameRel(0);  // 0 = most recent frame
  
  // no data available
  if (vec == NULL) return 0;

  int i;
  for (i=0; i<vec->N; i++) {
	_dataPointer[i] = vec->dataF[i];
  }

  SMILE_DBG(4,"ssiSink: data is back in ssi now!");

  return 1;
}

void ssiSink::setDataPointer(int n_samples, float *samples)
{	
	_sampleNum = n_samples;
	_dataPointer = samples;
}

int ssiSink::getNumberOfOutputValues()
{
	// which is the correct one?
	//int nf = reader_->getLevelNf();
	//int n = reader_->getLevelN();

	return reader_->getLevelN();
}

ssiSink::~ssiSink()
{
}

