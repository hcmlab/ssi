// arff_scanner.h
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

#ifndef __INCLUDED_ARFF_SCANNER_H__
#define __INCLUDED_ARFF_SCANNER_H__
/**
 * @file arff_scanner.h
 * @brief Contains the 'ArffScanner' class
 */


#include <stdio.h>
#include <string>
#include "arff_utils.h"

namespace ssi {

/**
 * @class ArffScanner arff_scanner.h
 * @brief Class responsible for reading the 'arff' file
 *
 * This class assumes linux-style newlines!
 */
class ArffScanner {
public:
    /**
     * @brief Constructor
     * @param _file file to be read
     */
    ArffScanner(const std::string& _file);

    /**
     * @brief Destructor
     */
    ~ArffScanner();

    /**
     * @brief Return the next character in the stream
     * @return character
     */
    char next();

    /**
     * @brief Returns the currently read char from the file
     * @return current character
     */
    char current() const;

    /**
     * @brief Returns the previously read char from the file
     * @return previous character
     */
    char previous() const;

    /**
     * @brief Returns the current line position
     * @return current line
     */
    int64 line() const;

    /**
     * @brief Returns the current column position
     * @return current column
     */
    int64 column() const;

    /**
     * @brief Whether the file has reached end or not
     * @return true if end-of-file, else false
     */
    bool eof() const;

    /**
     * @brief Give a nice error message along with file,line,col info
     * @param msg actual error message to be prepended with the above info
     * @return prepended 'meaningful' error message
     */
    std::string err_msg(const std::string& msg) const;

    /**
     * @brief Checks whether the given character is newline or not
     * @param c the character
     * @return true if the character is newline, else false
     */
    bool is_newline(char c) const;


    /** new-line character */
    static const char NEWLINE;


private:


    /** file being read */
    std::string m_file;
    /** current line being read */
    int64 m_line;
    /** current position in the row being read */
    int64 m_col;
    /** current character read from the file */
    char m_char;
    /** previous character read from the file */
    char m_prev_char;
    /** file pointer */
    FILE* m_fp;
};

}

/* DO NOT WRITE ANYTHING BELOW THIS LINE!!! */
#endif // __INCLUDED_ARFF_SCANNER_H__
