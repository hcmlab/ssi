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
#include "thread/Lock.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

#define LZ4IO_HEADERMAX 20
#define LZ4IO_MAGICNUMBER_SIZE    4
#define LZ4IO_MAGICNUMBER   0x184D2204
#define LZ4IO_SKIPPABLE0    0x184D2A50
#define LZ4IO_SKIPPABLEMASK 0xFFFFFFF0

static unsigned LZ4IO_readLE32(const void* s)
{
	const unsigned char* srcPtr = (const unsigned char*)s;
	unsigned value32 = srcPtr[0];
	value32 += (srcPtr[1] << 8);
	value32 += (srcPtr[2] << 16);
	value32 += (srcPtr[3] << 24);
	return value32;
}

static void LZ4IO_writeLE32(void* p, unsigned value32)
{
	unsigned char* dstPtr = (unsigned char*)p;
	dstPtr[0] = (unsigned char)value32;
	dstPtr[1] = (unsigned char)(value32 >> 8);
	dstPtr[2] = (unsigned char)(value32 >> 16);
	dstPtr[3] = (unsigned char)(value32 >> 24);
}

static int LZ4IO_isSkippableMagicNumber(unsigned int magic) { return (magic & LZ4IO_SKIPPABLEMASK) == LZ4IO_SKIPPABLE0; }

FileBinaryLZ4::FileBinaryLZ4()
: in_buff (0), 
out_buff (0) {
}

FileBinaryLZ4::~FileBinaryLZ4() {

}

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
			_file = ssi_fopen (_path, "rb");
			decompress = true;
			break;
		}
		case File::WRITE: {
			_file = ssi_fopen (_path, "wb");
			break;
		}
		default:
			ssi_wrn("LZ4 supports only binary read/write oprations");
			return false;
	}

	if (!_file) {
		ssi_wrn ("could not open '%s'", _path);
		return false;
	}
	
	_is_open = true;

	ssi_msg (SSI_LOG_LEVEL_DETAIL, "opened ('%s', %s, %s)", _path, MODE_NAMES[_mode], TYPE_NAMES[_type]);

	//init LZ4
	LZ4F_errorCode_t errorCode;

	if (!decompress) 
	{

		memset(&prefs, 0, sizeof(prefs));

		errorCode = LZ4F_createCompressionContext(&ctx, LZ4F_VERSION);

		prefs.autoFlush = 1;
		prefs.compressionLevel = 16;
		prefs.frameInfo.blockMode = (blockMode_t)1;
		prefs.frameInfo.blockSizeID = (blockSizeID_t)7;
		prefs.frameInfo.contentChecksumFlag = (contentChecksum_t)1;

		firstRun = true;
	}
	else
	{

		ssi_wrn("not implemented");
		return false;

		//init LZ4 decompress

		//get archive header
		unsigned char U32store[LZ4IO_MAGICNUMBER_SIZE];
		unsigned magicNumber;

		size_t nbReadBytes = fread(U32store, 1, LZ4IO_MAGICNUMBER_SIZE, _file);

		if (nbReadBytes != LZ4IO_MAGICNUMBER_SIZE)
		{
			ssi_wrn("LZ4: unrecognized header");
			return false;
		}

		magicNumber = LZ4IO_readLE32(U32store);   // Little Endian format
		if (LZ4IO_isSkippableMagicNumber(magicNumber)) magicNumber = LZ4IO_SKIPPABLE0;  // fold skippable magic numbers

		if (magicNumber != LZ4IO_MAGICNUMBER)
		{
			ssi_wrn("LZ4: unknown archive format");
			return false;
		}

		errorCode = LZ4F_createDecompressionContext(&ctx, LZ4F_VERSION);

		//char headerBuff[LZ4IO_HEADERMAX];
		//LZ4IO_writeLE32(headerBuff, LZ4IO_MAGICNUMBER);

		//in_buff = new char[256 * 1024];
		//out_buff = new char[256 * 1024];

		//// Init feed with magic number (already consumed from _file)
		//size_t inSize = 4;
		//size_t outSize = 0;
		//LZ4IO_writeLE32(in_buff, LZ4IO_MAGICNUMBER);
		//errorCode = LZ4F_decompress(ctx, out_buff, &outSize, in_buff, &inSize, NULL);
	}

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
	{
		Lock lock(mutex);

		delete[] in_buff; in_buff = 0;
		delete[] out_buff; out_buff = 0;
		LZ4F_freeCompressionContext(ctx);
	}

	return true;
}

ssi_size_t FileBinaryLZ4::read (void *ptr, ssi_size_t n_bytes_ptr, ssi_size_t n_compressed_bytes_to_read) {

	if (!_is_open) {
		ssi_wrn ("file not open (path=%s, mode=%d, type=%d, shared=false)", _path, _mode, _type);
		return 0;
	}

	if (!_read_mode) {
		ssi_wrn ("file not in read mode (path=%s, mode=%d, type=%d, shared=false)", _path, _mode, _type);
		return 0;
	}

	ssi_wrn("not implemented");
	
	return 0;
}

ssi_size_t FileBinaryLZ4::write (const void *ptr, ssi_size_t size, ssi_size_t count) {

	if (!_is_open) {
		ssi_wrn ("file not open (path=%s, mode=%d, type=%d, shared=false)", _path, _mode, _type);
		return 0;
	}

	if (!_write_mode) {
		ssi_wrn ("file not in write mode (path=%s, mode=%d, type=%d, shared=false)", _path, _mode, _type);
		return 0;
	}

	int64_t old_pos = tell();

	int32_t outSize = 0;
	ssi_size_t res = 0;

	//protect in_buff / out_buff on exit
	{
		Lock lock(mutex);

		if (firstRun) {

			//set buffers on first run with blocksize (in bytes)
			blocksize = size*count;
			in_buff = new char[blocksize];
			outBuffSize = (int32_t)LZ4F_compressBound(blocksize, &prefs);
			out_buff = new char[outBuffSize];

			int32_t headerSize = (int32_t)LZ4F_compressBegin(ctx, out_buff, outBuffSize, &prefs);
			int32_t sizeCheck = (int32_t)fwrite(out_buff, 1, headerSize, _file);

			firstRun = false;
		}
		else {

			if (blocksize != size*count)
			{
				ssi_wrn("illegal blocksize change!");
				return 0;
			}
		}

		memcpy(in_buff, ptr, count*size);

		outSize = (int32_t)LZ4F_compressUpdate(ctx, out_buff, outBuffSize, in_buff, count*size, NULL);
		res = ssi_cast(ssi_size_t, fwrite(out_buff, 1, outSize, _file));
	}	

	ssi_msg (SSI_LOG_LEVEL_DETAIL, "LZ4 compr: bytes in: %i | out: %i  | out size: %.2f%%", count*size, outSize, (100.f / (count*size)) * outSize );

	if (res != outSize) {
		ssi_wrn ("fwrite() failed (path=%s, mode=%d, type=%d, shared=false)", _path, _mode, _type);
		return 0;
	}

	int64_t new_pos = tell();

	return ssi_size_t(new_pos - old_pos);
}

ssi_size_t FileBinaryLZ4::writeLine (const ssi_char_t *string) {

	ssi_err("write line for LZ4 files not yet implemented!");

	if (!_is_open) {
		ssi_wrn ("file not open (path=%s, mode=%d, type=%d, shared=false)", _path, _mode, _type);
		return 0;
	}

	if (!_write_mode) {
		ssi_wrn ("file not in write mode (path=%s, mode=%d, type=%d, shared=false)", _path, _mode, _type);
		return 0;
	}

	ssi_size_t result_1, result_2;

	ssi_size_t len = ssi_cast (ssi_size_t, strlen (string));
	result_1 = write (&len, sizeof (ssi_size_t), 1);
	if (!result_1) {
		ssi_wrn ("write() failed (path=%s, mode=%d, type=%d, shared=false)", _path, _mode, _type);
		return 0;
	}
	result_2 = write (string, sizeof (ssi_char_t), len);
	if (!result_2) {
		ssi_wrn ("write() failed (path=%s, mode=%d, type=%d, shared=false)", _path, _mode, _type);
		return 0;
	}

	return result_1+result_2;
}

ssi_size_t FileBinaryLZ4::readLine (ssi_size_t num, ssi_char_t *string) {

	ssi_err("read line for LZ4 files not yet implemented!");

	if (!_is_open) {
		ssi_wrn ("file not open (path=%s, mode=%d, type=%d, shared=false)", _path, _mode, _type);
		return 0;
	}

	if (!_read_mode) {
		ssi_wrn ("file not in read mode (path=%s, mode=%d, type=%d, shared=false)", _path, _mode, _type);
		return 0;
	}

	ssi_wrn("not implemented");

	return 0;


	ssi_size_t result_1, result_2;

	ssi_size_t len;
	result_1 = read (&len, sizeof (ssi_size_t), 1);
	if (!result_1)
	{
		return 0;
	}
	if (num <= len) 
	{	
		ssi_wrn ("input string too short (path=%s, mode=%d, type=%d, shared=false)", _path, _mode, _type);
		return 0;
	}
	result_2 = read (string, ssi_cast (ssi_size_t, sizeof (ssi_char_t)), len);
	if (!result_2)
	{
		return 0;
	}
	string[len] = '\0';

	return result_1 + result_2;
}


}

