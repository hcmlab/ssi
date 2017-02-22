// Window.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2007/10/15
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

//#include "graphic/Window.h"
#include "graphic/WindowFallback.h"

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

const char *Window::_wClassName = "SSIWindow";
ssi_handle_t Window::_hParent = 0;
ssi_handle_t Window::_hInstance = 0;
ssi_handle_t Window::_hIcon = 0;

Window::Window ()
: Thread (true),
	_client (0),
	_min_width (0),
	_max_width (0),
	_min_height (0),
	_max_height(0),
	_icons(ICONS::NONE),
	_hWnd(0),
	_systemTray(0),
	_showAndHideCallback(0) {

	RegisterWindowClass();



	_title[0] = '\0';

}

Window::~Window () {


}

ssi_handle_t Window::getHandle() {
        return NULL;
}

void Window::setClient(IWindowClient *client) {

}

void Window::setLimits(ssi_size_t min_width, ssi_size_t max_width, ssi_size_t min_height, ssi_size_t max_height) {

}

void Window::PrintLastError()
{

}

void Window::RegisterWindowClass() {

}
static int WindowProc(int hwnd, int msg, int wParam, int lParam)
{
return NULL;
}

int windowProc(int hwnd, int msg, int wParam, int lParam)
{
return NULL;
}

void Window::enter() {
	

}

void Window::run () {


}

void Window::create() {


}

bool Window::isVisible() {

        return  false;
}

void Window::show() {


}

void Window::hide() {


}

void Window::update() {


}

void Window::setTitle(const ssi_char_t *title) {

}

bool Window::setPosition(const ssi_char_t *position) {

		
	return true;
}

void Window::setPosition(ssi_rect_t rect) {


}

Window::ICON Window::getIcons()
{
    return 0;
}

void Window::close() {


}

void Window::setIcons(ICON icons)
{

}
void Window::setCallback(Window::ShowAndHideCallback *callback) {

}

}
