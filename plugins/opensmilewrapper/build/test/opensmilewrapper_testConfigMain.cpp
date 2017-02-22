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

/*

This is the main commandline application

*/

#include <core/smileCommon.hpp>

#include <core/configManager.hpp>
#include <core/componentManager.hpp>
#include <core/vectorProcessor.hpp>

#define MODULE "openSmileTestExtracter"

/************** Ctrl+C signal handler **/
#include  <signal.h>

cComponentManager *cmanGlob = NULL;

void INThandler(int);
int ctrlc = 0;

void INThandler(int sig)
{
	signal(sig, SIG_IGN);
	if (cmanGlob != NULL) 
		cmanGlob->requestAbort();
	signal(SIGINT, INThandler);
	ctrlc = 1;
}
/*******************************************/


int main(int argc, char *argv[])
{
	const char * configFile = "GenevaMinimalistic.conf";
	
	try {

		// set up the smile logger
		//LOGGER.setLogLevel(2);
		//LOGGER.enableConsoleOutput();

		SMILE_MSG(2,"openSMILE starting!");
		SMILE_MSG(2,"config file is: %s", configFile);

		// create configManager:
		cConfigManager *configManager = new cConfigManager();
		cComponentManager *cMan = new cComponentManager(configManager,componentlist);
		configManager->addReader(new cFileConfigReader(configFile));
		configManager->readConfig();
		cMan->createInstances(0); // 0 = do not read config(we already did that above..)
		cmanGlob = cMan;
		signal(SIGINT, INThandler); // install Ctrl+C signal handler

		/* run single or mutli-threaded, depending on componentManager config in config file */
		long long nTicks = cMan->runMultiThreaded();

		/* it is important that configManager is deleted BEFORE componentManger! 
		(since component Manger unregisters plugin Dlls, which might have allocated configTypes, etc.) */
		delete configManager;
		delete cMan;

	} catch(cSMILException *c) { 
		// free exception ?? 
		return EXIT_ERROR; 
	} 

	if (ctrlc) return EXIT_CTRLC;
	return EXIT_SUCCESS;
}
