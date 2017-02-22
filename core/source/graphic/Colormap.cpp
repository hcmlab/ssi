// Colormap.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2007/11/15
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

#include "graphic/Colormap.h"
#include "graphic/Painter.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

Colormap::Colormap(ssi_rgb_t color)
: _n_colors(1) {

	_colors = new ssi_rgb_t;
	_brushes = new IPainter::ITool *[1];
	_brushes[0] = new Painter::Brush(_colors[0]);
	_pens = new IPainter::ITool *[1];
	_pens[0] = new Painter::Pen(_colors[0]);
}


Colormap::Colormap(ssi_size_t n_colors, ssi_rgb_t *colors)
: _n_colors(n_colors) {

	_colors = new ssi_rgb_t[n_colors];
	_brushes = new IPainter::ITool *[_n_colors];
	_pens = new IPainter::ITool *[_n_colors];
	for (unsigned int i = 0; i < _n_colors; i++) {
		_colors[i] = colors[i];
		_brushes[i] = new Painter::Brush (_colors[i]);
		_pens[i] = new Painter::Pen(_colors[i]);
	}
}

Colormap::Colormap(COLORMAP::List type) {

	loadDefault (type);
	_brushes = new IPainter::ITool *[_n_colors];
	_pens = new IPainter::ITool *[_n_colors];
	for (unsigned int i = 0; i < _n_colors; i++) {
		_brushes[i] = new Painter::Brush(_colors[i]);
		_pens[i] = new Painter::Pen(_colors[i]);
	}
}

void Colormap::loadDefault(COLORMAP::List type) {

	switch (type) {

		case COLORMAP::BLACKANDWHITE: {

			_n_colors = 2;
			_colors = new ssi_rgb_t[_n_colors];
			ssi_rgb_t *colors_ptr = _colors;
            *colors_ptr++ = ssi_rgb(0,0,0);
            *colors_ptr++ = ssi_rgb(255,255,255);
			break;
		}

		case COLORMAP::COLOR16: {

			_n_colors = 16;
			_colors = new ssi_rgb_t[_n_colors];
			ssi_rgb_t *colors_ptr = _colors;
            *colors_ptr++ = ssi_rgb(0,0,0);
            *colors_ptr++ = ssi_rgb(128,0,0);
            *colors_ptr++ = ssi_rgb(0,128,0);
            *colors_ptr++ = ssi_rgb(0,0,128);
            *colors_ptr++ = ssi_rgb(128,0,128);
            *colors_ptr++ = ssi_rgb(0,128,128);
            *colors_ptr++ = ssi_rgb(192,192,192);
            *colors_ptr++ = ssi_rgb(128,128,128);
            *colors_ptr++ = ssi_rgb(255,0,0);
            *colors_ptr++ = ssi_rgb(0,255,0);
            *colors_ptr++ = ssi_rgb(255,255,0);
            *colors_ptr++ = ssi_rgb(0,0,255);
            *colors_ptr++ = ssi_rgb(255,0,255);
            *colors_ptr++ = ssi_rgb(0,255,255);
            *colors_ptr++ = ssi_rgb(255,255,255);
			break;
		}
		case COLORMAP::COLOR64: {
		
			_n_colors = 64;
			_colors = new ssi_rgb_t[_n_colors];
			ssi_rgb_t *colors_ptr = _colors;
            *colors_ptr++ = ssi_rgb(0,0,143);
            *colors_ptr++ = ssi_rgb(0,0,159);
            *colors_ptr++ = ssi_rgb(0,0,175);
            *colors_ptr++ = ssi_rgb(0,0,191);
            *colors_ptr++ = ssi_rgb(0,0,207);
            *colors_ptr++ = ssi_rgb(0,0,223);
            *colors_ptr++ = ssi_rgb(0,0,239);
            *colors_ptr++ = ssi_rgb(0,0,255);
            *colors_ptr++ = ssi_rgb(0,15,255);
            *colors_ptr++ = ssi_rgb(0,31,255);
            *colors_ptr++ = ssi_rgb(0,47,255);
            *colors_ptr++ = ssi_rgb(0,63,255);
            *colors_ptr++ = ssi_rgb(0,79,255);
            *colors_ptr++ = ssi_rgb(0,95,255);
            *colors_ptr++ = ssi_rgb(0,111,255);
            *colors_ptr++ = ssi_rgb(0,127,255);
            *colors_ptr++ = ssi_rgb(0,143,255);
            *colors_ptr++ = ssi_rgb(0,159,255);
            *colors_ptr++ = ssi_rgb(0,175,255);
            *colors_ptr++ = ssi_rgb(0,191,255);
            *colors_ptr++ = ssi_rgb(0,207,255);
            *colors_ptr++ = ssi_rgb(0,223,255);
            *colors_ptr++ = ssi_rgb(0,239,255);
            *colors_ptr++ = ssi_rgb(0,255,255);
            *colors_ptr++ = ssi_rgb(15,255,239);
            *colors_ptr++ = ssi_rgb(31,255,223);
            *colors_ptr++ = ssi_rgb(47,255,207);
            *colors_ptr++ = ssi_rgb(63,255,191);
            *colors_ptr++ = ssi_rgb(79,255,175);
            *colors_ptr++ = ssi_rgb(95,255,159);
            *colors_ptr++ = ssi_rgb(111,255,143);
            *colors_ptr++ = ssi_rgb(127,255,127);
            *colors_ptr++ = ssi_rgb(143,255,111);
            *colors_ptr++ = ssi_rgb(159,255,95);
            *colors_ptr++ = ssi_rgb(175,255,79);
            *colors_ptr++ = ssi_rgb(191,255,63);
            *colors_ptr++ = ssi_rgb(207,255,47);
            *colors_ptr++ = ssi_rgb(223,255,31);
            *colors_ptr++ = ssi_rgb(239,255,15);
            *colors_ptr++ = ssi_rgb(255,255,0);
            *colors_ptr++ = ssi_rgb(255,239,0);
            *colors_ptr++ = ssi_rgb(255,223,0);
            *colors_ptr++ = ssi_rgb(255,207,0);
            *colors_ptr++ = ssi_rgb(255,191,0);
            *colors_ptr++ = ssi_rgb(255,175,0);
            *colors_ptr++ = ssi_rgb(255,159,0);
            *colors_ptr++ = ssi_rgb(255,143,0);
            *colors_ptr++ = ssi_rgb(255,127,0);
            *colors_ptr++ = ssi_rgb(255,111,0);
            *colors_ptr++ = ssi_rgb(255,95,0);
            *colors_ptr++ = ssi_rgb(255,79,0);
            *colors_ptr++ = ssi_rgb(255,63,0);
            *colors_ptr++ = ssi_rgb(255,47,0);
            *colors_ptr++ = ssi_rgb(255,31,0);
            *colors_ptr++ = ssi_rgb(255,15,0);
            *colors_ptr++ = ssi_rgb(255,0,0);
            *colors_ptr++ = ssi_rgb(239,0,0);
            *colors_ptr++ = ssi_rgb(223,0,0);
            *colors_ptr++ = ssi_rgb(207,0,0);
            *colors_ptr++ = ssi_rgb(191,0,0);
            *colors_ptr++ = ssi_rgb(175,0,0);
            *colors_ptr++ = ssi_rgb(159,0,0);
            *colors_ptr++ = ssi_rgb(143,0,0);
            *colors_ptr++ = ssi_rgb(127,0,0);
			break;
		}
		case COLORMAP::GRAY64: {

			_n_colors = 64;
			_colors = new ssi_rgb_t[_n_colors];
			ssi_rgb_t *colors_ptr = _colors;
            *colors_ptr++ = ssi_rgb(	0	,	0	,	0	);
            *colors_ptr++ = ssi_rgb(	4	,	4	,	4	);
            *colors_ptr++ = ssi_rgb(	8	,	8	,	8	);
            *colors_ptr++ = ssi_rgb(	12	,	12	,	12	);
            *colors_ptr++ = ssi_rgb(	16	,	16	,	16	);
            *colors_ptr++ = ssi_rgb(	20	,	20	,	20	);
            *colors_ptr++ = ssi_rgb(	24	,	24	,	24	);
            *colors_ptr++ = ssi_rgb(	28	,	28	,	28	);
            *colors_ptr++ = ssi_rgb(	32	,	32	,	32	);
            *colors_ptr++ = ssi_rgb(	36	,	36	,	36	);
            *colors_ptr++ = ssi_rgb(	40	,	40	,	40	);
            *colors_ptr++ = ssi_rgb(	45	,	45	,	45	);
            *colors_ptr++ = ssi_rgb(	49	,	49	,	49	);
            *colors_ptr++ = ssi_rgb(	53	,	53	,	53	);
            *colors_ptr++ = ssi_rgb(	57	,	57	,	57	);
            *colors_ptr++ = ssi_rgb(	61	,	61	,	61	);
            *colors_ptr++ = ssi_rgb(	65	,	65	,	65	);
            *colors_ptr++ = ssi_rgb(	69	,	69	,	69	);
            *colors_ptr++ = ssi_rgb(	73	,	73	,	73	);
            *colors_ptr++ = ssi_rgb(	77	,	77	,	77	);
            *colors_ptr++ = ssi_rgb(	81	,	81	,	81	);
            *colors_ptr++ = ssi_rgb(	85	,	85	,	85	);
            *colors_ptr++ = ssi_rgb(	89	,	89	,	89	);
            *colors_ptr++ = ssi_rgb(	93	,	93	,	93	);
            *colors_ptr++ = ssi_rgb(	97	,	97	,	97	);
            *colors_ptr++ = ssi_rgb(	101	,	101	,	101	);
            *colors_ptr++ = ssi_rgb(	105	,	105	,	105	);
            *colors_ptr++ = ssi_rgb(	109	,	109	,	109	);
            *colors_ptr++ = ssi_rgb(	113	,	113	,	113	);
            *colors_ptr++ = ssi_rgb(	117	,	117	,	117	);
            *colors_ptr++ = ssi_rgb(	121	,	121	,	121	);
            *colors_ptr++ = ssi_rgb(	125	,	125	,	125	);
            *colors_ptr++ = ssi_rgb(	130	,	130	,	130	);
            *colors_ptr++ = ssi_rgb(	134	,	134	,	134	);
            *colors_ptr++ = ssi_rgb(	138	,	138	,	138	);
            *colors_ptr++ = ssi_rgb(	142	,	142	,	142	);
            *colors_ptr++ = ssi_rgb(	146	,	146	,	146	);
            *colors_ptr++ = ssi_rgb(	150	,	150	,	150	);
            *colors_ptr++ = ssi_rgb(	154	,	154	,	154	);
            *colors_ptr++ = ssi_rgb(	158	,	158	,	158	);
            *colors_ptr++ = ssi_rgb(	162	,	162	,	162	);
            *colors_ptr++ = ssi_rgb(	166	,	166	,	166	);
            *colors_ptr++ = ssi_rgb(	170	,	170	,	170	);
            *colors_ptr++ = ssi_rgb(	174	,	174	,	174	);
            *colors_ptr++ = ssi_rgb(	178	,	178	,	178	);
            *colors_ptr++ = ssi_rgb(	182	,	182	,	182	);
            *colors_ptr++ = ssi_rgb(	186	,	186	,	186	);
            *colors_ptr++ = ssi_rgb(	190	,	190	,	190	);
            *colors_ptr++ = ssi_rgb(	194	,	194	,	194	);
            *colors_ptr++ = ssi_rgb(	198	,	198	,	198	);
            *colors_ptr++ = ssi_rgb(	202	,	202	,	202	);
            *colors_ptr++ = ssi_rgb(	206	,	206	,	206	);
            *colors_ptr++ = ssi_rgb(	210	,	210	,	210	);
            *colors_ptr++ = ssi_rgb(	215	,	215	,	215	);
            *colors_ptr++ = ssi_rgb(	219	,	219	,	219	);
            *colors_ptr++ = ssi_rgb(	223	,	223	,	223	);
            *colors_ptr++ = ssi_rgb(	227	,	227	,	227	);
            *colors_ptr++ = ssi_rgb(	231	,	231	,	231	);
            *colors_ptr++ = ssi_rgb(	235	,	235	,	235	);
            *colors_ptr++ = ssi_rgb(	239	,	239	,	239	);
            *colors_ptr++ = ssi_rgb(	243	,	243	,	243	);
            *colors_ptr++ = ssi_rgb(	247	,	247	,	247	);
            *colors_ptr++ = ssi_rgb(	251	,	251	,	251	);
            *colors_ptr++ = ssi_rgb(	255	,	255	,	255	);
			break;
		}

		default:
			ssi_err ("unkown colormap type");
	}
}

Colormap::~Colormap () {

	for (unsigned int i = 0; i < _n_colors; i++) {
		delete _brushes[i];
		delete _pens[i];
	}
	delete[] _brushes;
	delete[] _pens;
	delete[] _colors;
}

ssi_rgb_t Colormap::getColor(ssi_size_t index) {

	if (index < 0) {
		index = 0;
	} else if (index >= _n_colors) {
		index = _n_colors - 1;
	}
	return _colors[index];
}

IPainter::ITool *Colormap::getBrush(ssi_size_t index) {

	if (index < 0) {
		index = 0;
	} else if (index >= _n_colors) {
		index = _n_colors - 1;
	}
	return _brushes[index];
}

IPainter::ITool *Colormap::getPen(ssi_size_t index) {

	if (index < 0) {
		index = 0;
	}
	else if (index >= _n_colors) {
		index = _n_colors - 1;
	}
	return _pens[index];
}

ssi_rgb_t Colormap::getColor(ssi_real_t value) {

	return getColor(ssi_cast(ssi_size_t, 0.5f + value * (_n_colors - 1)));
}

IPainter::ITool *Colormap::getBrush(ssi_real_t value) {

	return getBrush(ssi_cast(ssi_size_t, 0.5f + value * (_n_colors - 1)));
}

IPainter::ITool *Colormap::getPen(ssi_real_t value) {

	return getPen(ssi_cast(ssi_size_t, 0.5f + value * (_n_colors - 1)));
}

}
