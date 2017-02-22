// Copyright (C) 2003--2004 Johnny Mariethoz (Johnny.Mariethoz@idiap.ch)
//                
// This file is part of Torch 3.1.
//
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. The name of the author may not be used to endorse or promote products
//    derived from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "IOHTK.h"
#include "DiskXFile.h"

namespace Torch {

//several kind of features
static const char *pmkmap[] = {"WAVEFORM", "LPC", "LPREFC", "LPCEPSTRA", 
  "LPDELCEP", "IREFC", 
  "MFCC", "FBANK", "MELSPEC",
  "USER", "DISCRETE", 
  "ANON"};


IOHTK::IOHTK(const char *filename_, bool one_file_is_one_sequence_, int max_load_, bool is_sequential_)
{
  // Boaf...
  one_file_is_one_sequence = one_file_is_one_sequence_;
  max_load = max_load_;
  is_sequential = is_sequential_;

  filename = (char *)allocator->alloc(strlen(filename_)+1);
  header = (HTKHeader *)allocator->alloc(sizeof(HTKHeader));
  strcpy(filename, filename_);

  // Read the file...
  file = new(allocator) DiskXFile(filename, "r");

	// Read the header
  readHeader(file);

	//vector size and number of frames
  n_total_frames = header->n_samples;
  frame_size =  header->sample_size/4;
	
	//some error check
  if(!(n_total_frames >= 0)||!(frame_size>0))
    error("IOHTK: file %s is probably not an HTK file format\n",filename);

  if( (max_load > 0) && (max_load < n_total_frames) && (!one_file_is_one_sequence) )
  {
    n_total_frames = max_load;
    message("IOHTK: loading only %d rows", n_total_frames);
  }

  // Prepare the sequence buffer...
  if(one_file_is_one_sequence)
    n_sequences = 1;
  else
    n_sequences = n_total_frames;
  
  current_frame_index = -1;  
	allocator->free(file);
}

void IOHTK::readHeader(XFile* file_){

  file_->read(&header->n_samples,sizeof(long),1);
  file_->read(&header->sample_period,sizeof(long),1);
  file_->read(&header->sample_size,sizeof(short),1);
  file_->read(&header->sample_kind,sizeof(short),1);

}


void IOHTK::getSequence(int t, Sequence* sequence)
{
  // Cas simple: on lit tout le bordel
  if(one_file_is_one_sequence)
  {
    file = new(allocator) DiskXFile(filename, "r");
		readHeader(file);

#ifdef USE_DOUBLE
		float* temp = (float*)allocator->alloc(sizeof(float)*frame_size);
    for(int i = 0; i < n_total_frames; i++){
			file->read(temp, sizeof(float),frame_size);
			for(int j = 0; j < frame_size; j++)
				sequence->frames[i][j] = temp[j];
		}
		allocator->free(temp);
#else
    for(int i = 0; i < n_total_frames; i++)
      file->read(sequence->frames[i], sizeof(real), frame_size);
#endif
    allocator->free(file);
  }
  else
  {
    // Sequentiel ?
    if(is_sequential)
    {
      if(t != current_frame_index+1)
        error("IOBin: sorry, data are accessible only in a sequential way");
      
      // Doit-on ouvrir le putain de fichier ?
      if(current_frame_index < 0)
      {
        file = new(allocator) DiskXFile(filename, "r");
				readHeader(file);
      }
    }
    else
    {
      file = new(allocator) DiskXFile(filename, "r");
      if(file->seek(t*frame_size*sizeof(real)+2*sizeof(long)+2*sizeof(short), SEEK_CUR) != 0)
        error("IOBin: cannot seek in your file!");
    }

    // Lis la frame mec
#ifdef USE_DOUBLE
		float* temp = (float*)allocator->alloc(sizeof(float)*frame_size);
			file->read(temp, sizeof(float),frame_size);
			for(int j = 0; j < frame_size; j++)
				sequence->frames[0][j] = temp[j];
		allocator->free(temp);
#else
    file->read(sequence->frames[0], sizeof(real), frame_size);
#endif

    if(is_sequential)
    {
      // Si je suis a la fin du fichier, je le zigouille.
      current_frame_index++;
      if(current_frame_index == n_total_frames-1)
      {
        allocator->free(file);
        current_frame_index = -1;
      }
    }
    else
      allocator->free(file);
  }

}

int IOHTK::getNumberOfFrames(int t)
{
  if(one_file_is_one_sequence)
    return n_total_frames;
  else
    return 1;
}

int IOHTK::getTotalNumberOfFrames()
{
  return n_total_frames;
}


void IOHTK::saveSequence(XFile *file, Sequence* sequence, HTKHeader* header_)
{
		file->write(&header_->n_samples,sizeof(long),1);
		file->write(&header_->sample_period,sizeof(long),1);
		file->write(&header_->sample_size,sizeof(short),1);
		file->write(&header_->sample_kind,sizeof(short),1);
#ifdef USE_DOUBLE
		float* temp = (float*)Allocator::sysAlloc(sizeof(float)*sequence->frame_size);
			for(int i = 0; i < sequence->n_frames; i++){
				for(int j = 0; j < sequence->frame_size; j++)
					temp[j] = (float)sequence->frames[i][j];
				file->write(temp, sizeof(float), sequence->frame_size);
			}
		free(temp);
#else
  for(int i = 0; i < sequence->n_frames; i++)
    file->write(sequence->frames[i], sizeof(real), sequence->frame_size);
#endif
}


IOHTK::~IOHTK()
{
}

/******************* HTK source code **********************/

char* IOHTK::parmKind2Str(ParmKind the_kind, char* buf)
{
  strcpy(buf,pmkmap[baseParmKind(the_kind)]);
  if (hasEnergy(the_kind))    strcat(buf,"_E");
  if (hasDelta(the_kind))     strcat(buf,"_D");
  if (hasNulle(the_kind))     strcat(buf,"_N");
  if (hasAccs(the_kind))      strcat(buf,"_A");
  if (hasCompx(the_kind))     strcat(buf,"_C");
  if (hasCrcc(the_kind))      strcat(buf,"_K");
  if (hasZerom(the_kind))     strcat(buf,"_Z");
  if (hasZeroc(the_kind))     strcat(buf,"_0");
  if (hasVQ(the_kind))        strcat(buf,"_V");
  return buf;
}

ParmKind IOHTK::str2ParmKind(char *str)
{
  ParmKind i = -1;
  char *s,buf[255];
  bool hasE,hasD,hasN,hasA,hasC,hasK,hasZ,has0,hasV,found;
  int len;

  hasV=hasE=hasD=hasN=hasA=hasC=hasK=hasZ=has0=false;
  strcpy(buf,str);len=strlen(buf);
  s=buf+len-2;
  while (len>2 && *s=='_') {
    switch(*(s+1)){
      case 'E': hasE = true;break;
      case 'D': hasD = true;break;
      case 'N': hasN = true;break;
      case 'A': hasA = true;break;
      case 'C': hasC = true;break;
      case 'K': hasK = true;break;
      case 'Z': hasZ = true;break;
      case '0': has0 = true;break;
      case 'V': hasV = true;break;
      default: error("str2ParmKind: unknown ParmKind qualifier %s",str);
               exit (-1);
    }
    *s = '\0';len -= 2;s -= 2;
  }
  found = false;
  do {
    s=(char*)pmkmap[++i];
    if (strcmp(buf,s) == 0) {
      found = true;
      break;
    }
  } while (strcmp("ANON",s)!=0);
  if (!found)
    return ANON;
  if (i == LPDELCEP)         /* for backward compatibility with V1.2 */
    i = LPCEPSTRA | HASDELTA;
  if (hasE) i |= HASENERGY;
  if (hasD) i |= HASDELTA;
  if (hasN) i |= HASNULLE;
  if (hasA) i |= HASACCS;
  if (hasK) i |= HASCRCC;
  if (hasC) i |= HASCOMPX;
  if (hasZ) i |= HASZEROM;
  if (has0) i |= HASZEROC;
  if (hasV) i |= HASVQ;
  return i;
}

ParmKind IOHTK::baseParmKind(ParmKind k) { return k & BASEMASK;}

/* EXPORT->HasXXXX: returns true if XXXX included in ParmKind */
bool IOHTK::hasEnergy(ParmKind k){return (k & HASENERGY) != 0;}
bool IOHTK::hasDelta(ParmKind k) {return (k & HASDELTA) != 0;}
bool IOHTK::hasAccs(ParmKind k)  {return (k & HASACCS) != 0;}
bool IOHTK::hasNulle(ParmKind k) {return (k & HASNULLE) != 0;}
bool IOHTK::hasCompx(ParmKind k) {return (k & HASCOMPX) != 0;}
bool IOHTK::hasCrcc(ParmKind k)  {return (k & HASCRCC) != 0;}
bool IOHTK::hasZerom(ParmKind k) {return (k & HASZEROM) != 0;}
bool IOHTK::hasZeroc(ParmKind k) {return (k & HASZEROC) != 0;}
bool IOHTK::hasVQ(ParmKind k)    {return (k & HASVQ) != 0;}

}
