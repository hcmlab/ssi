// arff_lexer.cpp
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

#include "arff_lexer.h"

namespace ssi {

const char ArffLexer::SPACE   = ' ';
const char ArffLexer::TAB     = '\t';
const char ArffLexer::COMMENT = '%';
const char ArffLexer::B_OPEN  = '{';
const char ArffLexer::B_CLOSE = '}';
const char ArffLexer::S_QUOTE = '\'';
const char ArffLexer::D_QUOTE = '"';
const char ArffLexer::COMMA   = ',';
const char ArffLexer::MISS    = '?';


ArffLexer::ArffLexer(const std::string& _file): m_scanner(NULL),
                                                m_b_close(false) {
    m_scanner = new ArffScanner(_file);
}

ArffLexer::~ArffLexer() {
    if(m_scanner != NULL) {
        delete m_scanner;
        m_scanner = NULL;
    }
}

bool ArffLexer::_is_space(char c) const {
    return ((c == SPACE) || (c == TAB) || m_scanner->is_newline(c));
}

bool ArffLexer::_is_comment(char c) const {
    return (c == COMMENT);
}

bool ArffLexer::_is_bracket_open(char c) const {
    return (c == B_OPEN);
}

bool ArffLexer::_is_bracket_close(char c) const {
    return (c == B_CLOSE);
}

bool ArffLexer::_is_s_quote(char c) const {
    return (c == S_QUOTE);
}

bool ArffLexer::_is_d_quote(char c) const {
    return (c == D_QUOTE);
}

bool ArffLexer::_is_comma(char c) const {
    return (c == COMMA);
}

bool ArffLexer::_is_missing(char c) const {
    return (c == MISS);
}

bool ArffLexer::_skip_comments() {
    char c = m_scanner->current();
    if(!(m_scanner->is_newline(m_scanner->previous()) && _is_comment(c))) {
        return false;
    }
    if(m_scanner->eof()) {
        return false;
    }
    char data;
    while((data = m_scanner->next()) >= 0) {
        if(m_scanner->is_newline(data)) {
            // end of line reached
            // get ready for the next char
            m_scanner->next();
            break;
        }
    }
    return true;
}

void ArffLexer::_skip_spaces() {
    while(_is_space(m_scanner->current())) {
        m_scanner->next();
    }
    return;
}

ArffToken ArffLexer::next_token() {
    // in case the current character is a '}'
    if(m_b_close) {
        m_b_close = false;
        return ArffToken("}", BRKT_CLOSE);
    }
    m_scanner->next();
    _skip_spaces();
    while(_skip_comments());
    _skip_spaces();
    std::string str = _read_str();
    ArffTokenEnum token = END_OF_FILE;
    if(icompare(str, "@relation")) {
        token = RELATION;
    }
    else if(icompare(str, "@attribute")) {
        token = ATTRIBUTE;
    }
    else if(icompare(str, "@data")) {
        token = DATA_TOKEN;
    }
    else if(icompare(str, "numeric")) {
        token = NUMERIC_TOKEN;
    }
    else if(icompare(str, "string")) {
        token = STRING_TOKEN;
    }
    else if(icompare(str, "date")) {
        token = DATE_TOKEN;
    }
    else if(icompare(str, "{")) {
        token = BRKT_OPEN;
    }
    else if(icompare(str, "}")) {
        token = BRKT_CLOSE;
    }
    else if(icompare(str, "?")) {
        token = MISSING_TOKEN;
    }
    else if(icompare(str, "")) {
        token = END_OF_FILE;
    }
    else {
        token = VALUE_TOKEN;
    }
    return ArffToken(str, token);
}

std::string ArffLexer::_read_str() {
    std::string str("");
    char c = m_scanner->current();
    if(c < 0) {
        return str;
    }
    else if(_is_bracket_open(c)) {
        return "{";
    }
    else if(_is_bracket_close(c)) {
        return "}";
    }
    else if(_is_missing(c)) {
		while(!_is_comma(c) && !m_scanner->is_newline (c)) {
            if(c < 0) {
                break;
            }
            c = m_scanner->next();
        }
        return "?";
    }
    else if(_is_s_quote(c)) {
        do {
            c = m_scanner->next();
            if(c < 0) {
                break;
            }
            if(_is_bracket_close(c)) {
                m_b_close = true;
                break;
            }
            if(!_is_s_quote(c)) {
                str += c;
            }
        } while(!_is_s_quote(c));
		m_scanner->next();
    }
    else if(_is_d_quote(c)) {
        do {
            c = m_scanner->next();
            if(c < 0) {
                break;
            }
            if(_is_bracket_close(c)) {
                m_b_close = true;
                break;
            }
            if(!_is_d_quote(c)) {
                str += c;
            }
        } while(!_is_d_quote(c));
		m_scanner->next();
    }
    else {
        while(!_is_space(c) && !_is_comma(c)) {
            if(c < 0) {
                break;
            }
            if(_is_bracket_close(c)) {
                m_b_close = true;
                break;
            }
            str += c;
            c = m_scanner->next();
        }
    }
    return str;
}

}
