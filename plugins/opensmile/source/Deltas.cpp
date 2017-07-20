// Mfcc_Deltas.cpp
// author: Andrew Sadek <andrew.sadek.se@gmail.com>
// created: 2012/04/27 
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

#include "Deltas.h"
#include "base/Factory.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

Deltas::Deltas (const ssi_char_t *file)
	: _file (0) {

	if (file) {

		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	
	}
}

Deltas::~Deltas () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}

	
}

void Deltas::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	
}

void Deltas::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

		ssi_real_t *src = ssi_pcast (ssi_real_t, stream_in.ptr);
		ssi_real_t *dst = ssi_pcast (ssi_real_t, stream_out.ptr);

		ssi_size_t num = stream_in.num - 1;
		ssi_size_t dim = stream_in.dim;
		ssi_size_t new_dim = getSampleDimensionOut(dim);
		ssi_size_t N = 0;
				
		
		for(ssi_size_t i=0; i<num; i++) {

			for(ssi_size_t j=0 ; j<new_dim; j++){

				if(j>=dim){

					dst[N++] = (src[(i*dim)+(j-dim)] - src[((i+1)*dim)+(j-dim)]);
					
				}
				
				else
					
					dst[N++] = src[(i*dim)+j];
			}

		
		}

		for(ssi_size_t k=0 ; k<new_dim; k++){

				if(k>=dim)

					dst[N++] = 0.0;
				
				else

					dst[N++] = src[(num*dim)+k];
		}



		//ssi_print("MFCC_DELTAS ---> Mean = %f \n",dst[51]);
	
}

void Deltas::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	
}


}

