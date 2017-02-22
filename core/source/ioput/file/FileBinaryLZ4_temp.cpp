// FileBinaryLZ4.cpp
// author: Andreas Seiderer <seiderer@hcm-lab.de>
// created: 2015/04/06
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

#include "ioput/file/FileBinaryLZ4.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

<<<<<<< .mine
//see lz4io.c of LZ4 library
||||||| .r8820
=======
FileBinaryLZ4::FileBinaryLZ4()
: in_buff (0), 
out_buff (0) {
}
>>>>>>> .r9014

<<<<<<< .mine
#define MAGICNUMBER_SIZE    4
#define LZ4IO_MAGICNUMBER   0x184D2204
#define LZ4IO_SKIPPABLE0    0x184D2A50
#define LZ4IO_SKIPPABLEMASK 0xFFFFFFF0

static unsigned LZ4IO_readLE32 (const void* s)
{
    const unsigned char* srcPtr = (const unsigned char*)s;
    unsigned value32 = srcPtr[0];
    value32 += (srcPtr[1]<<8);
    value32 += (srcPtr[2]<<16);
    value32 += (srcPtr[3]<<24);
    return value32;
}

static void LZ4IO_writeLE32 (void* p, unsigned value32)
{
    unsigned char* dstPtr = (unsigned char*)p;
    dstPtr[0] = (unsigned char)value32;
    dstPtr[1] = (unsigned char)(value32 >> 8);
    dstPtr[2] = (unsigned char)(value32 >> 16);
    dstPtr[3] = (unsigned char)(value32 >> 24);
}

static int LZ4IO_isSkippableMagicNumber(unsigned int magic) { return (magic & LZ4IO_SKIPPABLEMASK) == LZ4IO_SKIPPABLE0; }



//------------------------------------------------------------


||||||| .r8820
=======
FileBinaryLZ4::~FileBinaryLZ4() {

}

>>>>>>> .r9014
bool FileBinaryLZ4::open () {

	if (_is_open) {
		ssi_wrn ("file already open ('%s', %s, %s)", _path, MODE_NAMES[_mode], TYPE_NAMES[_type]);
		return true;
	}

	if (_file) {
		ssi_wrn ("file pointer not empty ('%s', %s, %s)", _path, MODE_NAMES[_mode], TYPE_NAMES[_type]);
		return true;
	}	

	if (!_close_file) {
		ssi_wrn ("no permission to open file ('%s', %s, %s)", _path, MODE_NAMES[_mode], TYPE_NAMES[_type]);
		return false;
	}

	bool decompress = false;

	switch (_mode) {
		case File::READ: {
			_file = fopen (_path, "rb");
			decompress = true;
			break;
		}
		case File::WRITE: {
			_file = fopen (_path, "wb");
			break;
		}
		case File::APPEND: {
			_file = fopen (_path, "ab");
			break;
		}
		case File::READPLUS: {
			_file = fopen (_path, "r+b");
			break;
		}
		case File::WRITEPLUS: {
			_file = fopen (_path, "w+b");
			break;
		}
		case File::APPENDPLUS: {
			_file = fopen (_path, "a+b");
			break;
		}
	}

	if (!_file) {
		ssi_wrn ("fopen() failed (\"%s\")", _path);
		return false;
	}
	
	_is_open = true;

	ssi_msg (SSI_LOG_LEVEL_DETAIL, "opened ('%s', %s, %s)", _path, MODE_NAMES[_mode], TYPE_NAMES[_type]);


	LZ4F_errorCode_t errorCode;

	if (!decompress) {
		//init LZ4 compress
		memset(&prefs, 0, sizeof(prefs));

		errorCode = LZ4F_createCompressionContext(&ctx, LZ4F_VERSION);

		prefs.autoFlush = 1;
		prefs.compressionLevel = 16;
		prefs.frameInfo.blockMode = (blockMode_t)1;
		prefs.frameInfo.blockSizeID = (blockSizeID_t)7;
		prefs.frameInfo.contentChecksumFlag = (contentChecksum_t)1;
	}
	else {
		//init LZ4 decompress

		//get archive header
		unsigned char U32store[MAGICNUMBER_SIZE];
		unsigned magicNumber, size;

		size_t nbReadBytes = fread(U32store, 1, MAGICNUMBER_SIZE, _file);

		if (nbReadBytes != MAGICNUMBER_SIZE)
			ssi_err("LZ4: Unrecognized header!");

		magicNumber = LZ4IO_readLE32(U32store);   /* Little Endian format */
		if (LZ4IO_isSkippableMagicNumber(magicNumber)) magicNumber = LZ4IO_SKIPPABLE0;  /* fold skippable magic numbers */

		if (magicNumber!= LZ4IO_MAGICNUMBER)
			ssi_err("Unknown LZ4 archive format!");

		errorCode = LZ4F_createDecompressionContext(&ctx, LZ4F_VERSION);

		#   define HEADERMAX 20
		char  headerBuff[HEADERMAX];
		LZ4IO_writeLE32(headerBuff, LZ4IO_MAGICNUMBER);

		in_buff = new char[256*1024];
		out_buff = new char[256*1024];

		/* Init feed with magic number (already consumed from _file) */
		size_t inSize = 4;
        size_t outSize=0;
        LZ4IO_writeLE32(in_buff, LZ4IO_MAGICNUMBER);
        errorCode = LZ4F_decompress(ctx, out_buff, &outSize, in_buff, &inSize, NULL);

	}

	firstRun = true;

	return true;
}



bool FileBinaryLZ4::close () {

	if (!_is_open) {
		ssi_wrn ("file already closed ('%s', %s, %s)", _path, MODE_NAMES[_mode], TYPE_NAMES[_type]);
		return true;
	}

	if (!_file) {
		ssi_wrn ("file pointer empty ('%s', %s, %s)", _path, MODE_NAMES[_mode], TYPE_NAMES[_type]);
		return true;
	}

	if (!_close_file) {
		ssi_wrn ("no permission to close file ('%s', %s, %s)", _path, MODE_NAMES[_mode], TYPE_NAMES[_type]);
		return false;
	}

	if (fclose (_file)) {
		ssi_wrn ("fclose() failed ('%s', %s, %s)", _path, MODE_NAMES[_mode], TYPE_NAMES[_type]);
		return false;
	}

	_is_open = false;
	_file = 0;

	ssi_msg (SSI_LOG_LEVEL_DETAIL, "closed ('%s', %s, %s)", _path, MODE_NAMES[_mode], TYPE_NAMES[_type]);

	//clean LZ4

	//protect in_buff / out_buff on exit
	mutex.acquire();

	delete[] in_buff; in_buff = 0;
	delete[] out_buff; out_buff = 0;
	LZ4F_freeCompressionContext(ctx);

	mutex.release();

	return true;
}



bool FileBinaryLZ4::read (void *ptr, ssi_size_t size, ssi_size_t count) {

	ssi_err("reading of LZ4 files not yet implemented!");

	if (!_is_open) {
		ssi_wrn ("file not open (path=%s, mode=%d, type=%d, shared=false)", _path, _mode, _type);
		return false;
	}

	if (!_read_mode) {
		ssi_wrn ("file not in read mode (path=%s, mode=%d, type=%d, shared=false)", _path, _mode, _type);
		return false;
	}



	/*
	size_t res = fread (ptr, size, count, _file);
	if (res != count) {
		ssi_wrn ("fread() failed (path=%s, mode=%d, type=%d, shared=false)", _path, _mode, _type);
		return false;
	}*/



	if (firstRun) {


	}

	return true;
}

bool FileBinaryLZ4::write (const void *ptr, ssi_size_t size, ssi_size_t count) {

	if (!_is_open) {
		ssi_wrn ("file not open (path=%s, mode=%d, type=%d, shared=false)", _path, _mode, _type);
		return false;
	}

	if (!_write_mode) {
		ssi_wrn ("file not in write mode (path=%s, mode=%d, type=%d, shared=false)", _path, _mode, _type);
		return false;
	}

	//protect in_buff / out_buff on exit
	mutex.acquire();

	if (firstRun) {
		//set buffers on first run with blocksize (in bytes)
		blocksize = size*count;
		in_buff  = new char[blocksize];
		outBuffSize = (int32_t) LZ4F_compressBound(blocksize, &prefs);
		out_buff = new char[outBuffSize];

		int32_t headerSize = (int32_t)LZ4F_compressBegin(ctx, out_buff, outBuffSize, &prefs);
		int32_t sizeCheck = (int32_t) fwrite(out_buff, 1, headerSize, _file);
		
		firstRun = false;
	} else {

		if (blocksize != size*count)
			ssi_err("illegal blocksize change!");
	}

	memcpy(in_buff, ptr, count*size);

	int32_t outSize = (int32_t) LZ4F_compressUpdate(ctx, out_buff, outBuffSize, in_buff, count*size, NULL);
	ssi_size_t res = ssi_cast (ssi_size_t, fwrite (out_buff, 1, outSize, _file));

	mutex.release();

	ssi_msg (SSI_LOG_LEVEL_BASIC, "LZ4 compr: bytes in: %i | out: %i  | out size: %.2f%%", count*size, outSize, (100.f / (count*size)) * outSize );

	if (res != outSize) {
		ssi_wrn ("fwrite() failed (path=%s, mode=%d, type=%d, shared=false)", _path, _mode, _type);
		return false;
	}

	return true;
}

bool FileBinaryLZ4::writeLine (const ssi_char_t *string) {

	ssi_err("write line for LZ4 files not yet implemented!");


	if (!_is_open) {
		ssi_wrn ("file not open (path=%s, mode=%d, type=%d, shared=false)", _path, _mode, _type);
		return false;
	}

	if (!_write_mode) {
		ssi_wrn ("file not in write mode (path=%s, mode=%d, type=%d, shared=false)", _path, _mode, _type);
		return false;
	}

	bool status;

	ssi_size_t len = ssi_cast (ssi_size_t, strlen (string));
	status = write (&len, sizeof (ssi_size_t), 1);
	if (!status) {
		ssi_err ("write() failed (path=%s, mode=%d, type=%d, shared=false)", _path, _mode, _type);
	}
	status = write (string, sizeof (ssi_char_t), len);
	if (!status) {
		ssi_wrn ("write() failed (path=%s, mode=%d, type=%d, shared=false)", _path, _mode, _type);
		return false;
	}

	return true;
}

bool FileBinaryLZ4::readLine (ssi_size_t num, ssi_char_t *string) {

	ssi_err("read line for LZ4 files not yet implemented!");

	if (!_is_open) {
		ssi_wrn ("file not open (path=%s, mode=%d, type=%d, shared=false)", _path, _mode, _type);
		return false;
	}

	if (!_read_mode) {
		ssi_wrn ("file not in read mode (path=%s, mode=%d, type=%d, shared=false)", _path, _mode, _type);
		return false;
	}

	bool status;

	ssi_size_t len;
	status = read (&len, sizeof (ssi_size_t), 1);
	if (!status)
		return false;
	if (num <= len) {
		ssi_err ("input string too short (path=%s, mode=%d, type=%d, shared=false)", _path, _mode, _type);
		return false;
	}
	status = read (string, ssi_cast (ssi_size_t, sizeof (ssi_char_t)), len);
	if (!status)
		return false;
	string[len] = '\0';

	return true;
}


}

