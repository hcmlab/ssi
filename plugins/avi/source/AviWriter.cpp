// AviWriter.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/05/13
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

#include "AviWriter.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

const ssi_char_t *AviWriter::ssi_log_name = "aviwriter_";

AviWriter::AviWriter(const char *file)
	: avi (0),
	_audio_format (0),
	_file (0) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
}

AviWriter::~AviWriter() {

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}

void AviWriter::consume_enter (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	// video

	SSI_ASSERT (stream_in[0].sr == _video_format.framesPerSecond
		&& stream_in[0].byte == ssi_video_stride(_video_format) * _video_format.heightInPixels
		&& stream_in[0].dim == 1);

	if (ssi_exists (_options.path)) {
		remove (_options.path);
	}

	// audio
	if (stream_in_num > 1) {
		_audio_format = new WAVEFORMATEX (ssi_create_wfx (stream_in[1].sr, stream_in[1].dim, stream_in[1].byte));
	}

	HDC hdcscreen = ::GetDC(0), hdc = ::CreateCompatibleDC(hdcscreen); ::ReleaseDC(0,hdcscreen);
	::ZeroMemory(&bi,sizeof(bi)); 
	BITMAPINFOHEADER &bih = bi.bmiHeader;
	bih.biSize=sizeof(bih);
	bih.biWidth=_video_format.widthInPixels;
	bih.biHeight=_video_format.heightInPixels;
	bih.biPlanes=1;
	bih.biBitCount=_video_format.numOfChannels * _video_format.depthInBitsPerChannel;
	bih.biCompression=BI_RGB;
	bih.biSizeImage = ((bih.biWidth*bih.biBitCount/8+3)&0xFFFFFFFC)*bih.biHeight; 	
	bih.biXPelsPerMeter=10000;
	bih.biYPelsPerMeter=10000;
	bih.biClrUsed=0;
	bih.biClrImportant=0;
	hbm = ::CreateDIBSection(hdc,(BITMAPINFO*)&bih,DIB_RGB_COLORS,&bits,NULL,NULL);
	//
	holdb = ::SelectObject(hdc,hbm);
	hp = ::CreatePen(PS_SOLID,16,RGB(255,255,128));
	holdp = ::SelectObject(hdc,hp);

	avi = AviTools::CreateAvi(_options.path,ssi_cast (int, (1.0/_video_format.framesPerSecond)*1000),_audio_format);

	HRESULT r;
	if (_options.fourcc[0] != '\0') {
		AVICOMPRESSOPTIONS opts; 
		ZeroMemory (&opts, sizeof(opts));
		opts.fccHandler = ssi_fourcc (_options.fourcc[0], _options.fourcc[1], _options.fourcc[2], _options.fourcc[3]);
		//SetAviVideoCompression (hbm, &opts, true,0);
		//r = avi->compression (*dib_section, &opts, false, 0);
	    r = AviTools::SetAviVideoCompression(avi,hbm,&opts,false,0);
	} else {
		//r = avi->compression (*dib_section, 0, true, 0);
		r = AviTools::SetAviVideoCompression(avi,hbm,0,true,0);
	}

	if (r != S_OK) {
		AviTools::FormatAviMessage (r, errbuf, SSI_MAX_CHAR);
		ssi_err (errbuf);
	}
}

void AviWriter::consume (IConsumer::info consume_info,
	ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {
	
	HRESULT r;

	memcpy (bits, stream_in[0].ptr, stream_in[0].byte);
	//avi->add_frame (*dib_section);
	r = AviTools::AddAviFrame(avi,hbm);

	if (r != S_OK) {
		AviTools::FormatAviMessage (r, errbuf, SSI_MAX_CHAR);
		ssi_err (errbuf);
	}

	if (_audio_format) {
		//avi->add_audio (ssi_pcast (void, stream_in[1].ptr), stream_in[1].num * stream_in[1].dim * stream_in[1].byte);
		r = AviTools::AddAviAudio (avi, stream_in[1].ptr, stream_in[1].tot);
	
		if (r != S_OK) {
			AviTools::FormatAviMessage (r, errbuf, SSI_MAX_CHAR);
			ssi_err (errbuf);
		}
	}
}

void AviWriter::consume_flush (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	//delete avi;
	//delete dib_section;
	//delete memory_dc;

	delete _audio_format;
	_audio_format = 0;

	HRESULT r;

	r = AviTools::CloseAvi(avi);

	if (r != S_OK) {
		AviTools::FormatAviMessage (r, errbuf, SSI_MAX_CHAR);
		ssi_err (errbuf);
	}

	::SelectObject(hdc,holdb); ::SelectObject(hdc,holdp);
	::DeleteDC(hdc); ::DeleteObject(hbm); ::DeleteObject(hp);
}

// add empty frames
void AviWriter::consume_fail (const ssi_time_t fail_time, 
	const ssi_time_t fail_duration,
	ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	// number of missing frames
	ssi_size_t frames_to_add = ssi_cast (ssi_size_t, fail_duration * _video_format.framesPerSecond);	

	// create empty audio
	ssi_size_t len = ssi_cast (ssi_size_t, stream_in[1].sr / _video_format.framesPerSecond) * stream_in[1].byte * stream_in[1].dim;
	ssi_byte_t *empty_audio_data = new ssi_byte_t[len];
	ssi_byte_t *ptr = empty_audio_data;
	for (ssi_size_t i = 0; i < len; i++) {
		*ptr++ = 0;
	}

	// add empty frames
	for (ssi_size_t i = 0; i < frames_to_add; ++i) {
		//avi->add_frame (*dib_section, true);
		AviTools::TAviUtil *au = (AviTools::TAviUtil*) avi;
		au->nframe++;
		if (_audio_format) {

			HRESULT r;

			//avi->add_audio (empty_audio_data, len);
			r = AviTools::AddAviAudio (avi, empty_audio_data, len);

			if (r != S_OK) {
				AviTools::FormatAviMessage (r, errbuf, SSI_MAX_CHAR);
				ssi_err (errbuf);
			}
		}
	}
	delete[] empty_audio_data;

	ssi_msg (SSI_LOG_LEVEL_BASIC, "%.2Lf s were missing --> added %u empty frames", fail_duration, frames_to_add); 
}

}
