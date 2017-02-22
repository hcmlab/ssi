/**
* Autor:	Sascha Wendt
* Datum:	9.4.2001
*
* Beschreibung:	Vorwaertsmaskierung
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ev_messages.h"


#include "ev_fw_masking.h"
#include "ev_dsp.h"

/*#define MIN_DB -20*/

#define MAX_TOENE 100 /* Maximal 100 Delta-Toene und maximal 100 Masken
			 werden gespeichert */
#define MAX_DECAY 100 /* Nach spaetestens 100 Frames ist der Attack-Parameter
			 auf 0 gefallen => keine weietere Erhoehung mehr 
			 notwendig. Verhindert einen Overflow */
#define MIN_MASK 0.1  /* Masken mit weniger als 0.1 dB koennen geloescht 
			werden. Dadurch Begrenzung der Maskenanzahl */
#define MIN_DELTA 1.0 /* Toeneraenderungen um weniger als 1.0 dB werden nicht
			 als Aenderung wahrgenommen */
#define n_param 7     /* Anzahl der bestimmten Parametersaetze */


/* Speicherung der Toene */
/* static tone_t **toene_vektor=NULL;  */
/* static int *last_tones=NULL; */

/* Speicherung der Masken */
/* static tone_t **masken_vektor=NULL; */
/* static int *last_masks=NULL; */

/* Speicherung des jeweils letzten Tones fuer jede Frequenzgruppe */
/* static mx_real_t *last_ins = NULL; */

/* Neuer Ton in Frequenzgruppe? Ja => probe_on = 1, sonst 0 */
/* static int *probes_on = NULL; */

static mx_real_t _dsp_fw_masking(int channel, mx_real_t in, mx_real_t attack, mx_real_t release, mx_real_t slope, mx_real_t threshold, dsp_fwm_param_t* params);

static mx_real_t _dsp_calc_mask(mx_real_t spl, mx_real_t slope, mx_real_t attack, mx_real_t decay_attack, mx_real_t release, mx_real_t decay_release)

{
  mx_real_t out;

  out=(spl*(1-slope)*(1-pow(attack,decay_attack))*pow(release,decay_release));
  return out;
}

void
dsp_masking(mx_real_t* maskiert, mx_real_t *spektrum, int n_channel, dsp_fwm_param_t* params)
{
  int i,j;
  static int last_n_channel=-1;

  last_n_channel=n_channel;

/*   if((last_n_channel!=n_channel)||(toene_vektor==NULL)) */
/*     { */
      /* Re-Initialisierung von Masken, Toenen etc.*/
/*       toene_vektor=(tone_t**) malloc(sizeof(tone_t*)*n_channel); */
/*       for(i=0;i<n_channel;i++) */
/* 	{ */
/* 	  toene_vektor[i]=(tone_t*) malloc(sizeof(tone_t)*MAX_TOENE); */
/* 	  for(j=0;j<MAX_TOENE;j++) */
/* 	    { */
/* 	      toene_vektor[i][j].delta_spl=0.0; */
/* 	      toene_vektor[i][j].decay_a=0.0; */
/* 	      toene_vektor[i][j].decay_b=0.0; */
/* 	    } */
/* 	} */
/*       last_tones=(int*) malloc(sizeof(int)*n_channel); */
/*       for(i=0;i<n_channel;i++) */
/* 	{ */
/* 	  last_tones[i]=-1; */
/* 	} */
/*       masken_vektor=(tone_t**) malloc(sizeof(tone_t*)*n_channel); */
/*       for(i=0;i<n_channel;i++) */
/* 	{ */
/* 	  masken_vektor[i]=(tone_t*) malloc(sizeof(tone_t)*MAX_TOENE); */
/* 	  for(j=0;j<MAX_TOENE;j++) */
/* 	    { */
/* 	      masken_vektor[i][j].delta_spl=0.0; */
/* 	      masken_vektor[i][j].decay_a=0.0; */
/* 	      masken_vektor[i][j].decay_b=0.0; */
/* 	    } */
/* 	} */
/*       last_masks=(int*) malloc(sizeof(int)*n_channel); */
/*       for(i=0;i<n_channel;i++) */
/* 	{ */
/* 	  last_masks[i]=-1; */
/* 	} */

/*       last_ins=(mx_real_t*) malloc(sizeof(mx_real_t)*n_channel); */
/*       for(i=0;i<n_channel;i++) */
/* 	{ */
/* 	  last_ins[i]=0.0; */
/* 	} */

/*       probes_on=(int*) malloc(sizeof(int)*n_channel); */
/*       for(i=0;i<n_channel;i++) */
/* 	{ */
/* 	  probes_on[i]=0; */
/* 	}   */
/*     }  */

  /* BERECHNUNG DES MASKIERUNGSEFFEKTES FUER ALLE n KANAELE */
  for(i=0;i<n_channel;i++)
      maskiert[i]=_dsp_fw_masking(i,spektrum[i],params->attack[i],params->release[i],params->slope[i],params->threshold[i], params);
      
}

static mx_real_t 
_dsp_fw_masking(int channel, mx_real_t in, mx_real_t attack, mx_real_t release, mx_real_t slope, mx_real_t threshold, dsp_fwm_param_t* params)
/* Reine Maskierung ohne Rueckgabe der Addaption */
{

  int i,j,k;
  mx_real_t spl;

  mx_real_t out=0;
  tone_t *toene = params->toene_vektor[channel];
  int last_tone = params->last_tones[channel];
  tone_t *masken = params->masken_vektor[channel];
  int last_mask = params->last_masks[channel];
  mx_real_t last_in = params->last_ins[channel];
  int probe_on = params->probes_on[channel];
  
  mx_real_t masker = 0;
  mx_real_t new_mask = 0;

  i=0;
  while(i<=last_mask)
    {
      masken[i].decay_a++; /* Erhoehen der Verzoegerung um einen Frame*/
      new_mask=_dsp_calc_mask(masken[i].delta_spl,slope,attack,masken[i].decay_b,release,masken[i].decay_a); /* Maskierung neu berechnen */
      if(new_mask<MIN_MASK) /* Wenn der Maskierungseffekt sehr klein geworden 
			       ist, wird die Maske verworfen, um nicht zuviele
			       (unendlich viele) Masken speichern zu muessen */
	{
	  for(j=i;j<last_mask;j++)
	    {
	      masken[j]=masken[j+1];
	    }
	  last_mask--;
	} 
      else
	{
	  masker+=new_mask; /* Maskierungseffekte aufaddiern */
	  i++;
	}
    }

  if((in-masker)>threshold)
    {
      /* Ton liegt ueber dem Schwellwert => Maskierung moeglich */
      if(!(last_in>threshold))
	{
	  /* Neuer Ton, da letzter Ton nicht ueber Schwellwert => neue Maske */
	  last_in=in-masker;
	  last_tone++;
	  toene[last_tone].delta_spl=(in-threshold-masker);
	  toene[last_tone].decay_b=0;
	  toene[last_tone].decay_a=0;
	  probe_on=1;
	}
      else if(((in-masker)<(last_in+MIN_DELTA))&&((in-masker)>(last_in-MIN_DELTA))) 
	  /* KEINE WAHRNEHMBARE AENDERUNG DER TONINTENSITAET */
	{
	  last_in=last_in; /* Neuer Ton wurde als alter Ton empfunden */
	  for(i=0;i<=last_tone;i++)
	    {
	      if(toene[i].decay_b<MAX_DECAY)
		toene[i].decay_b++; /* Alle Delta-Toene dauern an */
	    }
	  
	}
      else if((in-masker)<last_in)
	{
	  /* Ton ist schwaecher geworden => Differenz wirkt maskierend */
	  spl=toene[0].delta_spl+threshold;
	  if(spl<(in-masker)) /* Der erste Ton ist kleiner als der aktuelle */
	    {
	      /* Aufsummieren aller Dalta-Toene */
	      for(i=1;i<=last_tone;i++)
		spl+=toene[i].delta_spl;

	      i=last_tone;
	      
	      while((spl>(in-masker))&&(i>-1))
		{
		  last_mask++;
		  if(i>0)
		    {
		      masken[last_mask].delta_spl=toene[i].delta_spl;
		    }
		  else
		    {
		      if((in-masker)>threshold)
			masken[last_mask].delta_spl=toene[0].delta_spl-(in-threshold-masker);
		      else
			masken[last_mask].delta_spl=toene[0].delta_spl;
		    }
		  masken[last_mask].decay_b=toene[i].decay_b;
		  masken[last_mask].decay_a=0;

		  new_mask=_dsp_calc_mask(masken[last_mask].delta_spl,slope,attack,masken[last_mask].decay_b,release,masken[last_mask].decay_a);
		  masker+=new_mask;
		  /*masker+=(masken[last_mask].delta_spl*(1-slope)*(1-pow(attack,masken[last_mask].decay_b))*pow(release,masken[last_mask].decay_a));*/
		  spl-=toene[i].delta_spl;
		  last_tone--;
		  i--;
		}

	      /* Der zu speichernde Delta_Ton entspricht der Differenz zwischen
		 aktuellem Ton und der Summe der Delta_Toene */
	      if(i>-1)
		{
		  if(i>0)
		    {
		      toene[i].delta_spl=in-masker-spl;
		      /* Die Dauer des aktuellen Tondes umfasst die Dauer aller
			 staerkeren Toene plus dem aktuellen Frame */
		      if(toene[i+1].decay_b<MAX_DECAY)
			toene[i].decay_b+=toene[i+1].decay_b;
		      toene[i].decay_a=0;  
		      last_tone=i;
		    }
		  else
		    {
		      toene[0].delta_spl=in-threshold-masker;
		      if(toene[0].delta_spl>0)
			{
			  if(toene[0].decay_b<MAX_DECAY)
			    toene[0].decay_b++;
			  toene[0].decay_a=0;
			  last_tone=0;
			}
		      else
			{
			  last_tone=-1;
			}
		    }
		}
	      last_in=in-masker;
	    }


	  else /* Der aktuelle Ton ist kleiner als der bisher schwaechste
		  Ton => aktueller Ton haelt als einziger weiter an */
	    {
	      for(j=last_tone;j>-1;j--)
		{
		  last_mask++;
		  if(j==0)
		    {
		      if((in-masker)>threshold)
			masken[last_mask].delta_spl=toene[j].delta_spl-(in-threshold-masker);
		      else
			masken[last_mask].delta_spl=toene[j].delta_spl;
		    }
		  else
		    masken[last_mask].delta_spl=toene[j].delta_spl;
		  
		  masken[last_mask].decay_b=toene[j].decay_b;
		  masken[last_mask].decay_a=0;
		  new_mask=_dsp_calc_mask(masken[last_mask].delta_spl,slope,attack,masken[last_mask].decay_b,release,masken[last_mask].decay_a);
		  masker+=new_mask;
		  /*masker+=(masken[last_mask].delta_spl*(1-slope)*(1-pow(attack,masken[last_mask].decay_b))*pow(release,masken[last_mask].decay_a));*/
		}
	      
	      toene[0].delta_spl=in-threshold-masker;
	      if(toene[0].delta_spl>0)
		{
		  if(toene[0].decay_b<MAX_DECAY)
		    toene[0].decay_b++;
		  toene[0].decay_a=0;
		  last_tone=0;
		}
	      else
		{
		  last_tone=-1;
		}
	      last_in=in-masker;
	    }

	}
      else if((in-masker)>last_in)
	{
	  /* Ton ist staerker geworden */
	  last_in=(in-masker);
	  for(i=0;i<=last_tone;i++)
	    {
	      if(toene[i].decay_b<MAX_DECAY)
		toene[i].decay_b++; /* Alle vorherigen Delta-Toene dauern an */
	    }
	  last_tone++;
	  toene[last_tone].delta_spl=in-threshold-masker; /* Neuen Ton speichern */
	  /* Jetz noch alle vorherigen Delta-Werte abziehen, damit
	     neuer Ton als Delta-Wert vorhanden ist */
	  for(i=0;i<last_tone;i++)
	    toene[last_tone].delta_spl-=toene[i].delta_spl;

	  toene[last_tone].decay_b=0;
	  toene[last_tone].decay_a=0;	  

	}
	
    }
  else 
    {
      /* Aktuell keine Ton ueber dem Schwellwert */
      if(probe_on==1)
	{
	  /* Im vorherigen Frame lag Ton ueber Schwellwert => neue Maske */
	  probe_on=0;
	  if(toene[0].decay_b>0)
	    {
	      for(i=0;i<=last_tone;i++)
	      {
		if(toene[i].decay_b>0)
		  {
		    last_mask++;
		    masken[last_mask].delta_spl=toene[i].delta_spl;
		    masken[last_mask].decay_b=toene[i].decay_b;
		    masken[last_mask].decay_a=toene[i].decay_a;
		    masken[last_mask].decay_a = 0;
		    new_mask=_dsp_calc_mask(masken[last_mask].delta_spl,slope,attack,masken[last_mask].decay_b,release,masken[last_mask].decay_a);
		    masker+=new_mask;
		    /*masker+=(masken[last_mask].delta_spl*(1-slope)*(1-pow(attack,masken[last_mask].decay_b))*pow(release,masken[last_mask].decay_a));*/
		  }
	      }
	    }
	  last_tone=-1;
	}
      last_in=in-masker;
    }

  /* Subtraktion der Berechneten Masken */
  out=in-masker;

  params->last_tones[channel] = last_tone; /* Anzahl Toene speichern */
  params->last_masks[channel]=last_mask;   /* Anzahl Masken speichern */
  params->last_ins[channel]=last_in;       /* Letze Tonstaerke speichern */
  params->probes_on[channel]=probe_on;     /* Neuer Ton ? */

  return out;
}


dsp_fwm_param_t* 
dsp_init_forward_masking(mx_real_t *mtf, int n_channel)
     /* Initialisierung der fuer die fuer die Vorwaertsmaskierung benoetigten
	Parameter */
{

  /* Frequenzen, fuer die die Parameter ermittelt bzw. extrapoliert wurden */
  mx_real_t freq[n_param]={100, 250, 500, 1000, 2000, 4000, 8000};

  /*slope*/
  mx_real_t m_param[n_param]={0.18, 0.19, 0.20, 0.26, 0.29, 0.34, 0.41}; 

  /*attack*/
  mx_real_t b_param[n_param]={0.442, 0.474, 0.510, 0.543, 0.525, 0.507, 0.496};

  /*release*/
  mx_real_t a_param[n_param]={0.868, 0.864, 0.854, 0.816, 0.851, 0.858, 0.865};

  /*static threshold*/
  mx_real_t t_param[n_param]={55, 40, 25, 20, 20, 25, 30};
 
  int last_param=n_param-1;
  int i,j,k;
  mx_real_t freq_diff,diff_prev,diff_next;
  mx_real_t weight_prev,weight_next;

  dsp_fwm_param_t* params=NULL;

  params=(dsp_fwm_param_t*) malloc(sizeof(dsp_fwm_param_t));
  params->attack=(mx_real_t*) malloc(sizeof(mx_real_t)*n_channel);
  params->release=(mx_real_t*) malloc(sizeof(mx_real_t)*n_channel);
  params->slope=(mx_real_t*) malloc(sizeof(mx_real_t)*n_channel);
  params->threshold=(mx_real_t*) malloc(sizeof(mx_real_t)*n_channel);
  params->hoerschwelle=(mx_real_t*) malloc(sizeof(mx_real_t)*n_channel);

  params->toene_vektor=(tone_t**) malloc(sizeof(tone_t*)*n_channel);
  for(i=0;i<n_channel;i++)
    {
      params->toene_vektor[i]=(tone_t*) malloc(sizeof(tone_t)*MAX_TOENE);
      for(j=0;j<MAX_TOENE;j++)
	{
	  params->toene_vektor[i][j].delta_spl=0.0;
	  params->toene_vektor[i][j].decay_a=0.0;
	  params->toene_vektor[i][j].decay_b=0.0;
	}
    }
  params->last_tones=(int*) malloc(sizeof(int)*n_channel);
  for(i=0;i<n_channel;i++)
    {
      params->last_tones[i]=-1;
    }
  params->masken_vektor=(tone_t**) malloc(sizeof(tone_t*)*n_channel);
  for(i=0;i<n_channel;i++)
    {
      params->masken_vektor[i]=(tone_t*) malloc(sizeof(tone_t)*MAX_TOENE);
      for(j=0;j<MAX_TOENE;j++)
	{
	  params->masken_vektor[i][j].delta_spl=0.0;
	  params->masken_vektor[i][j].decay_a=0.0;
	  params->masken_vektor[i][j].decay_b=0.0;
	}
    }
  params->last_masks=(int*) malloc(sizeof(int)*n_channel);
  for(i=0;i<n_channel;i++)
    {
      params->last_masks[i]=-1;
    }
  
  params->last_ins=(mx_real_t*) malloc(sizeof(mx_real_t)*n_channel);
  for(i=0;i<n_channel;i++)
    {
      params->last_ins[i]=0.0;
    }
  
  params->probes_on=(int*) malloc(sizeof(int)*n_channel);
  for(i=0;i<n_channel;i++)
    {
      params->probes_on[i]=0;
    }  
  
  for(i=0;i<n_channel;i++)
    /*params->hoerschwelle[i]=MIN_DB;*/
    params->hoerschwelle[i]=MX_REAL_MAX;

  i=0;  
  while(mtf[i]<=freq[0])
    {
      rs_msg("ACHTUNG!!! Frequenz %f unterhalb des Parameterbereiches!!!\n",mtf[i]);
      /* ACHTUNG: Parameter werden nicht mehr extrapoliert, sondern von der 
	 niedrigsten berechneten Frequenz direkt uebernommen */
      params->slope[i]=m_param[0];
      params->attack[i]=b_param[0];
      params->release[i]=a_param[0];
      params->threshold[i]=t_param[0];
      i++;
      if(i==n_channel)
	{
	  return params;
	}
    }
  k=0;
  while(mtf[i]<freq[last_param])
    {
      while(freq[k]<mtf[i])
	  k++;
      if(mtf[i]==freq[k])
	{
	  /* Parameter fuer Mittenfrequenz mtf direkt vorhanden */ 
	  params->slope[i]=m_param[k];
	  params->attack[i]=b_param[k];
	  params->release[i]=a_param[k];
	  params->threshold[i]=t_param[k];
	}
      else /* Gewichtete Mittelung der Parameter fuer Mittenfrequenz mtf*/
	{
	  freq_diff=freq[k]-freq[k-1];
	  diff_prev=mtf[i]-freq[k-1];
	  diff_next=freq[k]-mtf[i];
	  weight_prev=1-(diff_prev/freq_diff);
	  weight_next=1-(diff_next/freq_diff);
	  params->slope[i]=(m_param[k-1]*weight_prev)+(m_param[k]*weight_next);
	  params->attack[i]=(b_param[k-1]*weight_prev)+(b_param[k]*weight_next);
	  params->release[i]=(a_param[k-1]*weight_prev)+(a_param[k]*weight_next);      
	  params->threshold[i]=(t_param[k-1]*weight_prev)+(t_param[k]*weight_next);
	}
      i++;
      if(i==n_channel)
	{
	  /* Parameter fuer alle Mittenfrequenzen ermittelt */
	  return params;
	}
    }
  for(j=i;j<n_channel;j++)
    {
      if(mtf[j]>freq[last_param])
	 rs_msg("ACHTUNG!!! Frequenz %f ausserhalb des Parameterbereiches!!!\n",mtf[j]);
      /* ACHTUNG: Parameter werden nicht mehr extrapoliert, sondern von der 
	 hoechsten berechneten Frequenz direkt uebernommen */
      params->slope[j]=m_param[last_param];
      params->attack[j]=b_param[last_param];
      params->release[j]=a_param[last_param];      
      params->threshold[j]=t_param[last_param];
    }
  return params;
}

mx_real_t
dsp_calc_energy(mx_real_t* channels, int n_channel)
{
  /* Neuberechnung der Gesamtenergie */

  int i;
  mx_real_t energy = 0.0;

  for(i = 0; i < n_channel; i++)
    energy+=pow(10,channels[i]);

  energy=dsp_log10(energy);
  

  return energy;
}

void 
dsp_set_minima(dsp_fwm_param_t* params, mx_real_t *spektrum, int n_channel)
{
  /* Festlegung der Frequenzgruppen-Minima */

  int i;

  for(i=0;i<n_channel;i++)
    {
      if(spektrum[i]<params->hoerschwelle[i])
	params->hoerschwelle[i]=spektrum[i];
    }
}

void 
dsp_apply_minima(dsp_fwm_param_t* params, mx_real_t *spektrum, int n_channel)
{
  /* Anwendung der Frequenzgruppen-Minima */

  int i;

  for(i=0;i<n_channel;i++)
    {
      if(spektrum[i]<params->hoerschwelle[i])
	spektrum[i]=params->hoerschwelle[i];
    }
      
}
