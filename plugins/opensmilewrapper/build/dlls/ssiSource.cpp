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

example dataSource
writes data to data memory...

*/

#include "ssiSource.hpp"

#define MODULE "ssiSource"

SMILECOMPONENT_STATICS(ssiSource)

SMILECOMPONENT_REGCOMP(ssiSource)
{
  SMILECOMPONENT_REGCOMP_INIT
  scname = COMPONENT_NAME_CSSISOURCE;
  sdescription = COMPONENT_DESCRIPTION_CSSISOURCE;

  // we inherit cDataSource configType and extend it:
  SMILECOMPONENT_INHERIT_CONFIGTYPE("cDataSource")
  
  SMILECOMPONENT_IFNOTREGAGAIN(
    ct->setField("numberOfSamples", "The number of values provided by ssi", 1);
	ct->setField("sampleRate", "The sample rate", 16000);
  )

  SMILECOMPONENT_MAKEINFO(ssiSource);
}

SMILECOMPONENT_CREATE(ssiSource)

//-----

ssiSource::ssiSource(const char *_name) :
  cDataSource(_name),
	_dataPointer (0),
	_sampleNum (0),
	_finished (1)
{
}

void ssiSource::fetchConfig()
{
  cDataSource::fetchConfig();
  _sampleRate = getInt("sampleRate");
  _sampleNum = blocksizeW_ = getInt("numberOfSamples");
  allocMat(1, blocksizeW_);
}

void ssiSource::setDataPointer(int n_samples, float *samples)
{
	if (_sampleNum != n_samples) {
		SMILE_ERR (0, "#samples do not fit");	
	}
	if (_finished)
	{
		_dataPointer = samples;
		_finished = 0;
	}
}

int ssiSource::configureWriter(sDmLevelConfig &c)
{
  c.T = 1.0 / (double)_sampleRate;
  return 1;
}

int ssiSource::myConfigureInstance()
{
	return cDataSource::myConfigureInstance();
}

int ssiSource::myFinaliseInstance() {
	return cDataSource::myFinaliseInstance();
}

// NOTE: nEl is always 0 for dataSources....
int ssiSource::setupNewNames(long nEl)
{ 
  writer_->addField("pcm", 1);
  namesAreSet_=1;

  return 1;
}

int ssiSource::myTick(long long t)
{
	// do nothing if data was already written (finished == 1) or
	// if EOI was reached (don't know if this can happen)
	if (_finished || isEOI()) return 0;

	SMILE_DBG(4,"ssiSource: tick # %i, writing value vector",t);

	if (writer_->checkWrite(blocksizeW_)) {
		// write all samples into data memory
		for(int i = 0; i < _sampleNum; i++)
		{
			mat_->setF(0,i, *(_dataPointer + i));
		}

		//static FILE *fp = fopen("check.raw", "wb");
		//fwrite(mat_->dataF, mat_->nT, sizeof(float), fp);

		// save data in dataMemory buffers and check for success
		if (!writer_->setNextMatrix(mat_)) { 
			SMILE_IERR(1,"can't write, level full... (strange, level space was checked using checkWrite(bs=%i)",blocksizeW_);
		} else {
			_finished = 1;

			SMILE_DBG(4, "ssiSource: data is in os now!");

			// success
			return 1;
		}
	}

	// nothing was done
	return 0;
}

ssiSource::~ssiSource()
{
}
