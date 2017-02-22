// ev_segment.c
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
#include "ev_io.h"

#include "ev_segment.h"

#define SEGMENT_LENGTH  16000
#define MAX_SEGMENTS      20
#define BLOCKSIZE       160

int output =1;

int _asegment_vad(dsp_vad_t *vad, dsp_sample_t *signal, int n_samples, dsp_sample_t ***signal_segment, int **length);
int _asegment_fixed(char *seg_info, dsp_sample_t *signal, int n_samples, dsp_sample_t ***signal_segment, int **length);
int _asegment_info(char *seg_info, dsp_sample_t *signal, int n_samples, dsp_sample_t ***signal_segment, int **length);

/*
    Segmentiert eine einzelne Audio-Datei
  
    Argumente:
      file:          Name der Audio-Datei, die segmentiert werden soll
      method:        dsp_vad_t*    fuer voice activity detection, 
                     int           fuer frames mit fester Laenge
	             char*         fuer Segmentierungsinfo 
      type:          vad, fixed oder info
     signal_segment: Array, der die Signaldaten der Segmente enthaelt
     segment_length: Array, der die Laenge der Segmente enthaelt

    Rueckgabewert:
      Anzahl der Signalabschnitte in signal_segment; 
      -1, falls ein Fehler aufgetreten ist
 */

int emo_afile_segment(char *file, asegmentation_method_t *method, asegmentation_type_t type, dsp_sample_t ***signal_segment, int **segment_length) {
	int size=SEGMENT_LENGTH, n_samples=0, n_segments, samples_read;
	FILE *fp;
	dsp_sample_t *signal=NULL;
	dsp_vad_t *Vad;

	signal = (dsp_sample_t *) rs_malloc(sizeof(dsp_sample_t) * size, "Signal data");
	if (!method)
		method = (asegmentation_method_t *) rs_malloc(sizeof(asegmentation_method_t),"Audio segmentation method");

	if (strcmp(file,"-")==0)
		fp = stdin;
	else 
		fp = fopen(file,"r");
	if (!fp) {
		rs_warning("Cannot open file %s!",file);
		return -1;
	}

	while ((samples_read =fread(signal+n_samples,sizeof(dsp_sample_t),BLOCKSIZE,fp)) && samples_read >0) {
		n_samples+=samples_read;
		if (size <= n_samples) {
			size +=SEGMENT_LENGTH;
			signal = (dsp_sample_t *) rs_realloc(signal,sizeof(dsp_sample_t) * size, "Signal data");
		}
		if (samples_read != BLOCKSIZE)
			break;
	}
	fclose(fp);


	if (type == vad && !method->vad) {
		Vad  = dsp_vad_create(DSP_MK_VERSION(1,0),VAD_FRAME_LEN);
		method->vad = Vad;
	}

	n_segments = emo_asegment(method,type,signal,n_samples,signal_segment,segment_length);
	if (n_segments == -1)
		rs_error("Aborting during procession of file %s!",file);

	rs_free(signal);

	return n_segments;
}



/*
    Segmentiert eine Liste von Dateien
  
    Argumente:
      filelist:   Name der Datei, die die Dateien enthaelt, die segmentiert 
                  werden sollen
      method: dsp_vad_t*    fuer voice activity detection, 
              int           fuer frames mit fester Laenge
	      char*         fuer Segmentierungsinfo 
      type:   vad, fixed oder info

    Rueckgabewert:
      Anzahl der Signalabschnitte in signal_segment; 
      -1, falls ein Fehler aufgetreten ist
 */
int emo_afilelist_segment(char *filelist, asegmentation_method_t *method, asegmentation_type_t type,int audio_output){
	FILE *fp;
	int n_segments=0;
	int *segment_length=NULL;
	dsp_sample_t **signal_segment=NULL;

	fp = fopen(filelist,"r");
	if (!fp) {
		rs_warning("Cannot open input file: %s!",filelist);
		return -1;
	}

	switch (type) {
		case vad: {
			int stat=0, x=0;
			char filename[STRING_LENGTH], outdir[STRING_LENGTH];
	    
			if (!method)
				method = (asegmentation_method_t *) rs_malloc(sizeof(asegmentation_method_t),"Audio segmentation method");
			if (!method->vad)
				method->vad = dsp_vad_create(DSP_MK_VERSION(1,0),VAD_FRAME_LEN);
	    //rest der zeile ueberspringen
			while(fscanf(fp,"%s\t%s",filename,outdir)==2 && stat >=0) {
				if (output)
					fprintf(stderr,"%s\t%s\t",filename,outdir);
				stat=emo_afile_segment(filename,method,vad,&signal_segment,&segment_length);
				if (stat == -1){
					rs_warning("Segmentation of file %s failed!",filename);
					return -1;
				}
				if (audio_output && stat != emo_afile_output(filename,outdir,stat,signal_segment,segment_length)) {
					rs_warning("Output of segments of file %s failed!",filename);
					return -1;
				}
				if (segment_length)
					rs_free(segment_length);
				if (signal_segment) {
					int i;
					for (i=0;i<stat;i++)
						if (signal_segment[i])
							rs_free(signal_segment[i]);
					rs_free(signal_segment);
				}
				n_segments+=stat;
				while (x != '\n' && x != EOF)
					x=fgetc(fp);
				x=0;
			}
			break;
		}
		case fixed: {
			int stat=0, x=0;
			char filename[STRING_LENGTH], outdir[STRING_LENGTH];
	    
			if (!method->segmentation_info) {
				rs_warning("No segment length or shift info available!");
				return -1;
			}
	    
			while(fscanf(fp,"%s\t%s",filename,outdir)==2 && stat >=0) {
				stat=emo_afile_segment(filename,method,fixed,&signal_segment,&segment_length);
				if (stat == -1){
					rs_warning("Segmentation of file %s failed!",filename);
					return -1;
				}
				if (audio_output) {
					stat = emo_afile_output(filename,outdir,stat,signal_segment,segment_length);
					if (stat == -1){
						rs_warning("Output of segments of file %s failed!",filename);
					}
		    
					return -1;
				}
				n_segments+=stat;
				while (x != '\n' && x != EOF)
					x=fgetc(fp);
				x=0;
			}
			break;
		}
		case info: {
			int stat=0;
			char filename[STRING_LENGTH], outdir[STRING_LENGTH], seg_info[10*STRING_LENGTH];

			if (!method->segmentation_info) {
				rs_warning("No segment length or shift info available!");
				return -1;
			}
	    
			while(fscanf(fp,"%s\t%s\t%[^\n]s",filename,outdir,seg_info)==3 && stat >=0) {
				method->segmentation_info=seg_info;
				stat=emo_afile_segment(filename,method,info,&signal_segment,&segment_length);
	    
				if (stat == -1){
					rs_warning("Segmentation of file %s failed!",filename);
					return -1;
				}

				if (audio_output && stat != emo_afile_output(filename,outdir,stat,signal_segment,segment_length)) {
					rs_warning("Output of segments of file %s failed!",filename);
					return -1;
				}
				n_segments+=stat;
			}
			break;
		}
		default: rs_warning("Unknown segmentation type: %d",type);
		return -1;
	}
    
	fclose(fp);
	return n_segments;
}


/*  
    Segmentiert ein Signal in Signalabschnitte.
    Folgende Segmentierungen sind moeglich:
      - Pausen als Segmentgrenzen, 
        d.h. Vorhandensein von Stimme ('voice activity') wird ueberprueft
      - feste Segmentlaengen
      - vorgegebene Segmentierungsvorschrift der Form 
           a..b(-c..d)*(,e..f(-g..h)*)*
        mit a-h: Sample-Indexe
	Bedeutung: ',' trennen in Segmente,
	           '..' gibt Bereiche an, die in ein Segment gehoeren
		   '-' gibt Bereiche an, die nicht in ein Segment gehoeren
 
    Argumente:
      method:         dsp_vad_t*    fuer voice activity detection, 
                      int           fuer frames mit fester Laenge
	              char*         fuer Segmentierungsinfo 
      type:           vad, fixed oder info
      signal:         Sample-Werte des Signals
      n_samples:      Anzahl der Samples im Signal
      signal_segment: Signalabschnitte nach Segmentierung
      length:         Laenge der Signalsegmente
      
    Rueckgabewert:
      Anzahl der Signalabschnitte in signal_segment; 
      -1, falls ein Fehler aufgetreten ist
 */
int emo_asegment(asegmentation_method_t *method, asegmentation_type_t type, dsp_sample_t *signal, int n_samples, dsp_sample_t ***signal_segment, int **length) {
	switch (type) {
		case vad: 
			return _asegment_vad(method->vad,signal,n_samples,signal_segment,length);
		case fixed: 
			return _asegment_fixed(method->segmentation_info,signal,n_samples,signal_segment,length);
		case info: 
			return _asegment_info(method->segmentation_info,signal,n_samples,signal_segment,length);

		default:
			return -1;
	}    
}


/* 
   Gibt die Segmente einer Datei aus, entweder in Dateien oder auf die 
   Standardausgabe

   Argumente:
     file:           Name der Audiodatei, die segmentiert werden soll
     outdir:         Verzeichnis, in dem die Segmente gespeichert werden sollen
     n_segments:     Anzahl der Segmente der Audiodatei
     signal_segment: Array, der die Signaldaten der Segmente enthaelt
     segment_length: Array, der die Laenge der Segmente enthaelt

     Rueckgabe:
       0:            Kein Fehler
       -1:           Fehler
                     (besser: Anzahl der geschriebenen Segmente...)
     
*/
int emo_afile_output(char *file, char *outdir, int n_segments, dsp_sample_t **signal_segment, int *segment_length){
	int i=0, n_segments_out=0;
	char outfile[STRING_LENGTH], *name;
	FILE *out_fp;
   
	if (outdir && strcmp(outdir,"-")==0) {
		for (i=0;i<n_segments;i++) {
			if (output)
				fprintf(stderr,"%d\n",segment_length[i]);
			fwrite(signal_segment[i],sizeof(dsp_sample_t),segment_length[i],stdout);
			n_segments_out++;
		}
	}
	else {
		if (strcmp(file,"-")!=0) {
			int len;
			i=strlen(file)-1;
			while (i>=0 && file[i] != '.' && file[i] != '/')
				i--;
			if (i<0 || file[i] == '/')
				i=strlen(file);
			len=strrchr(file,'/')?strlen(strrchr(file,'/')):0;
			len = i+1-(strrchr(file,'/')?strlen(file)-strlen(strrchr(file,'/'))+1:0);
			name = (char *) rs_malloc(len*sizeof(char),"Target file name");
			strncpy(name,strrchr(file,'/')?strrchr(file,'/')+1:file,len-1);
			name[len-1]='\0';
		    
			while (i>=0 && file[i] != '/')
				i--;
		}
		else 
			name = "";
	
		if (!outdir) {
			if (strcmp(file,"-")!=0) {
				i=strlen(file);
				outdir = (char *) rs_malloc((i+1)*sizeof(char),"Target directory name");
				outdir[i]='\0';
				//
				while (i>-1) {
					outdir[i]=file[i];
					i--;
				}
			}
			else 
				outdir="";
		}
		else
			if (outdir[strlen(outdir)-1] != '/') {
			int len=strlen(outdir);
			char *temp= rs_malloc((len+2)*sizeof(char),"Audio output directory name");
			for (i=0;i<len;i++)
				temp[i]=outdir[i];
			temp[len]='/';
			temp[len+1]='\0';
			outdir=temp;
			}
	
			for (i=0;i<n_segments;i++) {
				if (i<9)
					sprintf(outfile,"%s%s_0%d.pcm",outdir,name,i+1);
				else 
					sprintf(outfile,"%s%s_%d.pcm",outdir,name,i+1);
				out_fp = fopen(outfile,"w");
				if (!out_fp) {
					rs_warning("Cannot write signal segment %d to file %s!",i,outfile);
					break;
				}
				if (output)
					fprintf(stderr,"\tSegment %d has length %d\n",i+1,segment_length[i]);
				if (fwrite(signal_segment[i],sizeof(dsp_sample_t),segment_length[i],out_fp) == segment_length[i])
					n_segments_out++;
				fclose(out_fp);
			}
			rs_free(name);
	}
	return n_segments_out;
}


int _asegment_vad(dsp_vad_t *vad, dsp_sample_t *signal, int n_samples, dsp_sample_t ***signal_segment, int **length) {
	int va=0, i, out_sample;
	int n_segments =0, last_va=0, samples=0;
	int max_segments=MAX_SEGMENTS, segment_length = SEGMENT_LENGTH;
	int delay_len=vad->sigbuf->length;
	int *_length;
	dsp_sample_t *voice;
	dsp_sample_t **_segments;
	int temp=0;
	dsp_sample_t *_signal;

	_signal = (dsp_sample_t *) rs_malloc(n_samples*sizeof(dsp_sample_t),"signal copy");
	for (i=0;i<n_samples;i++)
		_signal[i]=signal[i];

	if (!vad) {
		rs_warning("No voice activity detection info available!");
		return -1;
	}
	voice = (dsp_sample_t *) rs_malloc(vad->frame_len * sizeof(dsp_sample_t),"Voice frame");

	_segments = (dsp_sample_t **) rs_malloc(max_segments * sizeof(dsp_sample_t *),"Signal segments");
	_length = (int *) rs_malloc(max_segments * sizeof(int),"Segment lengths");
    
	for (i=0;i<=n_samples-VAD_FRAME_SHIFT || va >=0;i+=VAD_FRAME_SHIFT) {
		if (i > n_samples-VAD_FRAME_SHIFT) {
			va = dsp_vad_calc(voice,vad,NULL);
		}
		else {
			if (i>n_samples-vad->frame_len) {
				int j, new_len=i+vad->frame_len;
				_signal= (dsp_sample_t *) rs_realloc(_signal,new_len*sizeof(dsp_sample_t),"Signal buffer");
				for (j=n_samples;j<new_len;j++)
					_signal[j]=0;
			}
			va = dsp_vad_calc(voice, vad, _signal+i);
		}
		if (va >=0 && ((va && !last_va) || (!va && last_va)) && samples >0) {
			out_sample=i-(delay_len-vad->sigbuf->need_elems)*160;
			if (out_sample>=n_samples)
				out_sample=n_samples-1;
			if (va) {
				if (n_segments >= max_segments) {
					max_segments += MAX_SEGMENTS;
					_segments = (dsp_sample_t **) rs_realloc(_segments,max_segments * sizeof(dsp_sample_t *),"Signal segments");
					_length = (int *) rs_realloc(_length,max_segments * sizeof(int),"Segment lengths");
				}
				_segments[n_segments] = (dsp_sample_t *) rs_malloc(segment_length * sizeof(dsp_sample_t),"Signal segment");
				if (output)
					fprintf(stderr,"[%d..",out_sample);
			}
			else {
				if (output)
					fprintf(stderr,"%d] ",out_sample-1);
				_length[n_segments]=last_va*VAD_FRAME_SHIFT;
				n_segments++;
				segment_length=SEGMENT_LENGTH;
			}
		}


		if (!va) 
			last_va=0;
	
		if (va==1 && samples>0) {
			if ((last_va+1)*VAD_FRAME_SHIFT > segment_length) {
				segment_length += SEGMENT_LENGTH;
				_segments[n_segments] = (dsp_sample_t *) rs_realloc(_segments[n_segments],segment_length * sizeof(dsp_sample_t),"Signal segment");
			}
			samples+=VAD_FRAME_SHIFT;
			memcpy(_segments[n_segments]+last_va*VAD_FRAME_SHIFT,voice,VAD_FRAME_SHIFT*sizeof(dsp_sample_t));
			last_va++;
		}
	
		temp=0;
		while (va==0) {
			temp++;
			samples+=VAD_FRAME_SHIFT;
			va = dsp_vad_calc(voice, vad, NULL);
		}
	}

	if (last_va && samples >0) {
		out_sample=i-(delay_len-vad->sigbuf->need_elems)*160;
		if (out_sample>=n_samples)
			out_sample=n_samples;
		if (output)
			fprintf(stderr,"%d] ",out_sample-1);
		_length[n_segments]=last_va*VAD_FRAME_SHIFT;
		n_segments++;
	}
	if (output)
		fprintf(stderr,";\n");
    
	rs_free(voice);
	rs_free(_signal);
	*signal_segment=_segments;
	*length=_length;
	return n_segments;
}


int _asegment_fixed(char *seg_info, dsp_sample_t *signal, int n_samples, dsp_sample_t ***signal_segment, int **length) {
	int i, frame_length, frame_shift;
	int n_segments =0;
	int *_length;
	dsp_sample_t **_segments;
	
	if (sscanf(seg_info,"%d:%d",&frame_length,&frame_shift) != 2) {
		rs_error("Frame length and/or frame shift information in wrong format!");
		return -1;
	}
	n_segments= (n_samples - frame_length) / frame_shift + 1;

	_segments = (dsp_sample_t **) rs_malloc(n_segments * sizeof(dsp_sample_t *),"Signal segments");
	_length = (int *) rs_malloc(n_segments*sizeof(int),"Segment lengths");

	for (i=0;i<n_segments;i++) {
		_segments[i] = (dsp_sample_t *) rs_malloc(frame_length * sizeof(dsp_sample_t),"signal segments"); 
		_length[i]=frame_length;
	
		if (!memcpy(_segments[i],signal+i*frame_shift,frame_length*sizeof(dsp_sample_t))) {
			rs_warning("Signal was shorter than expected!");
			return -1;
		}
	}

	*signal_segment=_segments;
	*length=_length;
	return n_segments;
}

int _asegment_info(char *seg_info, dsp_sample_t *signal, int n_samples, dsp_sample_t ***signal_segment, int **length) {
	char delim;
	int stat=0, frame_length, start, end, i;
	int last=0, n_segments=0;
	int max_segments=MAX_SEGMENTS;
	int *_length;
	dsp_sample_t **_segments;
    
	_segments = (dsp_sample_t **) rs_malloc(max_segments * sizeof(dsp_sample_t *),"Signal segments");
	_length = (int *) rs_malloc(max_segments * sizeof(int),"Segment lengths");    
	if ((seg_info = strchr(seg_info,'[')))
		seg_info++;
    

	while (seg_info && (stat = sscanf(seg_info, "%d..%d%c", &start,&end,&delim)) == 3) {
		if (end +1 > n_samples) {
			rs_warning("Wrong segmentation info: Trying to read %d samples from an audio signal that is only %d samples long!",end,n_samples);
			return -1;
		}
		if (n_segments >= max_segments) {
			max_segments+=MAX_SEGMENTS;
			_segments = (dsp_sample_t**) rs_realloc(_segments,max_segments*sizeof(dsp_sample_t *),"Signal segments");
			_length = (int *) rs_realloc(_length,max_segments*sizeof(int),"Segment lengths");
		}
	    
		frame_length = end-start+1;
		if (last) {
			_segments[n_segments] = (dsp_sample_t *) rs_realloc(_segments[n_segments],(last+frame_length)*sizeof(dsp_sample_t),"signal segments");
			_length[n_segments]+=frame_length;
		}
		else {
			_segments[n_segments] = (dsp_sample_t *) rs_malloc(frame_length *sizeof(dsp_sample_t),"signal segments");
			_length[n_segments]=frame_length;
		}

		memcpy(_segments[n_segments]+last,signal+start,frame_length*sizeof(dsp_sample_t));
	
		switch (delim) {
			case '-': 
				last+=frame_length;
				if ((seg_info = strchr(seg_info,'-')))
					seg_info++;
				break;
			case ']':
				last=0;
				n_segments++;
				if ((seg_info = strchr(seg_info,'[')))
					seg_info++;
				break;
			default:
				rs_warning("Problem with segmentation information: wrong delimiter '%c'!",delim);
				rs_free(_length);
				for (i=0;i<n_segments;i++)
					rs_free(_segments[i]);
				rs_free(_segments);
				return -1;
		}
	}
    
	if (stat !=3) {
		rs_warning("Problem with segmentation information: %s!",seg_info);
		return -1;
	}
    
	*signal_segment=_segments;
	*length=_length;
	return n_segments;
}


void emo_aset_output(int op) {
	output=op;
}
