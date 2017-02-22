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

#ifndef SSI_IOPUT_CMDARGOPTION_H
#define SSI_IOPUT_CMDARGOPTION_H

namespace ssi {

/** This class defines an option for the command line.
    If you need special command line arguments/options,
    you have to create a new children of this class.

    @author Ronan Collobert (collober@idiap.ch)
    @see CmdLine
*/
class CmdArgOption
{
  private:
    // Special flags.
    bool is_option;
    bool is_argument;
    bool is_text;
    bool is_master_switch;

  public:
    /// Name of the option.
    char *name;

    /// Type name of the option.
    char *type_name;

    /** An help string.
        Cannot be NULL.
    */
    char *help;

    /** True is the option has to be saved
        when saving the command line.
    */
    bool save;

    /** True is the option has been setted after
        reading the command-line.
    */
    bool is_setted;

    //////////////////////

    ///
    CmdArgOption(const char *name_, const char *type_name_, const char *help_="", bool save_=false);

    /// Initialize the value of the option.
    virtual void initValue();

    /** Read the option on the command line.
        argv_ and argc_ have to point of the next
        option after that.
    */
    virtual bool read(int *argc_, char ***argv_);

    /* Return true if the option is on the command line.
       Decrements argc_ and increment argv_ if true.
    */
    bool isCurrent(int *argc_, char ***argv_);

    /** Returns true if it's an optional argument.
        If #set_# is true, set it to an optional argument.
    */
    bool isOption(bool set_=false);

    /** Returns true if it's a required argument.
        If #set_# is true, set it to a required argument.
    */
    bool isArgument(bool set_=false);

    /** Returns true if it's just text to be displayed in the command line.
        If #set_# is true, set it to text mode.
    */
    bool isText(bool set_=false);

    /** Returns true if it's a master switch.
        If #set_# is true, set it to a master switch.
    */
    bool isMasterSwitch(bool set_=false);
    
    virtual ~CmdArgOption();
};

/** This class defines a integer command-line option.

    @author Ronan Collobert (collober@idiap.ch)
    @see CmdLine
*/
class IntCmdOption : public CmdArgOption
{
  public:
    int *ptr;
    int init_value;

    ///
    IntCmdOption(const char *name_, int *ptr_, int init_value_, const char *help_="", bool save_=false);

    virtual void initValue();
    virtual bool read(int *argc_, char ***argv_);
    ~IntCmdOption();
};

/** This class defines a float command-line option.

    @author Ronan Collobert (collober@idiap.ch)
    @see CmdLine
*/
class FloatCmdOption : public CmdArgOption
{
  public:

    float *ptr;
    float init_value;

    ///
    FloatCmdOption(const char *name_, float *ptr_, float init_value_, const char *help_="", bool save_=false);

    virtual void initValue();
    virtual bool read(int *argc_, char ***argv_);
    ~FloatCmdOption();
};

/** This class defines a double command-line option.

    @author Ronan Collobert (collober@idiap.ch)
    @see CmdLine
*/
class DoubleCmdOption : public CmdArgOption
{
  public:

    double *ptr;
    double init_value;

    ///
    DoubleCmdOption(const char *name_, double *ptr_, double init_value_, const char *help_="", bool save_=false);

    virtual void initValue();
    virtual bool read(int *argc_, char ***argv_);
    ~DoubleCmdOption();
};

/** This class defines a bool command-line option.

    @author Ronan Collobert (collober@idiap.ch)
    @see CmdLine
*/
class BoolCmdOption : public CmdArgOption
{
  public:
    bool *ptr;
    bool init_value;

    ///
    BoolCmdOption(const char *name_, bool *ptr_, bool init_value_, const char *help_="", bool save_=false);

    virtual void initValue();
    virtual bool read(int *argc_, char ***argv_);
    ~BoolCmdOption();
};

/** This class defines a string command-line option.

    @author Ronan Collobert (collober@idiap.ch)
    @see CmdLine
*/
class StringCmdOption : public CmdArgOption
{
  public:
    char **ptr;
    char *init_value;

    ///
    StringCmdOption(const char *name_, char **ptr_, const char *init_value_, const char *help_="", bool save_=false);

    virtual void initValue();
    virtual bool read(int *argc_, char ***argv_);
    ~StringCmdOption();
};

}

#endif
