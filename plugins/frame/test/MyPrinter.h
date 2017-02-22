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

#ifndef _MYPRINTER_H
#define _MYPRINTER_H

#include "base/IConsumer.h"
#include "base/Factory.h"
#include "ioput/file/FileAscii.h"
#include "ioput/option/OptionList.h"
using namespace ssi;

class MyPrinter : public IConsumer {

public:

	IOptions *getOptions () { return 0; };
	const ssi_char_t *getName () { return GetCreateName(); };
	const ssi_char_t *getInfo () { return "Prints stream to a file."; };

	static IObject *Create(const ssi_char_t *file)
	{
		return new MyPrinter();
	}
	static const ssi_char_t *GetCreateName()
	{
		return "MyPrinter";
	}

	void consume_enter (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]) {

		file = File::CreateAndOpen (File::ASCII, File::WRITE, "check.txt");
		file->setType (stream_in[0].type);

		ssi_msg (SSI_LOG_LEVEL_BASIC, "consume_enter()");
	};

	void consume (IConsumer::info consume_info,
		ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]) {

		int consume_minutes, consume_seconds;
		int framework_minutes, framework_seconds;
		
		FormatTime (consume_info.time, &consume_minutes, &consume_seconds);
		FormatTime (Factory::GetFramework ()->GetElapsedTime (), &framework_minutes, &framework_seconds);
		
		ssi_sprint (_string, "%02d:%02d framework <> %02d:%02d consume", framework_minutes, framework_seconds, consume_minutes, consume_seconds);
		ssi_msg (SSI_LOG_LEVEL_BASIC, _string);

		file->write (stream_in[0].ptr, stream_in[0].dim, stream_in[0].num * stream_in[0].dim);

		//Sleep (3000);
	};

	void consume_flush (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]) {

		delete file;
		file = 0;

		ssi_msg (SSI_LOG_LEVEL_BASIC, "consume_flush()");
	};

	void consume_fail (ssi_time_t fail_time, 
		ssi_time_t fail_duration,
		ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]) {
	
		int fail_time_minutes, fail_time_seconds;
		FormatTime (fail_time, &fail_time_minutes, &fail_time_seconds);

		ssi_msg (SSI_LOG_LEVEL_BASIC, "consume_fail() at %02d:%02d for %Lf seconds", fail_time_minutes, fail_time_seconds, fail_duration);
	};

	static void FormatTime (const ssi_time_t time, 
		int *minutes, 
		int *seconds) {

		*minutes = static_cast<int> (time / 60);
		*seconds = static_cast<int> (time - (*minutes * 60));
	}

protected:

	MyPrinter()
		: file(0) {
		ssi_sprint(ssi_log_name, "myprinter_");
	};

	ssi_char_t ssi_log_name[255];
	ssi_char_t _string[255];

	File *file;
};

#endif
