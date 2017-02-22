// arff_attr.h
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

#ifndef __INCLUDED_ARFF_ATTR_H__
#define __INCLUDED_ARFF_ATTR_H__
/**
 * @file arff_attr.h
 * @brief Contains 'ArffAttr' class
 */

#include <string>
#include "arff_value.h"

namespace ssi {

/**
 * @class ArffAttr arff_attr.h
 * @brief Class to represent an ARFF attribute
 */
class ArffAttr {
public:
    /**
     * @brief Constructor
     * @param name name of this attribute
     * @param type attribute type
     */
    ArffAttr(const std::string& name, ArffValueEnum type);

    /**
     * @brief Destructor
     */
    ~ArffAttr();

    /**
     * @brief Name of this attribute
     * @return name
     */
    std::string name() const;

    /**
     * @brief Type of this attribute
     * @return type
     */
    ArffValueEnum type() const;


private:
    /** attribute name */
    std::string m_name;
    /** attribute type */
    ArffValueEnum m_enum;
};

}

/* DO NOT WRITE ANYTHING BELOW THIS LINE!!! */
#endif // __INCLUDED_ARFF_ATTR_H__
