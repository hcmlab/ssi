/***
 * @file lightstone.c
 * @brief Common functions for lightstone communication
 * @author Kyle Machulis (kyle@nonpolynomial.com)
 * @copyright (c) 2006-2011 Nonpolynomial Labs/Kyle Machulis
 * @license BSD License
 *
 * Project info at http://liblightstone.nonpolynomial.com/
 *
 */

#include "lightstone/lightstone.h"
#include <stdlib.h>
#include <string.h>

LIGHTSTONE_DECLSPEC unsigned int hex2dec(char *data, unsigned int len)
{
	unsigned int i;
	unsigned int value = 0;

	for (i = 0; i < len; i++) {
		value = value*0x10;
		if (data[i] >= '0' && data[i] <= '9')
			value += data[i]-'0';
		else if (data[i] >= 'A' && data[i] <= 'F')
			value += data[i]-'A' + 10;
		else
			return 0;
	}
	return value;
}

LIGHTSTONE_DECLSPEC int lightstone_valid(lightstone* d)
{	
	return d->_is_open;
}

LIGHTSTONE_DECLSPEC lightstone_info lightstone_get_info(lightstone* dev)
{
	lightstone_info ret;
	//hid_return t;
	ret.hrv = -1;
	ret.scl = -1;
	if (lightstone_valid(dev))
	{
		char rawAscii[300];
		unsigned char InputReport[256];
		char message_started = 0;
		int transferred = 0;
		int char_count = 0;
		int ii;

		while(1)
		{
			transferred = lightstone_read(dev, InputReport);
			if(transferred == 0x8 || transferred == 0x9)
			{
				for(ii = 1; ii < InputReport[0]+1; ++ii)
				{
					if(!message_started && InputReport[ii] != '<') continue;
					message_started = 1;
					if (InputReport[ii] != 10 && InputReport[ii] != 13)
					{
						rawAscii[char_count] = InputReport[ii];
						++char_count;
					}
					else if ( InputReport[ii] == 13 ) 
					{
						rawAscii[char_count] = 0;
						if ( strlen(rawAscii) > 18) 
						{
							ret.scl = ((float)(((hex2dec(rawAscii+5,2)) << 8) | (hex2dec(rawAscii+7,2)))) * .01;
							ret.hrv = ((float)(((hex2dec(rawAscii+10,2)) << 8) | (hex2dec(rawAscii+12,2)))) * .001;
							return ret;
						}
						message_started = 0;
						char_count = 0;
					}
				}
			}
		}
	}
	return ret;
}
