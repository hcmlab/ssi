// AviTools.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/08/31
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

// Based on code taken from:
//
// AVI utilities -- for creating avi files
// (c) 2002 Lucian Wischik. No restrictions on use.
//
// http://www.wischik.com/lu/programmer/avi_utils.html

#include "AviTools.h"
#include "base/Factory.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ssi_char_t *AviTools::ssi_log_name_static = "avitools__";

HAVI AviTools::CreateAvi(const char *fn, int frameperiod, const WAVEFORMATEX *wfx)
{ IAVIFile *pfile;
  AVIFileInit();
  HRESULT hr = AVIFileOpen(&pfile, fn, OF_WRITE|OF_CREATE, NULL);
  if (hr!=AVIERR_OK) {AVIFileExit(); return NULL;}
  TAviUtil *au = new TAviUtil;
  au->pfile = pfile;
  if (wfx==NULL) ZeroMemory(&au->wfx,sizeof(WAVEFORMATEX)); else CopyMemory(&au->wfx,wfx,sizeof(WAVEFORMATEX));
  au->period = frameperiod;
  au->as=0; au->ps=0; au->psCompressed=0;
  au->nframe=0; au->nsamp=0;
  au->iserr=false;
  return (HAVI)au;
}

HRESULT AviTools::CloseAvi(HAVI avi)
{ if (avi==NULL) return AVIERR_BADHANDLE;
  TAviUtil *au = (TAviUtil*)avi;
  if (au->as!=0) AVIStreamRelease(au->as); au->as=0;
  if (au->psCompressed!=0) AVIStreamRelease(au->psCompressed); au->psCompressed=0;
  if (au->ps!=0) AVIStreamRelease(au->ps); au->ps=0;
  if (au->pfile!=0) AVIFileRelease(au->pfile); au->pfile=0;
  AVIFileExit();
  delete au;
  return S_OK;
}


HRESULT AviTools::SetAviVideoCompression(HAVI avi, HBITMAP hbm, AVICOMPRESSOPTIONS *opts, bool ShowDialog, HWND hparent)
{ if (avi==NULL) return AVIERR_BADHANDLE;
  if (hbm==NULL) return AVIERR_BADPARAM;
  DIBSECTION dibs; int sbm = GetObject(hbm,sizeof(dibs),&dibs);
  if (sbm!=sizeof(DIBSECTION)) return AVIERR_BADPARAM;
  TAviUtil *au = (TAviUtil*)avi;
  if (au->iserr) return AVIERR_ERROR;
  if (au->psCompressed!=0) return AVIERR_COMPRESSOR;
  //
  if (au->ps==0) // create the stream, if it wasn't there before
  { AVISTREAMINFO strhdr; ZeroMemory(&strhdr,sizeof(strhdr));
    strhdr.fccType = streamtypeVIDEO;// stream type
    strhdr.fccHandler = 0; 
    strhdr.dwScale = au->period;
    strhdr.dwRate = 1000;
    strhdr.dwSuggestedBufferSize  = dibs.dsBmih.biSizeImage;
    SetRect(&strhdr.rcFrame, 0, 0, dibs.dsBmih.biWidth, dibs.dsBmih.biHeight);
    HRESULT hr=AVIFileCreateStream(au->pfile, &au->ps, &strhdr);
    if (hr!=AVIERR_OK) {au->iserr=true; return hr;}
  }
  //
  if (au->psCompressed==0) // set the compression, prompting dialog if necessary
  { AVICOMPRESSOPTIONS myopts; ZeroMemory(&myopts,sizeof(myopts));
    AVICOMPRESSOPTIONS *aopts[1];
    if (opts!=NULL) aopts[0]=opts; else aopts[0]=&myopts;
    if (ShowDialog)
    { BOOL res = (BOOL)AVISaveOptions(hparent,0,1,&au->ps,aopts);
      if (!res) {AVISaveOptionsFree(1,aopts); au->iserr=true; return AVIERR_USERABORT;}
    }
    HRESULT hr = AVIMakeCompressedStream(&au->psCompressed, au->ps, aopts[0], NULL);
    AVISaveOptionsFree(1,aopts);
    if (hr != AVIERR_OK) {au->iserr=true; return hr;}
    DIBSECTION dibs; GetObject(hbm,sizeof(dibs),&dibs);
    hr = AVIStreamSetFormat(au->psCompressed, 0, &dibs.dsBmih, dibs.dsBmih.biSize+dibs.dsBmih.biClrUsed*sizeof(RGBQUAD));
    if (hr!=AVIERR_OK) {au->iserr=true; return hr;}
  }
  //
  return AVIERR_OK;
}


HRESULT AviTools::AddAviFrame(HAVI avi, HBITMAP hbm)
{ if (avi==NULL) return AVIERR_BADHANDLE;
  if (hbm==NULL) return AVIERR_BADPARAM;
  DIBSECTION dibs; int sbm = GetObject(hbm,sizeof(dibs),&dibs);
  if (sbm!=sizeof(DIBSECTION)) return AVIERR_BADPARAM;
  TAviUtil *au = (TAviUtil*)avi;
  if (au->iserr) return AVIERR_ERROR;
  //
  if (au->ps==0) // create the stream, if it wasn't there before
  { AVISTREAMINFO strhdr; ZeroMemory(&strhdr,sizeof(strhdr));
    strhdr.fccType = streamtypeVIDEO;// stream type
    strhdr.fccHandler = 0; 
    strhdr.dwScale = au->period;
    strhdr.dwRate = 1000;
    strhdr.dwSuggestedBufferSize  = dibs.dsBmih.biSizeImage;
    SetRect(&strhdr.rcFrame, 0, 0, dibs.dsBmih.biWidth, dibs.dsBmih.biHeight);
    HRESULT hr=AVIFileCreateStream(au->pfile, &au->ps, &strhdr);
    if (hr!=AVIERR_OK) {au->iserr=true; return hr;}
  }
  //
  // create an empty compression, if the user hasn't set any
  if (au->psCompressed==0)
  { AVICOMPRESSOPTIONS opts; ZeroMemory(&opts,sizeof(opts));
    opts.fccHandler=mmioFOURCC('D','I','B',' '); 
    HRESULT hr = AVIMakeCompressedStream(&au->psCompressed, au->ps, &opts, NULL);
    if (hr != AVIERR_OK) {au->iserr=true; return hr;}
    hr = AVIStreamSetFormat(au->psCompressed, 0, &dibs.dsBmih, dibs.dsBmih.biSize+dibs.dsBmih.biClrUsed*sizeof(RGBQUAD));
    if (hr!=AVIERR_OK) {au->iserr=true; return hr;}
  }
  //
  //Now we can add the frame
  HRESULT hr = AVIStreamWrite(au->psCompressed, au->nframe, 1, dibs.dsBm.bmBits, dibs.dsBmih.biSizeImage, AVIIF_KEYFRAME, NULL, NULL);
  if (hr!=AVIERR_OK) {au->iserr=true; return hr;}
  au->nframe++; return S_OK;
}
  


HRESULT AviTools::AddAviAudio(HAVI avi, void *dat, unsigned long numbytes)
{ if (avi==NULL) return AVIERR_BADHANDLE;
  if (dat==NULL || numbytes==0) return AVIERR_BADPARAM;
  TAviUtil *au = (TAviUtil*)avi;
  if (au->iserr) return AVIERR_ERROR;
  if (au->wfx.nChannels==0) return AVIERR_BADFORMAT;
  unsigned long numsamps = numbytes*8 / au->wfx.wBitsPerSample;
  if ((numsamps*au->wfx.wBitsPerSample/8)!=numbytes) return AVIERR_BADPARAM;
  //
  if (au->as==0) // create the stream if necessary
  { AVISTREAMINFO ahdr; ZeroMemory(&ahdr,sizeof(ahdr));
    ahdr.fccType=streamtypeAUDIO;
    ahdr.dwScale=au->wfx.nBlockAlign;
    ahdr.dwRate=au->wfx.nSamplesPerSec*au->wfx.nBlockAlign; 
    ahdr.dwSampleSize=au->wfx.nBlockAlign;
    ahdr.dwQuality=(DWORD)-1;
    HRESULT hr = AVIFileCreateStream(au->pfile, &au->as, &ahdr);
    if (hr!=AVIERR_OK) {au->iserr=true; return hr;}
    hr = AVIStreamSetFormat(au->as,0,&au->wfx,sizeof(WAVEFORMATEX));
    if (hr!=AVIERR_OK) {au->iserr=true; return hr;}
  }
  //
  // now we can write the data
  HRESULT hr = AVIStreamWrite(au->as,au->nsamp,numsamps,dat,numbytes,0,NULL,NULL);
  if (hr!=AVIERR_OK) {au->iserr=true; return hr;}
  au->nsamp+=numsamps; return S_OK;
}



HRESULT AviTools::AddAviWav(HAVI avi, const char *src, DWORD flags)
{ if (avi==NULL) return AVIERR_BADHANDLE;
  if (flags!=SND_MEMORY && flags!=SND_FILENAME) return AVIERR_BADFLAGS;
  if (src==0) return AVIERR_BADPARAM;
  TAviUtil *au = (TAviUtil*)avi;
  if (au->iserr) return AVIERR_ERROR;
  //
  char *buf=0; WavChunk *wav = (WavChunk*)src;
  if (flags==SND_FILENAME)
  { HANDLE hf=CreateFile(src,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
    if (hf==INVALID_HANDLE_VALUE) {au->iserr=true; return AVIERR_FILEOPEN;}
    DWORD size = GetFileSize(hf,NULL);
    buf = new char[size];
    DWORD red; ReadFile(hf,buf,size,&red,NULL);
    CloseHandle(hf);
    wav = (WavChunk*)buf;
  }
  //
  // check that format doesn't clash
  bool badformat=false;
  if (au->wfx.nChannels==0)
  { au->wfx.wFormatTag=wav->fmt.wFormatTag;
    au->wfx.cbSize=0;
    au->wfx.nAvgBytesPerSec=wav->fmt.dwAvgBytesPerSec;
    au->wfx.nBlockAlign=wav->fmt.wBlockAlign;
    au->wfx.nChannels=wav->fmt.wChannels;
    au->wfx.nSamplesPerSec=wav->fmt.dwSamplesPerSec;
    au->wfx.wBitsPerSample=wav->fmt.wBitsPerSample;
  }
  else
  { if (au->wfx.wFormatTag!=wav->fmt.wFormatTag) badformat=true;
    if (au->wfx.nAvgBytesPerSec!=wav->fmt.dwAvgBytesPerSec) badformat=true;
    if (au->wfx.nBlockAlign!=wav->fmt.wBlockAlign) badformat=true;
    if (au->wfx.nChannels!=wav->fmt.wChannels) badformat=true;
    if (au->wfx.nSamplesPerSec!=wav->fmt.dwSamplesPerSec) badformat=true;
    if (au->wfx.wBitsPerSample!=wav->fmt.wBitsPerSample) badformat=true;
  }
  if (badformat) {if (buf!=0) delete[] buf; return AVIERR_BADFORMAT;}
  //
  if (au->as==0) // create the stream if necessary
  { AVISTREAMINFO ahdr; ZeroMemory(&ahdr,sizeof(ahdr));
    ahdr.fccType=streamtypeAUDIO;
    ahdr.dwScale=au->wfx.nBlockAlign;
    ahdr.dwRate=au->wfx.nSamplesPerSec*au->wfx.nBlockAlign; 
    ahdr.dwSampleSize=au->wfx.nBlockAlign;
    ahdr.dwQuality=(DWORD)-1;
    HRESULT hr = AVIFileCreateStream(au->pfile, &au->as, &ahdr);
    if (hr!=AVIERR_OK) {if (buf!=0) delete[] buf; au->iserr=true; return hr;}
    hr = AVIStreamSetFormat(au->as,0,&au->wfx,sizeof(WAVEFORMATEX));
    if (hr!=AVIERR_OK) {if (buf!=0) delete[] buf; au->iserr=true; return hr;}
  }
  //
  // now we can write the data
  unsigned long numbytes = wav->dat.size;
  unsigned long numsamps = numbytes*8 / au->wfx.wBitsPerSample;
  HRESULT hr = AVIStreamWrite(au->as,au->nsamp,numsamps,wav->dat.data,numbytes,0,NULL,NULL);
  if (buf!=0) delete[] buf;
  if (hr!=AVIERR_OK) {au->iserr=true; return hr;}
  au->nsamp+=numsamps; return S_OK;
}



unsigned int AviTools::FormatAviMessage(HRESULT code, char *buf,unsigned int len)
{ const char *msg="unknown avi result code";
  switch (code)
  { case S_OK: msg="Success"; break;
    case AVIERR_BADFORMAT: msg="AVIERR_BADFORMAT: corrupt file or unrecognized format"; break;
    case AVIERR_MEMORY: msg="AVIERR_MEMORY: insufficient memory"; break;
    case AVIERR_FILEREAD: msg="AVIERR_FILEREAD: disk error while reading file"; break;
    case AVIERR_FILEOPEN: msg="AVIERR_FILEOPEN: disk error while opening file"; break;
    case REGDB_E_CLASSNOTREG: msg="REGDB_E_CLASSNOTREG: file type not recognised"; break;
    case AVIERR_READONLY: msg="AVIERR_READONLY: file is read-only"; break;
    case AVIERR_NOCOMPRESSOR: msg="AVIERR_NOCOMPRESSOR: a suitable compressor could not be found"; break;
    case AVIERR_UNSUPPORTED: msg="AVIERR_UNSUPPORTED: compression is not supported for this type of data"; break;
    case AVIERR_INTERNAL: msg="AVIERR_INTERNAL: internal error"; break;
    case AVIERR_BADFLAGS: msg="AVIERR_BADFLAGS"; break;
    case AVIERR_BADPARAM: msg="AVIERR_BADPARAM"; break;
    case AVIERR_BADSIZE: msg="AVIERR_BADSIZE"; break;
    case AVIERR_BADHANDLE: msg="AVIERR_BADHANDLE"; break;
    case AVIERR_FILEWRITE: msg="AVIERR_FILEWRITE: disk error while writing file"; break;
    case AVIERR_COMPRESSOR: msg="AVIERR_COMPRESSOR"; break;
    case AVIERR_NODATA: msg="AVIERR_READONLY"; break;
    case AVIERR_BUFFERTOOSMALL: msg="AVIERR_BUFFERTOOSMALL"; break;
    case AVIERR_CANTCOMPRESS: msg="AVIERR_CANTCOMPRESS"; break;
    case AVIERR_USERABORT: msg="AVIERR_USERABORT"; break;
    case AVIERR_ERROR: msg="AVIERR_ERROR"; break;
  }
  unsigned int mlen=(unsigned int)strlen(msg);
  if (buf==0 || len==0) return mlen;
  unsigned int n=mlen; if (n+1>len) n=len-1;
  strncpy(buf,msg,n); buf[n]=0;
  return mlen;
}


bool AviTools::OpenAviFile (const ssi_char_t *filename, PAVIFILE &avi, AVIFILEINFO &avi_info) {

	int res = 0;

	// open avi file
	res = AVIFileOpen (&avi, filename, OF_READ, NULL);

	if (res != AVIERR_OK) {     
		if (avi) {
            AVIFileRelease(avi);
		}
		return false;        
    }

	// get avi info
    AVIFileInfo (avi, &avi_info, sizeof (AVIFILEINFO));

	ssi_msg_static (SSI_LOG_LEVEL_BASIC, "opened avi file '%s'", filename);
	if ( ssi_log_level >= SSI_LOG_LEVEL_DETAIL) {
		ssi_print ("\
             dimension: %ux%u\n\
             number of frames: %u frames\n\
             max bytes per second: %u\n\
             samples per second: %u\n\
             number of streams: %u\n\
             file Type: %s\n", 
			avi_info.dwWidth,
			avi_info.dwHeight,
			avi_info.dwLength,
			avi_info.dwMaxBytesPerSec,
			(DWORD) (avi_info.dwRate / avi_info.dwScale),
			avi_info.dwStreams,
			avi_info.szFileType);
	}

	return true;
}

bool AviTools::OpenAviStream (PAVIFILE &avi, PAVISTREAM &stream, AVISTREAMINFO &stream_info, int index, DWORD type) {

	int res = 0;

	res = AVIFileGetStream (avi, &stream, type, index);

	if (res != AVIERR_OK) {
		if (stream) {
			AVIStreamRelease(stream);
		}
		if (avi) {
            AVIFileRelease(avi);
		}
		AVIFileExit();
		return false;
	}

	AVIStreamInfo (stream, &stream_info, sizeof (stream_info));

	ssi_msg_static (SSI_LOG_LEVEL_BASIC, "opened %s stream #%d", (type == streamtypeVIDEO ? "video" : "audio"), index);
	if ( ssi_log_level >= SSI_LOG_LEVEL_DETAIL) {
		ssi_print ("\
             fcc type: %u\n\
             frames per second: %u\n\
             sample size in bytes: %u\n\
             length: %u\n", 
		stream_info.fccType,
		stream_info.dwRate / stream_info.dwScale,
		stream_info.dwSampleSize,
		stream_info.dwLength
		);
	}

	return true;
}

bool AviTools::GetAviStreamSize (PAVIFILE &avi, PAVISTREAM &stream, long &first_frame_index, long &total_frame_size) {

	first_frame_index = AVIStreamStart (stream);
    if (first_frame_index == -1) {        
		if (stream) {
            AVIStreamRelease(stream);
		}
		if (avi) {
            AVIFileRelease(avi);
		}
        AVIFileExit();

        return false;
    }

	total_frame_size = AVIStreamLength (stream);
    if (total_frame_size == -1) {
        if (stream) {
            AVIStreamRelease (stream);
		}        
		if (avi) {
            AVIFileRelease(avi);
		}
        AVIFileExit();

		return false;
    }

	return true;
}

bool AviTools::GetVideoFormat (PAVISTREAM &stream, BITMAPINFO &format_in, BITMAPINFO &format_out) {

	long n = 0;
	if (HRESULT res = AVIStreamReadFormat (stream, AVIStreamStart (stream), NULL, &n)) {
		return false;
	}
	LPBYTE pChunk = new BYTE[n];	
	if (HRESULT res = AVIStreamReadFormat (stream, AVIStreamStart (stream), pChunk, &n)) {
		return false;
	}
	memcpy (&format_in, pChunk, sizeof (format_in));

	format_out = format_in;
	format_out.bmiHeader.biCompression = 0;
	//format_out.bmiHeader.biSizeImage = format_in.bmiHeader.biWidth * format_in.bmiHeader.biHeight * format_in.bmiHeader.biBitCount;
	format_out.bmiHeader.biSizeImage = ((format_in.bmiHeader.biWidth*format_in.bmiHeader.biBitCount/8+3)&0xFFFFFFFC)*format_in.bmiHeader.biHeight;

	ssi_msg_static (SSI_LOG_LEVEL_BASIC, "read video format");
	if ( ssi_log_level >= SSI_LOG_LEVEL_DETAIL) {
		ssi_print ("\
             width in pixel: %u\n\
             height in pixel: %u\n\
             samples per pixel: %u\n", 
		format_in.bmiHeader.biWidth,
		format_in.bmiHeader.biHeight,
		format_in.bmiHeader.biBitCount
		);
	}

	delete[] pChunk;

	return true;
}

bool AviTools::GetAudioFormat (PAVISTREAM &stream, WAVEFORMAT &format) {

	long n = 0;
	if (HRESULT res = AVIStreamReadFormat (stream, AVIStreamStart (stream), NULL, &n)) {
		return false;
	}
	LPBYTE pChunk = new BYTE[n];	
	if (HRESULT res = AVIStreamReadFormat (stream, AVIStreamStart (stream), pChunk, &n)) {
		return false;
	}
	memcpy (&format, pChunk, sizeof (format));

	ssi_msg_static (SSI_LOG_LEVEL_BASIC, "read audio format");
	if ( ssi_log_level >= SSI_LOG_LEVEL_DETAIL) {
		ssi_print ("\
             number of channels: %u\n\
             sample rate: %u\n\
             bytes per sample: %u\n", 
		format.nChannels,
		format.nSamplesPerSec,
		format.nAvgBytesPerSec / (format.nSamplesPerSec * format.nChannels)
		);
	}

	delete[] pChunk;

	return true;
}

}
