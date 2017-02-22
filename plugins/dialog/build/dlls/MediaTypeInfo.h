// MediaTypeInfo.h
// author: Frank Jung <frank.jung@informatik.uni-augsburg.de>
// created: 2009/03/01
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

namespace DialogLib {

public ref class MediaTypeInfo
{
public:
	MediaTypeInfo(void);

	//AM_MEDIA_TYPE
	[System::ComponentModel::CategoryAttribute::Category("AM_MEDIA_TYPE"),
		System::ComponentModel::DescriptionAttribute::Description("Globally unique identifier (GUID) that specifies the major type of the media sample."),
		System::ComponentModel::ReadOnlyAttribute::ReadOnly(true)]
	property System::String^		majortype;
	[System::ComponentModel::CategoryAttribute::Category("AM_MEDIA_TYPE"),
		System::ComponentModel::DescriptionAttribute::Description("GUID that specifies the subtype of the media sample. For a list of possible subtypes, see Media Types. For some formats, the value might be MEDIASUBTYPE_None, which means the format does not require a subtype."),
		System::ComponentModel::ReadOnlyAttribute::ReadOnly(true)]
	property System::String^		subtype;
	[System::ComponentModel::CategoryAttribute::Category("AM_MEDIA_TYPE"),
		System::ComponentModel::DescriptionAttribute::Description("If TRUE, samples are of a fixed size. This field is informational only. For audio, it is generally set to TRUE. For video, it is usually TRUE for uncompressed video and FALSE for compressed video."),
		System::ComponentModel::ReadOnlyAttribute::ReadOnly(true)]
	property System::Boolean		bFixedSizeSamples;
	[System::ComponentModel::CategoryAttribute::Category("AM_MEDIA_TYPE"),
		System::ComponentModel::DescriptionAttribute::Description("If TRUE, samples are compressed using temporal (interframe) compression. A value of TRUE indicates that not all frames are key frames. This field is informational only."),
		System::ComponentModel::ReadOnlyAttribute::ReadOnly(true)]
	property System::Boolean		bTemporalCompression;
	[System::ComponentModel::CategoryAttribute::Category("AM_MEDIA_TYPE"),
		System::ComponentModel::DescriptionAttribute::Description("Size of the sample in bytes. For compressed data, the value can be zero."),
		System::ComponentModel::ReadOnlyAttribute::ReadOnly(true)]
	property System::UInt32			lSampleSize;
	[System::ComponentModel::CategoryAttribute::Category("AM_MEDIA_TYPE"),
		System::ComponentModel::DescriptionAttribute::Description("GUID that specifies the structure used for the format block. The pbFormat member points to the corresponding format structure. Format types include the following: "),
		System::ComponentModel::ReadOnlyAttribute::ReadOnly(true)]
	property System::String^		formattype;
	[System::ComponentModel::CategoryAttribute::Category("AM_MEDIA_TYPE"),
		System::ComponentModel::DescriptionAttribute::Description("Size of the format block, in bytes."),
		System::ComponentModel::ReadOnlyAttribute::ReadOnly(true)]
	property System::UInt32			cbFormat;

	//VIDEO_STREAM_CONFIG_CAPS
	[System::ComponentModel::CategoryAttribute::Category("VIDEO_STREAM_CONFIG_CAPS"),
		System::ComponentModel::DescriptionAttribute::Description("GUID that identifies the format type. For example, FORMAT_VideoInfo or FORMAT_VideoInfo2."),
		System::ComponentModel::ReadOnlyAttribute::ReadOnly(true)]
	property System::String^				guid;
	[System::ComponentModel::CategoryAttribute::Category("VIDEO_STREAM_CONFIG_CAPS"),
		System::ComponentModel::DescriptionAttribute::Description("Native size of the incoming video signal. For a compressor, the size is taken from the input pin. For a capture filter, the size is the largest signal the filter can digitize with every pixel remaining unique."),
		System::ComponentModel::ReadOnlyAttribute::ReadOnly(true)]
	property System::Drawing::Size			InputSize;
	[System::ComponentModel::CategoryAttribute::Category("VIDEO_STREAM_CONFIG_CAPS"),
		System::ComponentModel::DescriptionAttribute::Description("Smallest source rectangle allowed."),
		System::ComponentModel::ReadOnlyAttribute::ReadOnly(true)]
	property System::Drawing::Size			MinCroppingSize;
	[System::ComponentModel::CategoryAttribute::Category("VIDEO_STREAM_CONFIG_CAPS"),
		System::ComponentModel::DescriptionAttribute::Description("Largest source rectangle allowed."),
		System::ComponentModel::ReadOnlyAttribute::ReadOnly(true)]
	property System::Drawing::Size			MaxCroppingSize;
	[System::ComponentModel::CategoryAttribute::Category("VIDEO_STREAM_CONFIG_CAPS"),
		System::ComponentModel::DescriptionAttribute::Description("Horizontal granularity of the source rectangle. This value specifies the increments that are valid between MinCroppingSize and MaxCroppingSize."),
		System::ComponentModel::ReadOnlyAttribute::ReadOnly(true)]
	property System::Int32					CropGranularityX;
	[System::ComponentModel::CategoryAttribute::Category("VIDEO_STREAM_CONFIG_CAPS"),
		System::ComponentModel::DescriptionAttribute::Description("Vertical granularity of the source rectangle. This value specifies the increments that are valid between MinCroppingSize and MaxCroppingSize."),
		System::ComponentModel::ReadOnlyAttribute::ReadOnly(true)]
	property System::Int32					CropGranularityY;
	[System::ComponentModel::CategoryAttribute::Category("VIDEO_STREAM_CONFIG_CAPS"),
		System::ComponentModel::DescriptionAttribute::Description("Required horizontal alignment of the source rectangle."),
		System::ComponentModel::ReadOnlyAttribute::ReadOnly(true)]
	property System::Int32					CropAlignX;
	[System::ComponentModel::CategoryAttribute::Category("VIDEO_STREAM_CONFIG_CAPS"),
		System::ComponentModel::DescriptionAttribute::Description("Required vertical alignment of the source rectangle."),
		System::ComponentModel::ReadOnlyAttribute::ReadOnly(true)]
	property System::Int32					CropAlignY;
	[System::ComponentModel::CategoryAttribute::Category("VIDEO_STREAM_CONFIG_CAPS"),
		System::ComponentModel::DescriptionAttribute::Description("Minimum output size."),
		System::ComponentModel::ReadOnlyAttribute::ReadOnly(true)]
	property System::Drawing::Size			MinOutputSize;
	[System::ComponentModel::CategoryAttribute::Category("VIDEO_STREAM_CONFIG_CAPS"),
		System::ComponentModel::DescriptionAttribute::Description("Maximum output size."),
		System::ComponentModel::ReadOnlyAttribute::ReadOnly(true)]
	property System::Drawing::Size			MaxOutputSize;
	[System::ComponentModel::CategoryAttribute::Category("VIDEO_STREAM_CONFIG_CAPS"),
		System::ComponentModel::DescriptionAttribute::Description("Granularity of the output width. This value specifies the increments that are valid between MinOutputSize and MaxOutputSize."),
		System::ComponentModel::ReadOnlyAttribute::ReadOnly(true)]
	property System::Int32					OutputGranularityX;
	[System::ComponentModel::CategoryAttribute::Category("VIDEO_STREAM_CONFIG_CAPS"),
		System::ComponentModel::DescriptionAttribute::Description("Granularity of the output height. This value specifies the increments that are valid between MinOutputSize and MaxOutputSize."),
		System::ComponentModel::ReadOnlyAttribute::ReadOnly(true)]
	property System::Int32					OutputGranularityY;
	[System::ComponentModel::CategoryAttribute::Category("VIDEO_STREAM_CONFIG_CAPS"),
		System::ComponentModel::DescriptionAttribute::Description("Indicates how well the filter can stretch the image horizontally."),
		System::ComponentModel::ReadOnlyAttribute::ReadOnly(true)]
	property System::Int32					StretchTapsX;
	[System::ComponentModel::CategoryAttribute::Category("VIDEO_STREAM_CONFIG_CAPS"),
		System::ComponentModel::DescriptionAttribute::Description("Indicates how well the filter can stretch the image vertically."),
		System::ComponentModel::ReadOnlyAttribute::ReadOnly(true)]
	property System::Int32					StretchTapsY;
	[System::ComponentModel::CategoryAttribute::Category("VIDEO_STREAM_CONFIG_CAPS"),
		System::ComponentModel::DescriptionAttribute::Description("Indicates how well the filter can shrink the image horizontally."),
		System::ComponentModel::ReadOnlyAttribute::ReadOnly(true)]
	property System::Int32					ShrinkTapsX;
	[System::ComponentModel::CategoryAttribute::Category("VIDEO_STREAM_CONFIG_CAPS"),
		System::ComponentModel::DescriptionAttribute::Description("Indicates how well the filter can shrink the image vertically."),
		System::ComponentModel::ReadOnlyAttribute::ReadOnly(true)]
	property System::Int32					ShrinkTapsY;
	[System::ComponentModel::CategoryAttribute::Category("VIDEO_STREAM_CONFIG_CAPS"),
		System::ComponentModel::DescriptionAttribute::Description("The minimum frame duration, in 100-nanosecond units. This value applies only to capture filters."),
		System::ComponentModel::ReadOnlyAttribute::ReadOnly(true)]
	property System::Int64					MinFrameInterval;
	[System::ComponentModel::CategoryAttribute::Category("VIDEO_STREAM_CONFIG_CAPS"),
		System::ComponentModel::DescriptionAttribute::Description("The maximum frame duration, in 100-nanosecond units. This value applies only to capture filters."),
		System::ComponentModel::ReadOnlyAttribute::ReadOnly(true)]
	property System::Int64					MaxFrameInterval;
	[System::ComponentModel::CategoryAttribute::Category("VIDEO_STREAM_CONFIG_CAPS"),
		System::ComponentModel::DescriptionAttribute::Description("Minimum data rate this pin can produce."),
		System::ComponentModel::ReadOnlyAttribute::ReadOnly(true)]
	property System::Int32					MinBitsPerSecond;
	[System::ComponentModel::CategoryAttribute::Category("VIDEO_STREAM_CONFIG_CAPS"),
		System::ComponentModel::DescriptionAttribute::Description("Maximum data rate this pin can produce."),
		System::ComponentModel::ReadOnlyAttribute::ReadOnly(true)]
	property System::Int32					MaxBitsPerSecond;

	//VIDEOINFOHEADER
	[System::ComponentModel::CategoryAttribute::Category("VIDEOINFOHEADER"),
		System::ComponentModel::DescriptionAttribute::Description("RECT structure that specifies the source video window. This structure can be a clipping rectangle, to select a portion of the source video stream."),
		System::ComponentModel::ReadOnlyAttribute::ReadOnly(true)]
	property System::Drawing::Rectangle		rcSource;
	[System::ComponentModel::CategoryAttribute::Category("VIDEOINFOHEADER"),
		System::ComponentModel::DescriptionAttribute::Description("RECT structure that specifies the destination video window."),
		System::ComponentModel::ReadOnlyAttribute::ReadOnly(true)]
	property System::Drawing::Rectangle		rcTarget;
	[System::ComponentModel::CategoryAttribute::Category("VIDEOINFOHEADER"),
		System::ComponentModel::DescriptionAttribute::Description("Approximate data rate of the video stream, in bits per second."),
		System::ComponentModel::ReadOnlyAttribute::ReadOnly(true)]
	property System::UInt32					dwBitRate;
	[System::ComponentModel::CategoryAttribute::Category("VIDEOINFOHEADER"),
		System::ComponentModel::DescriptionAttribute::Description("Data error rate, in bit errors per second."),
		System::ComponentModel::ReadOnlyAttribute::ReadOnly(true)]
	property System::UInt32					dwBitErrorRate;
	[System::ComponentModel::CategoryAttribute::Category("VIDEOINFOHEADER"),
		System::ComponentModel::DescriptionAttribute::Description("The desired average display time of the video frames, in 100-nanosecond units. The actual time per frame may be longer."),
		System::ComponentModel::ReadOnlyAttribute::ReadOnly(true)]
	property System::Int64					AvgTimePerFrame;

	//BITMAPINFOHEADER
	[System::ComponentModel::CategoryAttribute::Category("BITMAPINFOHEADER"),
		System::ComponentModel::DescriptionAttribute::Description("Specifies the number of bytes required by the structure. This value does not include the size of the color table or the size of the color masks, if they are appended to the end of structure."),
		System::ComponentModel::ReadOnlyAttribute::ReadOnly(true)]
	property System::UInt32					biSize;
	[System::ComponentModel::CategoryAttribute::Category("BITMAPINFOHEADER"),
		System::ComponentModel::DescriptionAttribute::Description("Specifies the width of the bitmap, in pixels."),
		System::ComponentModel::ReadOnlyAttribute::ReadOnly(true)]
	property System::Int32					biWidth;
	[System::ComponentModel::CategoryAttribute::Category("BITMAPINFOHEADER"),
		System::ComponentModel::DescriptionAttribute::Description("Specifies the height of the bitmap, in pixels. For uncompressed RGB bitmaps, if biHeight is positive, the bitmap is a bottom-up DIB with the origin at the lower left corner. If biHeight is negative, the bitmap is a top-down DIB with the origin at the upper left corner. For YUV bitmaps, the bitmap is always top-down, regardless of the sign of biHeight. Decoders should offer YUV formats with postive biHeight, but for backward compatibility they should accept YUV formats with either positive or negative biHeight. For compressed formats, biHeight must be positive, regardless of image orientation."),
		System::ComponentModel::ReadOnlyAttribute::ReadOnly(true)]
	property System::Int32					biHeight;
	[System::ComponentModel::CategoryAttribute::Category("BITMAPINFOHEADER"),
		System::ComponentModel::DescriptionAttribute::Description("Specifies the number of planes for the target device. This value must be set to 1."),
		System::ComponentModel::ReadOnlyAttribute::ReadOnly(true)]
	property System::UInt16					biPlanes;
	[System::ComponentModel::CategoryAttribute::Category("BITMAPINFOHEADER"),
		System::ComponentModel::DescriptionAttribute::Description("Specifies the number of bits per pixel (bpp).  For uncompressed formats, this value gives to the average number of bits per pixel. For compressed formats, this value gives the implied bit depth of the uncompressed image, after the image has been decoded."),
		System::ComponentModel::ReadOnlyAttribute::ReadOnly(true)]
	property System::UInt16					biBitCount;
	[System::ComponentModel::CategoryAttribute::Category("BITMAPINFOHEADER"),
		System::ComponentModel::DescriptionAttribute::Description("For compressed video and YUV formats, this member is a FOURCC code, specified as a DWORD in little-endian order. For example, YUYV video has the FOURCC 'VYUY' or 0x56595559."),
		System::ComponentModel::ReadOnlyAttribute::ReadOnly(true)]
	property System::UInt32					biCompression;
	[System::ComponentModel::CategoryAttribute::Category("BITMAPINFOHEADER"),
		System::ComponentModel::DescriptionAttribute::Description("Specifies the size, in bytes, of the image. This can be set to 0 for uncompressed RGB bitmaps."),
		System::ComponentModel::ReadOnlyAttribute::ReadOnly(true)]
	property System::UInt32					biSizeImage;
	[System::ComponentModel::CategoryAttribute::Category("BITMAPINFOHEADER"),
		System::ComponentModel::DescriptionAttribute::Description("Specifies the horizontal resolution, in pixels per meter, of the target device for the bitmap."),
		System::ComponentModel::ReadOnlyAttribute::ReadOnly(true)]
	property System::Int32					biXPelsPerMeter;
	[System::ComponentModel::CategoryAttribute::Category("BITMAPINFOHEADER"),
		System::ComponentModel::DescriptionAttribute::Description("Specifies the vertical resolution, in pixels per meter, of the target device for the bitmap."),
		System::ComponentModel::ReadOnlyAttribute::ReadOnly(true)]
	property System::Int32					biYPelsPerMeter;
	[System::ComponentModel::CategoryAttribute::Category("BITMAPINFOHEADER"),
		System::ComponentModel::DescriptionAttribute::Description("Specifies the number of color indices in the color table that are actually used by the bitmap."),
		System::ComponentModel::ReadOnlyAttribute::ReadOnly(true)]
	property System::UInt32					biClrUsed;
	[System::ComponentModel::CategoryAttribute::Category("BITMAPINFOHEADER"),
		System::ComponentModel::DescriptionAttribute::Description("Specifies the number of color indices that are considered important for displaying the bitmap. If this value is zero, all colors are important."),
		System::ComponentModel::ReadOnlyAttribute::ReadOnly(true)]
	property System::UInt32					biClrImportant;

	//DVINFO
	[System::ComponentModel::CategoryAttribute::Category("DVINFO"),
		System::ComponentModel::DescriptionAttribute::Description("Specifies the Audio Auxiliary Data Source Pack for the first audio block (first 5 DV DIF sequences for 525-60 systems or 6 DV DIF sequences for 625-50 systems) of a frame. A DIF sequence is a data block that contains 150 DIF blocks. A DIF block consists of 80 bytes. The Audio Auxiliary Data Source Pack is defined in section D.7.1 of Part 2, Annex D, \"The Pack Header Table and Contents of Packs\" of the Specification of Consumer-use Digital VCRs."),
		System::ComponentModel::ReadOnlyAttribute::ReadOnly(true)]
	property System::UInt32					dwDVAAuxSrc;
	[System::ComponentModel::CategoryAttribute::Category("DVINFO"),
		System::ComponentModel::DescriptionAttribute::Description("Specifies the Audio Auxiliary Data Source Control Pack for the first audio block of a frame. The Audio Auxiliary Data Control Pack is defined in section D.7.2 of Part 2, Annex D, \"The Pack Header Table and Contents of Packs\" of the Specification of Consumer-use Digital VCRs."),
		System::ComponentModel::ReadOnlyAttribute::ReadOnly(true)]
	property System::UInt32					dwDVAAuxCtl;
	[System::ComponentModel::CategoryAttribute::Category("DVINFO"),
		System::ComponentModel::DescriptionAttribute::Description("Specifies the Audio Auxiliary Data Source Pack for the second audio block (second 5 DV DIF sequences for 525-60 systems or 6 DV DIF sequences for 625-50 systems) of a frame."),
		System::ComponentModel::ReadOnlyAttribute::ReadOnly(true)]
	property System::UInt32					dwDVAAuxSrc1;
	[System::ComponentModel::CategoryAttribute::Category("DVINFO"),
		System::ComponentModel::DescriptionAttribute::Description("Specifies the Audio Auxiliary Data Source Control Pack for the second audio block of a frame."),
		System::ComponentModel::ReadOnlyAttribute::ReadOnly(true)]
	property System::UInt32					dwDVAAuxCtl1;
	[System::ComponentModel::CategoryAttribute::Category("DVINFO"),
		System::ComponentModel::DescriptionAttribute::Description("Specifies the Video Auxiliary Data Source Pack as defined in section D.8.1 of Part 2, Annex D, \"The Pack Header Table and Contents of Packs\" of the Specification of Consumer-use Digital VCRs."),
		System::ComponentModel::ReadOnlyAttribute::ReadOnly(true)]
	property System::UInt32					dwDVVAuxSrc;
	[System::ComponentModel::CategoryAttribute::Category("DVINFO"),
		System::ComponentModel::DescriptionAttribute::Description("Specifies the Video Auxiliary Data Source Control Pack as defined in section D.8.2 of Part 2, Annex D, \"The Pack Header Table and Contents of Packs\" of the Specification of Consumer-use Digital VCRs."),
		System::ComponentModel::ReadOnlyAttribute::ReadOnly(true)]
	property System::UInt32					dwDVVAuxCtl;
};

}
