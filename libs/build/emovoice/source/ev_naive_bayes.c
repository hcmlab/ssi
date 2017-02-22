// ev_naive_bayes.c
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



#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "ev_io.h"
#include "ev_messages.h"
#include "ev_memory.h"

#define EMO_KERNEL
#include "ev_class.h"

// @begin_add_johannes
#ifndef M_PI
#define M_PI        3.14159265358979323846
#endif
// @end_add_johannes

// @begin_change_johannes
// replace class by class_ind
// @end_change_johannes

char *line_read(FILE *fp, char comment_char);

naive_bayes_classifier_t *nB_fread_classifier(char *model_name) {
    int class_ind, weight_no=0, n_classes, feature_dim, i;
    FILE *model_fp;
    char *line, *token;
    naive_bayes_classifier_t *naiveBayes=NULL;
    
    model_fp = fopen(model_name,"r");
    if (!model_fp)
	rs_error("can't open file %s!",model_name);      

    line = (char *) rs_line_read(model_fp,'#');
    token = (char *) strtok(line,"\t");
    n_classes = atoi(token);
    line += strlen(token)+1;
    feature_dim = atoi((char *) strtok(line,"\t"));
    

    naiveBayes = nB_new_classifier(n_classes,feature_dim);
    
    class_ind=0;
    while ((line= (char *) rs_line_read(model_fp,'#')) && class_ind < n_classes) {
	char *name =(char *) strtok(line,"\t");
	int len=strlen(name)+1;
	naiveBayes->mapping[class_ind] = (char *) rs_malloc(len*sizeof(char),"class name");
	strcpy(naiveBayes->mapping[class_ind],name);
	line+=len;
	naiveBayes->class_probs[class_ind] = (mx_real_t) atof((char *) strtok(line,"\t"));
	
	while (weight_no < feature_dim && fscanf(model_fp, "%g\t%g", &naiveBayes->means[class_ind][weight_no], &naiveBayes->std_dev[class_ind][weight_no]) == 2) 
	    weight_no++;
	if (weight_no<feature_dim-1)
	    rs_error("problem with means or standard devs in file %s!",model_name);
	weight_no=0;
	class_ind++;
    }

    if (class_ind <n_classes-1)
	rs_error("problem with class no %d in file %s!",class_ind,model_name);

    //fprintf(stderr,"Classifying into the following classes:\n");
    //for (i=0;i<n_classes;i++)
	//fprintf(stderr,"%s ",naiveBayes->mapping[i]);
    //fprintf(stderr,"\n");
    
    naiveBayes->finished=1;
    fclose(model_fp);
    return naiveBayes;
}


naive_bayes_classifier_t* nB_new_classifier(int classes, int feature_dim) {
  
  int i;
  naive_bayes_classifier_t *nB = (naive_bayes_classifier_t*) rs_malloc(sizeof(naive_bayes_classifier_t),"Naive Bayes classifier");
  
  nB->type = naive_bayes;
  nB->n_classes = classes;
  nB->feature_dim = feature_dim;
  nB->n_instances = (int *) rs_calloc(classes+1,sizeof(int),"numbers of instances in the classes");
  nB->class_probs = (mx_real_t *) rs_calloc(classes,sizeof(mx_real_t),"prior class probabilities");
  nB->means = (mx_real_t **) rs_malloc(classes * sizeof(mx_real_t*),"means for Naive Bayes classifier");
  for (i=0;i<classes;i++) 
    nB->means[i]= (mx_real_t *) rs_calloc(feature_dim,sizeof(mx_real_t),"means of features per class");
  nB->std_dev = (mx_real_t **) rs_malloc(classes * sizeof(mx_real_t),"standard deviations for Naive Bayes classifier");
  for (i=0;i<classes;i++) 
    nB->std_dev[i]= (mx_real_t *) rs_calloc(feature_dim,sizeof(mx_real_t),"std devs of features per class");
  nB->finished=0;
  nB->mapping = (char**) rs_malloc(classes*sizeof(char*),"class name to number mapping");
  for (i=0;i<classes;i++) 
      nB->mapping[i] = NULL;

  return nB;
}


naive_bayes_classifier_t *nB_update_classifier(naive_bayes_classifier_t *nB, int class_ind, mx_real_t *features) {

  int i;
  mx_real_t temp;

  nB->n_instances[class_ind]++;
  
  if (!nB->finished) {
    for (i=0;i<nB->feature_dim;i++) {
      nB->means[class_ind][i]+=features[i];
      nB->std_dev[class_ind][i]+=features[i]*features[i];
    }
  }
  else {
    for (i=0;i<nB->feature_dim;i++) {
      temp = nB->means[class_ind][i];
      nB->means[class_ind][i]=(nB->means[class_ind][i]*nB->n_instances[class_ind] + features[i]) / (nB->n_instances[class_ind]+ 1.0);
      nB->std_dev[class_ind][i] = sqrt(((mx_sqr(nB->std_dev[class_ind][i]) + temp * temp)*nB->n_instances[class_ind] + features[i]*features[i]) / (nB->n_instances[class_ind]+1.0) - nB->means[class_ind][i]*nB->means[class_ind][i]);
    }
    for (i=0;i<nB->n_classes; i++) {
      nB->class_probs[i]= 1.0*nB->n_instances[i]/(nB->n_instances[nB->n_classes]+1);
    }
  }
  nB->n_instances[nB->n_classes]++;
  return nB;
}


naive_bayes_classifier_t *nB_finish_classifier(naive_bayes_classifier_t *nB) {

    int i,j;
    
    if (!nB->finished) {
	for (j=0;j<nB->n_classes;j++) {
	    if (nB->n_instances[j] <1) 
		rs_warning("creating model for unseen class %s!",nB->mapping[j]);
	    for (i=0;i<nB->feature_dim;i++) {
		if (nB->n_instances[j] <1) {
		    nB->means[j][i] = 0;
		    nB->std_dev[j][i] = 0;
		}
		else {
		    nB->means[j][i]/=(1.0*nB->n_instances[j]);
			// @begin_change_johannes
		    //if (isnan(nB->means[j][i]))
            #if __MINGW32__||__gnu_linux__
            if ((nB->means[j][i])!=(nB->means[j][i]) )
            #else
			if (_isnan(nB->means[j][i]))
            #endif
			// @end_change_johannes
			nB->means[j][i]=0;
		    nB->std_dev[j][i]= sqrt(nB->std_dev[j][i]/(1.0*nB->n_instances[j]) - nB->means[j][i] * nB->means[j][i]);
			// @begin_change_johannes
		    //if (isnan(nB->std_dev[j][i]))
            #if __MINGW32__||__gnu_linux__
            if ((nB->std_dev[j][i])!=(nB->std_dev[j][i]))
            #else
			if (_isnan(nB->std_dev[j][i]))
            #endif
			// @end_change_johannes
			nB->std_dev[j][i]=0;
		}
	    }

	    if (nB->n_instances[nB->n_classes] <1)
		nB->class_probs[j] = 0;
	    else
		nB->class_probs[j] = 1.0*nB->n_instances[j]/nB->n_instances[nB->n_classes];
	}
	nB->finished=1;
    }
    else
	rs_warning("Trying to finish finished classifier!");

    return nB;
}

void nB_output_classifier(naive_bayes_classifier_t *nB, char *filename) {
  int i,j;
  FILE *fp;

  fp = fopen(filename, "w");
  if (!fp)
    rs_error("can't open file %s!",filename);

  fprintf(fp,"# Classifier type:\tnaive_bayes\n# number of classes\tfeature space dimension\n%d\t%d\n\n# class\tprior class probability\n# mean\tstandard deviation\n",nB->n_classes,nB->feature_dim);

  for (i=0;i<nB->n_classes;i++) {
    fprintf(fp,"%s\t%g\n",nB->mapping[i],nB->class_probs[i]);
    for (j=0;j<nB->feature_dim;j++) 
      fprintf(fp,"%g\t%g\n",nB->means[i][j],nB->std_dev[i][j]);
    fprintf(fp,"\n");	     
  }

  fclose(fp);
}

/* Standard normal distribution; often results in 0 for many attributes */
mx_real_t nB_class_prob_simple(naive_bayes_classifier_t *nB, mx_real_t *instance, int class_ind) {
    int j;
    mx_real_t prob=0;
    mx_real_t diff, temp, stddev;
    mx_real_t norm_const=sqrt(2 * M_PI);

    prob = nB->class_probs[class_ind];
    for (j=0; j<nB->feature_dim; j++) {
	diff = instance[j]-nB->means[class_ind][j];
	stddev=nB->std_dev[class_ind][j];
	if (stddev ==0)
	    stddev=MX_REAL_MIN;
	temp = (1 / (norm_const * stddev)) * exp(-(mx_sqr(diff) / (2 * mx_sqr(stddev))));
	prob *= temp;
    }

    return prob;
}

/* log normal distribution; can cope with many attribute, but difficult to normalise */
mx_real_t nB_class_prob(naive_bayes_classifier_t *nB, mx_real_t *instance, int class_ind) {

    int j;
    mx_real_t prob=0, sqr;

    prob = mx_log(nB->class_probs[class_ind]);
    for (j=0; j<nB->feature_dim; j++) {
		// stdev == 0 -> attribute is constant
    	if (nB->std_dev[class_ind][j]==0)
    		continue;
    	sqr = mx_sqr(nB->std_dev[class_ind][j]);
		if (sqr !=0) 
	    	prob+=-mx_log(nB->std_dev[class_ind][j])-mx_sqr(instance[j]-nB->means[class_ind][j])/(2*sqr);
		else 
	    	prob+=-mx_log(nB->std_dev[class_ind][j])-mx_sqr(instance[j]-nB->means[class_ind][j])/(2*MX_REAL_MIN);
	}
    
    // Transformation in vernünftigen Wertebereich
    return exp(prob/nB->feature_dim);
}




int nB_classify (naive_bayes_classifier_t *nB, mx_real_t *instance) {
    
    int i, best_class = 0;
    mx_real_t best_prob = nB_class_prob(nB,instance,0), prob;
    for (i=0;i<nB->n_classes;i++) {
	prob = nB_class_prob(nB,instance,i);
	if (prob > best_prob) {
	    best_prob = prob;
	    best_class = i;
	}
    }

    return best_class;
}


naive_bayes_classifier_t *nB_fscan(naive_bayes_classifier_t *nB, char *filename, fx_select_t *sel) { 
	int class_ind, start, end, i;
    FILE *fp = NULL, *feature_fp = NULL;
    mx_real_t *features=NULL;
    char *line=NULL, *token;
    char *fname =NULL;
    char *class_name = (char *) rs_malloc(STRING_LENGTH*sizeof(char),"String");
    int full_feature_dim=sel?sel->n_features:nB->feature_dim;

    if (!nB)
		rs_error("classifier could not be initialised!");
     
	fp = fopen(filename,"r");
    if (fp == NULL) {
		rs_error("Cannot open file %s!",filename);
    }
    else {
		features = (mx_real_t *) rs_calloc(full_feature_dim, sizeof(mx_real_t), "feature vector");
	 
	 	while ((line= (char *) rs_line_read(fp,';'))) {
	    	fname = (char *) strtok(line,"\t ");
	     	feature_fp = fopen(fname, "r");
	     	if (feature_fp == NULL) {
		 		rs_warning("can't open '%s '!", fname);
		 		while (getchar() != '\n');
		 			continue;
	     	}
	     	line+= strlen(fname)+1;

		    if (feature_fp) {
				for (token = (char *) strtok(line," "); token != NULL; token = (char *) strtok(NULL, " ")) {
		     		if (sscanf(token,"%[^[]s",class_name) !=1 ) 
						rs_error("wrong length information for file %s!",fname);
		     		token += strlen(class_name);
		     		if (sscanf(token,"[%d..%d]",&start,&end) !=2)
			 			rs_error("wrong length information for file %s!",fname);
		     		for (i=start;i<=end;i++) {
			 			if (fread(features,sizeof(mx_real_t),full_feature_dim,feature_fp)!=full_feature_dim)
			     			rs_perror("fread stopped after %d elements in file %s -- %d", i, fname, errno);
			 			else {
							if (sel) 
							    if ( fx_select_apply(&features,sel,features)!= sel->n_features)
									rs_error("Feature selection did not succeed!");
							class_ind = cl_name2number(&(nB->mapping),class_name,nB->n_classes);
							nB=nB_update_classifier(nB,class_ind,features);
			 			}
		     		}
				}
				fclose(feature_fp);
	     	}
		}
		nB_finish_classifier(nB);
	}
    fclose(fp);
    rs_free(class_name);
    rs_free(features);
    
    return nB;
}


void nB_destroy(naive_bayes_classifier_t *nB) {
    int i;

    rs_free(nB->n_instances);
    rs_free(nB->class_probs);
    for (i=0;i<nB->n_classes;i++)
		rs_free(nB->means[i]);
    rs_free(nB->means);
    for (i=0;i<nB->n_classes;i++)
		rs_free(nB->std_dev[i]);
    rs_free(nB->std_dev);
    for (i=0;i<nB->n_classes;i++)
		rs_free(nB->mapping[i]);
    rs_free(nB->mapping);
    rs_free(nB);
}



int nB_classify_file(naive_bayes_classifier_t *nB, char *filename, fx_select_t *sel, int **evaluation) {
	char x;
    int i, class_ind, n_instances=0, eval_size=1000, start,end;
    int *_eval;
    mx_real_t *instance;
    FILE *fp, *feature_fp;
    char *fname = (char *) rs_malloc(STRING_LENGTH*sizeof(char),"String");
    char *class_info = (char *) rs_malloc(STRING_LENGTH*sizeof(char),"String");
    char *class_name = (char *) rs_malloc(STRING_LENGTH*sizeof(char),"String");
    int full_n_features = sel? sel->n_features : nB->feature_dim;

	_eval = (int *) rs_malloc(eval_size*sizeof(int),"evaluation data");

    if (!nB)
		rs_error("Cannot classify without classifier - use -b or -m options to get one");
		
	if (sel)
		if (sel->n_selected != nB->feature_dim)
			rs_error("Wrong dimension of classifier!");
		
    instance = (mx_real_t *) rs_malloc(full_n_features*sizeof(mx_real_t),"instance to classify");
    fp = fopen(filename,"r");
    if (!fp)
		rs_error("cannot open file %s with instances to classify!",filename);
    while (fscanf(fp,"%s%c",fname,&x)==2) {
		feature_fp = fopen(fname,"r");
		if (!feature_fp)
    		rs_error("Cannot open feature file %s!",fname);

		if (x != '\n' && x != EOF ) {
			while (fscanf(fp,"%s",class_info) ==1) {
				if (strcmp(class_info,";")==0)
					break;
				if (sscanf(class_info,"%[^[]s",class_name)!=1)
				    rs_error("Class information for file %s is incorrect!",fname);
				
				if(sscanf(class_info+strlen(class_name),"[%d..%d]",&start,&end) !=2) 
				    rs_error("Time information for file %s is incorrect!",fname);
				
				for (i=start;i<=end;i++) {
					int k;
					fseek(feature_fp,i*sizeof(mx_real_t)*full_n_features,SEEK_SET);
		    		fread(instance,sizeof(mx_real_t)*full_n_features,1,feature_fp);

				    if (sel)
						if (fx_select_apply(&instance,sel,instance) != sel->n_features)
						    rs_error("Feature selection did not succeed!");
		    	    class_ind = nB_classify(nB,instance);
		    	    if ((n_instances+1)*2 > eval_size) {
		    	    	eval_size+=1000;
		    	    	_eval = (int *) rs_realloc(_eval,eval_size*sizeof(int),"evaluation data");
		    	    }
		    	    
		    	    k=cl_name2number(&(nB->mapping),class_name,nB->n_classes);
		    	    _eval[n_instances*2]=cl_name2number(&(nB->mapping),class_name,nB->n_classes);
		    	    _eval[n_instances*2+1]=class_ind;
		    	    n_instances++;
				}
			}
		}	
		else {	
			printf("%s\t",fname);
			i =  fread(instance, sizeof(mx_real_t), full_n_features, feature_fp);
			while (i == full_n_features) {
				if (sel) 
	    			if (fx_select_apply(&instance,sel,instance) != sel->n_features)
						rs_error("Feature selection did not succeed!");
		    	class_ind=nB_classify(nB,instance);
				n_instances++;
	    		printf("%s ",nB->mapping[class_ind]);
	    		i=fread(instance,sizeof(mx_real_t),full_n_features,feature_fp);
			}
			if (i!=0 && i != full_n_features)
			    rs_perror("fread stopped after %d elements -- %d", i, errno);
			printf(";\n");
		}
		fclose(feature_fp);
    }
    fclose(fp);
    
    rs_free(fname);
    rs_free(class_info);
    rs_free(class_name);
    rs_free(instance);
    
    *evaluation=_eval;
    return n_instances;
}


