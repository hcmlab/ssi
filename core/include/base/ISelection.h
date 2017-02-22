// ISelection.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2010/05/20
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

#ifndef SSI_ISELECTION_H
#define SSI_ISELECTION_H

#include "base/IModel.h"

namespace ssi {

class ISelection : public IObject {

public:

	struct score {
		ssi_size_t index; // feature index
		ssi_real_t value; // feature score
	};

	virtual void setModel (IModel &model) = 0;
	virtual bool train (ISamples &samples,
		ssi_size_t stream_index) = 0;	
	virtual bool isTrained () = 0;
	virtual bool isWrapper () = 0;
	virtual bool sortByScore () = 0;
	virtual void release () = 0;

	virtual ssi_size_t getSize () = 0;
	virtual const score *getScores () = 0;

	ssi_object_t getType () { return SSI_SELECTION; };

};

}

#endif
