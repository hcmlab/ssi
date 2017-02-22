// MyConsumer.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/02/26
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute itand/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#pragma once

#ifndef _PRINTCONSUMER_H
#define _PRINTCONSUMER_H

#include "base/IConsumer.h"
#include "base/Factory.h"
using namespace ssi;

class PrintTime : public IConsumer {

public:

	IOptions *getOptions () { return 0; };
	const ssi_char_t *getName () { return GetCreateName(); };
	const ssi_char_t *getInfo () { return "Prints time on console."; };

	static IObject *Create(const ssi_char_t *file)
	{
		return new PrintTime();
	}
	static const ssi_char_t *GetCreateName()
	{
		return "PrintTime";
	}

	void consume (IConsumer::info consume_info,
		ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]) {

		ssi_lsize_t consume_days, consume_hours, consume_minutes, consume_seconds;
		ssi_lsize_t framework_days, framework_hours, framework_minutes, framework_seconds;
		
		FormatTime (consume_info.time, &consume_days, &consume_hours, &consume_minutes, &consume_seconds);
		FormatTime (Factory::GetFramework ()->GetElapsedTime (), &framework_days, &framework_hours, &framework_minutes, &framework_seconds);
		
#if __gnu_linux__
		ssi_print("%03llu:%02llu:%02llu:%02llu %03llu:%02llu:%02llu:%02llu\n", (uint64_t)consume_days, (uint64_t)consume_hours, (uint64_t)consume_minutes, (uint64_t)consume_seconds, (uint64_t)framework_days, (uint64_t)framework_hours, (uint64_t)framework_minutes, (uint64_t)framework_seconds);
#else
		ssi_print("%03I64u:%02I64u:%02I64u:%02I64u %03I64u:%02I64u:%02I64u:%02I64u\n", (uint64_t) consume_days, (uint64_t)consume_hours, (uint64_t)consume_minutes, (uint64_t)consume_seconds, (uint64_t)framework_days, (uint64_t)framework_hours, (uint64_t)framework_minutes, (uint64_t)framework_seconds);
#endif
	};

	static void FormatTime (const ssi_time_t time, 
		ssi_lsize_t *days,
		ssi_lsize_t *hours,
		ssi_lsize_t *minutes, 
		ssi_lsize_t *seconds) {

		*days = static_cast<ssi_lsize_t> (time / 86400);
		*hours = static_cast<ssi_lsize_t> (time / 3600);
		*minutes = static_cast<ssi_lsize_t> (time / 60);
		*seconds = static_cast<ssi_lsize_t> (time - (*minutes * 60));
	}

protected:

	PrintTime()
		: file(0) {
		ssi_sprint(ssi_log_name, "printtime_");
	};

	ssi_char_t ssi_log_name[255];
	ssi_char_t _string[255];

	File *file;
};

#endif
