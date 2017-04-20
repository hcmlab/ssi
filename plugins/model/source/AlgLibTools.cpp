// AlgLibTools.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2012/12/23
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute itand/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#include "AlgLibTools.h"
#include "ssiml/include/ISSelectClass.h"
using namespace alglib_impl;

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

void AlgLibTools::Stream2vector (
	ssi_stream_t &stream,	
    ae_vector* v,
    ae_state *state)
{
	
	ae_int_t num = stream.dim;	
	ae_int_t j = 0;

    ae_vector_clear(v);
    ae_vector_set_length(v, num, state);
	
	ssi_real_t *ptr = ssi_pcast (ssi_real_t, stream.ptr);
    for (j = 0; j <= num-1; j++)
    {
		v->ptr.p_double[j] = ssi_cast (double, *ptr++);
    }
}

void AlgLibTools::Vector2stream (
	ae_vector* v,
	ssi_stream_t &stream,	    
    ae_state *state)
{
	ae_int_t num = v->cnt;
	ae_int_t j = 0;

	if (! (stream.num == 1 && stream.type == SSI_REAL && stream.dim == num)) {
		ssi_stream_destroy (stream);
		ssi_stream_init (stream, 1, ssi_cast (ssi_size_t, num), sizeof (ssi_real_t), SSI_REAL, 0, 0);
	}
	
	ssi_real_t *ptr = ssi_pcast (ssi_real_t, stream.ptr);
    for(j=0; j<=num-1; j++)
    {
		*ptr++ = ssi_cast (ssi_real_t, v->ptr.p_double[j]);
    }
}

void AlgLibTools::Sample2vector (
	ssi_sample_t &sample,
	ssi_size_t stream_id,
    ae_vector* v,
    ae_state *state)
{
	
	ae_int_t num = sample.streams[stream_id]->dim;	
	ae_int_t j = 0;

    ae_vector_clear(v);
    ae_vector_set_length(v, num, state);
	
	ssi_real_t *ptr = ssi_pcast (ssi_real_t, sample.streams[stream_id]->ptr);
    for(j=0; j<=num-1; j++)
    {
		v->ptr.p_double[j] = ssi_cast (double, *ptr++);
    }
}

void AlgLibTools::Samples2matrix (
	ISamples &samples,     
	ssi_size_t stream_id,
    ae_matrix* m,
    ae_state *state)
{
	
	ae_int_t nfeatures = samples.get (0)->streams[stream_id]->dim;
	ae_int_t nsamples = samples.getSize ();

    ae_int_t i = 0;
    ae_int_t j = 0;

    ae_matrix_clear(m);
    ae_matrix_set_length(m, nsamples, nfeatures, state);

	ssi_sample_t *sample;
	samples.reset ();	
	while (sample = samples.next ()) {    
		ssi_real_t *ptr = ssi_pcast (ssi_real_t, sample->streams[stream_id]->ptr);
        for (j = 0; j <= nfeatures-1; j++)
        {
			m->ptr.pp_double[i][j] = ssi_cast (double, *ptr++);
        }
		i++;
    }
}

void AlgLibTools::Samples2matrix (
	ISamples &samples,   
	ssi_size_t stream_id,
	ssi_size_t class_id,
    ae_matrix* m,
    ae_state *state)
{
	
	ae_int_t nfeatures = samples.get (0)->streams[stream_id]->dim;
	ae_int_t nsamples = samples.getSize (class_id);
    ae_int_t i = 0;
    ae_int_t j = 0;

    ae_matrix_clear(m);    
    ae_matrix_set_length(m, nsamples, nfeatures, state);

	ssi_sample_t *sample;
	ISSelectClass samples_s (&samples);
	samples_s.setSelection (class_id);
	samples_s.reset ();
	while (sample = samples_s.next ()) {    
		ssi_real_t *ptr = ssi_pcast (ssi_real_t, sample->streams[stream_id]->ptr);
        for(j=0; j<=nfeatures-1; j++)
        {
			m->ptr.pp_double[i][j] = ssi_cast (double, *ptr++);
        }        
		i++;
    }
}


void AlgLibTools::Samples2MatrixWithClass (ISamples &samples,     
		ssi_size_t stream_id, ae_matrix* m) {

	ae_int_t nfeatures = samples.get (0)->streams[stream_id]->dim;
	ae_int_t nsamples = samples.getSize ();

    ae_int_t i = 0;
    ae_int_t j = 0;
    ae_state state;
    ae_matrix_clear(m);
    ae_matrix_set_length(m, nsamples, nfeatures+1, &state);

	ssi_sample_t *sample;
	samples.reset ();	
	while (sample = samples.next ()) {    
		ssi_real_t *ptr = ssi_pcast (ssi_real_t, sample->streams[stream_id]->ptr);
        for (j = 0; j <= nfeatures-1; j++)
        {
			m->ptr.pp_double[i][j] = ssi_cast (double, *ptr++);
        }
		m->ptr.pp_double[i][j] = ssi_cast (double, sample->class_id);
		i++;
    }

	//delete sample;
}

void AlgLibTools::Invert (const ae_matrix *m, 
	ae_matrix *m_inv, 
	ae_state *state) {

	ae_int_t rows = m->rows;
	ae_int_t cols = m->cols;

	ae_matrix_clear (m_inv);
	ae_matrix_set_length (m_inv, cols, rows, state);

	for (ae_int_t r = 0; r < rows; r++) {
		for (ae_int_t c = 0; c < cols; c++) {
			m_inv->ptr.pp_double[c][r] = m->ptr.pp_double[r][c];
		}
	}
}

void AlgLibTools::Print (FILE *file, ae_matrix* m) {

	ae_int_t nfeatures = m->cols;
	ae_int_t nsamples = m->rows;
	ae_int_t i = 0;
    ae_int_t j = 0;

	for(i=0; i<=nsamples-1; i++) {    
        for(j=0; j<=nfeatures-1; j++)
        {
			ssi_fprint (file, "%.6lf ", m->ptr.pp_double[i][j]);
        }        		
		ssi_fprint (file, "\n");
    }
}

void AlgLibTools::Print (FILE *file, ae_vector* v) {

	ae_int_t nfeatures = v->cnt;
	ae_int_t i = 0;
    ae_int_t j = 0;
  
    for(j=0; j<=nfeatures-1; j++)
    {
		ssi_fprint (file, "%.6lf ", v->ptr.p_double[j]);
    }        		
	ssi_fprint (file, "\n");
}

void AlgLibTools::Meanm (const ae_matrix* m, ae_vector* mean, ae_state *state) {

	ae_int_t nfeatures = m->cols;
	ae_int_t nsamples = m->rows;
	ae_int_t i = 0;
    ae_int_t j = 0;

	ae_vector_clear(mean);    
    ae_vector_set_length(mean, nfeatures, state);
	for(j=0; j<=nfeatures-1; j++)
    {
		mean->ptr.p_double[j] = 0;
	}

	for(i=0; i<=nsamples-1; i++) {    
        for(j=0; j<=nfeatures-1; j++)
        {
			mean->ptr.p_double[j] += m->ptr.pp_double[i][j];
        }        
    }

	for(j=0; j<=nfeatures-1; j++)
    {
		mean->ptr.p_double[j] /= nsamples;
	}
}

void AlgLibTools::Meanms (ssi_size_t n, const ae_matrix *ms, ae_matrix* meanm, ae_state *state) {

	ae_int_t cols = ms[0].cols;
	ae_int_t rows = ms[0].rows;
	ae_int_t i = 0;
    ae_int_t j = 0;

	ae_matrix_clear(meanm);    
    ae_matrix_set_length(meanm, rows, cols, state);

	for(i=0; i<=rows-1; i++) {    
		for(j=0; j<=cols-1; j++)
		{
			meanm->ptr.pp_double[i][j] = 0;
		}        
	}

	for (ssi_size_t k=0; k < n; k++) {
		for(i=0; i<=rows-1; i++) {    
			for(j=0; j<=cols-1; j++)
			{
				meanm->ptr.pp_double[i][j] += ms[k].ptr.pp_double[i][j];
			}        
		}
	}

	for(i=0; i<=rows-1; i++) {    
		for(j=0; j<=cols-1; j++)
		{
			meanm->ptr.pp_double[i][j] /= n;
		}        
	}
}

void AlgLibTools::Subv (ae_vector* r, const ae_vector* a, const ae_vector* b, ae_state *state) {

	ae_int_t cnt = a->cnt;
	ae_int_t i = 0;

	ae_vector_clear (r);
	ae_vector_set_length (r, cnt, state);

	for (i=0; i < cnt; i++) {
		r->ptr.p_double[i] = a->ptr.p_double[i] - b->ptr.p_double[i];
	}
}

void AlgLibTools::CreateScaling (const ae_matrix *m, double **minvals, double **maxvals) {

	int i,j;
	double temp;
	ae_int_t rows = m->rows;
	ae_int_t cols = m->cols;

	*minvals = new double[cols];
	*maxvals = new double[cols];

	for (i=0;i<cols;i++){
		(*maxvals)[i]=-DBL_MAX;
		(*minvals)[i]=DBL_MAX;
	}

	for (i=0;i<rows;i++) {		
		for (j=0;j<cols;j++) {			
			temp=m->ptr.pp_double[i][j];					
			if (temp < (*minvals)[j])
				(*minvals)[j] = temp;
			if (temp > (*maxvals)[j])
				(*maxvals)[j] = temp;
		}
	}
}

void AlgLibTools::ApplyScaling (ae_matrix *m, const double *minvals, const double *maxvals) {

	int i,j;

	ae_int_t rows = m->rows;
	ae_int_t cols = m->cols;

	for (i=0;i<rows;i++) {		
		for (j=0;j<cols;j++) {			
			m->ptr.pp_double[i][j] = (m->ptr.pp_double[i][j] - minvals[j]) / (maxvals[j] - minvals[j]);
		}
	}
}

void AlgLibTools::ApplyScaling (ae_vector *v, const double *minvals, const double *maxvals) {

	int j;

	ae_int_t cols = v->cnt;
	
	for (j=0;j<cols;j++) {			
		v->ptr.p_double[j] = (v->ptr.p_double[j] - minvals[j]) / (maxvals[j] - minvals[j]);
	}
}

void AlgLibTools::Write (FILE *file, ae_matrix* m) {

	ae_int_t i, j;

	ae_int_t rows = m->rows;
	ae_int_t cols = m->cols;

	fwrite (&rows, sizeof (rows), 1, file);
	fwrite (&cols, sizeof (cols), 1, file);	

	for (i = 0; i < rows; i++) {			
		for (j = 0; j < cols; j++) {			
			fwrite (&m->ptr.pp_double[i][j], sizeof (double), 1, file);
		}
	}
}

void AlgLibTools::Write (FILE *file, ae_vector* v) {

	ae_int_t i;

	ae_int_t cnt = v->cnt;	

	fwrite (&cnt, sizeof (cnt), 1, file);	

	for (i = 0; i < cnt; i++) {			
		fwrite (&v->ptr.p_double[i], sizeof (double), 1, file);		
	}
}

void AlgLibTools::Read (FILE *file, ae_matrix* m) {

	ae_int_t i, j;
	ae_state state;

	ae_int_t rows;
	ae_int_t cols;
	
	fread (&rows, sizeof (rows), 1, file);
	fread (&cols, sizeof (cols), 1, file);	

	ae_matrix_set_length (m, rows, cols, &state);

	for (i = 0; i < rows; i++) {			
		for (j = 0; j < cols; j++) {			
			fread (&m->ptr.pp_double[i][j], sizeof (double), 1, file);
		}
	}
}

void AlgLibTools::Read (FILE *file, ae_vector* v) {

	ae_int_t i;
	ae_state state;

	ae_int_t cnt;	

	fread (&cnt, sizeof (cnt), 1, file);

	ae_vector_set_length (v, cnt, &state);

	for (i = 0; i < cnt; i++) {			
		fread (&v->ptr.p_double[i], sizeof (double), 1, file);		
	}
}

}
