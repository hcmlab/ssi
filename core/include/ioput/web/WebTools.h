// FileTools.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2017/04/20
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

#ifndef SSI_IOPUT_WEBTOOLS_H
#define SSI_IOPUT_WEBTOOLS_H

#include "base/StringList.h"

namespace ssi {

class WebTools {                                                                

public:	

	static bool UrlExists(const ssi_char_t *url, const ssi_char_t *pathToCertificate = 0);
	static bool DownloadFile(const ssi_char_t *url, const ssi_char_t *pathOnDisk, const ssi_char_t *pathToCertificate = 0, bool showProgress = true);

	static void SetLogLevel (int level) {
		ssi_log_level = level;
	}

protected:

	class CurlInit
	{
	public:
		CurlInit();
		virtual ~CurlInit();
	};

	static ssi_char_t *ssi_log_name;
	static int ssi_log_level;

	static void init_curl();
	static void init_request(void *handle, const ssi_char_t *url, const ssi_char_t *pathToCertificate, bool showProgress);
	static bool url_exists(void *handle, const ssi_char_t *url, const ssi_char_t *pathToCertificate);
	static size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream);
};

}

#endif
