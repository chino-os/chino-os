#include "BootVideo.hpp"
#include <stdarg.h>
#include <algorithm>

using namespace Chino::UefiGfx;

// these vectors together act as a corner of a bounding rect
// This allows GotoXY() to reposition all the text that follows it
static unsigned int _xPos = 0, _yPos = 0;
static unsigned _startX = 0, _startY = 0;

// current color
static unsigned _color = 0;

BootVideo::BootVideo(uint32_t * frameBuffer, size_t bufferSize, size_t frameWidth, size_t frameHeight)
{
	this->glyphProvider.Initialize(GetBootFont());
	this->frameBuffer = frameBuffer;
	this->bufferSize = bufferSize;
	this->frameWidth = frameWidth;
	this->frameHeight = frameHeight;
	this->currentX = 0;
	this->currentY = 0;
	this->currentFramePointer = frameBuffer;
	this->foreground = 0x00ff99ff;
}

void BootVideo::PutChar(wchar_t chr)
{
	auto& font = GetBootFont();
	if (chr == L'\r')
		currentX = 0;
	else if (chr == L'\n')
		currentY += font.Height;
	else
	{
		int cx, cy;
		const unsigned char *glyph = glyphProvider.GetGlyph(chr);
		auto pixel = currentFramePointer;

		auto startPixel = pixel;
		for (cy = 0; cy < font.Height; cy++, glyph++)
		{
			unsigned char line = *glyph;
			auto pixel = startPixel;
			for (cx = 0; cx < font.Width; cx++)
			{
				*pixel++ = (line & 0x80u) ? foreground : background;
				line <<= 1;
			}
			startPixel += frameWidth;
		}
		currentX += font.Width;
		if (currentX >= frameWidth)
		{
			currentX = 0;
			currentY += font.Height;
		}
	}
	if (currentY + font.Height >= frameHeight)
	{
		ClearScreen();
		currentY = 0;
	}
	FixCurrentFramePointer();
}

void BootVideo::PutString(const wchar_t * string)
{
	while (*string)
		PutChar(*string++);
}

void BootVideo::PutString(const wchar_t * string, size_t count)
{
	for (size_t i = 0; i < count; i++)
		PutChar(*string++);
}

wchar_t tbuf[64];
wchar_t bchars[] = { L'0',L'1',L'2',L'3',L'4',L'5',L'6',L'7',L'8',L'9',L'A',L'B',L'C',L'D',L'E',L'F' };
wchar_t str[64] = { 0 };

void _itow(uint64_t i, unsigned base, wchar_t* buf) {
	int pos = 0;
	int opos = 0;
	int top = 0;

	buf[0] = '0';
	buf[1] = '\0';

	if (i == 0 || base > 16) {
		buf[0] = '0';
		buf[1] = '\0';
		return;
	}

	while (i != 0) {
		tbuf[pos] = bchars[i % base];
		pos++;
		i /= base;
	}
	top = pos--;
	for (opos = 0; opos < top; pos--, opos++) {
		buf[opos] = tbuf[pos];
	}
	buf[opos] = 0;
}

void _itow_s(uint64_t i, unsigned base, wchar_t* buf) {

	if (base > 16) return;
	if (i < 0) {
		*buf++ = '-';
		i *= -1;
	}
	_itow(i, base, buf);
}

void BootVideo::PutFormat(const wchar_t * format, ...)
{
	if (!format)return;

	va_list		args;
	va_start(args, format);

	for (size_t i = 0; format[i]; i++) {

		switch (format[i]) {

		case L'%':

			switch (format[i + 1]) {

				/*** characters ***/
			case L'c': {
				wchar_t c = va_arg(args, int);
				PutChar(c);
				i++;		// go to next character
				break;
			}

					   /*** address of ***/
			case 's': {
				wchar_t* c = (wchar_t*)va_arg(args, wchar_t*);
				//char str[32]={0};
				//itoa(c, 16, str);
				PutString(c);
				i++;		// go to next character
				break;
			}

					  /*** integers ***/
			case 'd':
			case 'i': {
				int c = va_arg(args, int);
				_itow_s(c, 10, str);
				PutString(str);
				i++;		// go to next character
				break;
			}
			case 'l': {
				int64_t c = va_arg(args, int64_t);
				_itow_s(c, 10, str);
				PutString(str);
				i++;		// go to next character
				break;
			}
					  /*** display in hex ***/
			case 'X':
			case 'x': {
				int c = va_arg(args, int);
				//char str[32]={0};
				_itow_s(c, 16, str);
				PutString(str);
				i++;		// go to next character
				break;
			}

			default:
				va_end(args);
				return;
			}

			break;

		default:
			PutChar(format[i]);
			break;
		}

	}

	va_end(args);
}

void BootVideo::MovePositionTo(size_t x, size_t y)
{
	currentX = std::min(x, frameWidth - 1);
	currentY = std::min(y, frameHeight - 1);
	FixCurrentFramePointer();
}

void BootVideo::ClearScreen()
{
	auto end = (uint32_t*)((uint8_t*)frameBuffer + bufferSize) + 1;
	for (auto pixel = frameBuffer; pixel < end; pixel++)
		*pixel = background;
}

void BootVideo::SetBackground(uint32_t color)
{
	background = color;
}

void BootVideo::FixCurrentFramePointer()
{
	currentFramePointer = frameBuffer + currentY * frameWidth + currentX;
}