// arff_scanner.cpp
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

#include "arff_scanner.h"

namespace ssi {

const char ArffScanner::NEWLINE = '\n';


ArffScanner::ArffScanner(const std::string& _file): m_file(_file),
                                            m_line((int64)0),
                                            m_col((int64)0),
                                            m_char(NEWLINE),
                                            m_prev_char((char)0),
                                            m_fp(NULL) {
    m_fp = fopen(m_file.c_str(), "r");
    if(m_fp == NULL) {
        THROW("ArffScanner: failed to open the file '%s'!", m_file.c_str());
    }
}

ArffScanner::~ArffScanner() {
    if(m_fp != NULL) {
        fclose(m_fp);
        m_fp = NULL;
    }
}

bool ArffScanner::is_newline(char c) const {
    return (c == NEWLINE);
}

char ArffScanner::next() {
    if(eof()) {
        return (char)-1;
    }
    if(is_newline(m_char)) {
        ++m_line;
        m_col = 0;
    }
    ++m_col;
    m_prev_char = m_char;
    if(fread(&m_char, sizeof(m_char), 1, m_fp) != sizeof(m_char)) {
        m_char = (char)-1;  // you would have reached end-of-file?
    }
    return m_char;
}

char ArffScanner::current() const {
    return m_char;
}

char ArffScanner::previous() const {
    return m_prev_char;
}

int64 ArffScanner::line() const {
    return m_line;
}

int64 ArffScanner::column() const {
    return m_col;
}

bool ArffScanner::eof() const {
    return feof(m_fp) != 0;
}

std::string ArffScanner::err_msg(const std::string& msg) const {
    std::string err = m_file + ":" + num2str<int64>(m_line);
    err += ":" + num2str<int64>(m_col) + " " + msg;
    return err;
}

}
