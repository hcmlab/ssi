// VectorFusionTools.cpp
// author: Florian Lingenfelser <florian.lingenfelser@informatik.uni-augsburg.de>
// created: 2012/04/25
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

#include "../include/VectorFusionTools.h"
#include "ioput/file/File.h"
#include "model/ModelTools.h"
#include "ap.h"
#include "dataanalysis.h"

namespace ssi {

ssi_char_t *VectorFusionTools::ssi_log_name = "vftools___";

VectorFusionTools::VectorFusionTools (const ssi_char_t *file){}

VectorFusionTools::~VectorFusionTools (){}

void VectorFusionTools::save_lm(alglib::linearmodel *lm, const ssi_char_t *filepath){

	File *file = File::CreateAndOpen (File::ASCII, File::WRITE, filepath);
	file->setType(SSI_INT);
	file->write (&lm->c_ptr()->w.cnt, sizeof (alglib::ae_int_t), 1);
	file->write (&lm->c_ptr()->w.datatype, sizeof (alglib::ae_int_t), 1);
	file->setType(SSI_DOUBLE);
	for(ssi_size_t i = 0; i < (ssi_size_t)lm->c_ptr()->w.cnt; i++){
		file->write(&lm->c_ptr()->w.ptr.p_double[i], sizeof(SSI_DOUBLE), 1);
	}
	
	delete file;

}

void VectorFusionTools::load_lm(alglib::linearmodel *lm, const ssi_char_t *filepath){

	File *file = File::CreateAndOpen (File::ASCII, File::READ, filepath);
	file->setType(SSI_INT);
	
	file->read (&lm->c_ptr()->w.cnt, sizeof (alglib::ae_int_t), 1);
	file->read (&lm->c_ptr()->w.datatype, sizeof (alglib::ae_int_t), 1);

	alglib_impl::ae_state ar;
	ae_vector_init(&lm->c_ptr()->w, lm->c_ptr()->w.cnt, alglib_impl::DT_REAL, &ar, ae_true);
	ae_vector_set_length(&lm->c_ptr()->w, lm->c_ptr()->w.cnt,&ar);
	
	file->setType(SSI_DOUBLE);
	for(ssi_size_t i = 0; i < (ssi_size_t)lm->c_ptr()->w.cnt; i++){
		file->read(&lm->c_ptr()->w.ptr.p_double[i], sizeof(SSI_DOUBLE), 1);
	}

	delete file;

}

ssi_stream_t* VectorFusionTools::anno2stream(const ssi_char_t* dir, ssi_size_t line_length, ssi_size_t timestamp_index, ssi_size_t label_index){

	File *file = File::CreateAndOpen (File::ASCII, File::READ, dir);
	file->setType(SSI_FLOAT);
	
	ssi_real_t *line_ptr = new ssi_real_t[line_length];
	ssi_size_t data_counter = 0;
	ssi_real_t sr = 0.0f;

	while(file->read(line_ptr, 0, line_length)){
		if(data_counter == 0){
			sr = 1.0f / line_ptr[timestamp_index];
		}
		data_counter++;
	}

	/*ssi_print("\ndata_counter:\t%d\n", data_counter);*/
	
	file->seek(0);
	ssi_real_t *data = new ssi_real_t[data_counter];
	data_counter = 0;
	while(file->read(line_ptr, 0, line_length)){
		data[data_counter] = line_ptr[label_index];
		data_counter++;
	}
	
	/*ssi_print("\ndata:\n");
	for(ssi_size_t ndata = 0; ndata < data_counter; ndata++){
		ssi_print("%f\n", data[ndata]);
	}*/

	ssi_stream_t *stream = new ssi_stream_t();
	ssi_stream_init(*stream, data_counter, 1, sizeof(ssi_real_t), SSI_FLOAT, sr);

	ssi_real_t *data_ptr = ssi_pcast(ssi_real_t, stream->ptr);
	for(ssi_size_t ndata = 0; ndata < stream->num; ndata++){
		*data_ptr = data[ndata];
		data_ptr++;
	}
	data_ptr = 0;
	
	delete file;
	delete [] data;
	delete [] line_ptr;

	return stream;

}

alglib::linearmodel* VectorFusionTools::samples2regfunc(ISamples *samples, ssi_size_t data_index, ssi_size_t anno_index){

	alglib::linearmodel* lm = new alglib::linearmodel();

	alglib::ae_int_t nPoints = samples->getSize();
	alglib::ae_int_t nVars = samples->get(0)->streams[data_index]->dim;

	alglib::ae_int_t nRows = nPoints;
	alglib::ae_int_t nCols = nVars + 1;

	alglib::real_2d_array XY;
	XY.setlength(nRows, nCols);
	
	for(ssi_size_t n_rows = 0; n_rows < (ssi_size_t)nRows; n_rows++){
		ssi_real_t *ptr = ssi_pcast(ssi_real_t, samples->get(n_rows)->streams[0]->ptr);
		for(ssi_size_t n_cols = 0; n_cols < (ssi_size_t)nCols - 1; n_cols++){
			double datavar = (double)*ptr++;
			XY(n_rows, n_cols) = datavar;
		}
		ptr = ssi_pcast(ssi_real_t, samples->get(n_rows)->streams[anno_index]->ptr);
		ssi_real_t regvar;
		ssi_mean(samples->get(n_rows)->streams[anno_index]->num, samples->get(n_rows)->streams[anno_index]->dim, ptr, &regvar);
		XY(n_rows, nCols - 1) = regvar;
	}
	
	alglib::ae_int_t info;
	alglib::lrreport ar;
	
	lrbuild(XY, nPoints, nVars, info, *lm, ar);

	if(info == 1){
		return lm;
	}else{
		ssi_wrn ("model for linear regression not built");
		return 0;
	}

}

}
