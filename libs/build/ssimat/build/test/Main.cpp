// Main.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/04/23
// Copyright (C) University of Augsburg

#include "ssi.h"
using namespace ssi;

#include "MatlabFile.h"
#include "MatlabSamples.h"
#include <mex.h>

#pragma comment(lib, "libmex.lib")
typedef int (*p_mexCallMATLAB) (int	nlhs, mxArray *plhs[], int nrhs, mxArray *prhs[], const char *fcn_name);

// load libraries
#ifdef _MSC_VER 
#ifdef _DEBUG
#pragma comment(lib, "ssid.lib")
#pragma comment(lib, "ssimatd.lib")
#else
#pragma comment(lib, "ssi.lib")
#pragma comment(lib, "ssimat.lib")
#endif
#endif

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#define new DEBUG_NEW
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

void ex_matfile ();
void ex_matsamples ();
void ex_matscript ();

int main () {	

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	ex_matfile (); 	
	ex_matsamples ();
	ex_matscript ();

	ssi_print ("\n\n\tpress a key to quit\n");
	getchar ();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif
	
	return 0;
}

void ex_matfile () {

	MatlabFile matfile ("test.mat", MatlabFile::UPDATE);
	matfile.print (stdout);
	 
	MatlabVar &var = matfile["var"];
	var.printDouble (stdout);
	double *ptr = ssi_pcast (double, var.getPtr ());
	*ptr = ssi_cast (double, rand ()) / RAND_MAX;
	var.printDouble (stdout);

	double new_data[6] = {1.0f,2.0f,3.0f,4.0f,5.0f,6.0f};
	matfile.addSingleVar ("new_var", 2, 3).write (new_data, sizeof (float), 6);
	matfile["new_var"].printSingle (stdout);

	matfile.flush (); 

	ssi_print ("\n\n\tpress enter to continue\n");
	getchar ();
}

void ex_matsamples () {

	MatlabSamples samples ("samples.mat");
	ModelTools::SaveSampleList (samples, "samples", File::BINARY);
}

/* Subroutine for filling up data */
void fill(double *pr, int *pm, int *pn, int max)
{
  int i;  

  /* You can fill up to max elements, so (*pr) <= max. */
  *pm = max/2;
  *pn = 1;
  for (i = 0; i < (*pm); i++) 
    pr[i] = i * (4*3.14159/max);
}

#define MAX 1000

/* The gateway routine */
void mexFunction()
{
  int     m, n, max = MAX;
  mxArray *rhs[1], *lhs[1];
  
  rhs[0] = mxCreateDoubleMatrix(max, 1, mxREAL);
  
  /* Pass the pointers and let fill() fill up data. */
  fill(mxGetPr(rhs[0]), &m, &n, MAX);
  mxSetM(rhs[0], m);
  mxSetN(rhs[0], n);
  
	HMODULE hDLL = LoadLibrary ("libmex.dll");
	bool succeeded = false;		
	p_mexCallMATLAB fmex = (p_mexCallMATLAB) GetProcAddress (hDLL, "mexCallMATLAB");

  /* Get the sin wave and plot it. */
  fmex(1, lhs, 1, rhs, "sin");
  fmex(0, NULL, 1, lhs, "plot");
  
  /* Clean up allocated memory. */
  mxDestroyArray(rhs[0]);
  mxDestroyArray(lhs[0]);
  
  return;
}

void ex_matscript () {

	mexFunction ();

	ssi_print ("press enter to continue");
	getchar ();

}
