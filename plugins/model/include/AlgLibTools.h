// AlgLibTools.h
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

#pragma once

#ifndef SSI_ALGLIB_TOOLS_H
#define SSI_ALGLIB_TOOLS_H

#include "SSI_Cons.h"
#include "base/ISamples.h"
#include "linalg.h"
using namespace alglib_impl;

namespace ssi {

class AlgLibTools { 

public:

	static void Stream2vector (
		ssi_stream_t &stream,	
		ae_vector* v,
		ae_state *state);

	static void Vector2stream (
		ae_vector* v,
		ssi_stream_t &stream,	    
		ae_state *state);

	static void Sample2vector (ssi_sample_t &sample,
		ssi_size_t stream_id,
		ae_vector* v,
		ae_state *state);

	static void Samples2matrix (
		ISamples &samples,     
		ssi_size_t stream_id,
		ae_matrix* m,
		ae_state *state);

	static void Samples2matrix (
		ISamples &samples,   
		ssi_size_t stream_id,
		ssi_size_t class_id,
		ae_matrix* m,
		ae_state *state);

	static void Samples2MatrixWithClass
		(ISamples &samples,     
		ssi_size_t stream_id,
		ae_matrix* m);

	static void Meanm (const ae_matrix* m, 
		ae_vector* mean, 
		ae_state *state);

	static void Meanms (ssi_size_t n, 
		const ae_matrix *ms, 
		ae_matrix* meanm, 
		ae_state *state);

	static void Subv (ae_vector* r, 
		const ae_vector* a,
		const ae_vector* b,
		ae_state *state);

	static void Invert (const ae_matrix *m, 
		ae_matrix *m_inv, 
		ae_state *state);	

	static void CreateScaling (const ae_matrix *m, 
		double **minvals, 
		double **maxvals);
	static void ApplyScaling (ae_matrix *m, 
		const double *minvals, 
		const double *maxvals);
	static void ApplyScaling (ae_vector *v, 
		const double *minvals, 
		const double *maxvals);

	static void Print (FILE *file, ae_matrix* m);
	static void Print (FILE *file, ae_vector* v);

	static void Write (FILE *file, ae_matrix* m);
	static void Write (FILE *file, ae_vector* v);
	static void Read (FILE *file, ae_matrix* m);
	static void Read (FILE *file, ae_vector* v);
};

}

#endif
