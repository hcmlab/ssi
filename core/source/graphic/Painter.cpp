// Painter.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2015/01/06
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

#include "SSI_Define.h"

#ifndef SSI_USE_SDL

#include "graphic/Painter.h"

#include <commdlg.h>
#ifdef INC_OLE1
#include <ole.h>
#else
#include <ole2.h>
#endif
#include <gdiplus.h>
#include <winspool.h>

#pragma comment (lib,"Gdiplus.lib")

namespace ssi {

ssi_char_t *Painter::ssi_log_name = "painter___";

Painter::Painter()
	: _device(0), 
	_tmp_object(0),
	_gdiplus(0) {

	_area.height = _area.left = _area.top = _area.width = 0;

	_back_pen = new Painter::Pen(IPainter::ITool::COLORS::BLACK);
	_back_brush = new Painter::Pen(IPainter::ITool::COLORS::BLACK);	
}

Painter::~Painter() {

	if (_device) {
		end();
	}

	delete _back_pen;
	delete _back_brush;

	if (_gdiplus) {
		Gdiplus::GdiplusShutdown(_gdiplus);
	}
};

Painter::Pen::Pen(ssi_rgb_t color, WIDTH width, LINE_STYLES::List style) {
		
	_handle = ::CreatePen(style, width, color);
	_color = color;
}

Painter::Brush::Brush() {

	_handle = ::GetStockObject(HOLLOW_BRUSH);
	_color = -1;
}

Painter::Brush::Brush(ssi_rgb_t color) {

	_handle = ::CreateSolidBrush(color);
	_color = color;
}

Painter::Font::Font(const ssi_char_t *name, WIDTH size, FONT_STYLE font_style) {

	_handle = ::CreateFont(size, 0, 0, 0, 
		font_style & IPainter::ITool::FONT_STYLES::BOLD ? FW_BOLD : FW_NORMAL,
		font_style & IPainter::ITool::FONT_STYLES::ITALIC, 
		font_style & IPainter::ITool::FONT_STYLES::UNDERLINE, 
		font_style & IPainter::ITool::FONT_STYLES::STRIKEOUT,
		DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, 
		TEXT(name ? name : SSI_DEFAULT_FONT_NAME));

	if (!_handle) {
		ssi_wrn("could not create font '%s'", name);
	}
}

Painter::Tool::~Tool() {

	if (_handle) {
		::DeleteObject(_handle);
	}
}

ssi_handle_t Painter::Tool::getHandle() {
	return _handle;
}

ssi_rgb_t Painter::Tool::getColor() {
	return _color;
}

ssi_rect_t Painter::rectf2rect(ssi_rectf_t rect) {

	ssi_rect_t r;
	r.left = ssi_cast(ssi_int_t, rect.left + 0.5f);
	r.top = ssi_cast(ssi_int_t, rect.top + 0.5f);
	r.width = ssi_cast(ssi_int_t, rect.width + 0.5f);
	r.height = ssi_cast(ssi_int_t, rect.height + 0.5f);
	return r;
}

ssi_rect_t Painter::rectf2rect(ssi_rectf_t rect, ssi_rect_t area) {

	ssi_rect_t r;
	r.left = ssi_cast(ssi_int_t, rect.left * area.width + 0.5f);
	r.top = ssi_cast(ssi_int_t, rect.top * area.height + 0.5f);
	r.width = ssi_cast(ssi_int_t, rect.width * area.width + 0.5f);
	r.height = ssi_cast(ssi_int_t, rect.height * area.height + 0.5f);
	return r;
}

ssi_point_t Painter::pointf2point(ssi_pointf_t point) {

	ssi_point_t p;
	p.x = ssi_cast(ssi_int_t, point.x + 0.5f);
	p.y = ssi_cast(ssi_int_t, point.y + 0.5f);
	return p;
}

ssi_point_t Painter::pointf2point(ssi_pointf_t point, ssi_rect_t area) {

	ssi_point_t p;
	p.x = ssi_cast(ssi_int_t, point.x * area.width + 0.5f);
	p.y = ssi_cast(ssi_int_t, point.y * area.height + 0.5f);
	return p;
}

void Painter::begin(ssi_handle_t device, ssi_rect_t area) {

	if (_device) {
		end();
	}

	_device = device;
	_area = area;

	_tmp_font_color = ::GetTextColor((HDC)_device);
	_tmp_bk_mode = ::GetBkMode((HDC)_device);
	_tmp_bk_color = ::GetBkColor((HDC)_device);
	_tmp_object = ::GetStockObject(DC_PEN);
	_tmp_text_align = ::GetTextAlign((HDC)_device);
}

ssi_rect_t Painter::getArea() {
	return _area;
}

void Painter::setArea(ssi_rect_t area) {
	_area = area;	
}

void Painter::setBackground(ssi_rgb_t color) {

	delete _back_pen;
	delete _back_brush;
	_back_pen = new Painter::Pen(color);
	_back_brush = new Painter::Pen(color);
}

void Painter::blank() {

	rect(*_back_pen, *_back_brush, _area);
}

void Painter::pixel(ITool &pen, ssi_pointf_t point, bool relative) {

	if (relative) {
		Painter::pixel(pen, pointf2point(point, _area));
	} else {
		Painter::pixel(pen, pointf2point(point));
	}
}

void Painter::pixel(ITool &pen, ssi_point_t point) {

	::SelectObject((HDC)_device, pen.getHandle());
	::SetPixel(ssi_cast(HDC, _device), _area.left + point.x, _area.top + point.y, pen.getColor());
}

void Painter::fill(ITool &brush, ssi_rectf_t rect, bool relative) {

	if (relative) {
		Painter::fill(brush, rectf2rect(rect, _area));
	}
	else {
		Painter::fill(brush, rectf2rect(rect));
	}
}

void Painter::fill(ITool &brush, ssi_rect_t rect) {

	if (rect.width == 1 && rect.height == 1) {

		::SetPixel(ssi_cast(HDC, _device), _area.left + rect.left, _area.top + rect.top, brush.getColor());

	} else {

		::RECT r;
		r.left = _area.left + rect.left;
		r.top = _area.top + rect.top;
		r.right = _area.left + rect.left + rect.width;
		r.bottom = _area.top + rect.top + rect.height;

		::FillRect((HDC)_device, &r, (HBRUSH)brush.getHandle());
	}
}

void Painter::rect(ITool &pen, ssi_rectf_t rect, bool relative) {

	if (relative) {
		Painter::rect(pen, rectf2rect(rect, _area));
	} else {
		Painter::rect(pen, rectf2rect(rect));
	}
}

void Painter::rect(ITool &pen, ssi_rect_t rect) {

	if (rect.width == 1 && rect.height == 1) {

		::SetPixel(ssi_cast(HDC, _device), _area.left + rect.left, _area.top + rect.top, pen.getColor());

	} else {

		::SelectObject((HDC)_device, pen.getHandle());
		::SelectObject((HDC)_device, ::GetStockObject(HOLLOW_BRUSH));

		::RECT r;
		r.left = _area.left + rect.left;
		r.top = _area.top + rect.top;
		r.right = _area.left + rect.left + rect.width;
		r.bottom = _area.top + rect.top + rect.height;

		::Rectangle((HDC)_device, r.left, r.top, r.right, r.bottom);
	}
}

void Painter::rect(ITool &pen, ITool &brush, ssi_rectf_t rect, bool relative) {

	if (relative) {
		Painter::rect(pen, brush, rectf2rect(rect, _area));
	} else {
		Painter::rect(pen, brush, rectf2rect(rect));
	}
}

void Painter::rect(ITool &pen, ITool &brush, ssi_rect_t rect) {

	if (rect.width == 1 && rect.height == 1) {

		::SetPixel(ssi_cast(HDC, _device), _area.left + rect.left, _area.top + rect.top, pen.getColor());

	} else {

		::SelectObject((HDC)_device, pen.getHandle());
		::SelectObject((HDC)_device, brush.getHandle());

		::RECT r;
		r.left = _area.left + rect.left;
		r.top = _area.top + rect.top;
		r.right = _area.left + rect.left + rect.width;
		r.bottom = _area.top + rect.top + rect.height;

		::Rectangle((HDC)_device, r.left, r.top, r.right, r.bottom);
	}
}

void Painter::line(ITool &pen, ssi_pointf_t from, ssi_pointf_t to, bool relative) {

	if (relative) {
		Painter::line(pen, pointf2point(from, _area), pointf2point(to, _area));
	} else {
		Painter::line(pen, pointf2point(from), pointf2point(to));
	}
}

void Painter::line(ITool &pen, ssi_point_t from, ssi_point_t to) {

	::SelectObject((HDC)_device, pen.getHandle());
	::MoveToEx((HDC)_device, _area.left + from.x, _area.top + from.y, 0);
	::LineTo((HDC)_device, _area.left + to.x, _area.top + to.y);
}

void Painter::ellipse(ITool &pen, ssi_rectf_t rect, bool relative) {

	if (relative) {
		Painter::ellipse(pen, rectf2rect(rect, _area));
	}
	else {
		Painter::ellipse(pen, rectf2rect(rect));
	}
}

void Painter::ellipse(ITool &pen, ssi_rect_t rect) {

	::SelectObject((HDC)_device, pen.getHandle());
	::SelectObject((HDC)_device, ::GetStockObject(HOLLOW_BRUSH));
	::Ellipse((HDC)_device, _area.left + rect.left, _area.top + rect.top, _area.left + rect.left + rect.width, _area.top + rect.top + rect.height);
}

void Painter::ellipse(ITool &pen, ITool &brush, ssi_rectf_t rect, bool relative) {

	if (relative) {
		Painter::ellipse(pen, brush, rectf2rect(rect, _area));
	}
	else {
		Painter::ellipse(pen, brush, rectf2rect(rect));
	}
}

void Painter::ellipse(ITool &pen, ITool &brush, ssi_rect_t rect) {

	::SelectObject((HDC)_device, pen.getHandle());
	::SelectObject((HDC)_device, brush.getHandle());
	::Ellipse((HDC)_device, _area.left + rect.left, _area.top + rect.top, _area.left + rect.left + rect.width, _area.top + rect.top + rect.height);
}

void Painter::circle(ITool &pen, ITool &brush, ssi_point_t center, int radius) {

	ssi_rect_t rect;
	rect.left = center.x - radius;
	rect.top = center.y - radius;
	rect.height = 2 * radius;
	rect.width = 2 * radius;
	ellipse (pen, brush, rect);
}

void Painter::circle(ITool &pen, ssi_pointf_t center, ssi_real_t radius, bool relative) {
	
	ssi_rectf_t rect;
	rect.left = center.x - radius;
	rect.top = center.y - radius;
	rect.height = 2 * radius;
	rect.width = 2 * radius;
	ellipse(pen, rect, relative);
}

void Painter::circle(ITool &pen, ssi_point_t center, int radius) {

	ssi_rect_t rect;
	rect.left = center.x - radius;
	rect.top = center.y - radius;
	rect.height = 2 * radius;
	rect.width = 2 * radius;
	ellipse (pen, rect);
}

void Painter::circle(ITool &pen, ITool &brush, ssi_pointf_t center, ssi_real_t radius, bool relative) {

	ssi_rectf_t rect;
	rect.left = center.x - radius;
	rect.top = center.y - radius;
	rect.height = 2 * radius;
	rect.width = 2 * radius;
	ellipse(pen, brush, rect, relative);
}

void Painter::text(ITool &font, ITool &pen, ssi_pointf_t position, const ssi_char_t *text, bool relative, TEXT_ALIGN_HORZ::List align_horz, TEXT_ALIGN_VERT::List align_vert) {

	if (relative) {
		Painter::text(font, pen, pointf2point(position, _area), text, align_horz, align_vert);
	} else {
		Painter::text(font, pen, pointf2point(position), text, align_horz, align_vert);
	}
}

void Painter::text(ITool &font, ITool &pen, ssi_point_t position, const ssi_char_t *text, TEXT_ALIGN_HORZ::List align_horz, TEXT_ALIGN_VERT::List align_vert) {

	if (text[0] == '\0') {
		return;
	}

	::SetTextColor((HDC)_device, pen.getColor());
	::SetBkMode((HDC)_device, TRANSPARENT);
	::SelectObject((HDC)_device, font.getHandle());

	unsigned int align = 0;
	switch (align_horz) {
	case TEXT_ALIGN_HORZ::LEFT:
		align |= TA_LEFT;
		break;
	case TEXT_ALIGN_HORZ::RIGHT:
		align |= TA_RIGHT;
		break;
	case TEXT_ALIGN_HORZ::CENTER:
		align |= TA_CENTER;
		break;
	}
	switch (align_vert) {
	case TEXT_ALIGN_VERT::BOTTOM:
		align |= TA_BOTTOM;
		break;
	case TEXT_ALIGN_VERT::TOP:
		align |= TA_TOP;
		break;
	case TEXT_ALIGN_VERT::CENTER:
		align |= TA_BASELINE;
		break;
	}

	::SetTextAlign((HDC) _device, align);
	::TextOut((HDC)_device, _area.left + position.x, _area.top + position.y, text, ssi_strlen (text));
}

void Painter::text(ITool &font, ITool &pen, ITool &brush, ssi_pointf_t position, const ssi_char_t *text, bool relative, TEXT_ALIGN_HORZ::List align_horz, TEXT_ALIGN_VERT::List align_vert) {

	if (relative) {
		Painter::text(font, pen, brush, pointf2point(position, _area), text, align_horz, align_vert);
	} else {
		Painter::text(font, pen, brush, pointf2point(position), text, align_horz, align_vert);
	}
}

void Painter::text(ITool &font, ITool &pen, ITool &brush, ssi_point_t position, const ssi_char_t *text, TEXT_ALIGN_HORZ::List align_horz, TEXT_ALIGN_VERT::List align_vert) {

	if (text[0] == '\0') {
		return;
	}

	::SetTextColor((HDC)_device, pen.getColor());
	::SetBkMode((HDC)_device, OPAQUE);
	::SetBkColor((HDC)_device, brush.getColor());
	::SelectObject((HDC)_device, font.getHandle());

	unsigned int align = 0;
	switch (align_horz) {
	case TEXT_ALIGN_HORZ::LEFT:
		align |= TA_LEFT;
		break;
	case TEXT_ALIGN_HORZ::RIGHT:
		align |= TA_RIGHT;
		break;
	case TEXT_ALIGN_HORZ::CENTER:
		align |= TA_CENTER;
		break;
	}
	switch (align_vert) {
	case TEXT_ALIGN_VERT::BOTTOM:
		align |= TA_BOTTOM;
		break;
	case TEXT_ALIGN_VERT::TOP:
		align |= TA_TOP;
		break;
	case TEXT_ALIGN_VERT::CENTER:
		align |= TA_BASELINE;
		break;
	}

	::SetTextAlign((HDC) _device, align);
	ssi_size_t n = ssi_split_string_count(text, '|');
	if (n > 1)
	{
		ssi_char_t **tokens = new ssi_char_t *[n];
		ssi_split_string(n, tokens, text, '|');
		LONG offset = 0;
		SIZE size;
		for (ssi_size_t i = 0; i < n; i++)
		{			
			::GetTextExtentPoint32((HDC)_device, tokens[i], ssi_strlen(tokens[i]), &size);
			::TextOut((HDC)_device, _area.left + position.x, _area.top + position.y + offset, tokens[i], ssi_strlen(tokens[i]));
			offset += size.cy;
			delete[] tokens[i];
		}
		delete[] tokens;
	}
	else
	{
		::TextOut((HDC)_device, _area.left + position.x, _area.top + position.y, text, ssi_strlen(text));
	}
	
}

void Painter::image(ssi_video_params_t params, ssi_byte_t *buffer, bool scale) {

	if (!buffer) {
		return;
	}

	if (!_gdiplus) {
		Gdiplus::GdiplusStartupInput gdiplusStartupInput;
		if (Gdiplus::GdiplusStartup(&_gdiplus, &gdiplusStartupInput, NULL) != Gdiplus::Ok)
		{
			ssi_wrn("failed to call GdiplusStartup()");
		}
	}

	HDC hdc = (HDC)_device;
	
	RECT rect;
	rect.bottom = _area.top + _area.height;
	rect.left = _area.left;
	rect.right = _area.left + _area.width;
	rect.top = _area.top;
	int stride = ssi_video_stride(params);

	Gdiplus::Bitmap *bitmap = 0;

#ifdef USE_SSI_LEAK_DETECTOR
#ifdef _DEBUG
#undef new
#endif
#endif
	switch (params.numOfChannels)
	{
	case 4:
		bitmap = new Gdiplus::Bitmap(params.widthInPixels, params.heightInPixels, stride, PixelFormat32bppRGB, (BYTE*)buffer);
		break;
	case 1:
		bitmap = new Gdiplus::Bitmap(params.widthInPixels, params.heightInPixels, stride, PixelFormat24bppRGB, (BYTE*)buffer);
		break;
	default:
		bitmap = new Gdiplus::Bitmap(params.widthInPixels, params.heightInPixels, stride, PixelFormat24bppRGB, (BYTE*)buffer);
		break;
	}
#ifdef USE_SSI_LEAK_DETECTOR
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif
#endif

	Gdiplus::Graphics graphics(hdc);
#ifdef __MINGW32__
#else
	graphics.SetInterpolationMode(Gdiplus::InterpolationMode::InterpolationModeNearestNeighbor); //faster
#endif

	if (scale && (rect.right - rect.left != params.widthInPixels || rect.bottom - rect.top != params.heightInPixels))
	{
		graphics.DrawImage(bitmap, (int)rect.left, (int)rect.top, (int)rect.right - rect.left, (int)rect.bottom - rect.top);
	}
	else
	{
		graphics.DrawImage(bitmap, (int)rect.left, (int)rect.top);
	}

	delete bitmap;
}

void Painter::end() {

	::SelectObject((HDC) _device, (HGDIOBJ)_tmp_object);
	::SetTextColor((HDC)_device, _tmp_font_color);
	::SetBkMode((HDC)_device, _tmp_bk_mode);
	::SetBkColor((HDC)_device, _tmp_bk_color);
	::SetTextAlign((HDC)_device, _tmp_text_align);

	_device = 0;
	_tmp_object = 0;
	_area.height = _area.left = _area.top = _area.width = 0;
}

}

#endif