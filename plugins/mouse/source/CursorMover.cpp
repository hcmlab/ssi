// CursorMover.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2010/04/25
// Copyright (C) University of Augsburg

#include "CursorMover.h"
#if __gnu_linux__
static Window create_win(Display *dpy)
{
	
    Window win = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), 0, 0, 200,
            200, 0, 0, WhitePixel(dpy, 0));



    XMapWindow(dpy, win);
    XSync(dpy, False);
    return win;
}
#endif
namespace ssi {

CursorMover::CursorMover (const ssi_char_t *file)
: _max_x (0),
_max_y (0),
_file (0) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
	#if __gnu_linux__
	
	    display_name = XOpenDisplay(NULL);
        screen = DefaultScreen(display_name);

		root=DefaultRootWindow(display_name);
		win=create_win(  display_name );
      
XIGetClientPointer(
    display_name,
    0,
    &corep
);
	#endif
}

CursorMover::~CursorMover () {

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}

void CursorMover::consume_enter (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {
#if _WIN32||WIN64

	HWND desktop = GetDesktopWindow ();
	RECT rect;
	GetWindowRect (desktop, &rect);
	_max_x = ssi_cast (int, rect.right);
	_max_y = ssi_cast (int, rect.bottom);
#else
	_max_x =ssi_cast (int,DisplayWidth(display_name,screen));
	_max_y =ssi_cast (int,DisplayHeight(display_name,screen));
	
#endif
};

void CursorMover::consume (IConsumer::info consume_info,
	ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	ssi_real_t *ptr_in = ssi_pcast (ssi_real_t, stream_in[0].ptr);

	ptr_in += stream_in[0].dim * (stream_in[0].num - 1);
	float x = *(ptr_in + _options.indx);
	float y = *(ptr_in + _options.indy);

	if (_options.scale) {

		if (x != _options.skip && y != _options.skip
			&& x >= 0 && x <= 1.0f && y >= 0 && y <= 1.0f) {

			int X = 0;
			int Y = 0;

			x = (x - _options.minx) / (_options.maxx - _options.minx);
			y = (y - _options.miny) / (_options.maxy - _options.miny);
			X = ssi_cast (int, x * _max_x);
			Y = ssi_cast (int, y * _max_y);
		
			if (_options.fliph) {
				X = _max_x - X;
			}
			if (_options.flipv) {
				Y = _max_y - Y;
			}
		#if _WIN32|_WIN64
			::SetCursorPos(X,Y);	
		#endif
		}

	} else {

		if (x != _options.skip && y != _options.skip) {

			int X = ssi_cast (int, x);
			int Y = ssi_cast (int, y);

			if (_options.fliph) {
				X = _max_x - X;
			}
			if (_options.flipv) {
				Y = _max_y - Y;
			}
		
		#if _WIN32|_WIN64
			::SetCursorPos(X,Y);	
		#endif
		}
	}
};

void CursorMover::consume_flush (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {
};

}

