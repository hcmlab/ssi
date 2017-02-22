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

/* openSMILE plugin loader */

#include <core/smileCommon.hpp>
#include <core/smileLogger.hpp>
#include <core/componentManager.hpp>


//++ include all your component headers here: ----------------
#include "ssiSink.hpp"
#include "ssiSource.hpp"


//++ ---------------------------------------------------------

// dll export for msvc++
#ifdef _MSC_VER 
#define DLLEXPORT_P __declspec(dllexport)
#else
#define DLLEXPORT_P
#endif


#define MODULE "pluginLoader"

static DLLLOCAL const registerFunction complist[] = {
//++ add your component register functions here: -------------
  ssiSource::registerComponent,
  ssiSink::registerComponent,


//++ ---------------------------------------------------------
  NULL  // last element must always be NULL to terminate the list !!
};

//Library:
extern "C" DLLEXPORT_P sComponentInfo * registerPluginComponent(cConfigManager *_confman, cComponentManager *_compman) {
  registerFunction f;
  sComponentInfo *master=NULL;
  sComponentInfo *cur = NULL, *last = NULL;
  int i=0;
  f = complist[i];
  while (f!=NULL) {
    cur = (f)(_confman, _compman);
	cur->next = NULL;
    if (i==0) master = cur;
    else { last->next = cur; }
    last = cur;
    f=complist[++i];
  }
  return master;
}



