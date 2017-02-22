// arff_instance.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/1209
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Smart Sensor Integration (SSI) developed at the 
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

// ARFF file-reader, a C++ library to parse ARFF files
// Copyright (C) 2011 Tejaswi <rao.thejaswi@gmail.com>
// http://ficklemind.blogspot.com/2011/09/arff-file-reader-in-c.html

#include "arff_instance.h"

namespace ssi {

ArffInstance::ArffInstance(): m_size(0), m_data() {
}

ArffInstance::~ArffInstance() {
    std::vector<ArffValue*>::iterator itr;
    for(itr=m_data.begin();itr!=m_data.end();++itr) {
        delete *itr;
    }
}

int32 ArffInstance::size() const {
    return m_size;
}

void ArffInstance::add(ArffValue* val) {
    m_data.push_back(val);
    ++m_size;
}

ArffValue* ArffInstance::get(int idx) const {
    if((idx < 0) || (idx >= m_size)) {
        THROW("ArffInstance::get Index out of bounds! idx=%d size=%d",
              idx, m_size);
    }
    return m_data[idx];
}

}
