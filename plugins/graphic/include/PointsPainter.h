// PointsPainter.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2015/07/06
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

#ifndef SSI_GRAPHIC_POINTSPAINTER_H
#define SSI_GRAPHIC_POINTSPAINTER_H

#include "base/IConsumer.h"
#include "PaintPoints.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class PointsPainter : public IConsumer {

public:

	class Options : public OptionList {

	public:

		Options ()
			: type(PaintPointsType::DOTS), precision(1), relative(false), swap(false), labels(false) {
			
			name[0] = '\0';		
			setPos (0,0,100,100);	

			addOption ("title", name, SSI_MAX_CHAR, SSI_CHAR, "plot caption");
			addOption ("type", &type, 1, SSI_UCHAR, "plot type (0=dots, 1=lines)");					
			addOption ("pos", pos, 4, SSI_INT, "window position (top, left, width, height)");  
			addOption ("relative", &relative, 1, SSI_BOOL, "relative point values in interval [0..1]");
			addOption ("swap", &swap, 1, SSI_BOOL, "swap values along y-axis");
			addOption ("labels", &labels, 1, SSI_BOOL, "show position labels");
			addOption ("precision", &precision, 1, SSI_SIZE, "precision for position labels");
		};

		void setTitle(const ssi_char_t *name) {
			ssi_strcpy (this->name, name);
		}
		void setPos(int left, int top, int width, int height) {
			pos[0] = left; pos[1] = top; pos[2] = width; pos[3] = height;
		}

		ssi_char_t name[SSI_MAX_CHAR];
		PaintPointsType::TYPE type;
		int pos[4];
		ssi_size_t precision;
		bool relative;
		bool swap;
		bool labels;
	};

public: 

	static const ssi_char_t *GetCreateName () { return "PointsPainter"; };
	static IObject *Create (const ssi_char_t *file) { return new PointsPainter (file); };
	~PointsPainter ();
	
	PointsPainter::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Viewer for position streams."; };
	
	void setLogLevel (int level) {
		ssi_log_level = level;
	}

	void consume_enter (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);
	void consume (IConsumer::info consume_info,
		ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);
	void consume_flush (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);

	bool notify(INotify::COMMAND::List command, const ssi_char_t *message);

protected:

	PointsPainter (const ssi_char_t *file = 0);
	PointsPainter::Options _options;
	ssi_char_t *_file;

	IWindow *_window;
	ICanvas *_canvas;
	PaintPoints *_client;

	static const ssi_char_t *ssi_log_name;
	static int ssi_log_level;
};

}

#endif
