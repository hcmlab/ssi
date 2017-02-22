// VideoPainter.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/10/07
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

#ifndef SSI_GRAPHIC_VIDEOPAINTER_H
#define SSI_GRAPHIC_VIDEOPAINTER_H

#include "base/IConsumer.h"
#include "PaintVideo.h"
#include "PaintPoints.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class VideoPainter : public IConsumer {

public:

	class Options : public OptionList {

	public:

		Options ()
			: scale(true), flip(true), mirror(false), maxValue(-1), precision(1), relative(false), swap(false), labels(false), accurate(false) {

			name[0] = '\0';
			setPos (0,0,100,100);			

			addOption ("title", name, SSI_MAX_CHAR, SSI_CHAR, "window caption");
			addOption ("pos", pos, 4, SSI_INT, "window position (top, left, width, height)");
			addOption ("accurate", &accurate, 1, SSI_BOOL, "window position describes client area");
			addOption ("flip", &flip, 1, SSI_BOOL, "flip video image");
			addOption ("mirror", &mirror, 1, SSI_BOOL, "mirror video image");
			addOption ("maxValue", &maxValue, 1, SSI_DOUBLE, "max pixel value for video image");
			addOption ("scale", &scale, 1, SSI_BOOL, "scale video image");			
			addOption ("type", &type, 1, SSI_UCHAR, "additional position stream: plot type (0=dots, 1=lines)");			
			addOption ("relative", &relative, 1, SSI_BOOL, "additional position stream: relative point values in interval [0..1]");
			addOption ("swap", &swap, 1, SSI_BOOL, "additional position stream: swap values along y-axis");
			addOption ("labels", &labels, 1, SSI_BOOL, "additional position stream: show position labels");
			addOption ("precision", &precision, 1, SSI_SIZE, "additional position stream: precision of position labels");
		};

		void setTitle (const ssi_char_t *name) {
			ssi_strcpy (this->name, name);
		}
		void setPos (int left, int top, int width, int height, bool accurate = false) {
			pos[0] = left; pos[1] = top; pos[2] = width; pos[3] = height; this->accurate = accurate;
		}

		ssi_char_t name[SSI_MAX_CHAR];
		bool flip;
		/*mirror currently only implemented for 16 bit single channel stream and 8 bit 3 channel*/
		bool mirror;
		/*maxvalue currently only implemented for 16 bit single channel stream and 8 bit 3 channel*/
		double maxValue;
		bool scale;

		bool accurate;
		int pos[4];

		PaintPointsType::TYPE type;
		ssi_size_t precision;
		bool relative;
		bool swap;
		bool labels;
	};

public: 

	static const ssi_char_t *GetCreateName () { return "VideoPainter"; };
	static IObject *Create (const ssi_char_t *file) { return new VideoPainter (file); };
	~VideoPainter ();
	
	VideoPainter::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Viewer for video streams."; };

	void consume_enter (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);
	void consume (IConsumer::info consume_info,
		ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);
	void consume_flush (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);

	void setVideoFormat (ssi_video_params_t video_format) { 
		_video_format = video_format;
	};
	void setMetaData (ssi_size_t size, const void *meta) {
		if (sizeof (_video_format) != size) {
			ssi_err ("invalid meta size");
		}
		memcpy (&_video_format, meta, size);
	}
	ssi_video_params_t getVideoFormat () { return _video_format; };
	PaintVideo &getObject () { return *_video; };

	bool notify(INotify::COMMAND::List command, const ssi_char_t *message);

protected:

	VideoPainter (const ssi_char_t *file = 0);
	VideoPainter::Options _options;
	ssi_char_t *_file;

	IWindow *_window;
	ICanvas *_canvas;
	PaintVideo *_video;
	ssi_video_params_t _video_format;
	ssi_size_t _n_faces;
	PaintPoints **_faces;

	static const ssi_char_t *ssi_log_name;
	static const ssi_char_t *_name;
	static const ssi_char_t *_info;
};

}

#endif
