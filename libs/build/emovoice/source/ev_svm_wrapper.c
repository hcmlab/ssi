// ev_wrapper.c
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
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
// @begin_change_johannes
//#include <unistd.h>
int mkstemp(char *temp) {
	return 5;
}
// @end_change_johannes

// @begin_change_johannes
// replace class by class_ind
// @end_change_johannes

#include "ev_io.h"
#include "ev_memory.h"
#include "ev_messages.h"

#include "ev_class.h"

#define MAX_INSTANCES 1000
#define SCALE_UPPER 1
#define SCALE_LOWER -1
#define BUFFER_SIZE 1000

#define C_G_ITER 110
const int C[C_G_ITER] = {5 , -1 , 5 , -1 , 11 , 11 , 5 , -1 , 11 , -3 , -3 , -3 , 5 , -1 , 11 , -3 , 9 , 9 , 9 , 9 , 5 , -1 , 11 , -3 , 9 , 3 , 3 , 3 , 3 , 3 , 5 , -1 , 11 , -3 , 9 , 3 , 15 , 15 , 15 , 15 , 15 , 15 , 5 , -1 , 11 , -3 , 9 , 3 , 15 , -5 , -5 , -5 , -5 , -5 , -5 , -5 , 5 , -1 , 11 , -3 , 9 , 3 , 15 , -5 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 5 , -1 , 11 , -3 , 9 , 3 , 15 , -5 , 7 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 5 , -1 , 11 , -3 , 9 , 3 , 15 , -5 , 7 , 1 , 13 , 13 , 13 , 13 , 13 , 13 , 13 , 13 , 13 , 13};
const int G[C_G_ITER] = {-7 , -7 , -1 , -1 , -7 , -1 , -13 , -13 , -13 , -7 , -1 , -13 , 1 , 1 , 1 , 1 , -7 , -1 , -13 , 1 , -11 , -11 , -11 , -11 , -11 , -7 , -1 , -13 , 1 , -11 , -5 , -5 , -5 , -5 , -5 , -5 , -7 , -1 , -13 , 1 , -11 , -5 , -15 , -15 , -15 , -15 , -15 , -15 , -15 , -7 , -1 , -13 , 1 , -11 , -5 , -15 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , -7 , -1 , -13 , 1 , -11 , -5 , -15 , 3 , -9 , -9 , -9 , -9 , -9 , -9 , -9 , -9 , -9 , -7 , -1 , -13 , 1 , -11 , -5 , -15 , 3 , -9 , -3 , -3 , -3 , -3 , -3 , -3 , -3 , -3 , -3 , -3 , -7 , -1 , -13 , 1 , -11 , -5 , -15 , 3 , -9 , -3};


void _set_parameters(struct svm_parameter *svm_param, char *param);
void exit_with_help();
void _create_scaling(struct svm_problem problem, int n_features, double **_max, double **_min);
void _scale_instance(struct svm_node **instance, int n_features, double *max, double *min);

svm_classifier_t* svm_new_classifier(int classes, int feature_dim, char *param) {
  
  int i;
  svm_classifier_t *svm_cl = (svm_classifier_t*) rs_malloc(sizeof(svm_classifier_t),"Naive Bayes classifier");
  
  svm_cl->type = svm;
  svm_cl->n_classes = classes;
  svm_cl->feature_dim = feature_dim;
  svm_cl->finished = 0;
  svm_cl->mapping = (char**) rs_malloc(classes*sizeof(char*),"class name to number mapping");
  for (i=0;i<classes;i++) 
      svm_cl->mapping[i] = NULL;

  svm_cl->max_instances=MAX_INSTANCES;
  svm_cl->max= (double *) rs_malloc(sizeof(double)*feature_dim,"SVM array of maxima");
  svm_cl->min= (double *) rs_malloc(sizeof(double)*feature_dim,"SVM array of minima");
  svm_cl->problem.y = (double *) rs_malloc(sizeof(double)*svm_cl->max_instances,"SVM number of instances");
  svm_cl->problem.x = (struct svm_node **) rs_malloc(sizeof(struct svm_node*)*svm_cl->max_instances,"SVM list of instances");
  svm_cl->problem.l=0;

  _set_parameters(&(svm_cl->param),param);
  if (!svm_cl->param.gamma)
      svm_cl->param.gamma=1.0/svm_cl->feature_dim;

  return svm_cl;
}


svm_classifier_t *svm_update_classifier(svm_classifier_t *svm, int class_ind, mx_real_t *features) {
    int i, j, cur;
    struct svm_problem *prob = &(svm->problem);

	if (svm->finished)
		rs_warning("SVM classifier is being updated after training!");

    cur=prob->l++;
    if (prob->l > svm->max_instances) {
		svm->max_instances += MAX_INSTANCES;
		prob->x = (struct svm_node **) rs_realloc(prob->x,sizeof(struct svm_node *)*svm->max_instances,"SVM list of instances");
		prob->y = (double *) rs_realloc(prob->y,sizeof(double)*svm->max_instances,"SVM number of instances");
    }

	prob->y[cur] = (mx_real_t) class_ind;
    prob->x[cur]= (struct svm_node *) rs_malloc(sizeof(struct svm_node)*(svm->feature_dim+1),"SVM instance");
    for (i=0,j=0;i<svm->feature_dim;i++) {
		if (features[i]) {
			prob->x[cur][j].index=i+1;
			prob->x[cur][j++].value=features[i];
		}
    }
    prob->x[cur][j].index=-1;

    return svm;
}

svm_classifier_t *svm_finish_classifier(svm_classifier_t *svm) {
    int i, j, nr_fold=5, total_correct, best_correct=0, best_c=svm->param.C, best_g=svm->param.gamma;
    const char *error_msg;
    double *result = (double *) rs_malloc(sizeof(double)*svm->problem.l,"cross validation result");

    rs_msg("Building SVM classifier...");

    if (svm->finished)
	rs_warning("SVM classifier is already trained!");

    error_msg = svm_check_parameter(&(svm->problem),&(svm->param));
    
    if(error_msg) 
	rs_error("%s",error_msg);
    
    /* Skalierung */
    _create_scaling(svm->problem,svm->feature_dim,&(svm->max),&(svm->min));
    
    for (i=0;i<svm->problem.l;i++) 
	_scale_instance(&(svm->problem.x[i]),svm->feature_dim,svm->max,svm->min);
    
    /* Cross-Validation, um C und G zu bestimmen bei RBF-Kernel */
    if (svm->param.kernel_type == RBF) {
	svm->param.probability = 0;
	for (i=0;i<C_G_ITER;i++) {
	    total_correct=0;
	    svm->param.C=pow(2,C[i]);
	    svm->param.gamma=pow(2,G[i]);
	    svm_cross_validation(&(svm->problem),&(svm->param),nr_fold,result);
	    for(j=0;j<svm->problem.l;j++) {
		if(result[j] == svm->problem.y[j])
		    ++total_correct;
	    }
	    if (total_correct > best_correct) {
		best_correct=total_correct;
		best_c=C[i];
		best_g=G[i];
	    }
	    rs_msg("C-G-Selektion-Iteration # %d: tried c=%g and g=%g => CV rate is %g; current best c=%g and g=%g with CV rate %g",i+1,pow(2,C[i]),pow(2,G[i]),total_correct*100.0/svm->problem.l,pow(2,best_c),pow(2,best_g),best_correct*100.0/svm->problem.l);
	}
	
	/* Training */
	svm->param.C=pow(2,best_c);
	svm->param.gamma=pow(2,best_g);
	svm->param.probability = 1;
    }
    
    svm->model=svm_train(&(svm->problem),&(svm->param));
    svm->finished=1;

	// @begin_add_johannes
	rs_free (result);
	// @end_add_johannes

    return svm;
}

void svm_output_classifier(svm_classifier_t *svm, char *filename) {
    int i, temp_fd;
    FILE *fp, *temp_fp;
    char *file_content/*, temp[L_tmpnam]*/;
    //char temp[] = "/tmp/svmXXXXXX";
    struct stat attribute;

//    tmpnam(temp);
	/*temp_fd = mkstemp(temp);
	close(temp_fd);
    svm_save_model(temp,svm->model);
    if (stat(temp,&attribute) == -1)
	rs_error("Cannot determine size of temporary svm model file!");
    
    if (!(temp_fp = fopen(temp,"r")))
	rs_error("Cannot open temporary svm model file!");
    file_content = (char *) rs_malloc(attribute.st_size*sizeof(char),"File contents string");
    
    if (fread(file_content,sizeof(char),attribute.st_size,temp_fp) != attribute.st_size)
	rs_error("Could not read file %s correctly!",filename);
    fclose(temp_fp);
    remove(temp);*/

    if (!(fp = fopen(filename,"w")))
	rs_error("Cannot open file %s!",filename);
    fprintf(fp,"# Classifier type:\tsvm\n# number of classes\tfeature space dimension\n%d\t%d\n# class names\n",svm->n_classes,svm->feature_dim);
    
    for (i=0;i<svm->n_classes-1;i++)
	fprintf(fp,"%s ",svm->mapping[i]);
    fprintf(fp,"%s\n",svm->mapping[i]);

    fprintf(fp,"# Scaling: max\tmin\n");
    for (i=0;i<svm->feature_dim;i++)
	fprintf(fp,"%g\t%g\n",svm->max[i],svm->min[i]);
    fprintf(fp,"\n");

    //fwrite(file_content,sizeof(char),attribute.st_size,fp);
	svm_save_model_h(fp,svm->model);

    fclose(fp);
}


svm_classifier_t *svm_fread_classifier(char *model_name) {
    int n_classes=0, feature_dim=0, i, temp_fd;
    FILE *model_fp;
    char *line, *token, buffer[BUFFER_SIZE];
   // char temp[] = "/tmp/svmXXXXXX";
    svm_classifier_t *svm=NULL;
    struct stat attribute;

    if (stat(model_name,&attribute) == -1)
		rs_error("Cannot determine size of svm model file %s!",model_name);

    model_fp = fopen(model_name,"r");
    if (!model_fp)
		rs_error("can't open file %s!",model_name);      

    line = (char *) rs_line_read(model_fp,'#');
    token = (char *) strtok(line,"\t");
    if (token)
		n_classes = atoi(token);
    else
		rs_error("Cannot read number of classes from svm classifier file '%s'!",model_name);
    token = (char *) strtok(NULL,"\t");
    if (token)
		feature_dim = atoi(token); 
    else
		rs_error("Cannot read feature dimension from svm classifier file '%s'!",model_name);

    svm = svm_new_classifier(n_classes,feature_dim,NULL);

    line = (char *) rs_line_read(model_fp,'#');
    token = (char *) strtok(line," ");
    i=0;
	do {		
		svm->mapping[i] = (char *) rs_malloc((strlen (token)+1)*sizeof(char),"class name");
		strcpy(svm->mapping[i++],token);		
	}
    while ((token = (char *) strtok(NULL," ")) && i < n_classes);

    fscanf(model_fp,"# Scaling: max\tmin\n");
    for (i=0;i<feature_dim;i++)
	if (fscanf(model_fp,"%lg %lg",&(svm->max[i]),&(svm->min[i])) != 2)
	    rs_error("Cannot read scaling information for SVM classifier from file '%s'!",model_name);


    fscanf(model_fp,"\n\n");

	svm->model = svm_load_model_h(model_fp);

    fclose(model_fp);

	/*temp_fd = mkstemp(temp);
    
    while ((i=fread(buffer,1,BUFFER_SIZE,model_fp)) >0) {
		if (write(temp_fd,buffer,i) <=0)
    			rs_error("Error when writing temporary svm file!");
    }
    if (ferror(model_fp))
    	rs_error("Error when reading from svm model file!");
    
	
	close (temp_fd);

    svm->model = svm_load_model(temp);
    remove(temp);
    if (!svm->model)
    	rs_error("No valid svm model was created - aborting!");

    fprintf(stderr,"Classifying into the following classes:\n");
    for (i=0;i<n_classes;i++)
	fprintf(stderr,"%s ",svm->mapping[i]);
    fprintf(stderr,"\n");
*/    
    svm->finished=1;

    return svm;
}

int svm_classify (svm_classifier_t *svm, mx_real_t *instance) {
    int i;
    mx_real_t best_class;
    struct svm_node *x;

    x = (struct svm_node *) rs_malloc((svm->feature_dim+1)*sizeof(struct svm_node),"Feature vector representation for svm");
    for (i=0;i<svm->feature_dim;i++) {
	x[i].index=i+1;
	x[i].value=instance[i];
    }
    x[i].index=-1;

    _scale_instance(&x,svm->feature_dim,svm->max,svm->min);
    
    best_class = svm_predict(svm->model,x);
    rs_free(x);
			
    return (int) best_class;
}


mx_real_t svm_class_prob(svm_classifier_t *svm, mx_real_t *instance, int class_ind) {
    int i, svm_type=svm_get_svm_type(svm->model);
    double *prob_estimates=NULL;
    struct svm_node *x;
	mx_real_t class_prob;

    if (svm_type!=C_SVC && svm_type != NU_SVC)
	rs_error("Cannot give class probability for 1-class or regression SVM!");

    x = (struct svm_node *) rs_malloc((svm->feature_dim+1)*sizeof(struct svm_node),"Feature vector representation for svm");
    for (i=0;i<svm->feature_dim;i++) {
		x[i].index=i+1;
		x[i].value=instance[i];
    }
    x[i].index=-1;

    _scale_instance(&x,svm->feature_dim,svm->max,svm->min);
    
    prob_estimates = (double *) rs_malloc(svm->n_classes*sizeof(double),"SVM probability estimates");
    
    svm_predict_probability(svm->model,x,prob_estimates);
    
	class_prob = prob_estimates[class_ind];
	rs_free (x);
	rs_free (prob_estimates);
	return class_prob;
}


int svm_classify_file(svm_classifier_t *svm, char *filename, fx_select_t *sel, int **evaluation) {
	char x;
    int i, class, n_instances=0, eval_size=1000, start,end;
    int *_eval;
    mx_real_t *instance;
    FILE *fp, *feature_fp;
    char *fname = (char *) rs_malloc(STRING_LENGTH*sizeof(char),"String");
    char *class_info = (char *) rs_malloc(STRING_LENGTH*sizeof(char),"String");
    char *class_name = (char *) rs_malloc(STRING_LENGTH*sizeof(char),"String");
    int full_n_features = sel? sel->n_features : svm->feature_dim;

	_eval = (int *) rs_malloc(eval_size*sizeof(int),"evaluation data");

    if (!svm)
		rs_error("Cannot classify without classifier - use -b or -m options to get one");
		
	if (sel)
		if (sel->n_selected != svm->feature_dim)
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
		    	    class = svm_classify(svm,instance);
		    	    if ((n_instances+1)*2 > eval_size) {
		    	    	eval_size+=1000;
		    	    	_eval = (int *) rs_realloc(_eval,eval_size*sizeof(int),"evaluation data");
		    	    }
		    	    
		    	    k=cl_name2number(&(svm->mapping),class_name,svm->n_classes);
		    	    _eval[n_instances*2]=cl_name2number(&(svm->mapping),class_name,svm->n_classes);
		    	    _eval[n_instances*2+1]=class;
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
		    	class=svm_classify(svm,instance);
				n_instances++;
	    		printf("%s ",svm->mapping[class]);
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


void svm_destroy(svm_classifier_t *svm) {
    int i;

    svm_destroy_model(svm->model);
    svm_destroy_param(&(svm->param));
    rs_free(svm->problem.y);
    for (i=0;i<svm->problem.l;i++)
    	rs_free(svm->problem.x[i]);
    rs_free(svm->problem.x);
    for (i=0;i<svm->n_classes;i++)
		rs_free(svm->mapping[i]);
    rs_free(svm->mapping);
    rs_free(svm->max);
    rs_free(svm->min);
    rs_free(svm);
}



svm_classifier_t *svm_fscan(svm_classifier_t *svm, char *filename, fx_select_t *sel) { 
	int class_ind, start, end, i;
    FILE *fp = NULL, *feature_fp = NULL;
    mx_real_t *features=NULL;
    char *line, *token;
    char *fname = NULL;
    char *class_name = (char *) rs_malloc(STRING_LENGTH*sizeof(char),"String");
	int full_feature_dim=sel?sel->n_features:svm->feature_dim;

    if (!svm)
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
							    if (fx_select_apply(&features,sel,features) != sel->n_features)
									rs_error("Feature selection did not succeed!");

							class_ind = cl_name2number(&(svm->mapping),class_name,svm->n_classes);
			    			 svm=svm_update_classifier(svm,class_ind,features);
			 			}
		     		}
				}
				fclose(feature_fp);
			}
	 	}
		svm_finish_classifier(svm);
     }
     fclose(fp);
     rs_free(class_name);
     rs_free(features);
     return svm;
}


void _set_parameters(struct svm_parameter *svm_param, char *param) {
    char *opt, *arg;


    // default values
    svm_param->svm_type = C_SVC;
    svm_param->kernel_type = LINEAR;
    svm_param->degree = 1;
    svm_param->gamma = 0.01;	// 1/k
    svm_param->coef0 = 0;
    svm_param->nu = 0.5;
    svm_param->cache_size = 1048576;
    svm_param->C = 1;
    svm_param->eps = 1e-1; //1e-12
    svm_param->p = 0.1;
    svm_param->shrinking = 1;
    svm_param->probability = 1;
    svm_param->nr_weight = 0;
    svm_param->weight_label = NULL;
    svm_param->weight = NULL;

    if (!param)
	return;

    opt = strtok(param," ");

    while (opt != NULL) {
	if(opt[0]!= '-')
	    break;
	arg=strtok(NULL," ");
	if (!arg)
 	    exit_with_help();

	switch(opt[1]) {
	    case 'c':
		svm_param->C = atof(arg);
		break;
	    case 'd':
		svm_param->degree = atof(arg);
		break;
	    case 'e':
		svm_param->eps = atof(arg);
		break;
	    case 'g':
		svm_param->gamma = atof(arg);
		break;
	    case 'h':
		svm_param->shrinking = atoi(arg);
		break;
	    case 'm':
		svm_param->cache_size = atof(arg);
		break;
	    case 'n':
		svm_param->nu = atof(arg);
		break;
	    case 'p':
		svm_param->p = atof(arg);
		break;
	    case 'r':
		svm_param->coef0 = atof(arg);
		break;
	    case 's':
		svm_param->svm_type = atoi(arg);
		break;
	    case 't':
		svm_param->kernel_type = atoi(arg);
		break;
	    case 'w':
		++svm_param->nr_weight;
		svm_param->weight_label = (int *) rs_realloc(svm_param->weight_label,sizeof(int)*svm_param->nr_weight,"SVM param weight label");
		svm_param->weight = (double *) rs_realloc(svm_param->weight,sizeof(double) *svm_param->nr_weight, "SVM param nr weight");
		//urspruenglich: = atoi(& opt[2]);
		svm_param->weight_label[svm_param->nr_weight-1] = atoi(opt+2);
		svm_param->weight[svm_param->nr_weight-1] = atof(arg);
		break;
	    default:
		fprintf(stderr,"unknown svm option: %c\n",opt[1]);
		exit_with_help();
	}
	opt = strtok(NULL," ");
    }
    
}


void exit_with_help() {
    printf(
	"available svm options are:\n"
	"-s svm_type : set type of SVM (default 0)\n"
	"	0 -- C-SVC\n"
	"	1 -- nu-SVC\n"
	"	2 -- one-class SVM\n"
	"	3 -- epsilon-SVR\n"
	"	4 -- nu-SVR\n"
	"-t kernel_type : set type of kernel function (default 0)\n"
	"	0 -- linear: u'*v\n"
	"	1 -- polynomial: (gamma*u'*v + coef0)^degree\n"
	"	2 -- radial basis function: exp(-gamma*|u-v|^2)\n"
	"	3 -- sigmoid: tanh(gamma*u'*v + coef0)\n"
	"-d degree : set degree in kernel function (default 3)\n"
	"-g gamma : set gamma in kernel function (default 1/k)\n"
	"-r coef0 : set coef0 in kernel function (default 0)\n"
	"-c cost : set the parameter C of C-SVC, epsilon-SVR, and nu-SVR (default 1)\n"
	"-n nu : set the parameter nu of nu-SVC, one-class SVM, and nu-SVR (default 0.5)\n"
	"-p epsilon : set the epsilon in loss function of epsilon-SVR (default 0.1)\n"
	"-m cachesize : set cache memory size in MB (default 100)\n"
	"-e epsilon : set tolerance of termination criterion (default 0.001)\n"
	"-h shrinking: whether to use the shrinking heuristics, 0 or 1 (default 1)\n"
	"-wi weight: set the parameter C of class i to weight*C, for C-SVC (default 1)\n\n"
	);
    exit(1);
}


void _create_scaling(struct svm_problem problem, int n_features, double **_max, double **_min) {
    int i,j, idx;
    double temp;
    double *max=*_max, *min=*_min;

    if (!max)
	max = (double *) rs_malloc(sizeof(double)*n_features,"feature maxima");
    if (!min)
	min = (double *) rs_malloc(sizeof(double)*n_features,"feature minima");

    for (i=0;i<n_features;i++){
	max[i]=-DBL_MAX;
	min[i]=DBL_MAX;
    }

    for (i=0;i<problem.l;i++) {
	idx=0;
	for (j=0;j<n_features;j++) {
	    if (problem.x[i][idx].index != j+1)
		temp=0;
	    else {
		temp=problem.x[i][idx].value;
		idx++;
	    }

	    if (temp < min[j])
		min[j] = temp;
	    if (temp > max[j])
		max[j] = temp;
	}
    }
}

void _scale_instance(struct svm_node *instance[], int n_features, double *max, double *min) {
    int j=0, idx=0, n_idx=0;
    struct svm_node *usInstance;
    double temp;

    usInstance= (struct svm_node *) rs_malloc(sizeof(struct svm_node)*(n_features+1),"copy of unscaled SVM instance");

    while ((* instance)[j].index != -1) {
	usInstance[j].index=(* instance)[j].index;
	usInstance[j].value=(* instance)[j].value;
	j++;
    }
    usInstance[j].index=-1;

    for (j=0;j<n_features;j++) {
	if (usInstance[idx].index != j+1)
		temp=0;
	else 
	    temp=usInstance[idx++].value;
	if (max[j]-min[j])
	    temp=SCALE_LOWER+(SCALE_UPPER-SCALE_LOWER)*(temp-min[j])/(max[j]-min[j]);
	else
	    temp=SCALE_LOWER+(SCALE_UPPER-SCALE_LOWER)*(temp-min[j])/MX_REAL_MIN;
	if (temp) {
	    (* instance)[n_idx].index=j+1;
	    (* instance)[n_idx++].value=temp;
	}
    }
    (* instance)[n_idx].index=-1;
    rs_free(usInstance);
}
