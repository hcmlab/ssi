// arff_token.cpp
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

#include "arff_token.h"

namespace ssi {

std::string arff_token2str(ArffTokenEnum type) {
    switch(type) {
    case RELATION:      return "RELATION";
    case ATTRIBUTE:     return "ATTRIBUTE";
    case DATA_TOKEN:    return "DATA_TOKEN";
    case MISSING_TOKEN: return "MISSING_TOKEN";
    case NUMERIC_TOKEN: return "NUMERIC_TOKEN";
    case STRING_TOKEN:  return "STRING_TOKEN";
    case DATE_TOKEN:    return "DATE_TOKEN";
    case VALUE_TOKEN:   return "VALUE_TOKEN";
    case BRKT_OPEN:     return "BRKT_OPEN";
    case BRKT_CLOSE:    return "BRKT_CLOSE";
    case END_OF_FILE:   return "END_OF_FILE";
    default:            return "UNKNOWN";
    }
}



ArffToken::ArffToken(const std::string& _str, ArffTokenEnum _token) :
    m_str(_str), m_enum(_token) {
}

ArffToken::ArffToken(const ArffToken& _src) :
    m_str(_src.m_str), m_enum(_src.m_enum) {
}

ArffToken::~ArffToken() {
}

std::string ArffToken::token_str() const {
    return m_str;
}

ArffTokenEnum ArffToken::token_enum() const {
    return m_enum;
}

int32 ArffToken::token_int32() const {
    return (int32)token_int64();
}

int64 ArffToken::token_int64() const {
    if(m_enum != VALUE_TOKEN) {
        THROW("ArffToken::token_int64 token is not '%s', it's '%s'!",
              "VALUE_TOKEN", arff_token2str(m_enum).c_str());
    }
    int64 num;
    str2num<int64>(m_str, num);
    return num;
}

float ArffToken::token_float() const {
    return (float)token_double();
}

double ArffToken::token_double() const {
    if(m_enum != VALUE_TOKEN) {
        THROW("ArffToken::token_double token is not '%s', it's '%s'!",
              "VALUE_TOKEN", arff_token2str(m_enum).c_str());
    }
    double num;
    str2num<double>(m_str, num);
    return num;
}

}
