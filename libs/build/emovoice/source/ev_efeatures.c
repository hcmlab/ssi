// ev_efeatures.c
// author: Thurid Vogt <thurid.vogt@informatik.uni-augsburg.de>
// created: 
// Copyright (C) 2003-9 University of Augsburg, Thurid Vogt
//
// *************************************************************************************************
//
// This file is part of EmoVoice/SSI developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#include <string.h>

#include "ev_io.h"
#include "ev_messages.h"
#include "ev_memory.h"

#include "ev_efeatures.h"

int feature_output1(char *filename, mx_real_t *features, int n_features, int binary, char delim, char *class) {
    int i;
    FILE *fp;

    if (strcmp(filename,"-")==0)
	fp=stdout;
    else {
	fp = fopen(filename,"w");
	if (!fp)
	    rs_error("Could not open file %s for writing!",filename);
    }

    if (binary)
	fwrite (features, sizeof(mx_real_t), n_features, fp);
    else {
	for (i=0;i<n_features-1;i++)
	    fprintf(fp,"%g%c",features[i],delim);
	if (class)
	    fprintf(fp,"%g%c%s\n",features[n_features-1],delim,class);
	else
	    fprintf(fp,"%g\n",features[n_features-1]);
    }

    if (strcmp(filename,"-") != 0)
	fclose(fp);
    
    return 1;
}


int feature_append_output1(char *filename, mx_real_t *features, int n_features, int binary, char delim, char *class) {
    int i;
    FILE *fp;

    if (strcmp(filename,"-")==0)
	fp=stdout;
    else {
	fp = fopen(filename,"a");
	if (!fp)
	    rs_error("Could not open file %s for writing!",filename);
    }

    if (binary)
	fwrite (features, sizeof(mx_real_t), n_features, fp);
    else {
	for (i=0;i<n_features-1;i++)
	    fprintf(fp,"%g%c",features[i],delim);
	if (class)
	    fprintf(fp,"%g%c%s\n",features[n_features-1],delim,class);
	else
	    fprintf(fp,"%g\n",features[n_features-1]);
    }

    if (strcmp(filename,"-") != 0)
	fclose(fp);
    
    return 1;
}



int feature_outputN(char *filename,mx_real_t **feature_vector,int n_segments,int n_features,int binary,char delim,char *class_info) {
    char *class=NULL;
    FILE *fp=NULL;
    int i,j;

    if (!filename) {
	rs_warning("No output file was specified!");
	return -1;
    }

    if (strcmp(filename,"-")==0)
	fp=stdout;
    else {
	fp = fopen(filename,"w");
	if (!fp)
	    rs_error("Could not open file %s for writing!",filename);
    }
  
    for (i=0;i<n_segments;i++) {
	if (binary)
	    fwrite(feature_vector[i],sizeof(mx_real_t),n_features,fp);
	else {
	    if (class_info) {
		class = (char *) rs_malloc(STRING_LENGTH * sizeof(char),"class name");
		if (sscanf(class_info,"%[^[]",class) !=1) {
		    rs_warning("Cannot determine class of feature vector %d of file %s!",i+1,filename);
		    rs_free(class);
		    class=NULL;
		}
		else {
		    class_info = strchr(class_info,' ');
		    if (class_info && class_info +1)
			class_info++;
		}
	    }
		

	    for (j=0;j<n_features-1;j++)
		fprintf(fp,"%g%c",feature_vector[i][j],delim);
	    if (class)
		fprintf(fp,"%g%c%s\n",feature_vector[i][n_features-1],delim,class);
	    else
		fprintf(fp,"%g\n",feature_vector[i][n_features-1]);

	    if (class) {
		rs_free(class);
		class=NULL;
	    }
	}
    }
    if (fp != stdout)
	fclose(fp);
    
    return i;
}

void printRawXML(mx_real_t *values, int nvalues, char *type) {
	int i;
	printf("<raw type=\"%s\" num=\"%d\">\n",type,nvalues);
	for (i=0;i<nvalues;i++)
		printf("<r num=\"%d\">%g</r>\n",i+1,values[i]);
	printf("</raw>\n");
}


