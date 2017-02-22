// ev_class.h
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

// @begin_add_johannes
#ifndef EV_CLASS_H
#define EV_CLASS_H
// @end_add_johannes

#include <ctype.h>

#include "ev_real.h"
#include "ev_matrix.h"
#include "ev_select.h"

#include "ev_emo.h"
#include "ev_svm.h"

// @begin_comment_johannes
// replaced "class" by "class_ind"
// @end_comment_johannes

typedef enum {naive_bayes, svm, unknown} cType;

typedef struct {
    cType type;
    int n_classes;
    int finished;
    int feature_dim;
    int *n_instances;
    mx_real_t *class_probs;
    mx_real_t **means;
    mx_real_t **std_dev;
    char **mapping;
} naive_bayes_classifier_t;


typedef struct {
    cType type;
    int n_classes;
    int feature_dim;
    int max_instances;
    int finished;
    char **mapping;
    double *max;
    double *min;
    struct svm_parameter param;
    struct svm_problem problem;
    struct svm_model *model;
} svm_classifier_t;


typedef union {
    naive_bayes_classifier_t *naiveBayes;
    svm_classifier_t *svm;
} classifier_t ;


/* general classifier functions */
int classify(classifier_t *cl, cType cl_type, mx_real_t *instance);
mx_real_t class_prob(classifier_t *cl, cType cl_type, mx_real_t *instance, int class_ind);
int cl_get_n_classes(classifier_t *cl, cType cl_type);
int cl_get_feature_dim(classifier_t *cl, cType cl_type);
int cl_set_feature_dim(classifier_t *cl, cType cl_type, int new_dim);
char **cl_get_class_map(classifier_t *cl, cType cl_type);
char *cl_get_mapping(classifier_t *cl, cType cl_type, int class_ind);
void cl_destroy(classifier_t *cl, cType cl_type);
classifier_t *cl_new_classifier(cType cl_type,int n_classes,int n_features, char *param);
classifier_t *cl_create_classifier(char *filename, cType cl_type);
classifier_t *cl_update_classifier(classifier_t *cl, cType cl_type, int class_ind, mx_real_t *features);
classifier_t *cl_finish_classifier(classifier_t *cl, cType cl_type);
void cl_output_classifier(classifier_t *cl, cType type, char *outfile);
cType cl_name2type(char *base_type_name);
classifier_t *cl_fscan(classifier_t *cl, cType cl_type, char *filename, fx_select_t *sel);
int cl_classify_file(classifier_t *cl, cType cl_type, char *filename, fx_select_t *sel, int **evaluation);


 /* general functions */
int cl_name2number(char ***mapping,char* class_name,int n_classes);
//void matrixInversion(mx_matrix_t *result, mx_matrix_t a, int N);
double cl_eval(int *evaluation, int n_classes, int n_instances);
char *cl_eval_stats(int *evaluation, int n_classes, int n_instances, char **mapping);


/* Naive Bayes classifier functions */

naive_bayes_classifier_t* nB_new_classifier(int classes, int feature_dim);
naive_bayes_classifier_t *nB_fscan(naive_bayes_classifier_t *nB, char *filename, fx_select_t *sel);
int nB_classify (naive_bayes_classifier_t *nB, mx_real_t *instance);
mx_real_t nB_class_prob(naive_bayes_classifier_t *nB, mx_real_t *instance, int class_ind);
void nB_output_classifier(naive_bayes_classifier_t *nB, char *filename);
naive_bayes_classifier_t *nB_fread_classifier(char *model_name);
void nB_destroy(naive_bayes_classifier_t *nB);
int nB_classify_file(naive_bayes_classifier_t *nB, char *filename, fx_select_t *sel, int **evaluation);
naive_bayes_classifier_t *nB_update_classifier(naive_bayes_classifier_t *nB, int class_ind, mx_real_t *features);
naive_bayes_classifier_t *nB_finish_classifier(naive_bayes_classifier_t *nB);


/* SVM classifier functions */

svm_classifier_t* svm_new_classifier(int classes, int feature_dim, char *param);
svm_classifier_t *svm_fscan(svm_classifier_t *svm, char *filename, fx_select_t *sel);
int svm_classify (svm_classifier_t *svm, mx_real_t *instance);
mx_real_t svm_class_prob(svm_classifier_t *svm, mx_real_t *instance, int class_ind);
void svm_output_classifier(svm_classifier_t *svm, char *filename);
svm_classifier_t *svm_fread_classifier(char *model_name);
void svm_destroy(svm_classifier_t *svm);
int svm_classify_file(svm_classifier_t *svm, char *filename, fx_select_t *sel, int **evaluation); 
svm_classifier_t *svm_update_classifier(svm_classifier_t *svm, int class_ind, mx_real_t *features);
svm_classifier_t *svm_finish_classifier(svm_classifier_t *svm);

// @begin_add_johannes
#endif EV_CLASS_H
// @end_add_johannes
