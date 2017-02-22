// arff_parser.h
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

#ifndef __INCLUDED_ARFF_PARSER_H__
#define __INCLUDED_ARFF_PARSER_H__
/**
 * @file arff_parser.h
 * @brief Contains class 'ArffParser'
 */

#include <string>
#include "arff_lexer.h"
#include "arff_data.h"

namespace ssi {

/**
 * @class ArffParser arff_parser.h
 * @brief Main class for parsing ARFF files
 */
class ArffParser {
public:
    /**
     * @brief Constructor
     * @param _file File to be parsed
     */
    ArffParser(const std::string& _file);

    /**
     * @brief Destructor
     */
    ~ArffParser();

    /**
     * @brief Main function for parsing the file
     * @return the 'ArffData' object after parsing the file
     *
     * Note that this pointer will still be owned by this class!
     */
    ArffData* parse();


private:
    /**
     * @brief Reads the 'relation' token
     */
    void _read_relation();

    /**
     * @brief Reads the attributes
     */
    void _read_attrs();

    /**
     * @brief Reads one attribute
     */
    void _read_attr();

    /**
     * @brief Reads the data
     */
    void _read_instances();


    /** lexer for generating tokens */
    ArffLexer* m_lexer;
    /** whether you have already parsed the file or not */
    bool m_parsed;
    /** the data parsed from the ARFF file */
    ArffData* m_data;
};

}

/* DO NOT WRITE ANYTHING BELOW THIS LINE!!! */
#endif // __INCLUDED_ARFF_PARSER_H__
