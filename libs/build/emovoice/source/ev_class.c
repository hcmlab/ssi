// ev_class.c
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

#include "ev_memory.h"
#include "ev_messages.h"

#include "ev_class.h"

// @begin_change_johannes
// replace class by class_ind
// @end_change_johannes

char *_type2name(cType cl_type);

int classify(classifier_t *cl, cType cl_type, mx_real_t *instance) {

    switch(cl_type) {
	case naive_bayes:
	    return nB_classify(cl->naiveBayes,instance);
	case svm:
	    return svm_classify(cl->svm,instance);
	default:
	    printf("\n");
	    rs_error("Cannot classify: unknown classifier type %s",_type2name(cl_type));
	}

    return -1;
}

int cl_get_feature_dim(classifier_t *cl, cType cl_type) {
    switch(cl_type) {
	case naive_bayes:
	    return cl->naiveBayes->feature_dim;
	case svm:
	    return cl->svm->feature_dim;
	default:
	    rs_error("Cannot give feature dimension of unknown classifier type %s",_type2name(cl_type));
    }
    return -1;
} 

int cl_get_n_classes(classifier_t *cl, cType cl_type) {
    switch(cl_type) {
	case naive_bayes:
	    return cl->naiveBayes->n_classes;
	case svm:
	    return cl->svm->n_classes;
	default:
	    rs_error("Cannot give number of classes of unknown classifier type %s",_type2name(cl_type));
    }
    return -1;
} 

int cl_set_feature_dim(classifier_t *cl, cType cl_type, int new_dim){
    switch(cl_type) {
	case naive_bayes:
	    cl->naiveBayes->feature_dim=new_dim;
	    return cl->naiveBayes->feature_dim;
	case svm:
	    cl->svm->feature_dim=new_dim;
	    return cl->svm->feature_dim;
	default:
	    rs_error("Cannot set feature dimension of unknown classifier type %s",_type2name(cl_type));
    }
    return -1;
} 

char *cl_get_mapping(classifier_t *cl, cType cl_type, int class_ind) {
    switch (cl_type) {
	case naive_bayes:
	    if (class_ind >=0 && class_ind < cl->naiveBayes->n_classes)
		return cl->naiveBayes->mapping[class_ind];
	    else
		rs_error("Class index %d does not exist!",class_ind);
	case svm:
	    if (class_ind >=0 && class_ind < cl->svm->n_classes)
		return cl->svm->mapping[class_ind];
	    else
		rs_error("Class index %d does not exist!",class_ind);
	default:
	    rs_error("Cannot map class names of unknown classifier type %s",_type2name(cl_type));
    }
    return NULL;
} 

char **cl_get_class_map(classifier_t *cl, cType cl_type) {

    switch (cl_type) {
	case (naive_bayes):
	    return cl->naiveBayes->mapping;
	case (svm):
	    return cl->svm->mapping;
	default:
	    rs_error("Cannot get class mapping of unknown classifier type %s",_type2name(cl_type));
    }
    return NULL;
} 

void cl_destroy(classifier_t *cl, cType cl_type)  {
    switch (cl_type) {
	case (naive_bayes):
	    nB_destroy(cl->naiveBayes);
	    break;
	case (svm):
	    svm_destroy(cl->svm);
	    break;
	default:
	    rs_error("Cannot destroy unknown classifier type %s",_type2name(cl_type));
    }
    rs_free(cl);
    
} 


classifier_t *cl_update_classifier(classifier_t *cl, cType cl_type, int class_ind, mx_real_t *features) {
    switch (cl_type) {
	case naive_bayes:
	    nB_update_classifier(cl->naiveBayes,class_ind,features);
	    break;
	case svm:
	    svm_update_classifier(cl->svm,class_ind,features);
	    break;
	default:
	    rs_error("Cannot update unknown classifier type %s",_type2name(cl_type));
    }
    return cl;
}

classifier_t *cl_fscan(classifier_t *cl, cType cl_type, char *filename, fx_select_t *sel) {
    switch (cl_type) {
	case naive_bayes:
	    cl->naiveBayes=nB_fscan(cl->naiveBayes,filename,sel);
	    break;
	case svm:
	    cl->svm=svm_fscan(cl->svm,filename,sel);
	    break;
	default:
	    rs_error("Cannot update unknown classifier type %s",_type2name(cl_type));
    }
    return cl;
}

int cl_classify_file(classifier_t *cl, cType cl_type, char *filename, fx_select_t *sel, int **evaluation) {
	int n_instances=0;
	
    switch (cl_type) {
	case naive_bayes:
	    n_instances=nB_classify_file(cl->naiveBayes,filename,sel,evaluation);
	    break;
	case svm:
	    n_instances=svm_classify_file(cl->svm,filename,sel,evaluation);
	    break;
	default:
	    rs_error("Cannot update unknown classifier type %s",_type2name(cl_type));
    }
    return n_instances;
}

classifier_t *cl_finish_classifier(classifier_t *cl, cType cl_type) {
    switch (cl_type) {
	case naive_bayes:
	    nB_finish_classifier(cl->naiveBayes);
	    break;
	case svm:
	    svm_finish_classifier(cl->svm);
	    break;
	default:
	    rs_error("Cannot finish unknown classifier type %s",_type2name(cl_type));
    }
    return cl;
}


classifier_t *cl_new_classifier(cType cl_type,int n_classes,int n_features, char *param) {
    classifier_t *classifier;
    
    classifier = (classifier_t *) rs_malloc(sizeof(classifier_t),"Classifier");
    
    switch (cl_type) {
	case (naive_bayes): 
	    classifier->naiveBayes = (naive_bayes_classifier_t *) nB_new_classifier(n_classes,n_features);
	    if (!classifier->naiveBayes)
			rs_error("Naive Bayes classifier could not be built!");
	    break;
	case (svm): 
	    classifier->svm = (svm_classifier_t *) svm_new_classifier(n_classes,n_features,param);
	    if (!classifier->svm)
		rs_error("SVM classifier could not be built!");
	    break;	    
	default:
	    rs_error("Cannot create classifier: type %s not available!",_type2name(cl_type));
    }
    
    return classifier;
}

classifier_t *cl_create_classifier(char *filename, cType cl_type) {
    classifier_t *classifier;

    classifier = (classifier_t *) rs_malloc(sizeof(classifier_t),"Classifier");
    
    switch (cl_type) {
	case (naive_bayes) :
	    classifier->naiveBayes = (naive_bayes_classifier_t *) nB_fread_classifier(filename);
	    break;
	case (svm):
	    classifier->svm = (svm_classifier_t *) svm_fread_classifier(filename);
	    break;
	default:
	    rs_error("Cannot create/read unknown classifier type: %s",_type2name(cl_type));
    }

    return classifier;
}

void cl_output_classifier(classifier_t *cl, cType type, char *outfile) {

    switch (type) {
	case (naive_bayes): 
	    nB_output_classifier(cl->naiveBayes,outfile);
	  break;
	case (svm): 
	    svm_output_classifier(cl->svm,outfile);
	    break;
	default:
	    rs_error("Cannot output classifier: type %s not available!",_type2name(type));
    }
}

cType cl_name2type(char *base_type_name) {

    if (strcmp(base_type_name,"naive_bayes")==0)
	return naive_bayes;
    else
	    if (strcmp(base_type_name,"svm")==0)
		return svm;
	    else
		rs_error("Cannot map unknown classifier type to name: %s",base_type_name);
    return unknown;
}


char *_type2name(cType cl_type) {
    switch (cl_type) {
	case naive_bayes:
	    return "naive_bayes";
	case svm:
	    return "svm";
	default:
	    rs_error("Cannot map unknown classifier type %d to name!",cl_type);
    }
    return NULL;
}


mx_real_t class_prob(classifier_t *cl, cType cl_type, mx_real_t *instance, int class_ind) {
    switch (cl_type) {
	case naive_bayes:
	    return nB_class_prob(cl->naiveBayes,instance,class_ind);
	case svm:
	    return svm_class_prob(cl->svm,instance,class_ind);
	default:
	    rs_error("Cannot give class probability of unknown classifier type %s",_type2name(cl_type));
    }
    return -1;
}


int cl_name2number(char ***mapping,char* class_name,int n_classes) {
    int i=0;
    char **_mapping=*mapping;

    while ((i<n_classes) && _mapping[i]!=NULL) {
		if (strcmp(_mapping[i],class_name)==0)
		    return i;	
		i++;
    }

    if (i>=n_classes) {
    	int j;
    	rs_msg("File contains more classes than specified!");
    	rs_msg("Number of classes is %d",n_classes);
    	rs_msg("Trying to add a class named \"%s\" to the following class set:",class_name);
    	for (j=0;j<n_classes-1;j++)
    		rs_msg("%s, ",_mapping[j]);
		if (n_classes >0)
	    	rs_error("%s\n",_mapping[n_classes-1]);
    }

	_mapping[i]=rs_malloc((strlen(class_name)+1)*sizeof(char),"a class name");
    strcpy(_mapping[i],class_name);
    return i;
}

double cl_eval(int *evaluation, int n_classes, int n_instances) {
	int *n_correct = (int *) rs_calloc(n_classes,sizeof(int),"correct instances");
	int *n_total = (int *) rs_calloc(n_classes,sizeof(int),"total number of instances");
	int i, n_occ_classes=0;
	double classwiseAcc=0;

	for (i=0; i<n_instances*2;i+=2) {
		if (evaluation[i] <0 || evaluation[i] >=n_classes)
			rs_error("Undefined class index: %d",evaluation[i]);
		if (evaluation[i+1] <0 || evaluation[i+1] >=n_classes)
			rs_error("Undefined class index: %d",evaluation[i+1]);
			
		n_total[evaluation[i]]++;
		if (evaluation[i]==evaluation[i+1])
			n_correct[evaluation[i]]++;
	}
		
	for (i=0;i<n_classes;i++)
		if (n_total[i]>0) {
			classwiseAcc+=1.0*n_correct[i]/n_total[i];
			n_occ_classes++;
		}
	
	if (n_occ_classes>0)	
		classwiseAcc/=n_occ_classes;
	else
		classwiseAcc=0;
	

	rs_free(n_correct);
	rs_free(n_total);
	
	return classwiseAcc;
}


char *cl_eval_stats(int *evaluation, int n_classes, int n_instances, char **mapping) {
	int *n_correct = (int *) rs_calloc(n_classes,sizeof(int),"correct instances");
	int *n_total = (int *) rs_calloc(n_classes,sizeof(int),"total number of instances");
	int **confusion_matrix= (int **) rs_malloc(n_classes*sizeof(int*),"confusion matrix row");
	int i, n_occ_classes=0, n_corr_instances=0, tab_len=8, max_len=0, len;
	double classwiseAcc=0;
	char *result = (char *) rs_malloc(20000*sizeof(char),"result string");
	char temp[STRING_LENGTH];
    
    for (i=0;i<n_classes;i++)
    	confusion_matrix[i]= (int *) rs_calloc(n_classes,sizeof(int),"confusion matrix column");
	
	for (i=0; i<n_instances*2;i+=2) {
		if (evaluation[i] <0 || evaluation[i] >=n_classes)
			rs_error("Undefined class index: %d",evaluation[i]);
		if (evaluation[i+1] <0 || evaluation[i+1] >=n_classes)
			rs_error("Undefined class index: %d",evaluation[i+1]);
			
		n_total[evaluation[i]]++;
		confusion_matrix[evaluation[i]][evaluation[i+1]]++;
		if (evaluation[i]==evaluation[i+1])
			n_correct[evaluation[i]]++;
	}
		
	for (i=0;i<n_classes;i++)
		if (n_total[i]>0) {
			classwiseAcc+=1.0*n_correct[i]/n_total[i];
			n_occ_classes++;
		}
			
	if (n_occ_classes>0)	
		classwiseAcc/=n_occ_classes;
	else
		classwiseAcc=0;

	for (i=0;i<n_classes;i++)
		n_corr_instances+=n_correct[i];		
		
	sprintf(temp,"Total number of instances: %d\n",n_instances);
	result=strcat(result,temp);
	sprintf(temp,"Correctly classified instances: %d\n",n_corr_instances);
	result=strcat(result,temp);
	sprintf(temp,"Incorrectly classified instances: %d\n",n_instances-n_corr_instances);
	result=strcat(result,temp);
	sprintf(temp,"Overall recognition accuracy: %.4g %%\n",100.0* n_corr_instances/n_instances);
	result=strcat(result,temp);
	sprintf(temp,"Class-wise recognition accuracy: %.4g %%\n\n",100*classwiseAcc);
	result=strcat(result,temp);
	
	result=strcat(result,"TP rate\t\t# instances\tClass\n");
	for (i=0;i<n_classes;i++){
		sprintf(temp,"%.4g %%\t\t%d\t\t%s\n",100.0*n_correct[i]/n_total[i],n_total[i],mapping[i]);
		result=strcat(result,temp);
	}
		
	result=strcat(result,"\nConfusion matrix:\n");
	for (i=0;i<n_classes;i++) {
		len=strlen(mapping[i]);
		if (len>max_len)
			max_len=strlen(mapping[i]);
	}
	for (i=1;i<=max_len/tab_len+1;i++)
		result=strcat(result,"\t");
	for (i=0;i<n_classes;i++) {
		if (strlen(mapping[i]) <=7) {
			sprintf(temp,"%s\t",mapping[i]);
			result=strcat(result,temp);
		}
		else {
			sprintf(temp,"%c%c%c..%c%c\t",mapping[i][0],mapping[i][1],mapping[i][2],mapping[i][strlen(mapping[i])-2],mapping[i][strlen(mapping[i])-1]);
			result=strcat(result,temp);
		}
	}

	result=strcat(result,"\n");
	
	for (i=0;i<n_classes; i++) {
		int j;
		sprintf(temp,"%s",mapping[i]);
		result=strcat(result,temp);
		len=strlen(mapping[i]);
		for (j=0;j<max_len/tab_len+1-len/tab_len;j++)
			result=strcat(result,"\t");
		for (j=0;j<n_classes;j++) {
			sprintf(temp,"%d\t",confusion_matrix[i][j]);
			result=strcat(result,temp);
		}
		sprintf(temp,"\n");
		result=strcat(result,temp);
	}
	sprintf(temp,"\n");
	result=strcat(result,temp);
	
	for (i=0;i<n_classes;i++)
		rs_free(confusion_matrix[i]);
	rs_free(confusion_matrix);
	rs_free(n_correct);
	rs_free(n_total);
	
	return result;
}
