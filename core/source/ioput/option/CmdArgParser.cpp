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
#include "ioput/option/CmdArgParser.h"

namespace ssi {

char CmdArgParser::ssi_log_name[] = "cmdparser_";

CmdArgParser::CmdArgParser() {

	n_master_switches = 1; // the default!
	n_cmd_options = (int *) malloc(sizeof(int));
	cmd_options = (CmdArgOption ***) malloc(sizeof(CmdArgOption **));
	n_cmd_options[0] = 0;
	cmd_options[0] = NULL;
	text_info = NULL;
	master_switch = -1;
	program_name = new char[1];
	*program_name = '\0';
}

void CmdArgParser::info(const char *text)
{
	if (text_info) {
		delete text_info;
	}

	text_info = new char [strlen (text)+1];
	strcpy (text_info, text);
}

void CmdArgParser::addCmdOption(CmdArgOption *option)
{
	if(option->isMasterSwitch())
	{
		// n_cmd_options = (int *)allocator->realloc(n_cmd_options, sizeof(int)*(n_master_switches+1));
		n_cmd_options = (int *) realloc (n_cmd_options, sizeof(int)*(n_master_switches+1));
		
		// cmd_options = (CmdArgOption ***)allocator->realloc(cmd_options, sizeof(CmdArgOption **)*(n_master_switches+1));
		cmd_options = (CmdArgOption ***) realloc (cmd_options, sizeof(CmdArgOption **)*(n_master_switches+1));
		
		n_cmd_options[n_master_switches] = 0;
		cmd_options[n_master_switches] = NULL;
		n_master_switches++;
	}

	int n = n_master_switches-1;
	// cmd_options[n] = (CmdArgOption **)allocator->realloc(cmd_options[n], (n_cmd_options[n]+1)*sizeof(CmdArgOption *));
	cmd_options[n] = (CmdArgOption **) realloc(cmd_options[n], (n_cmd_options[n]+1)*sizeof(CmdArgOption *));
	
	cmd_options[n][n_cmd_options[n]] = option;

	n_cmd_options[n]++;
}

void CmdArgParser::addMasterSwitch(const char *text)
{
	CmdArgOption *option = new CmdArgOption(text, "", "", false);
	option->isMasterSwitch(true);
	addCmdOption(option);
}

void CmdArgParser::addICmdOption(const char *name, int *ptr, int init_value, const char *help, bool save_it)
{
	IntCmdOption *option = new IntCmdOption(name, ptr, init_value, help, save_it);
	addCmdOption(option);
}

void CmdArgParser::addBCmdOption(const char *name, bool *ptr, bool init_value, const char *help, bool save_it)
{
	BoolCmdOption *option = new BoolCmdOption(name, ptr, init_value, help, save_it);
	addCmdOption(option);
}

void CmdArgParser::addFCmdOption(const char *name, float *ptr, float init_value, const char *help, bool save_it)
{
	FloatCmdOption *option = new FloatCmdOption(name, ptr, init_value, help, save_it);
	addCmdOption(option);
}

void CmdArgParser::addDCmdOption(const char *name, double *ptr, double init_value, const char *help, bool save_it)
{
	DoubleCmdOption *option = new DoubleCmdOption(name, ptr, init_value, help, save_it);
	addCmdOption(option);
}

void CmdArgParser::addSCmdOption(const char *name, char **ptr, const char *init_value, const char *help, bool save_it)
{
	StringCmdOption *option = new StringCmdOption(name, ptr, init_value, help, save_it);
	addCmdOption(option);
}

void CmdArgParser::addICmdArg(const char *name, int *ptr, const char *help, bool save_it)
{
	IntCmdOption *option = new IntCmdOption(name, ptr, 0, help, save_it);
	option->isArgument(true);
	addCmdOption(option);
}

void CmdArgParser::addBCmdArg(const char *name, bool *ptr, const char *help, bool save_it)
{
	BoolCmdOption *option = new BoolCmdOption(name, ptr, false, help, save_it);
	option->isArgument(true);
	addCmdOption(option);
}

void CmdArgParser::addFCmdArg(const char *name, float *ptr, const char *help, bool save_it)
{
	FloatCmdOption *option = new FloatCmdOption(name, ptr, 0., help, save_it);
	option->isArgument(true);
	addCmdOption(option);
}

void CmdArgParser::addDCmdArg(const char *name, double *ptr, const char *help, bool save_it)
{
	DoubleCmdOption *option = new DoubleCmdOption(name, ptr, 0., help, save_it);
	option->isArgument(true);
	addCmdOption(option);
}

void CmdArgParser::addSCmdArg(const char *name, char **ptr, const char *help, bool save_it)
{
	StringCmdOption *option = new StringCmdOption(name, ptr, "", help, save_it);
	option->isArgument(true);
	addCmdOption(option);
}

void CmdArgParser::addText(const char *text)
{
	CmdArgOption *option = new CmdArgOption(text, "", "", false);
	option->isText(true);
	addCmdOption(option);
}

bool CmdArgParser::read(int argc_, char **argv_)
{
	delete[] program_name;
	program_name = new char[(strlen(argv_[0])+1)];
	strcpy(program_name, argv_[0]);
	argv = argv_+1;
	argc = argc_-1;

	// Look for help request and the Master Switch
	master_switch = 0;
	if(argc >= 1)
	{
		if( ! (strcmp(argv[0], "-h") && strcmp(argv[0], "-help") && strcmp(argv[0], "--help")) ) {
			help();
			return false;
		}

		for(int i = 1; i < n_master_switches; i++)
		{
			if(cmd_options[i][0]->isCurrent(&argc, &argv))
			{
				master_switch = i;
				break;
			}
		}
	}

	CmdArgOption **cmd_options_ = cmd_options[master_switch];
	int n_cmd_options_ = n_cmd_options[master_switch];

	// Initialize the options.
	for(int i = 0; i < n_cmd_options_; i++)
		cmd_options_[i]->initValue();

	while(argc > 0)
	{
		// First, check the option.
		int current_option = -1;    
		for(int i = 0; i < n_cmd_options_; i++)
		{
			if(cmd_options_[i]->isCurrent(&argc, &argv))
			{
				current_option = i;
				break;
			}
		}

		if(current_option >= 0)
		{
			if(cmd_options_[current_option]->is_setted) {
				ssi_wrn ("option %s is setted twice", cmd_options_[current_option]->name);
				return false;
			}
			if (!cmd_options_[current_option]->read(&argc, &argv)) {
				return false;
			}
			cmd_options_[current_option]->is_setted = true;
		}
		else
		{
			// Check for arguments
			for(int i = 0; i < n_cmd_options_; i++)
			{
				if(cmd_options_[i]->isArgument() && (!cmd_options_[i]->is_setted))
				{
					current_option = i;
					break;
				}
			}

			if(current_option >= 0)
			{
				if (!cmd_options_[current_option]->read(&argc, &argv)) {
					return false;
				}
				cmd_options_[current_option]->is_setted = true;        
			}
			else {
				ssi_wrn("parse error near <%s>, too many arguments.", argv[0]);
				return false;
			}
		}    
	}

	// Check for empty arguments
	for(int i = 0; i < n_cmd_options_; i++)
	{
		if(cmd_options_[i]->isArgument() && (!cmd_options_[i]->is_setted))
		{
			ssi_wrn ("not enough arguments");
			help();
			return false;
		}
	}

	return true;
}

// RhhAHha AH AHa hha hahaAH Ha ha ha

void CmdArgParser::help()
{
	if(text_info)
		printf ("%s\n", text_info);

	for(int master_switch_ = 0; master_switch_ < n_master_switches; master_switch_++)
	{
		int n_cmd_options_ = n_cmd_options[master_switch_];
		CmdArgOption **cmd_options_ = cmd_options[master_switch_];

		int n_real_options = 0;
		for(int i = 0; i < n_cmd_options_; i++)
		{
			if(cmd_options_[i]->isOption())
				n_real_options++;
		}

		if(master_switch_ == 0)
		{
			printf ("#\n");
			printf ("# usage: %s", program_name);
			if(n_real_options > 0)
				printf (" [options]");
		}
		else
		{
			printf ("\n#\n");
			printf ("# or: %s %s", program_name, cmd_options_[0]->name);
			if(n_real_options > 0)
				printf (" [options]");
		}

		for(int i = 0; i < n_cmd_options_; i++)
		{
			if(cmd_options_[i]->isArgument())
				printf (" <%s>", cmd_options_[i]->name);
		}
		printf ("\n#\n");

		// Cherche la longueur max du param
		int long_max = 0;
		for(int i = 0; i < n_cmd_options_; i++)
		{
			int laurence = 0;
			if(cmd_options_[i]->isArgument())
				laurence = ((int) strlen(cmd_options_[i]->name))+2;

			if(cmd_options_[i]->isOption())
				laurence = ((int) strlen(cmd_options_[i]->name))+((int) strlen(cmd_options_[i]->type_name))+1;

			if(long_max < laurence)
				long_max = laurence;
		}

		for(int i = 0; i < n_cmd_options_; i++)
		{
			int z = 0;
			if(cmd_options_[i]->isText())
			{
				z = -1;
				printf ("%s", cmd_options_[i]->name);
			}

			if(cmd_options_[i]->isArgument())
			{
				z = ((int) strlen(cmd_options_[i]->name))+2;
				printf ("  ");
				printf ("<%s>", cmd_options_[i]->name);
			}

			if(cmd_options_[i]->isOption())
			{
				z = ((int) strlen(cmd_options_[i]->name))+((int) strlen(cmd_options_[i]->type_name))+1;
				printf ("  ");
				printf ("%s", cmd_options_[i]->name);
				printf (" %s", cmd_options_[i]->type_name);
			}

			if(z >= 0)
			{
				for(int i = 0; i < long_max+1-z; i++)
					printf (" ");
			}

			if( cmd_options_[i]->isOption() || cmd_options_[i]->isArgument() )
				printf ("-> %s", cmd_options_[i]->help);

			if(cmd_options_[i]->isArgument())
				printf (" (%s)", cmd_options_[i]->type_name);

			if(!cmd_options_[i]->isMasterSwitch())
				printf ("\n");
		}
	}  
}

CmdArgParser::~CmdArgParser()
{
	for (int i = 0; i < n_master_switches; i++) {
		for (int j = 0; j < n_cmd_options[i]; j++) {
			delete cmd_options[i][j];
		}
		delete[] cmd_options[i];
	}
	delete[] n_cmd_options;
	delete[] cmd_options;
	delete[] program_name;
	if (text_info)
		delete[] text_info;
}

}