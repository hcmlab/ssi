// Copyright (C) 2003--2004 Ronan Collobert (collober@idiap.ch)
//                
// This file is part of Torch 3.1.
//
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. The name of the author may not be used to endorse or promote products
//    derived from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "SSI_Cons.h"
#include "ioput/option/CmdArgOption.h"

namespace ssi {

CmdArgOption::CmdArgOption(const char *name_, const char *type_name_, const char *help_, bool save_)
{
  name = new char[strlen(name_)+1];
  strcpy (name, name_);
  type_name = new char[strlen(type_name_)+1];
  strcpy (type_name, type_name_);
  help = new char[strlen(help_)+1];
  strcpy (help, help_);
  save = save_;
  is_setted = false;

  is_option = true;
  is_argument = false;
  is_text = false;
  is_master_switch = false;
}

bool CmdArgOption::isOption(bool set_)
{
  if(set_)
  {
    is_option = true;
    is_argument = false;
    is_text = false;
    is_master_switch = false;    
  }
  return is_option;
}

bool CmdArgOption::isArgument(bool set_)
{
  if(set_)
  {
    is_option = false;
    is_argument = true;
    is_text = false;
    is_master_switch = false;    
  }
  return is_argument;
}

bool CmdArgOption::isText(bool set_)
{
  if(set_)
  {
    is_option = false;
    is_argument = false;
    is_text = true;
    is_master_switch = false;    
  }
  return is_text;
}

bool CmdArgOption::isMasterSwitch(bool set_)
{
  if(set_)
  {
    is_option = false;
    is_argument = false;
    is_text = false;
    is_master_switch = true;
  }
  return is_master_switch;
}

void CmdArgOption::initValue()
{
}

bool CmdArgOption::read(int *argc_, char ***argv_)
{
	return true;
}

bool CmdArgOption::isCurrent(int *argc_, char ***argv_)
{
  if(!is_option && !is_master_switch)
    return false;

  if(strcmp((*argv_)[0], name))
    return false;
  else
  {
    (*argc_)--;
    (*argv_)++;
    return true;
  }
}

CmdArgOption::~CmdArgOption()
{
	delete[] name;
	delete[] type_name;
	delete[] help;
}

//-------------------------- int

IntCmdOption::IntCmdOption(const char *name_, int *ptr_, int init_value_, const char *help_, bool save_)
  : CmdArgOption(name_, "<int>", help_, save_)
{
  ptr = ptr_;
  init_value = init_value_;
}

void IntCmdOption::initValue()
{
  *ptr = init_value;
}

bool IntCmdOption::read(int *argc_, char ***argv_)
{
  char **argv = *argv_;
  char *maryline;

  if(*argc_ == 0) {
    printf ("IntCmdOption: cannot correctly set <%s>", name);
	return false;
  }

  *ptr = strtol(argv[0], &maryline, 10);
  if( *maryline != '\0' ) {
    printf ("IntCmdOption: <%s> requires an integer", name);
	return false;
  }

  (*argc_)--;
  (*argv_)++;

  return true;
}

IntCmdOption::~IntCmdOption()
{
}


//-------------------------- float

FloatCmdOption::FloatCmdOption(const char *name_, float *ptr_, float init_value_, const char *help_, bool save_)
  : CmdArgOption(name_, "<float>", help_, save_)
{
  ptr = ptr_;
  init_value = init_value_;
}

void FloatCmdOption::initValue()
{
  *ptr = init_value;
}



bool FloatCmdOption::read(int *argc_, char ***argv_)
{
  char **argv = *argv_;
  char *maryline;

  if(*argc_ == 0) {
    printf ("RealCmdOption: cannot correctly set <%s>", name);
	return false;
  }

  *ptr = (float) strtod(argv[0], &maryline);
  if( *maryline != '\0' ) {
    printf ("RealCmdOption: <%s> requires a real", name);
	return false;
  }

  (*argc_)--;
  (*argv_)++;

  return true;
}

FloatCmdOption::~FloatCmdOption()
{
}


//-------------------------- double

DoubleCmdOption::DoubleCmdOption(const char *name_, double *ptr_, double init_value_, const char *help_, bool save_)
  : CmdArgOption(name_, "<double>", help_, save_)
{
  ptr = ptr_;
  init_value = init_value_;
}

void DoubleCmdOption::initValue()
{
  *ptr = init_value;
}



bool DoubleCmdOption::read(int *argc_, char ***argv_)
{
  char **argv = *argv_;
  char *maryline;

  if(*argc_ == 0) {
    printf ("RealCmdOption: cannot correctly set <%s>", name);
	return false;
  }

  *ptr = strtod(argv[0], &maryline);
  if( *maryline != '\0' ) {
    printf ("RealCmdOption: <%s> requires a real", name);
	return false;
  }

  (*argc_)--;
  (*argv_)++;

  return true;
}

DoubleCmdOption::~DoubleCmdOption()
{
}

//-------------------------- switch

BoolCmdOption::BoolCmdOption(const char *name_, bool *ptr_, bool init_value_, const char *help_, bool save_)
  : CmdArgOption(name_, "", help_, save_)
{
  ptr = ptr_;
  init_value = init_value_;
}

void BoolCmdOption::initValue()
{
  *ptr = init_value;
}

bool BoolCmdOption::read(int *argc_, char ***argv_)
{
  *ptr = !(*ptr);
  return true;
}

BoolCmdOption::~BoolCmdOption()
{
}

//-------------------------- string

StringCmdOption::StringCmdOption(const char *name_, char **ptr_, const char *init_value_, const char *help_, bool save_)
  : CmdArgOption(name_, "<string>", help_, save_)
{
  ptr = ptr_;  
  init_value = new char[strlen(init_value_)+1];
  strcpy(init_value, init_value_);
}

void StringCmdOption::initValue()
{
  *ptr = new char[strlen(init_value)+1];
  strcpy(*ptr, init_value);
}

bool StringCmdOption::read(int *argc_, char ***argv_)
{
  char **argv = *argv_;

  if(*argc_ == 0) {
    printf ("StringCmdOption: cannot correctly set <%s>", name);
	return false;
  }

  if (*ptr)
	delete[] *ptr;
  *ptr = new char[strlen(argv[0])+1];
  strcpy(*ptr, argv[0]);

  (*argc_)--;
  (*argv_)++;

  return true;
}

StringCmdOption::~StringCmdOption()
{
	delete[] init_value;
}

}
