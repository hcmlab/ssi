// LibsoxFilterTools.h
// author: Andreas Seiderer
// created: 2013/02/08
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
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

#include "LibsoxFilterTools.h"

namespace ssi {

void LibsoxFilterTools::consume_whitespace (const char **input)
{
  while (isspace (**input)) (*input)++;
}

char **LibsoxFilterTools::buildargv (const char *input, int *argc_out)
{
	char *arg;
	char *copybuf;
	int squote = 0, dquote = 0, bsquote = 0;
	int argc = 0;
	int maxargc = 0;
	char **argv = NULL;
	char **nargv;

	if (input != NULL)
	{
		copybuf = (char *) malloc (strlen (input) + 1);
		/* Is a do{}while to always execute the loop once.  Always return an
		argv, even for null strings.*/
		do
		{
			/* Pick off argv[argc] */
			LibsoxFilterTools::consume_whitespace (&input);

			if ((maxargc == 0) || (argc >= (maxargc - 1)))
			{
				/* argv needs initialization, or expansion */
				if (argv == NULL)
				{
					maxargc = 8;
					nargv = (char **) malloc (maxargc * sizeof (char *));
				}
				else
				{
					maxargc *= 2;
					nargv = (char **) realloc (argv, maxargc * sizeof (char *));
				}
				argv = nargv;
				argv[argc] = NULL;
			}
			/* Begin scanning arg */
			arg = copybuf;
			while (*input != '\0')
			{
				if (isspace (*input) && !squote && !dquote && !bsquote) break;
				else
				{
					if (bsquote)
					{
						bsquote = 0;
						*arg++ = *input;
					}
					else if (*input == '\\') bsquote = 1;
					else if (squote)
					{
						if (*input == '\'') squote = 0;
						else *arg++ = *input;
					}
					else if (dquote)
					{
						if (*input == '"') dquote = 0;
						else *arg++ = *input;
					}
					else
					{
						if (*input == '\'') squote = 1;
						else if (*input == '"') dquote = 1;
						else *arg++ = *input;
					}
					input++;
				}
			}
			*arg = '\0';
			argv[argc] = strdup (copybuf);
			argc++;
			argv[argc] = NULL;

			consume_whitespace (&input);
		}
		while (*input != '\0');

		free (copybuf);
	}
	*argc_out = argc;
	return (argv);
}

}

