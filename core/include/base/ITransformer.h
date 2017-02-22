// ITransformer.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/02/26
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute itand/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#pragma once

#ifndef SSI_ITRANSFORMER_H
#define SSI_ITRANSFORMER_H

#include "base/IObject.h"
#include "base/IComponent.h"

/**
 * \brief Interface for a service that manipulates information. 
 *
 * A transformer constantly reads information from one or more buffer, transforms it, 
 * and writes the result back to another buffer. As long as output is produced 
 * with a constant sample rate, its format may differ from the input in sample dimension, 
 * sample rate and sample bytes.
 *
 * In SSI each Transfromer runs in its own thread. Just before the thread is created
 * transform_enter is called. This gives the possibility to complete necessary initialization 
 * steps. Yet, no processing takes place here.
 *
 * Once transform_enter has been passed the thread is created and transform is 
 * constantly called when data is available. The incoming data is passed in the
 * stream_in variable. Since it contains a deep copy of the input buffer changes
 * to the data stream_in points at are possible. However you are not supposed to apply
 * changes to the pointer or the size of the data, nor to any of ther other fields, such as 
 * dimension or sample rate. Note that these values are fixed throughout the whole
 * run. The final output of the transformation is stored in stream_out. The required
 * memory is pre-allocated. Again, you are not supposed to change any other field.
 *
 * When the thread is stopped transform_flush is called. This gives you the chance
 * to clean up temporary memory. Again, no processing takes place here.
 * 
 */

namespace ssi {

class ITransformer : public IObject, public IComponent {

public:

	/**
	* The info struct contains additional information on the streams.
	*/
	struct info {
		ssi_time_t time;
		ssi_size_t frame_num;
		ssi_size_t delta_num;
	};

	/**
	 * \brief Called before the first transformation step.
	 * 
	 * @param stream_in The main input stream.
	 * @param stream_out The output stream.
	 * @param xtra_stream_in_num The number of additional input streams.
	 * @param xtra_stream_in[] An array containing the additional input streams.
	 */ 
	virtual void transform_enter (ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0) {};

	/**
	 * \brief Called at each transformation step.
	 * 
	 * @param sample_number_delta The number of additional samples added to each frame in the main input stream.
	 * @param stream_in The input stream.
	 * @param stream_out The output stream.
	 * @param xtra_stream_in_num The number of additional input streams.
	 * @param xtra_stream_in[] An array containing the additional input streams.
	 */ 
	virtual void transform (ITransformer::info info,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0) = 0;

	/**
	 * \brief Called after the last transformation step.
	 * 
	 * @param stream_in The input stream.
	 * @param stream_out The output stream.
	 * @param xtra_stream_in_num The number of additional input streams.
	 * @param xtra_stream_in[] An array containing the additional input streams.
	 */  
	virtual void transform_flush (ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0) {};

	/**
	 * \brief Returns the dimension of the output stream as a function of the dimension of the input stream.
	 * 
	 * @param sample_dimension_in The dimension of the input stream.
	 * @return The dimension of the output stream.
	 */ 
	virtual ssi_size_t getSampleDimensionOut (ssi_size_t sample_dimension_in) = 0;

	/**
	 * \brief Returns the number of samples in the output stream as a function of the number of samples in the input stream.
	 * 
	 * @param sample_dimension_in The number of samples in the input stream.
	 * @return The number of samples in the output stream.
	 */ 
	virtual ssi_size_t getSampleNumberOut (ssi_size_t sample_number_in) = 0;

	/**
	 * \brief Returns the number of bytes of a sample value in the output stream as a function of the number of bytes of a sample value in the input stream.
	 * 
	 * @param sample_bytes_in The number of bytes of a single sample value in the input stream.
	 * @return The number of bytes of a sample value in the output stream.
	 */ 
	virtual ssi_size_t getSampleBytesOut (ssi_size_t sample_bytes_in) = 0;

	/**
	 * \brief Returns the sample type in the output stream as a function of the sample type of the sample type in the input stream.
	 * 
	 * @param sample_type_in The sample type of a single sample value in the input stream.
	 * @return The sample type of a sample value in the output stream.
	 */ 
	virtual ssi_type_t getSampleTypeOut (ssi_type_t sample_type_in) = 0;

	ssi_object_t getType () { return SSI_TRANSFORMER; };

};

}

#endif
