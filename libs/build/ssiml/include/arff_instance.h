// arff_instance.h
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

#ifndef __INCLUDED_ARFF_INSTANCE_H__
#define __INCLUDED_ARFF_INSTANCE_H__
/**
 * @file arff_instance.h
 * @brief Contains the 'ArffInstance' class
 */

#include <vector>
#include "arff_utils.h"
#include "arff_value.h"

namespace ssi {

/**
 * @class ArffInstance arff_instance.h
 * @brief Class to represent one single instance of data
 */
class ArffInstance {
public:
    /**
     * @brief Constructor
     */
    ArffInstance();

    /**
     * @brief Destructor
     */
    ~ArffInstance();

    /**
     * @brief Number of elements in the instance
     * @return number
     */
    int32 size()const;

    /**
     * @brief Add an instance data into the list
     * @param val the data to be added
     *
     * Note that this pointer will be owned by this class from here onwards!
     */
    void add(ArffValue* val);

    /**
     * @brief Get an instance data at the given location
     * @param idx location (starts from 0)
     * @return data
     *
     * Note that this pointer will still be owned by this class!
     */
    ArffValue* get(int idx) const;


private:
    /** instance size */
    int32 m_size;
    /** instance data */
    std::vector<ArffValue*> m_data;
}; 

}

/* DO NOT WRITE ANYTHING BELOW THIS LINE!!! */
#endif // __INCLUDED_ARFF_INSTANCE_H__
