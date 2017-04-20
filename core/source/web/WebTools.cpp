// WebTools.cpp
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

#include "ioput/web/WebTools.h"
#include "curl/ssicurl.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ssi_char_t *WebTools::ssi_log_name = "webtools__";
int WebTools::ssi_log_level = SSI_LOG_LEVEL_DEFAULT;

size_t WebTools::write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
	size_t written = fwrite(ptr, size, nmemb, stream);
	return written;
}

WebTools::CurlInit::CurlInit()
{
	curl_global_init(CURL_GLOBAL_ALL);
}

WebTools::CurlInit::~CurlInit()
{
	curl_global_cleanup();
}

void WebTools::init_curl()
{
	static CurlInit init;	
}

void WebTools::init_request(void *handle, const ssi_char_t *url, const ssi_char_t *pathToCertificate, bool showProgress)
{
	CURL *curl = (CURL *)handle;

	curl_easy_setopt(curl, CURLOPT_URL, url);
	if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG)
	{
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	}
	else
	{
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
	}
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, showProgress ? 0L : 1L);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(handle, CURLOPT_MAXREDIRS, 10);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1); //Prevent "longjmp causes uninitialized stack frame" bug
	curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "deflate");
	if (pathToCertificate)
	{
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
		curl_easy_setopt(curl, CURLOPT_CAINFO, pathToCertificate);
	}
	else
	{
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	}
}

bool WebTools::url_exists(void *handle, const ssi_char_t *url, const ssi_char_t *pathToCertificate)
{
	CURL *curl = (CURL *)handle;

	init_request(curl, url, pathToCertificate, false);

	curl_easy_setopt(curl, CURLOPT_HEADER, 1);
	curl_easy_setopt(curl, CURLOPT_NOBODY, 1);
	curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);

#if _WIN32|_WIN64
	FILE *null_device = std::fopen("nul", "w");
#else
	FILE *null_device = std::fopen("/dev/null", "w");
#endif

	curl_easy_setopt(curl, CURLOPT_WRITEDATA, null_device);

	return curl_easy_perform(curl) == CURLE_OK;
}

bool WebTools::UrlExists(const ssi_char_t *url, const ssi_char_t *pathToCertificate)
{	
	bool result = false;
	init_curl();

	CURL* curl = curl_easy_init();
	result = url_exists(curl, url, pathToCertificate);
	curl_easy_cleanup(curl);

	return result;
}

bool WebTools::DownloadFile(const ssi_char_t *url, const ssi_char_t *pathOnDisk, const ssi_char_t *pathToCertificate, bool showProgress)
{
	bool result = false;
	init_curl();

	CURL *curl = curl_easy_init();

	if (url_exists(curl, url, pathToCertificate))
	{
		curl_easy_reset(curl);

		init_request(curl, url, pathToCertificate, showProgress);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);

		FILE *file = fopen(pathOnDisk, "wb");
		if (file) {

			curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
			CURLcode code = curl_easy_perform(curl);
			fclose(file);

			if (code != CURLE_OK)
			{
				ssi_wrn(curl_easy_strerror(code));
				ssi_remove(pathOnDisk);
			}
			else
			{
				result = true;
			}
		}
		else
		{
			ssi_wrn("could not create file '%s'", pathOnDisk);
		}
	}
	else
	{
		ssi_wrn("url does not exist '%s'", url);
	}

	curl_easy_cleanup(curl);

	return result;
}

}

