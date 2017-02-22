/* From: 
 * Feature Selection Algorithms - A Brief Guide
 * P. Somol, P. Pudil
 * http://ro.utia.cz/fs/fs_guideline.html
 *
 * Changes by Thurid Vogt, 17.04.2007
 * 
 * meaning of constants and some identifiers you may need to set for calling the procedure
 * (local variables are described inside the procedure):
 * - int n: full feature set size
 * - int d: desired feature subset size
 * - int delta: floating search stopping constraint
 * 		0 - full search (the procedure will not stop after reaching dimension d; SFFS will reach n, SFBS will reach 0)
 * 		delta>0 - SFFS will float until dimension d+delta is reached, SFBS will float until d-delta is reached
 * 		-1 - the value of delta limit is computed during the course of the algorithm by averaging the length of backtracking
 * 		-2 - the value of delta limit is set during the course of the algorithm as the maximum length of backtracking
 * - int r - generalization level and direction of search
 * 		+1 - classical SFFS
 * 		-1 - classical SFBS
 * 		r>1 - generalized SFFS with r as generalization level
 * 		r<-1 - generalized SFBS with -r as generalization level
 * - TSubset *bset - pointer to a structure for storing results (see its definition later)
 * - int detail - textual output detail level (use NOTHING=0 or STANDARD=2)
 * 
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "ev_memory.h"
#include "ev_messages.h"

#include "ev_fselect.h"

void TimeString(char *t, double tdiff);
int Criterion(double *value, int* indices, int dim, int n_classes, fx_select_t *sel, char **file_names, int n_splits, cType classifier, char *cparam);
int shift_indexes(fx_select_t *sel,int *bestset, int **returnset,int dim);

void printFset(int *indices,int n) {
	int i;
	
	for (i=0;i<n-1;i++)
		fprintf(stderr,"%d,",indices[i]+1);
	fprintf(stderr,"%d\n",indices[n-1]+1);
}

int FloatSearch(int n, int d, int delta, int r, TSubset *bset, int detail, int n_classes, fx_select_t *sel, char **file_names, int n_splits, cType classifier, char *cparam, int float_level) {
  /* a non-redundand coding of subset configurations is used.
     For easier understanding imagine our goal is to find a subset by exhaustive search,
     when d=5 and n=12
    - initial configuration is 111110000000   (actually stored in "bin" field)
    - in every step: a) find the leftmost block of 1's
                     b) shift its rightmost 1 to the right
                     c) shift the rest of this block to the left boundary (if it is not there)
    - this algorithm generates increasingly all binary representations of subsets
    - in context of floating search this algorithm is used for computation of generalized steps
      (in case of simple SFFS and simple SFBS only single-feature configurations are tested)
    - for purposes of floating search more identifiers than 0 and 1 are used to
      identify temporarily freezed features etc. (2,-1)
    - because of possibility to exchange meanings of 0 and 1 it was suitable to
      incorporate both forward and backward versions of floating search into one procedure
  */

  int *bin;   /* of size n, stores 0/1 information on currently selected features */
  int *index; /* of size d, it is computed from bin, stores indexes of selected features */
  int *bestset; double bestvalue; /* best subset */
  int *globalbestset; double *globalbestvalue; /* so-far best subsets in all dimensions */
  int wasthebest; /* indicates, if a better subset has been found during last step*/
  double value=0;
  int sumrint;
  int i;
  int sumr;     /* current subset size */
  int zmenasumr; /* how to change sumr (when changing direction of search): -2,-1,+1,+2 steps */
  int rr;
  int pom;
  int beg,piv;  /* beg - beginning of block of identifiers, piv - "pivot" - last identifier in a block */
  int stopex;    /* identifies end of internal exhaustive search */
  int stopfloat; /* identifies end of it all */
  int stopfloatmez=0;  /* identifies the dimension, for which the algorithm should end */
  int id0=0,id2=2;  /* these identifiers (0 and 2) are exchanged in case of backward search */
  int sbs=0;    /* 0=sfs, 1=sbs current search direction*/
  int vykyv=0; // stores current backtracking depth (+forward,-backward)
  float vykyvmez=0.0; // predicted delta estimate
  int vykyvcount=0; // number of direction changes (needed for delta averaging)
  int sfbs=0;   /* 0=sffs, 1=sfbs main search direction */
  int error=0;
  long globcit/*,cit*/,kombcit;
//  int timcet=0;
    double temp=-1;
  int best=0;
  int n_floats=0;
  
  time_t tbegin,telp,tlast;   /* for measuring computational time only */
//  time_t tact;
  
  tbegin=time(NULL);
  tlast=tbegin;

	if (float_level ==-1)
		float_level=n;

  if(r<0) { /* indicates SFBS, first stage will be SBS */
    sfbs=1; sbs=1; id0=2; id2=0; r=-r;
    if(delta==0) 
    	stopfloatmez=1;
	else 
		if(delta>0) {
			stopfloatmez=d-delta; 
			if(stopfloatmez<1) 
				stopfloatmez=1;
			}
    // for delta -1 and -2, stopfloatmez will be set later
  }
  else { /* indicates SFFS, first stage will be SFS */
    if(delta==0) 
    	stopfloatmez=n;
    else 
    	if(delta>0) {
    		stopfloatmez=d+delta; 
    		if(stopfloatmez>n) 
    			stopfloatmez=n;
    	}
    // for delta -1 and -2, stopfloatmez will be set later
  }
  vykyv=0;
  vykyvmez=0.0;
  vykyvcount=0;

  if((d<1)||(d>n)){
	return(24);
  } /* nothing to search for */
  
  if((r<1)||(r>=n)||((!sfbs)&&(r>=d))||((sfbs)&&(r>=n-d))){
	return(25);} /* no sense */
  if(n<2){
	return(30);
  }
  
  if((bin=(int *) rs_malloc((n+1)*sizeof(int),"bin, FloatSearch"))==NULL) {
	return(3);
	}
  if((index=(int *) rs_malloc(n*sizeof(int),"index, FloatSearch"))==NULL) {
  	rs_free(bin);
    return(3);
    }
  if((bestset=(int *) rs_malloc(n*sizeof(int),"bestset, FloatSearch"))==NULL) {
  	rs_free(index);
  	rs_free(bin);
  	return(3);
  	}
  if((globalbestset=(int *) rs_malloc(((n*(n+1L))/2L)*sizeof(int),"globalbestset, FloatSearch"))==NULL) {
  	rs_free(bestset);
  	rs_free(index);
  	rs_free(bin);
  	return(3);
  	}
  if((globalbestvalue=(double *) rs_malloc(n*sizeof(double),"globalbestvalue, FloatSearch"))==NULL) {
  	rs_free(globalbestset);
  	rs_free(bestset);
  	rs_free(index);
  	rs_free(bin);
  	return(3);
  	}
  /* k-th set of size k is stored at [(k*(k-1))/2] */
  for(i=0;i<(n*(n+1))/2;i++) 
  	globalbestset[i]=0;
  for(i=0;i<n;i++) 
  	globalbestvalue[i]=-SAFEUP;

  for(i=0;i<=n;i++) 
  	bin[i]=id0; /* bin[n]=id0 for testing the end */

  rr=r; /* initial generalization level */
  if(sfbs) {
      sumr=n-rr; 
      if(detail&STANDARD) {
           if(r>1) 
           	printf("Generalized "); 
           printf("Sequential Floating Backward Search"); 
           if(r>1) {
               printf(" (r=%d)",r);
           } 
           printf(":\n");
      }
  }          
  else {
      sumr=rr; 
      if(detail&STANDARD){
          if(r>1) 
          	printf("Generalized "); 
          printf("Sequential Floating Forward Search"); 
          if(r>1) {
              printf(" (r=%d)",r);
          } 
          printf(":\n");
      }
  }
  
  if(detail&STANDARD) {
  	printf("started on "); 
  	printf(ctime(&tbegin));}
  stopfloat=0;
  
//  hcreate(100000); //initialize hash table for set->result mapping
 
  do {

    for(i=0;i<n;i++) 
    	bestset[i]=0; 
    bestvalue=-SAFEUP;
    pom=rr;
    for(i=0;((pom>0)&&(i<n));i++) 
    	if(bin[i]==id0) {
    		bin[i]=id2; 
    		pom--;
    	} /* initialize bin for exhaustive step */
    sumrint=sumr*sizeof(int);
    globcit=0;
    // estimate the number of steps
    kombcit=1;
    if(sbs) 
    	pom=sumr+rr; 
    else 
    	pom=n-(sumr-rr); 
    for(i=0;i<rr;i++) 
    	kombcit*=pom-i; 
    for(i=2;i<=rr;i++) 
    	kombcit/=i;

    stopex=0;
    do {
      pom=0; /* convert "bin" to "index" */
      for(i=0;i<sumr;i++) {
	    while(bin[pom]<=0) 
	    	pom++;
	    index[i]=pom;
	    pom++;
      }



      /* ----------- following block serves only for outputting the information about current algorithm state ----------*/
      /* ------------it may be discarded */
//      if(!(globcit%PERCENTDETAIL)){
//          tact=time(NULL);
//          if(difftime(tact,tlast)>SECONDS) {
//              tlast=tact; 
//              timcet=true;
//              // test cancel
//              if((error=GetStopFlag())!=0) {
//              	rs_free(globalbestvalue);
//              	rs_free(globalbestset);
//              	rs_free(bestset);
//              	rs_free(index);
//              	rs_free(bin);
//              	return(error);
//              }
//          }
//      }
//      if((!globcit)/*||(!(globcit%cit))*/||timcet) {
//          pom = (int)floor((100.0*(double)sumr)/(double)n);
//          ProcessTextCS->Acquire();
//          if(sbs) ProcessText[0]='v'; else ProcessText[0]='^';
//          sprintf(ProcessText+1," k=%d, (%ld/%ld), delta=%d:%d, Cr=%g",sumr,globcit,kombcit,delta,(int)(DELTAMUL*vykyvmez+DELTAADD),globalbestvalue[d-1]);
//          ProcessTextCS->Release();
//          SetProcessFlag(pom);
//          if(detail&PERCENT) {
//              printtext("\r");
//              ProcessTextCS->Acquire();
//              printtext(ProcessText); 
//              ProcessTextCS->Release();
//              printtext(" ");
//          }
//      }
//      timcet=false;
//      globcit++;
      /* ------------previous block may be discarded */
      /* ----------- previous block served only for outputting the information about current algorithm state ----------*/      
 


      // result of criterion function should be stored to "value". When calling the criterion function, "index" field
      // contains indexes (beginning by 0) of features in the subset being currently tested, having dimension "sumr"
      if((error=Criterion(&value,index,sumr, n_classes, sel, file_names,n_splits,classifier, cparam))!=0) {
      	rs_free(globalbestvalue);
      	rs_free(globalbestset);
      	rs_free(bestset);
      	rs_free(index);
      	rs_free(bin);
      	return(error);
      }
      if(value>bestvalue) {
	    memcpy(bestset,index,sumrint);
	    bestvalue=value;
      }

      /* finding the new configuration during internal exhaustive step */
      for(beg=0;bin[beg]!=id2;beg++)
      	;
      for(piv=beg;(piv<n)&&(bin[piv]!=id0);piv++)
      	;
      if(piv==n) 
      	stopex=1;
      else {
	    pom=piv; /* remember the position of first 0 on the right */
	    do 
	    	piv--; 
	    while(bin[piv]!=id2); /* find a real pivot */
	    bin[piv]=id0; 
	    bin[pom]=id2; /* shift pivot to the right */
	    pom=0;
	    /* run "pom" from left, "piv" from right. the 0,2 pairs found are changed to 2,0 */
	    do 
	    	piv--; 
	    while((piv>0)&&(bin[piv]!=id2));
	    while((pom<piv)&&(bin[pom]!=id0))
	    	pom++;
	    while(piv-pom>0)
	    {
	      bin[piv]=id0; bin[pom]=id2;
	      do 
	      	piv--; 
	      while((piv>0)&&(bin[piv]!=id2));
	      while((pom<piv)&&(bin[pom]!=id0))
	      	pom++;
	    }
      }
    }while(!stopex);

    if(bestvalue>globalbestvalue[sumr-1]) {// sumr is from interval <1,n>
      memcpy(&globalbestset[(sumr*(sumr-1))/2],bestset,sumrint);
      globalbestvalue[sumr-1]=bestvalue;
      wasthebest=1;
    }
    else 
    	wasthebest=0;

	if (detail&STANDARD) {
		fprintf(stderr,"current best set of size %d and goodness %g:\n",sumr,bestvalue);
		printFset(bestset,sumr);
	}

    if(sfbs) {
      if(sbs) /* last step was sbs */ {
	    if(sumr<n-r) /* if adding is possible, prepare sfs */ {
	      for(i=0;i<n;i++) 
	      	bin[i]=0; /* conversion to sfs format */
	      for(i=0;i<sumr;i++) 
	      	bin[bestset[i]]=1;
	      sbs=0; 
	      id0=0; 
	      id2=2;
	      zmenasumr=1;
	    }
	    else {
	      zmenasumr=-1; /* otherwise stay by sbs */
	      for(i=0;i<n;i++) 
	      	bin[i]=-1;
	      for(i=0;i<sumr;i++) 
	      	bin[bestset[i]]=2; /* freeze the change */
	    }
      }
      else /* last step was sfs */ {
	    if(wasthebest) /* better solution was found */ {
	      if(sumr<n-r) {
	        zmenasumr=1; /* repeat sfs */
	        for(i=0;i<n;i++) 
	        	bin[i]=0;
	        for(i=0;i<sumr;i++) 
	        	bin[bestset[i]]=1; /* freeze the change */
	      }
	      else { /* nothing may be added, switch to sbs */
	        for(i=0;i<n;i++) 
	        	bin[i]=-1; 
	        for(i=0;i<sumr;i++) 
	        	bin[bestset[i]]=2;
	        sbs=1; 
	        id0=2; 
	        id2=0;
	        zmenasumr=-1;
	      }
	    }
	    else /* no improvement during last step (sfs) */ {
	      /* change "bin" for sbs but after the change of "sumr" */
	      sbs=1; 
	      id0=2; 
	      id2=0;
	      zmenasumr=-2; /* forget last step and perform one new sbs step */
	    }
      }
      /* actualize sumr and rr */
      if(zmenasumr==1) {
      	if((sumr==d)&&((n-d)%r!=0)) {
      		sumr=d+(n-d)%r; 
      		rr=(n-d)%r;
      	} 
      	else {
      		sumr+=r; 
      		rr=r;
      	}
        if(vykyv>0) 
        	vykyv+=rr; // continue
        else vykyv=rr; // begin           
      }
      else /* zmenasumr== -1 or -2 */ {
      	if((sumr>d)&&(sumr-r<d)) {
      		sumr=d; 
      		rr=(n-d)%r;
      	} 
      	else {
      		sumr-=r; 
      		rr=r;
      	}
        if(vykyv>0) { // end of going up
        	if(delta==-1){ //averaging
            	vykyvmez=(vykyvcount*vykyvmez+vykyv)/(vykyvcount+1);
                vykyvcount++;
            }
            else{ // maximization
            	if(vykyv>vykyvmez) 
            		vykyvmez=vykyv;
            }
        }
        vykyv=0;
      }
      if(zmenasumr==-2) /* once more */ {
	    for(i=0;i<n;i++) 
	    	bin[i]=-1; /* change to sbs with changed "sumr" */
	    pom=(sumr*(sumr-1))/2;
	    for(i=0;i<sumr;i++) 
	    	bin[globalbestset[pom+i]]=2;
	    if((sumr>d)&&(sumr-r<d)) {
	    	sumr=d; 
	    	rr=(n-d)%r;
	    } 
	    else {
	    	sumr-=r; 
	    	rr=r;
	    }
            // no change in direction
      }

      if(delta<0){
      	stopfloatmez=d-DELTAMUL*vykyvmez-DELTAADD; 
      	if(stopfloatmez<1) 
      		stopfloatmez=1;
      }
      if(sumr<stopfloatmez) 
      	stopfloat=1; /* end if delta reached */
    }
    else /* sffs */ {
      if(sbs) /* last step was sbs */ {
	    if(wasthebest && n_floats < float_level) /* a better subset was found */ {
	      if(sumr>r && n_floats < float_level) {
	        zmenasumr=-1; /* so repeat sbs */
	        for(i=0;i<n;i++) 
	        	bin[i]=-1;
	        for(i=0;i<sumr;i++) 
	        	bin[bestset[i]]=2; /* freeze changes */
	        n_floats++;
	      }
	      else if (wasthebest && n_floats >= float_level) {
	        for(i=0;i<n;i++) 
	        	bin[i]=0; 
	        for(i=0;i<sumr;i++) 
	        	bin[bestset[i]]=2; /* freeze changes */
	        sbs=0; 
	        id0=0; 
	        id2=2;
	        zmenasumr=1;
	      	n_floats=0;
	      }
	      else { /* nothing to remove, change to sfs */
	        for(i=0;i<n;i++) 
	        	bin[i]=0; 
	        for(i=0;i<sumr;i++) 
	        	bin[bestset[i]]=1;
	        sbs=0; 
	        id0=0; 
	        id2=2;
	        zmenasumr=1;
	        n_floats=0;
	      }
	    }
	    else /* no improvement during last step (sbs) */ {
	      /* change to "bin" for sfs later after "sumr" gets its original value */
	      sbs=0; 
	      id0=0; 
	      id2=2;
	      zmenasumr=2; /* forget last step and perform one new sfs step */
	    }
      }
      else /* last step was sfs */ {
	    if(sumr>r) /* if removing is possible, prepare sbs */ {
	      for(i=0;i<n;i++) 
	      	bin[i]=-1;
	      for(i=0;i<sumr;i++) 
	      	bin[bestset[i]]=2;
	      sbs=1; 
	      id0=2; 
	      id2=0;
	      zmenasumr=-1;
	    }
	    else {
	      zmenasumr=1; /* othervise stay by sfs */
	      for(i=0;i<n;i++) 
	      	bin[i]=0;
	      for(i=0;i<sumr;i++) 
	      	bin[bestset[i]]=1; /* freeze changes */
	    }
      }
      /* renew sumr and rr */
      if(zmenasumr==-1) {
      	if((sumr==d)&&(d%r!=0)) {
      		sumr=d-d%r; 
      		rr=d%r;
      	} 
      	else {
      		sumr-=r; 
      		rr=r;
      	}
        if(vykyv<0) 
        	vykyv-=rr; // continue
        else 
        	vykyv=-rr; // begin
      }
      else /* zmenasumr== 1 or 2 */ {
      	if((sumr<d)&&(sumr+r>d)) {
      		sumr=d; 
      		rr=d%r;
      	} 
      	else {
      		sumr+=r; 
      		rr=r;
      	}
        if(vykyv<0) { // end of going down
        	if(delta==-1){ //averaging
            	vykyvmez=(vykyvcount*vykyvmez+(-vykyv))/(vykyvcount+1);
                vykyvcount++;
            }
            else{ // maximizing
            	if(-vykyv>vykyvmez) 
            		vykyvmez=-vykyv;
            }
        }
        vykyv=0;
      }
      if(zmenasumr==2) /* once more and renew "bin"*/ {
	    for(i=0;i<n;i++) 
	    	bin[i]=0; /* change to sfs format now with actualized "sumr" */
	    pom=(sumr*(sumr-1))/2;
	    for(i=0;i<sumr;i++) 
	    	bin[globalbestset[pom+i]]=1;
	    if((sumr<d)&&(sumr+r>d)) {
	    	sumr=d; 
	    	rr=d%r;
	    } 
	    else {
	    	sumr+=r; 
	    	rr=r;
	    }
            // no direction change
      }

      if(delta<0) {
      	stopfloatmez=d+DELTAMUL*vykyvmez+DELTAADD; 
      	if(stopfloatmez>n) 
      		stopfloatmez=n;
      }
      if(sumr>stopfloatmez) 
      	stopfloat=1; /* end if delta reached */
    }


  } while(!stopfloat);


  telp=time(NULL); 
  TimeString(bset->dobavypoctu,difftime(telp,tbegin));

  /* 
   * modified by T. Vogt, 17.04.2007:
   * return the feature set with the highest evaluation and fewest features
   */
  
	for (i=0;i<d;i++) {
  	if (globalbestvalue[i]>temp) {
  		best=i;
  		temp=globalbestvalue[i];
  	}
  }
  d=best+1;

  pom=(d*(d-1))/2;
  bset->subsetsize=d;
  if((bset->featureset=(int *) rs_malloc(d*sizeof(int),"bset->featureset, FloatSearch"))==NULL) {
  	rs_free(globalbestvalue);
  	rs_free(globalbestset);
  	rs_free(bestset);
  	rs_free(index);
  	rs_free(bin);
  	return(3);
  }
  
  if (shift_indexes(sel,globalbestset+pom,&(bset->featureset),d))
  	return (3);

  bset->critvalue=globalbestvalue[d-1];
  
  rs_free(globalbestset);
  rs_free(globalbestvalue);
  rs_free(bestset);
  rs_free(index);
  rs_free(bin);
//  hdestroy();
  return(0);
}

int shift_indexes(fx_select_t *sel,int *bestset, int **returnset,int dim) {
	int i, j, n_features=-1;
	
	if (sel) {
		j=0;
		for (i=0;i<sel->n_features&&j<dim;i++) {
			if (sel->selected[i]) {
				n_features++;
				if (bestset[j]==n_features) {
					(*returnset)[j]=i+1;
					j++;
				}
			}
			
		}
		if (j!=dim)
			return(3);
	}
	else 
	  for(i=0;i<dim;i++)
  		(*returnset)[i]=bestset[i]+1;

	return 0;
}

void TimeString(char *t, double time) {
	int temp,i=0;

	rs_msg("time %g",time);
	temp =time/3600;
	sprintf(t,"%d:",temp);
	i=temp/10;
	i+=2;

	time-=temp*3600;
	temp=time/60;
	if (temp<10)
		sprintf(t+i,"0%d:",temp);
	else
		sprintf(t+i,"%d:",temp);
	i+=3;		
		
	time-=temp*60;	
	temp=time;
	if (temp<10) 
		sprintf(t+i,"0%d",temp);
	else
		sprintf(t+i,"%d",temp);
	i+=3;
	t[i]='\0';
}


int Criterion(double *value, int* indices, int dim, int n_classes, fx_select_t *sel, char **file_names, int n_splits, cType classifier, char *cparam) {
	int i, n_feat=-1, j, n_instances=0,n; 
	int *evaluation=NULL, *_eval=NULL;
	fx_select_t *sel_subset = fx_select_create(sel->n_features);
	classifier_t *cl = NULL; 
	double val=0;
	
	if (ev_verbose) {	
		fprintf(stderr,"Now evaluating subset ");
		printFset(indices,dim);
	}	

		
	j=0;
	for (i=0; i<sel_subset->n_features && j<dim;i++) {
		if (sel->selected[i]==1) {
			n_feat++;
			if (indices[j]==n_feat) {
				sel_subset->selected[i]=1;
				j++;
			}
		}
	}
	if (j!=dim) {
		rs_warning("Problem with feature subset selection!");
		return -1;
	}
	sel_subset->n_selected=dim;
	
	for (i=0;i<n_splits*2;i+=2) {
		char *tmpparam=NULL;
		if (cparam) {
			tmpparam=(char *) rs_malloc((strlen(cparam)+1)*sizeof(char),"classifier param");
			strcpy(tmpparam,cparam);
		}
		cl=cl_new_classifier(classifier,n_classes,sel_subset->n_selected,tmpparam);
		cl=cl_fscan(cl,classifier,file_names[i],sel_subset);
		n=cl_classify_file(cl,classifier,file_names[i+1],sel_subset,&_eval);
		n_instances+=n;
		if (n_instances-n==0) 
			evaluation=rs_malloc(n_instances*2*sizeof(int),"evaluation data");
		else
			evaluation=rs_realloc(evaluation,n_instances*2*sizeof(int),"evaluation data");
		for (j=0;j<n*2;j++) 
				evaluation[j+(n_instances-n)*2]=_eval[j];
		rs_free(_eval);
		if (tmpparam)
			rs_free(tmpparam);
		cl_destroy(cl,classifier);
	}
	
	if (evaluation)
		val=cl_eval(evaluation,n_classes,n_instances);
	
	rs_free(evaluation);
 
	fx_select_destroy(sel_subset);
	
	if (ev_verbose)
		fprintf(stderr,"... has evaluation %g\n",val);

	*value=val;
	return 0;
}


