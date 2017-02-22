// arff_utils.cpp
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

#include <stdexcept>

#include "arff_utils.h"

namespace ssi {

#define STR_LENGTH 2048
void throw_ex(const char* file, int64 line, const char* fmt, ...) {
    char msg[STR_LENGTH];
    va_list va;
    va_start(va, fmt);
    vsprintf(msg, fmt, va);
    va_end(va);
    std::string err(file);
    err += ":" + num2str<int64>(line) + " -- ";
    err += msg;
    std::runtime_error ex(err);
    throw ex;
}
#undef STR_LENGTH

char to_lower(char c) {
    if((c >= 'A') && (c <= 'Z')) {
        return ((c - 'A') + 'a');
    }
    return c;
}

bool icompare(const std::string& str, const std::string& ref) {
    size_t s1 = str.size();
    size_t s2 = ref.size();
    if(s1 != s2) {
        return false;
    }
    const char* ch1 = str.c_str();
    const char* ch2 = ref.c_str();
    for(size_t i=0;i<s1;++i) {
        if(to_lower(ch1[i]) != to_lower(ch2[i])) {
            return false;
        }
    }
    return true;
}

}
